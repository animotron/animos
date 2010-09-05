/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * Kernel ready: yes
 * Preliminary: no
 *
 *
 **/

#include <kernel/vm.h>

#include "vm/root.h"
#include "vm/exec.h"
#include "vm/object.h"
#include "vm/object_flags.h"
#include "vm/alloc.h"
#include "vm/internal.h"
#include "vm/internal_da.h"
#include "ids/opcode_ids.h"

#include <assert.h>



/**
 *
 * Create general object.
 * TODO: catch for special object creation and throw?
 *
 **/


static struct pvm_object
pvm_object_create_fixed( struct pvm_object object_class )
{
	return pvm_object_create_dynamic( object_class, -1 );
}

struct pvm_object
pvm_object_create_dynamic( struct pvm_object object_class, int das )
{
	struct data_area_4_class *cda = (struct data_area_4_class *)(&(object_class.data->da));

	if( das < 0 )
		das = cda->object_data_area_size;

	unsigned int flags = cda->object_flags;

	struct pvm_object_storage * out = pvm_object_alloc( das, flags, 0 );
	out->_class = object_class;
	//out->_da_size = das; // alloc does it
	//out->_flags = flags; // alloc does it

	struct pvm_object ret;
	ret.data = out;
	ret.interface = cda->object_default_interface.data;

	return ret;
}


/**
 *
 * The most general way to create object. Must be used if no other requirements exist.
 *
 **/

struct pvm_object     pvm_create_object(struct pvm_object type)
{
	struct data_area_4_class *cda = pvm_data_area(type,class);

	struct pvm_object ret = pvm_object_create_fixed(type);

	if( cda->object_flags & PHANTOM_OBJECT_STORAGE_FLAG_IS_INTERNAL )
	{
        // init internal object

		int rec = pvm_iclass_by_class( type.data );

		pvm_internal_classes[rec].init( ret.data );
	}

	return ret;
}






/**
 *
 * Create special null object.
 *
 **/

//__inline__
struct pvm_object     pvm_create_null_object()
{
    return pvm_root.null_object;  // already created once forever
}

void pvm_internal_init_void(struct pvm_object_storage * os) { (void)os; }

void pvm_gc_iter_void(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
}

struct pvm_object     pvm_create_int_object(int _value)
{
	struct pvm_object	out = pvm_object_create_fixed( pvm_get_int_class() );
	((struct data_area_4_int*)&(out.data->da))->value = _value;
	return out;
}

void pvm_internal_init_int(struct pvm_object_storage * os)
{
    (void)os;
}

void pvm_gc_iter_int(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)os;
    (void)arg;
    (void)func;
    // Empty
}


struct pvm_object     pvm_create_string_object_binary(const char *value, int n_bytes)
{
	int das = sizeof(struct data_area_4_string)+n_bytes;
	struct pvm_object string_class = pvm_get_string_class();

	struct pvm_object	_data = pvm_object_create_dynamic( string_class, das );

	struct data_area_4_string* data_area = (struct data_area_4_string*)&(_data.data->da);

	pvm_internal_init_string(_data.data);

	if(value)
		memcpy(data_area->data, value, data_area->length = n_bytes );

	return _data;

}

struct pvm_object     pvm_create_string_object(const char *value)
{
	return pvm_create_string_object_binary(value, strlen(value));
}

struct pvm_object     pvm_create_string_object_binary_cat(
		const char *value1, int n_bytes1,
		const char *value2, int n_bytes2
)
{
	int das = sizeof(struct data_area_4_string)+n_bytes1+n_bytes2;
	struct pvm_object string_class = pvm_get_string_class();

	struct pvm_object	_data = pvm_object_create_dynamic( string_class, das );

	struct data_area_4_string* data_area = (struct data_area_4_string*)&(_data.data->da);

	pvm_internal_init_string(_data.data);

	if(value1)
	{
		memcpy(data_area->data, value1, data_area->length = n_bytes1 );
		if(value2)
		{
			memcpy(data_area->data+n_bytes1, value2, n_bytes2 );
			data_area->length += n_bytes2;
		}
	}

	return _data;

}


void pvm_internal_init_string(struct pvm_object_storage * os)
{
	struct data_area_4_string* data_area = (struct data_area_4_string*)&(os->da);

	memset( (void *)data_area, 0, os->_da_size );
	data_area->length = 0;
}

void pvm_gc_iter_string(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
}


//struct pvm_object     pvm_create_array_object();

struct pvm_object
pvm_create_page_object( int n_slots, struct pvm_object *init, int init_slots )
{
	int das = n_slots*sizeof(struct pvm_object);

	struct pvm_object _data = pvm_object_create_dynamic( pvm_get_page_class(), das );

	struct pvm_object * data_area = (struct pvm_object *)&(_data.data->da);

	// NB! Bug! Here is the way to set object pointers to some garbage value
	//if( init_value )	memcpy( data_area, init_value, das );
	//else            	memset( data_area, 0, das );

	assert(init_slots < n_slots);

	int i;
	for( i = 0; i < init_slots; i++ )
		data_area[i] = *init++;

	for( ; i < n_slots; i++ )
		data_area[i] = pvm_get_null_object();

	return _data;
}

void pvm_internal_init_page(struct pvm_object_storage * os)
{
	assert( ((os->_da_size) % sizeof(struct pvm_object)) == 0); // Natural num of

	int n_slots = (os->_da_size) / sizeof(struct pvm_object);
	struct pvm_object * data_area = (struct pvm_object *)&(os->da);

	int i;
	for( i = 0; i < n_slots; i++ )
		data_area[i] = pvm_get_null_object();
}

void pvm_gc_iter_page(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
	int n_slots = (os->_da_size) / sizeof(struct pvm_object);
	struct pvm_object * data_area = (struct pvm_object *)&(os->da);

	int i;
	for( i = 0; i < n_slots; i++ )
	{
		func( data_area[i], arg );
	}
}






struct pvm_object     pvm_create_call_frame_object()
{
	struct pvm_object _data = pvm_object_create_fixed( pvm_get_call_frame_class() );

	pvm_internal_init_call_frame( _data.data );
	return _data;
}

void pvm_internal_init_call_frame(struct pvm_object_storage * os)
{
	//struct data_area_4_call_frame *da = pvm_data_area( os, call_frame );
	struct data_area_4_call_frame *da = (struct data_area_4_call_frame *)&(os->da);

	da->IP_max = 0;
	da->IP = 0;
	da->code = 0;

	da->this_object = pvm_get_null_object();
	da->prev = pvm_get_null_object();

	da->istack = pvm_create_istack_object();
	da->ostack = pvm_create_ostack_object();
	da->estack = pvm_create_estack_object();
}



#define gc_fcall( f, arg, o )   f( o, arg )


void pvm_gc_iter_call_frame(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
	struct data_area_4_call_frame *da = (struct data_area_4_call_frame *)&(os->da);
	gc_fcall( func, arg, da->this_object );
	gc_fcall( func, arg, da->prev ); // FYI - shall never be followed in normal situation, must contain zero data ptr if being considered by refcount
	gc_fcall( func, arg, da->istack );
	gc_fcall( func, arg, da->ostack );
	gc_fcall( func, arg, da->estack );
}


// interface is a quite usual phantom object.
// (just has a specific flag - for performance only!)
// we just need an internal interface creation code
// to be able to start Phantom from scratch

static struct pvm_object create_interface_worker( int n_methods )
{
	//if(debug_init) printf("create interface\n");

	int das = n_methods*sizeof(struct pvm_object);

	pvm_object_t ret = pvm_object_create_dynamic( pvm_get_interface_class(), das );
	return ret;
}


struct pvm_object     pvm_create_interface_object( int n_methods, struct pvm_object parent_class )
{
	struct pvm_object	ret = create_interface_worker( n_methods );
	struct pvm_object * data_area = (struct pvm_object *)ret.data->da;

	if(pvm_is_null( parent_class ))
		pvm_exec_throw( "create interface: parent is null" );

	struct pvm_object_storage *base_i =  ((struct data_area_4_class*)parent_class.data->da)->object_default_interface.data;

	int base_icount = da_po_limit(base_i);

	if(base_icount > n_methods)
	{
		//pvm_exec_throw( "create interface: child has less methods than parent" );
		base_icount = n_methods; // TODO: what?
	}

	int i = 0;
	// copy methods from parent
	for( ; i < base_icount; i++ )
	{
		pvm_object_t baseMethod = (da_po_ptr(base_i->da))[i];
		ref_inc_o(baseMethod);
		data_area[i] = baseMethod;
	}

	// fill others with nulls
	for( ; i < n_methods; i++ )
		data_area[i] = pvm_get_null_object(); // null

	return ret;
}


void pvm_internal_init_interface(struct pvm_object_storage * os)
{
	memset( os->da, 0, os->_da_size );
}

void pvm_gc_iter_interface(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
    // Must not be called - this is not really an internal object
    panic("Interface GC iterator called");
}





struct pvm_object pvm_create_class_object(struct pvm_object name, struct pvm_object iface, int da_size)
{
	struct pvm_object _data = pvm_object_create_fixed( pvm_get_class_class() );

	struct data_area_4_class *      da = (struct data_area_4_class *)_data.data->da;

	da->object_data_area_size   	= da_size;
	da->object_flags     		= 0;
	da->object_default_interface 	= iface;
	da->sys_table_id             	= -1;

	da->class_name 			= name;
	da->class_parent 		= pvm_get_null_class();

	return _data;
}

void pvm_internal_init_class(struct pvm_object_storage * os)
{
	struct data_area_4_class *      da = (struct data_area_4_class *)os->da;

	da->object_data_area_size   	= 0;
	da->object_flags     		= 0;
	da->object_default_interface 	= pvm_get_null_class();
	da->sys_table_id             	= -1;

	da->class_name 			= pvm_get_null_object();
	da->class_parent		= pvm_get_null_class();
}


void pvm_gc_iter_class(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
	struct data_area_4_class *da = (struct data_area_4_class *)&(os->da);
	gc_fcall( func, arg, da->object_default_interface );
	gc_fcall( func, arg, da->class_name );
	gc_fcall( func, arg, da->class_parent );
}



void pvm_internal_init_thread(struct pvm_object_storage * os)
{
	struct data_area_4_thread *      da = (struct data_area_4_thread *)os->da;

	hal_spin_init(&da->spin);
	da->sleep_flag                      = 0;
	//hal_cond_init(&(da->wakeup_cond), "VmThrdWake");

	da->call_frame   			= pvm_create_call_frame_object();
	da->stack_depth				= 1;

	da->owner.data = 0;
	da->environment.data = 0;

	da->code.code     			= 0;
	da->code.IP 			= 0;
	da->code.IP_max             	= 0;

	//da->_this_object 			= pvm_get_null_object();

	pvm_exec_load_fast_acc(da); // Just to fill shadows with something non-null
}


void pvm_gc_iter_thread(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
	struct data_area_4_thread *da = (struct data_area_4_thread *)&(os->da);
	gc_fcall( func, arg, da->call_frame );
	gc_fcall( func, arg, da->owner );
	gc_fcall( func, arg, da->environment );
}


void pvm_internal_init_code(struct pvm_object_storage * os)
{
	struct data_area_4_code *      da = (struct data_area_4_code *)os->da;

	da->code_size     			= 0;
}

void pvm_gc_iter_code(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
}



void pvm_internal_init_array(struct pvm_object_storage * os)
{
	struct data_area_4_array *      da = (struct data_area_4_array *)os->da;

	da->used_slots     			= 0;
	da->page_size                       = 16;
	da->page.data                       = 0;
}


void pvm_gc_iter_array(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
	struct data_area_4_array *da = (struct data_area_4_array *)&(os->da);
	if(da->page.data != 0)
		gc_fcall( func, arg, da->page );
}



void pvm_internal_init_boot(struct pvm_object_storage * os)
{
     (void)os; 
     // Nohing!
}

void pvm_gc_iter_boot(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
}




void pvm_internal_init_mutex(struct pvm_object_storage * os)
{
    (void)os;

    struct data_area_4_mutex *      da = (struct data_area_4_mutex *)os->da;

    //hal_spin_init( &da->lock );

}

void pvm_gc_iter_mutex(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    //(void)func;
    //(void)os;
    //(void)arg;
    // Empty

    struct data_area_4_mutex *      da = (struct data_area_4_mutex *)os->da;
    int i;

    gc_fcall( func, arg, da->waiting_threads_array );

    for( i = 0; i < MAX_MUTEX_THREADS; i++ )
        gc_fcall( func, arg, da->waiting_threads[i] );

}




void pvm_internal_init_binary(struct pvm_object_storage * os)
{
    (void)os;
    //struct data_area_4_binary *      da = (struct data_area_4_binary *)os->da;
    // empty
}

void pvm_gc_iter_binary(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
}


struct pvm_object     pvm_create_binary_object(int size, void *init)
{
	struct pvm_object ret = pvm_object_create_dynamic( pvm_get_binary_class(), size + sizeof(struct data_area_4_binary) );

	struct data_area_4_binary *da = (struct data_area_4_binary *)ret.data->da;
	//da->code_size = size;
	if( init != NULL ) memcpy( da->data, init, size );
	return ret;
}



void pvm_internal_init_bitmap(struct pvm_object_storage * os)
{
    (void)os;
    // Nothing
}

void pvm_gc_iter_bitmap(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
	struct data_area_4_bitmap *da = (struct data_area_4_bitmap *)&(os->da);
	if(da->image.data != 0)
		gc_fcall( func, arg, da->image );
}



void pvm_internal_init_world(struct pvm_object_storage * os)
{
    (void)os;
    //struct data_area_4_binary *      da = (struct data_area_4_binary *)os->da;
    // empty
}

void pvm_gc_iter_world(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    (void)func;
    (void)os;
    (void)arg;
    // Empty
}



void pvm_internal_init_closure(struct pvm_object_storage * os)
{
	struct data_area_4_closure *      da = (struct data_area_4_closure *)os->da;
	da->object.data = 0;
}

void pvm_gc_iter_closure(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    struct data_area_4_closure *      da = (struct data_area_4_closure *)os->da;
    //if(da->image.object != 0)
    gc_fcall( func, arg, da->object );
}






void pvm_internal_init_weakref(struct pvm_object_storage * os)
{
    struct data_area_4_weakref *      da = (struct data_area_4_weakref *)os->da;
    da->object.data = 0;
#if WEAKREF_SPIN
    hal_spin_init( &da->lock );
#else
    hal_mutex_init( &da->mutex, "WeakRef" );
#endif
}

void pvm_gc_iter_weakref(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
    struct data_area_4_weakref *      da = (struct data_area_4_weakref *)os->da;

    (void) da;
    // No! We are weak ref and do not count our reference or make GC
    // know of it so that our reference does not prevent ref'd object
    // from being GC'ed
    //gc_fcall( func, arg, da->object );
}


struct pvm_object     pvm_create_weakref_object(struct pvm_object owned )
{
    if(owned.data->_satellites.data != 0)
        return owned.data->_satellites;

    struct pvm_object ret = pvm_object_create_fixed( pvm_get_weakref_class() );
    struct data_area_4_weakref *da = (struct data_area_4_weakref *)ret.data->da;

    // Interlocked to make sure no races can happen
    // (ref ass'ment seems to be non-atomic)

#if WEAKREF_SPIN
    wire_page_for_addr( &da->lock );
    int ie = hal_save_cli();
    hal_spin_lock( &da->lock );
#else
    hal_mutex_lock( &da->mutex );
#endif

    // No ref inc!
    da->object = owned;
    owned.data->_satellites = ret;

#if WEAKREF_SPIN
    hal_spin_unlock( &da->lock );
    if( ie ) hal_sti();
    unwire_page_for_addr( &da->lock );
#else
    hal_mutex_unlock( &da->mutex );
#endif

    return ret;
}



struct pvm_object pvm_weakref_get_object(struct pvm_object wr )
{
    struct data_area_4_weakref *da = pvm_object_da( wr, weakref );

    // HACK HACK HACK BUG - we access target here to prevent
    // page fault in spinlock!
    //struct pvm_object out = da->object;
    struct pvm_object out;

    // All we do is return new reference to our object,
    // incrementing refcount before

#if WEAKREF_SPIN
    wire_page_for_addr( &da->lock );
    int ie = hal_save_cli();
    hal_spin_lock( &da->lock );
#else
    hal_mutex_lock( &da->mutex );
#endif

    out = ref_inc_o( da->object );

#if WEAKREF_SPIN
    hal_spin_unlock( &da->lock );
    if( ie ) hal_sti();
    unwire_page_for_addr( &da->lock );
#else
    hal_mutex_unlock( &da->mutex );
#endif

    return out;
}









void pvm_internal_init_window(struct pvm_object_storage * os)
{
    {
    pvm_object_t o;
    o.data = os;
    o.interface = pvm_get_default_interface(os).data;

    // This object needs OS attention at restart
    // TODO do it by class flag in create fixed or earlier?
    pvm_add_object_to_restart_list( o );
    }

}


void pvm_gc_iter_window(gc_iterator_call_t func, struct pvm_object_storage * os, void *arg)
{
}


struct pvm_object     pvm_create_window_object(struct pvm_object owned )
{
    pvm_object_t ret = pvm_object_create_fixed( pvm_get_window_class() );
    struct data_area_4_window *da = (struct data_area_4_window *)ret.data->da;

    /*
    // This object needs OS attention at restart
    // TODO do it by class flag in create fixed or earlier?
    pvm_add_object_to_restart_list( ret );
    */
    return ret;
}

void pvm_gc_finalizer_window( struct pvm_object_storage * os )
{
    // is it called?
}

#include <event.h>

void pvm_restart_window( pvm_object_t o )
{
    struct data_area_4_window *da = pvm_object_da( o, window );

    drv_video_window_enter_allwq( &da->w );
    event_q_put_win( 0, 0, UI_EVENT_WIN_REDECORATE, &da->w );
}



// -----------------------------------------------------------------------
// Specials
// -----------------------------------------------------------------------



struct pvm_object     pvm_create_code_object(int size, void *code)
{
	struct pvm_object ret = pvm_object_create_dynamic( pvm_get_code_class(), size + sizeof(struct data_area_4_code) );

	struct data_area_4_code *da = (struct data_area_4_code *)ret.data->da;
	da->code_size = size;
	memcpy( da->code, code, size );
	return ret;
}



struct pvm_object     pvm_create_thread_object(struct pvm_object start_cf )
{
	struct pvm_object ret = pvm_object_create_fixed( pvm_get_thread_class() );
	struct data_area_4_thread *da = (struct data_area_4_thread *)ret.data->da;

	da->call_frame = start_cf;
	da->stack_depth = 1;

	pvm_exec_load_fast_acc(da);

	// add to system threads list
	pvm_append_array(pvm_root.threads_list.data, ret);
	ref_inc_o(ret);

	// not for each and every one
	//phantom_activate_thread(ret);

	return ret;
}


void   pvm_release_thread_object( struct pvm_object thread )
{
    // the thread was decr once... while executing class loader code... TODO: should be rewritten!

	// remove from system threads list.
	pvm_pop_array(pvm_root.threads_list.data, thread);

	ref_dec_o( thread );  //free now
}




/**
 *
 * Create special syscall-only interface for internal classes.
 *
 **/

static struct pvm_object pvm_create_syscall_code( int sys_num );

// creates interface such that each method
// has just syscall (with a number corresponding to method no)
// and return
void pvm_fill_syscall_interface( struct pvm_object iface, int syscall_count )
{
	struct pvm_object *da = (struct pvm_object *)iface.data->da;

	int i;
	for( i = 0; i < syscall_count; i++ )
		da[i] = pvm_create_syscall_code( i );
}


// syscall code segment generator
static struct pvm_object pvm_create_syscall_code( int sys_num )
{

	if( sys_num <= 15 )
	{
		char code[2];
		code[0] = opcode_sys_0+(unsigned char)sys_num;
		code[1] = opcode_ret;
		return pvm_create_code_object( sizeof(code), code );
	}
	else
	{
		char code[3];
		code[0] = opcode_sys_8bit;
		code[1] = (unsigned char)sys_num;
		code[2] = opcode_ret;
		return pvm_create_code_object( sizeof(code), code );
	}

}



