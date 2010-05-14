#if HAVE_NET
/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * Trivial Remote File System client.
 * Paritally implemented, not complete, not tested well.
 *
 *
**/

#define DEBUG_MSG_PREFIX "TRFS"
#include "debug_ext.h"
#define debug_level_flow 6
#define debug_level_error 10
#define debug_level_info 10

#include "net.h"
#include "udp.h"
#include "disk.h"
#include <errno.h>
#include <assert.h>
#include <threads.h>

#include "trfs.h"

// TODO put elements to the beginning of Q, remove (on resend)
// from end. this way received replied will spend less time
// searching q. possibly.


// --------------------------------------------------------
// State
// --------------------------------------------------------

static queue_head_t     requests;

static int		trfs_inited = 0;

static void *		trfs_socket = 0;
static int 		trfs_failed = 0;
static sockaddr		trfs_addr;

static u_int64_t        sessionId = 0;

static hal_mutex_t      lock;


// --------------------------------------------------------
// Local funcs
// --------------------------------------------------------


static void trfs_signal_done(trfs_queue_t *qe);


// --------------------------------------------------------
// UDP IO
// --------------------------------------------------------


static int connect_trfs(void)
{
    if( udp_open(&trfs_socket) )
    {
        SHOW_ERROR0( 0, "UDP trfs - can't open endpoint");
        return -1;
    }

    trfs_addr.port = TRFS_PORT; // local port to be the same

    trfs_addr.addr.len = 4;
    trfs_addr.addr.type = ADDR_TYPE_IP;
    NETADDR_TO_IPV4(trfs_addr.addr) = IPV4_DOTADDR_TO_ADDR(192, 168, 1, 100);

    int rc;
    if( 0 != (rc = udp_bind(trfs_socket, &trfs_addr)) )
        return rc;

    trfs_addr.port = TRFS_PORT; // Remote port

    SHOW_FLOW0( 1, "started" );

    return 0;
}



static errno_t trfs_send(void *pkt, int pktsize)
{
    if(trfs_failed || !phantom_tcpip_active)
        return ENOTCONN;

    SHOW_FLOW0( 1, "sending" );
    int rc;
    if( 0 == (rc = udp_sendto(trfs_socket, pkt, pktsize, &trfs_addr)) )
        return 0;

    if(rc == ERR_NET_NO_ROUTE)
    {
        SHOW_ERROR( 0, "No route", rc);
        return EHOSTUNREACH;
    }
    else
        SHOW_ERROR( 0, "can't send, rc = %d", rc);
    return EIO;
}

static int trfs_recv(void *pkt, int pktsize)
{
    SHOW_FLOW0( 1, "receiving" );
    return udp_recvfrom(trfs_socket, pkt, pktsize, &trfs_addr, 0, 0);
}


// --------------------------------------------------------
// Special ones
// --------------------------------------------------------


static void trfs_reset_session(u_int64_t new_sessionId)
{
    sessionId = new_sessionId;
    SHOW_FLOW0( 1, "NOT IMPL" );
    // TODO reestablish all fileIds
    // TODO rerequest all data
}


// --------------------------------------------------------
// Queue processing
// --------------------------------------------------------


static int ioid = 1;
static int getIoId()
{
    return ioid++;
}



trfs_queue_t *findRequest( trfs_fio_t *fio, u_int32_t type )
{
    trfs_queue_t *elt;
    hal_mutex_lock(&lock);
    queue_iterate( &requests, elt, trfs_queue_t *, chain)
    {
        if( elt->type != type )
            continue;

        if(
           (elt->fio.fileId != fio->fileId) ||
           (elt->fio.ioId != fio->ioId)
          )
            continue;

        u_int64_t our_start = elt->fio.startSector;
        u_int64_t our_end = our_start + elt->fio.nSectors; // one after

        if( fio->startSector < our_start || fio->startSector >= our_end )
        {
            SHOW_ERROR0( 0, "reply is out of req bounds");
            continue;
        }

        u_int64_t his_end = fio->startSector + fio->nSectors;

        if( his_end > our_end )
            SHOW_ERROR( 0, "warning: reply brought too many sectors (%d against %d)", his_end, our_end );

        hal_mutex_unlock(&lock);
        return elt;
    }
    hal_mutex_unlock(&lock);
    return 0;
}


void addRequest(trfs_queue_t *r)
{
    hal_mutex_lock(&lock);
    queue_enter(&requests, r, trfs_queue_t *, chain);
    hal_mutex_unlock(&lock);
}

void removeRequest(trfs_queue_t *r)
{
    hal_mutex_lock(&lock);
    queue_remove(&requests, r, trfs_queue_t *, chain);
    hal_mutex_unlock(&lock);
}


// --------------------------------------------------------
// Request processing
// --------------------------------------------------------

void trfs_mark_recombine_map(trfs_queue_t *qe, u_int64_t start, u_int32_t num)
{
    start -= qe->fio.startSector;

    while( num-- )
    {
        assert(start < sizeof(u_int32_t)*8);

        qe->recombine_map |= (1 << start);
        start++;
    }
}

int trfs_request_complete(trfs_queue_t *qe)
{
    int num = qe->fio.nSectors;
    int start = 0;

    assert( num <= sizeof(u_int32_t)*8 );

    while( num-- )
    {
        if( ! (qe->recombine_map & (1 << start) ) )
            return 0;
        start++;
    }
    return 1;
}


void trfs_process_received_data(trfs_queue_t *qe, trfs_fio_t *fio, void *data)
{
    u_int64_t firstReq = qe->fio.startSector;
    u_int64_t oneAfterReq = qe->fio.startSector + qe->fio.nSectors;

    u_int64_t firstIn = fio->startSector;
    u_int64_t oneAfterIn = fio->startSector + fio->nSectors;

    if( oneAfterIn > oneAfterReq )
        oneAfterIn = oneAfterReq;

    if( firstIn < firstReq )
        firstIn = firstReq;

    if( firstIn >= oneAfterReq )
    {
        SHOW_ERROR0( 0, "TRFS: firstIn >= oneAfterReq" );
        return;
    }
    SHOW_FLOW0( 1, "got data" );

    int64_t _len = (int64_t) (oneAfterIn - firstIn);

    if( _len < 0 )
    {
        SHOW_ERROR0( 0, "TRFS: len < 0" );
        return;
    }

    if( _len > 32 )
    {
        SHOW_ERROR0( 0, "TRFS: len > 32");
        return;
    }

    int len = (int)_len*TRFS_SECTOR_SIZE;
    int shift = (int)(firstIn-firstReq)*TRFS_SECTOR_SIZE;

    memcpy_v2p( (qe->orig_request->phys_page) + shift, data, len );

    //#warning move_data( data, firstIn, len );
    // WRONG!
    //memcpy_v2p( qe->orig_request->phys_page, data, nDataBytes );
}




// --------------------------------------------------------
// Send/Recv packets of specific types
// --------------------------------------------------------




static errno_t sendReadRq( trfs_queue_t *qe )
{
    trfs_pkt_t rq;

    assert( qe->type == TRFS_QEL_TYPE_READ );

    rq.type = PKT_T_ReadRQ;
    rq.sessionId = sessionId;

    rq.readRq.nRequests = 1;
    rq.readRq.request[0] = qe->fio;

    SHOW_FLOW0( 1, "send read rq" );
    return trfs_send(&rq, sizeof(rq)) ? EIO : 0;
}


static errno_t sendWriteRq( trfs_queue_t *qe )
{
    assert( qe->type == TRFS_QEL_TYPE_WRITE );

    // TODO #warning decompose in more packets?

    // Wrong! It counts max trfs_pkt_t size, not one for write rq
    int nDataBytes = sizeof(trfs_pkt_t) + (qe->fio.nSectors * 512);
    int nPktBytes = sizeof(trfs_pkt_t) + nDataBytes;

    trfs_pkt_t *rq = calloc( 1, nPktBytes );


    rq->type = PKT_T_WriteRQ;
    rq->sessionId = sessionId;

    rq->writeRq.info = qe->fio;

    memcpy_p2v( &(rq->writeRq.data), qe->orig_request->phys_page, nDataBytes );

    SHOW_FLOW0( 1, "send wr rq" );
    return trfs_send(rq, nPktBytes) ? EIO : 0;
}


void recvError(trfs_pkt_t *rq)
{
    SHOW_ERROR( 0, "error %d received, '%.*s'",
           rq->error.errorId,
           rq->error.textLen,
           rq->error.text
          );
}


void recvReadReply(trfs_pkt_t *rq)
{
    trfs_fio_t *fio = &(rq->readReply.info);
    void *data = rq->readReply.data;

    trfs_queue_t *qe = findRequest( fio, TRFS_QEL_TYPE_READ );
    if( qe == 0 )
    {
        SHOW_ERROR0( 0, "TRFS: No request for read reply");
        return;
    }

    trfs_process_received_data(qe, fio, data);
    trfs_mark_recombine_map(qe, fio->startSector, fio->nSectors);
    if( trfs_request_complete(qe) )
    {
        removeRequest(qe);
        trfs_signal_done(qe);
    }
}

void recvFindReply(trfs_pkt_t *rq)
{
    SHOW_ERROR0( 0, "TRFS: unexpected findReply");
}





// --------------------------------------------------------
// Control
// --------------------------------------------------------

void sendRequest(trfs_queue_t *qe)
{
    if( qe->type == TRFS_QEL_TYPE_READ )
        sendReadRq(qe);
    else
    if( qe->type == TRFS_QEL_TYPE_WRITE )
        sendWriteRq(qe);
    else
        SHOW_ERROR( 0, "unknown req type %d", qe->type );

}


static void trfs_recv_thread(void *arg)
{
    u_int8_t    buf[TRFS_MAX_PKT];

    hal_set_thread_name("TRFS Recv");

    while(connect_trfs())
    {
        SHOW_ERROR0( 1, "Unable to connect" );
        hal_sleep_msec(20000);
        //return;
    }
    while(1)
    {
        if( trfs_recv( &buf, TRFS_MAX_PKT) )
        {
            hal_sleep_msec( 100 ); // Against tight loop
            continue;
        }

        trfs_pkt_t *rq = (trfs_pkt_t *)buf;

        SHOW_FLOW( 1, "got pkt type %d", rq->type );

        if(rq->sessionId != sessionId)
        {
            trfs_reset_session(sessionId);
            if(rq->type != PKT_T_Error)
                continue;
        }

        switch(rq->type)
        {
        case PKT_T_Error:    		recvError(rq);          break;
    	case PKT_T_ReadReply:        	recvReadReply(rq);      break;
    	case PKT_T_FindReply:        	recvFindReply(rq);      break;

        default:
            SHOW_ERROR( 0, "TRFS: unknown packet type %d", rq->type);

        }
    }
}

static void trfs_resend_thread(void *arg)
{
    hal_set_thread_name("TRFS Resend");

    while(1)
    {
        hal_sleep_msec( 1000 ); // between bursts
        hal_mutex_lock(&lock);
    again:
        if(queue_empty(&requests))
        {
            hal_mutex_unlock(&lock);
            continue;
        }

        trfs_queue_t *elt;

        // Move to end
        queue_remove_first( &requests, elt, trfs_queue_t *, chain );
        queue_enter(&requests, elt, trfs_queue_t *, chain);

        if( TRFS_NEED_RESEND(elt) )
        {
            SHOW_FLOW0( 1, "rerequest" );
            sendRequest(elt);
            hal_sleep_msec( 100 ); // between packets
            goto again;
        }

        hal_mutex_unlock(&lock);
    }
}

// --------------------------------------------------------
// Init and interface
// --------------------------------------------------------


void phantom_trfs_init()
{
/*
    if(connect_trfs())
        return;
*/
    queue_init(&requests);
    hal_mutex_init(&lock,"TRFS");

    hal_start_kernel_thread_arg( trfs_recv_thread, 0 );
    hal_start_kernel_thread_arg( trfs_resend_thread, 0 );

    trfs_inited = 1;
}





errno_t trfsAsyncIo( struct phantom_disk_partition *p, pager_io_request *rq )
{
/*
    assert(p->specific != 0);
    // Temp! Rewrite!
    assert(p->base == 0 );
*/
    trfs_queue_t *qe = calloc( 1, sizeof(trfs_queue_t) );
    if(qe == 0)
        return ENOMEM;

    qe->orig_request = rq;

    qe->fio.fileId = 0;
    qe->fio.ioId = getIoId();
    qe->fio.nSectors = rq->nSect;
    qe->fio.startSector = rq->blockNo;

    qe->type = rq->flag_pageout ? TRFS_QEL_TYPE_WRITE : TRFS_QEL_TYPE_READ;

    qe->recombine_map = 0; // nothing is ready

    SHOW_FLOW0( 1, "new request" );
    addRequest(qe);
    sendRequest(qe);

    return 0;
}


void trfs_signal_done(trfs_queue_t *qe)
{
    SHOW_FLOW0( 1, "done" );
    pager_io_request_done( qe->orig_request );
}

#endif // HAVE_NET


