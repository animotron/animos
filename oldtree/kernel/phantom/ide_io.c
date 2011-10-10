#ifdef ARCH_ia32
#if PAGING_PARTITION
/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * IDE IO
 *
 *
**/

#define DEBUG_MSG_PREFIX "ide"
#include <debug_ext.h>
#define debug_level_flow 0
#define debug_level_error 12
#define debug_level_info 12

#include <errno.h>
#include <disk.h>
#include <disk_q.h>

#include <threads.h>
#include <kernel/config.h>
#include <kernel/stats.h>
#include <kernel/page.h>
#include <kernel/libkern.h>
#include <kernel/info/idisk.h>

#include <pager_io_req.h>

#include <hal.h>

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <kernel/bus/pci.h>

#include "ataio.h"
#include <kernel/dpc.h>


#define IO_PANIC 0
#define BLOCKED_IO 1

static errno_t simple_ide_idenify_device(int dev);



static hal_mutex_t      ide_io;
//static long 		disk_sectors[4];

static i_disk_t         disk_info[4];


// TODO RENAME here and in phantom/dev/ata*
unsigned char buffer[PAGE_SIZE];

static char * devTypeStr[] = { "NO DEVICE", "UNKNOWN TYPE", "ATA", "ATAPI" };


void setup_simple_ide()
{
    hal_mutex_init( &ide_io, "IDE IO" );

    int bm_base_reg = 0xC000;

    // tell ATADRVR how big the buffer is
    reg_buffer_size = PAGE_SIZE; //BUFFER_SIZE;

    SHOW_FLOW0( 1, "Init");

    pci_cfg_t cfg;
    int found = !phantom_pci_find( &cfg, 0x8086, 0x7010 );
    if(!found) found = !phantom_pci_find( &cfg, 0x1106, 0x0571 );

    if(!found)
    {
        SHOW_ERROR0( 0 , "PCI IDE - no known vendor/dev, looking up by class" );
        found = !phantom_pci_find_class( &cfg, 1, 1 ); // class 1.1 is IDE
    }

    if(found)
    {
        bm_base_reg = cfg.base[4];
        SHOW_INFO( 0 , "Found PCI IDE, BM base reg=0x%X, irq %d", bm_base_reg, cfg.interrupt );

        SHOW_INFO( 8 , "base 0 = 0x%X, base 1 = 0x%X, base 2 = 0x%X, base 3 = 0x%X, base 4 = 0x%X, base 5 = 0x%X", cfg.base[0], cfg.base[1], cfg.base[2], cfg.base[3], cfg.base[4], cfg.base[5] );
        SHOW_INFO( 8 , "size 0 = 0x%X, size 1 = 0x%X, size 2 = 0x%X, size 3 = 0x%X, size 4 = 0x%X, size 5 = 0x%X", cfg.size[0], cfg.size[1], cfg.size[2], cfg.size[3], cfg.size[4], cfg.size[5] );
    }
    else
        SHOW_ERROR0( 0 , "PCI IDE not found" );

    pio_set_iobase_addr( 0x1f0, 0x3f0, 0 );


    if(found)
    {
        //int bm_b2 = 0xc000+2;
        int bm_b2 = bm_base_reg+2;
        int rc;
        if( (rc = int_enable_irq( 0, 14, bm_b2, 0x1F0+7 )) )
            printf("Error %d enabling IDE irq\n", rc );

        int dma_rc = dma_pci_config( bm_base_reg );
        if(dma_rc)
            SHOW_ERROR( 0, "Error %d enabling IDE DMA", rc );
    }

    // 2) find out what devices are present -- this is the step
    // many driver writers ignore.  You really can't just do
    // resets and commands without first knowing what is out there.
    // Even if you don't care the driver does care.
    int numDev = reg_config();
    SHOW_INFO( 0, "Found %d devices, dev 0 is %s, dev 1 is %s.\n",
               numDev,
               devTypeStr[ reg_config_info[0] ],
               devTypeStr[ reg_config_info[1] ] );

    int dev = 0;

    int rc = reg_reset( 0, dev );
    if ( rc ) SHOW_ERROR( 0, "can't reset ide dev %d", dev );

    dev = 1;

    rc = reg_reset( 0, dev );
    if ( rc ) SHOW_ERROR( 0, "can't reset ide dev %d", dev );




}





static errno_t ide_write( int ndev, long physaddr, long secno, int nsec )
{
    hal_mutex_lock( &ide_io );

    int i;
    for( i = nsec; i > 0; i-- )
    {
        int rc = dma_pci_lba28(
                               ndev, CMD_WRITE_DMA,
                               0, 1, // feature reg, sect count
                               secno, physaddr,
                               1L );
        if ( rc )
        {
#if IO_PANIC
            panic("IDE write failure sec %ld", secno);
#else
            return EIO;
#endif
        }
        secno++;
        physaddr += 512;

    }

    hal_mutex_unlock( &ide_io );
    return 0;
}

static errno_t ide_read( int ndev, long physaddr, long secno, int nsec )
{
    int tries = 5;

    hal_mutex_lock( &ide_io );

#if BLOCKED_IO
    //if(nsec > 1) printf("nsec %d\n", nsec );

retry:;
    while( nsec > 0 )
    {
        unsigned int secread = nsec;

        if( secread > disk_info[ndev].maxMultSectors )
            secread = disk_info[ndev].maxMultSectors;

        // For some reason fails on QEMU if > 8
        if( secread > 8 )
            secread = 8;

        if( secread > 1 )
            secread &= ~1; // even num of sect

        SHOW_FLOW( 11, "start sect rd sect %d + %d", secno, secread );
        int rc = dma_pci_lba28(
                               ndev, CMD_READ_DMA,
                               0, secread, // feature reg, sect count
                               secno, physaddr,
                               secread );
        SHOW_FLOW( 12, "end   sect rd sect %d", secno );

        if ( rc )
        {
            if( tries-- <= 0 )
            {
                hal_mutex_unlock( &ide_io );
                return EIO;
            }
            else
            {
                printf("IDE read failure sec %ld, retry\n", secno );
                goto retry;
            }
        }
        secno += secread;
        physaddr += (512*secread);
        nsec -= secread;
    }


#else // BLOCKED_IO

retry:;
    int i;
    for( i = nsec; i > 0; i-- )
    {
        SHOW_FLOW( 12, "start sect rd sect %d", secno );
        int rc = dma_pci_lba28(
                               ndev, CMD_READ_DMA,
                               0, 1, // feature reg, sect count
                               secno, physaddr,
                               1L );
        SHOW_FLOW( 12, "end   sect rd sect %d", secno );

        if ( rc )
        {
            if( tries-- <= 0 )
            {
#if IO_PANIC
                panic("IDE read failure sec %ld", secno );
#else
                hal_mutex_unlock( &ide_io );
                return EIO;
#endif
            }
            else
            {
                printf("IDE read failure sec %ld, retry\n", secno );
                goto retry;
            }
        }
        secno++;
        physaddr += 512;
    }

#endif // BLOCKED_IO

    hal_mutex_unlock( &ide_io );
    return 0;
}






// Serves both uits
static phantom_disk_partition_t *both;
static struct disk_q *ideq;
static dpc_request ide_dpc;


static void startIo( struct disk_q *q )
{
    ideq = q; // it is allways the same
    dpc_request_trigger( &ide_dpc, 0 );
}

static void dpc_func(void *a)
{
    (void) a;

    pager_io_request *rq = ideq->current;
    assert(rq);

    if( (rq->unit > 1) || (rq->unit < 0) )
    {
        SHOW_ERROR( 1, "wrong IDE unit %d", rq->unit );
        ideq->ioDone( ideq, ENXIO );
        return;
    }

    if( (rq->blockNo >= (int)disk_info[rq->unit].nSectors) || (rq->blockNo < 0) )
    {
        SHOW_ERROR( 1, "wrong IDE blk %d on unit %d", rq->blockNo, rq->unit );
        ideq->ioDone( ideq, EINVAL );
        return;
    }


    SHOW_FLOW( 7, "starting io on rq %p blk %d", rq, rq->blockNo );

    errno_t rc;

    if(rq->flag_pageout)
    {
        rc = ide_write( rq->unit, rq->phys_page, rq->blockNo, rq->nSect );
    }
    else
    {
        rc = ide_read( rq->unit, rq->phys_page, rq->blockNo, rq->nSect );
    }

    SHOW_FLOW( 7, "calling iodone on q %p, rq %p", ideq, rq );
    assert(rq == ideq->current);
    ideq->ioDone( ideq, rc ? EIO : 0 );
}

errno_t u0AsyncIo( struct phantom_disk_partition *p, pager_io_request *rq )
{
    rq->unit = 0;

    SHOW_FLOW( 11, "part io sect %d, shift %d, o sect %d", rq->blockNo, p->shift, rq->blockNo + p->shift );

    return both->asyncIo( both, rq );
}

errno_t u1AsyncIo( struct phantom_disk_partition *p, pager_io_request *rq )
{
    rq->unit = 1;

    SHOW_FLOW( 11, "part io sect %d, shift %d, o sect %d", rq->blockNo, p->shift, rq->blockNo + p->shift );

    return both->asyncIo( both, rq );
}


static errno_t ideDequeue( struct phantom_disk_partition *p, pager_io_request *rq )
{
    (void) p;

    if( 0 == both->dequeue )
        return ENODEV;

    SHOW_FLOW( 11, "ide dequeue rq %p", rq );

    return both->dequeue( both, rq );
}

static errno_t ideRaise( struct phantom_disk_partition *p, pager_io_request *rq )
{
    (void) p;

    if( 0 == both->raise )
        return ENODEV;

    SHOW_FLOW( 11, "ide raise rq prio %p", rq );

    return both->raise( both, rq );
}



static void make_unit_part( int unit, void *aio )
{
    phantom_disk_partition_t *p = phantom_create_partition_struct( both, 0, disk_info[unit].nSectors );
    assert(p);
    p->flags |= PART_FLAG_IS_WHOLE_DISK;
    p->specific = (void *)-1; // disk supposed to have that
    p->base = 0; // and don't have this
    p->asyncIo = aio;
    p->dequeue = ideDequeue;
    p->raise = ideRaise;
    snprintf( p->name, sizeof(p->name), "Ide%d", unit );
    errno_t err = phantom_register_disk_drive(p);
    if(err)
        SHOW_ERROR( 0, "Ide %d err %d", unit, err );

}


void connect_ide_io(void)
{
    setup_simple_ide();

    dpc_request_init( &ide_dpc, dpc_func );

    int have_dev_0 = !simple_ide_idenify_device(0);
    int have_dev_1 = !simple_ide_idenify_device(1);

    long both_size = lmax(disk_info[0].nSectors, disk_info[1].nSectors);

    both = phantom_create_disk_partition_struct( both_size, 0, 0, startIo );

    if(have_dev_0)         make_unit_part( 0, u0AsyncIo );
    if(have_dev_1)         make_unit_part( 1, u1AsyncIo );


}



void phantom_check_disk_check_virtmem( void * a, int n )
{
    (void) a;
    (void) n;
}

void phantom_check_disk_save_virtmem( void * a, int n )
{
    (void) a;
    (void) n;
}





/*
static void intcpy( char *to, const char *from, int nwords )
{
    while( nwords-- )
    {
        to[0] = from[1];
        to[1] = from[0];

        to += 2;
        from += 2;
    }
}*/



// Fields in the structure returned by the IDENTIFY DEVICE (ECh) and
// IDENTIFY PACKET DEVICE (A1h) commands (WORD offsets)
#define IDENTIFY_FIELD_VALIDITY        53
#define IDENTIFY_DMA_MODES             63
#define IDENTIFY_ADVANCED_PIO          64
#define IDENTIFY_48BIT_ADDRESSING      83
#define IDENTIFY_COMMAND_SET_SUPPORT   83
#define IDENTIFY_COMMAND_SET_ENABLED   86
#define IDENTIFY_UDMA_MODES            88



errno_t simple_ide_idenify_device(int dev)
{
    assert(dev < 4);

    SHOW_INFO( 11, "--- Identifying IDE %d: ", dev );

    u_int16_t buf[256];
    int rc = reg_pio_data_in_lba28(
                                   dev, CMD_IDENTIFY_DEVICE, //CMD_IDENTIFY_DEVICE_PACKET,
                                   0, 0,
                                   0L, //0L,
                                   buf, 1L, 0 );
    if(rc)
    {
        SHOW_ERROR( 0 , "can't identify IDE %d", dev );
        return EIO;
    }

    parse_i_disk_ata( &disk_info[dev], buf );
    dump_i_disk( &disk_info[dev] );
    
#if 0
    //int isATA = buf[0] & 0x8000;
    int isATA = (buf[0] >> 15) & 0x1;


    int isRemovable = buf[0] & 0x0080;
    int is48bitAddr = (buf[IDENTIFY_48BIT_ADDRESSING] >> 10) & 0x1;
    //int hasTrim = buf[169] & 0x1;

#if 0
    if( !isATA )
    {
        printf("Not an ATA device\n");
        return ENXIO;
    }
#endif

    u_int32_t size = (buf[61] << 16) | buf[60];
    //printf( "nSect=%d / %d ", buf[60], buf[61] );


    char serial[21];
    intcpy( serial, ((char*)&buf)+10*2, 10 );
    serial[20] = 0;

    char model[41];
    intcpy( model, ((char*)&buf)+27*2, 20 );
    model[40] = 0;

    SHOW_INFO( 0, "IDE %d is %s ATA, nSect=%d Mb Model: '%s' %s, %s bit LBA",
               dev,
               isATA ? "" : "not",
               size/2048, model,
               isRemovable ? "removable" : "fixed", is48bitAddr ? "48" : "28"
             );

    disk_sectors[dev] = size;

    if( !(buf[49] & 0x0300) )
    {
        SHOW_ERROR0( 0, "Not LBA/DMA disk");
        return ENXIO;
    }
#endif
    return 0;
}



#endif // PAGING_PARTITION
#endif // ARCH_ia32
