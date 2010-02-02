/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2008 Dmitry Zavalishin, dz@dz.ru
 *
 * Kernel ready: yes
 * Preliminary: no
 *
 *
**/


#include <phantom_libc.h>

#include "vm/syscall.h"
#include "vm/object.h"
#include "vm/root.h"
#include "vm/exec.h"
#include "vm/bulk.h"
#include "vm/alloc.h"


#include "vm/p2c.h"

#include "vm/wrappers.h"

//extern syscall_func_t  pvm_exec_systables[PVM_SYSTABLE_SIZE][32];


static int debug_print = 0;


//static struct pvm_object root_os_interface_object;

//
//	Default syscalls.
//
//	Any class with internal implementation will present at least:
//
//	sys 0:	Construct. No args.
//
//	sys 1:	Destruct. No args.
//
//	sys 2:	GetClass. No args. Returns class object.
//
//	sys 3:	clone. No args. Returns copy of this, if possible.
//
//	sys 4:	equals. arg is object. Compares by value.
//
//	sys 5:	ToString. No args, returns string object, representing
//			contents of this object.
//
//	sys 6:  ToXML. Returns string object, representing
//			contents of this object in XML form.
//
//	sys 7:	fromXML.
//
//	sys 8:	default activity. depends on class
//
//	sys 9:	secondary activity, depends on class
//
//	sys 10:	third activity
//
//	sys 11:	fourth activity
//
//	sys 15:	int hashCode - returns int
//






// --------- invalid method stub --------------------------------------------

int invalid_syscall(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
printf("invalid syscal for object: "); dumpo( (int)(o.data) );//pvm_object_print( o ); printf("\n");
//printf("invalid value's class: "); pvm_object_print( o.data->_class); printf("\n");
    SYSCALL_THROW_STRING( "invalid syscall called" );
}


// --------- void aka object ------------------------------------------------

int si_void_0_construct(struct pvm_object oo, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN_NOTHING;
}

int si_void_1_destruct(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN_NOTHING;
}

int si_void_2_class(struct pvm_object this_obj, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    //ref_inc_o( this_obj.data->_class );  //increment if class is refcounted
    SYSCALL_RETURN(this_obj.data->_class);
}

int si_void_3_clone(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_THROW_STRING( "void clone called" );
}

int si_void_4_equals(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object him = POP_ARG;

    int ret = (me.data == him.data);

    SYS_FREE_O(him);

    SYSCALL_RETURN(pvm_create_int_object( ret ) );
}

int si_void_5_tostring(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "(void)" ));
}

int si_void_6_toXML(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "<void>" ));
}

int si_void_7_fromXML(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "(void)" ));
}

int si_void_8_def_op_1(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_THROW_STRING( "void default op 1 called" );
}

int si_void_9_def_op_2(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_THROW_STRING( "void default op 2 called" );
}


int si_void_15_hashcode(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_int_object(
       ((int)me.data)^0x3685A634^((int)&si_void_15_hashcode)
                                                     ));
}



syscall_func_t	syscall_table_4_void[16] =
//pvm_exec_systables[PVM_SYSTABLE_ID_NULL][] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_void_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode

};
DECLARE_SIZE(void);





// --------- int ------------------------------------------------------------

static int si_int_3_clone(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_int_object( pvm_get_int(me) ));
}

static int si_int_4_equals(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object him = POP_ARG;

    int same_class = me.data->_class.data == him.data->_class.data;
    int same_value = pvm_get_int(me) == pvm_get_int(him);

    SYS_FREE_O(him);

    SYSCALL_RETURN(pvm_create_int_object( same_class && same_value));
}

static int si_int_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    char buf[32];
    snprintf( buf, sizeof(buf), "%d", pvm_get_int(me) );
    SYSCALL_RETURN(pvm_create_string_object( buf ));
}

static int si_int_6_toXML(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    char buf[32];
    snprintf( buf, 31, "%d", pvm_get_int(me) );
	//SYSCALL_RETURN(pvm_create_string_object( "<void>" ));
    SYSCALL_THROW_STRING( "int toXML called" );
}


syscall_func_t	syscall_table_4_int[16] =
{
    &si_void_0_construct,           	&si_void_1_destruct,
    &si_void_2_class,               	&si_int_3_clone,
    &si_int_4_equals,     		&si_int_5_tostring,
    &si_int_6_toXML,      		&si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            	&si_void_9_def_op_2,
    &invalid_syscall,               	&invalid_syscall,
    &invalid_syscall,               	&invalid_syscall,
    &invalid_syscall,               	&si_void_15_hashcode
};
//int	n_syscall_table_4_int =	(sizeof syscall_table_4_int) / sizeof(syscall_func_t);
DECLARE_SIZE(int);








// --------- string ---------------------------------------------------------

static int si_string_3_clone(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    ASSERT_STRING(me);
    struct data_area_4_string *meda = pvm_object_da( me, string );
    SYSCALL_RETURN(pvm_create_string_object_binary( meda->data, meda->length ));
}

static int si_string_4_equals(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object him = POP_ARG;
    ASSERT_STRING(him);

    struct data_area_4_string *meda = pvm_object_da( me, string );
    struct data_area_4_string *himda = pvm_object_da( him, string );

    int ret =
        me.data->_class.data == him.data->_class.data &&
        meda->length == himda->length &&
        0 == strncmp( (const char*)meda->data, (const char*)himda->data, meda->length )
        ;

    SYS_FREE_O(him);

    // BUG - can compare just same classes
    SYSCALL_RETURN(pvm_create_int_object( ret ) );
}

static int si_string_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(me);
}

static int si_string_8_substring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    ASSERT_STRING(me);
    struct data_area_4_string *meda = pvm_object_da( me, string );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 2);

    int parmlen = POP_INT();
    int index = POP_INT();


    if( index < 0 || index >= meda->length )
        SYSCALL_THROW_STRING( "string.substring index is out of bounds" );

    int len = meda->length - index;
    if( parmlen < len ) len = parmlen;

    if( len < 0 )
        SYSCALL_THROW_STRING( "string.substring length is negative" );


    //printf("substr inx %x len %d parmlen %d\n", index, len, parmlen);

    SYSCALL_RETURN(pvm_create_string_object_binary( meda->data + index, len ));
}

static int si_string_9_charat(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    ASSERT_STRING(me);
    struct data_area_4_string *meda = pvm_object_da( me, string );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    int index = POP_INT();


    int len = meda->length;

    if(index > len-1 )
        SYSCALL_THROW_STRING( "string.charAt index is out of bounds" );

    SYSCALL_RETURN(pvm_create_int_object( meda->data[index]  ));
}


static int si_string_10_concat(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object him = POP_ARG;
    ASSERT_STRING(him);

    struct data_area_4_string *meda = pvm_object_da( me, string );
    struct data_area_4_string *himda = pvm_object_da( him, string );

    pvm_object_t ret = pvm_create_string_object_binary_cat(
    		meda->data, meda->length,
                himda->data, himda->length );

    SYS_FREE_O(him);

    SYSCALL_RETURN( ret );
}


static int si_string_11_length(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    ASSERT_STRING(me);
    struct data_area_4_string *meda = pvm_object_da( me, string );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 0);

    SYSCALL_RETURN(pvm_create_int_object( meda->length ));
}

static int si_string_12_find(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object him = POP_ARG;
    ASSERT_STRING(him);

    struct data_area_4_string *meda = pvm_object_da( me, string );
    struct data_area_4_string *himda = pvm_object_da( him, string );

    unsigned char * ret = strnstrn(
    		meda->data, meda->length,
                himda->data, himda->length );

    SYS_FREE_O(him);

    int pos = -1;

    if( ret != 0 )
        pos = ret - (meda->data);

    SYSCALL_RETURN(pvm_create_int_object( pos ));
}


syscall_func_t	syscall_table_4_string[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_string_3_clone,
    &si_string_4_equals,  &si_string_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_string_8_substring, 		&si_string_9_charat,
    &si_string_10_concat, 		&si_string_11_length,
    &si_string_12_find,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};
DECLARE_SIZE(string);





// --------- thread ---------------------------------------------------------

static int si_thread_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "thread" ));
}

static int si_thread_8_start(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    //phantom_activate_thread(me);
    //SYSCALL_RETURN(pvm_create_string_object( "thread" ));
    SYSCALL_RETURN_NOTHING;
}


static int si_thread_10_pause(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_thread *meda = pvm_object_da( me, thread );

    if(meda != tc)
    	SYSCALL_THROW_STRING("Thread can pause itself only");

    SYSCALL_PUT_THIS_THREAD_ASLEEP();

    SYSCALL_RETURN_NOTHING;
}

static int si_thread_11_continue(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_thread *meda = pvm_object_da( me, thread );

    hal_spin_lock(&meda->spin);
    if( !meda->sleep_flag )
    	SYSCALL_THROW_STRING("Thread is not sleeping in continue");
    hal_spin_unlock(&meda->spin);

    SYSCALL_WAKE_THREAD_UP(meda);

    SYSCALL_RETURN_NOTHING;
}

static int si_thread_14_getOsInterface(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct pvm_object_storage *root = get_root_object_storage();
    struct pvm_object o = pvm_get_field( root, PVM_ROOT_OBJECT_OS_ENTRY );
    SYSCALL_RETURN( ref_inc_o( o ) );
}

static int si_thread_13_getUser(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_thread *meda = pvm_object_da( me, thread );

    SYSCALL_RETURN(meda->owner);
}


static int si_thread_12_getEnvironment(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_thread *meda = pvm_object_da( me, thread );

    if( pvm_is_null(meda->environment) )
    {
        struct pvm_object env = pvm_create_string_object(".phantom.environment");
        struct pvm_object cl = pvm_exec_lookup_class_by_name( env );
        meda->environment = pvm_create_object(cl);
        ref_dec_o(env);
        //ref_dec_o(cl);  // object keep class ref
    }

    SYSCALL_RETURN(meda->environment);
}


syscall_func_t	syscall_table_4_thread[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_thread_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &si_thread_10_pause,            &si_thread_11_continue,
    &si_thread_12_getEnvironment,   &si_thread_13_getUser,
    &si_thread_14_getOsInterface,   &si_void_15_hashcode
};

DECLARE_SIZE(thread);



// --------- call frame -----------------------------------------------------

static int si_call_frame_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "call_frame" ));
}


syscall_func_t	syscall_table_4_call_frame[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_call_frame_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(call_frame);


// --------- istack ---------------------------------------------------------

static int si_istack_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "istack" ));
}


syscall_func_t	syscall_table_4_istack[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_istack_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(istack);


// --------- ostack ---------------------------------------------------------

static int si_ostack_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "ostack" ));
}


syscall_func_t	syscall_table_4_ostack[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_ostack_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(ostack);


// --------- estack ---------------------------------------------------------

static int si_estack_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "estack" ));
}


syscall_func_t	syscall_table_4_estack[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_estack_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(estack);

// --------- class class ---------------------------------------------------------

static int si_class_class_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "class" ));
}

static int si_class_class_8_new_class(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;

    CHECK_PARAM_COUNT(n_param, 3);

    struct pvm_object class_name = POP_ARG;
    int n_object_slots = POP_INT();
    struct pvm_object iface = POP_ARG;

    ASSERT_STRING(class_name);

    struct pvm_object new_class = pvm_create_class_object(class_name, iface, sizeof(struct pvm_object) * n_object_slots);

    //SYS_FREE_O(class_name);  //linked in class object
    //SYS_FREE_O(iface);  //linked in class object

    SYSCALL_RETURN( new_class );
}

syscall_func_t	syscall_table_4_class[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_class_class_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_class_class_8_new_class,    &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(class);


// --------- interface ---------------------------------------------------------

static int si_interface_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "interface" ));
}


syscall_func_t	syscall_table_4_interface[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_interface_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(interface);


// --------- code ---------------------------------------------------------

static int si_code_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "code" ));
}


syscall_func_t	syscall_table_4_code[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_code_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(code);


// --------- page ---------------------------------------------------------

static int si_page_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "page" ));
}


syscall_func_t	syscall_table_4_page[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_page_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_void_8_def_op_1,            &si_void_9_def_op_2,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
};

DECLARE_SIZE(page);


// ---------------------------------------------------------------------------
// --------- bootstrap -------------------------------------------------------
// ---------------------------------------------------------------------------

static int si_bootstrap_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "bootstrap" ));
}


static int si_bootstrap_8_load_class(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    const int bufs = 1024;
    char buf[bufs+1];

    {
    struct pvm_object name = POP_ARG;
    ASSERT_STRING(name);

    struct data_area_4_string *nameda = pvm_object_da( name, string );


    int len = nameda->length > bufs ? bufs : nameda->length;
    memcpy( buf, nameda->data, len );
    buf[len] = '\0';

    SYS_FREE_O(name);
    }

    // BUG! Need some diagnostics from loader here

    struct pvm_object new_class;

    // TODO - is it available just in non-kernel builds?
    if( pvm_load_class_from_module(buf, &new_class))
    {
        SYSCALL_THROW_STRING( "class load error" );
    }
    else
    {
    	SYSCALL_RETURN(new_class);
    }
}

#if 0
static int si_bootstrap_9_load_code(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object name = POP_ARG;
    ASSERT_STRING(name);

#if 0
    const int bufs = 1024;
    char buf[bufs+1];


    struct data_area_4_string *nameda = pvm_object_da( name, string );

    int len = nameda->length > bufs ? bufs : nameda->length;
    memcpy( buf, nameda->data, len );
    buf[len] = '\0';
    SYS_FREE_O(name);

    code_seg cs = load_code(buf);

    SYSCALL_RETURN(pvm_object_storage::create_code( cs.get_code_size(), cs.get_code() ));
#else
    SYS_FREE_O(name);
    SYSCALL_THROW_STRING( "load code not implemented" );
#endif
}
#endif

static int si_bootstrap_16_print(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;

    while( n_param-- )
        {
    	struct pvm_object o = POP_ARG;
        pvm_object_print( o );
        SYS_FREE_O( o );
        }

    SYSCALL_RETURN_NOTHING;
}

static int si_bootstrap_17_register_class_loader(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object loader = POP_ARG;

    pvm_root.class_loader = loader;
    pvm_object_storage_t *root = get_root_object_storage();
    pvm_set_field( root, PVM_ROOT_OBJECT_CLASS_LOADER, pvm_root.class_loader );

    // Don't need do SYS_FREE_O(loader) since we store it

    SYSCALL_RETURN_NOTHING;
}


static int si_bootstrap_18_thread(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object object = POP_ARG;

    // Don't need do SYS_FREE_O(object) since we store it as 'this'

#if 1
    // TODO check object class to be runnable or subclass

    {
    struct pvm_object new_cf = pvm_create_call_frame_object();
    struct data_area_4_call_frame* cfda = pvm_object_da( new_cf, call_frame );

    pvm_ostack_push( pvm_object_da(cfda->ostack, object_stack), me );
    pvm_istack_push( pvm_object_da(cfda->istack, integer_stack), 1); // pass him real number of parameters

    struct pvm_object_storage *code = pvm_exec_find_method( object, 8 );
    pvm_exec_set_cs( cfda, code );
    cfda->this_object = object;

    struct pvm_object thread = pvm_create_thread_object( new_cf );

    printf("here?\n");

    phantom_activate_thread(thread);
    }
#endif


    SYSCALL_RETURN_NOTHING;
}


// THIS IS JUST A TEMP SHORTCUT!
static int si_bootstrap_19_create_binary(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    int nbytes = POP_INT();

    SYSCALL_RETURN( pvm_create_binary_object(nbytes, NULL) );
}

static int si_bootstrap_20_set_screen_background(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    struct pvm_object _bmp = POP_ARG;

    // BUG! Must store it and repaint on OS restart
    if( drv_video_bmpblt(_bmp,0,0) )
    	SYSCALL_THROW_STRING( "not a bitmap" );

    drv_video_window_update_generation();

    // Remove it if will store bmp!
    SYS_FREE_O(_bmp);

    SYSCALL_RETURN_NOTHING;
}

static int si_bootstrap_21_sleep(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    int msec = POP_INT();

    phantom_wakeup_after_msec(msec,tc);

    if(phantom_is_a_real_kernel())
        SYSCALL_PUT_THIS_THREAD_ASLEEP();

    SYSCALL_RETURN_NOTHING;
}

static int si_bootstrap_22_set_os_interface(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    pvm_root.os_entry = POP_ARG;
    ref_saturate_o(pvm_root.os_entry); // make sure refcount is disabled for this object
    struct pvm_object_storage *root = get_root_object_storage();
    pvm_set_field( root, PVM_ROOT_OBJECT_OS_ENTRY, pvm_root.os_entry );
    // No ref dec - we store it.

    SYSCALL_RETURN_NOTHING;
}

static int si_bootstrap_23_getenv(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
	ref_inc_o(pvm_root.kernel_environment);
    SYSCALL_RETURN(pvm_root.kernel_environment);
}


syscall_func_t	syscall_table_4_boot[24] =
{
    &si_void_0_construct,           	&si_void_1_destruct,
    &si_void_2_class,               	&si_void_3_clone,
    &si_void_4_equals,              	&si_bootstrap_5_tostring,
    &si_void_6_toXML,               	&si_void_7_fromXML,
    // 8
    &si_bootstrap_8_load_class,     	&invalid_syscall, //&si_bootstrap_9_load_code,
    &invalid_syscall,               	&invalid_syscall,
    &invalid_syscall,               	&invalid_syscall,
    &invalid_syscall,               	&si_void_15_hashcode,
    // 16
    &si_bootstrap_16_print,             &si_bootstrap_17_register_class_loader,
    &si_bootstrap_18_thread,            &si_bootstrap_19_create_binary,
    &si_bootstrap_20_set_screen_background, &si_bootstrap_21_sleep,
    &si_bootstrap_22_set_os_interface,  &si_bootstrap_23_getenv,
    // 24
    //&si_bootstrap_24_getenv_val
};
DECLARE_SIZE(boot);


// --------- array -------------------------------------------------------

static int si_array_5_tostring(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    // BUG? Recursively call tostring for all of them?
    SYSCALL_RETURN(pvm_create_string_object( "array" ));
}


static int si_array_8_get_iterator(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 0);
    SYSCALL_THROW_STRING( "get iterator is not implemented yet" );
}


static int si_array_9_get_subarray(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 2);

    /*
    int len = POP_INT();
    int base = POP_INT();
    SYSCALL_RETURN();
    */

    SYSCALL_THROW_STRING( "get subarray is not implemented yet" );
}


static int si_array_10_get(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    unsigned int index = POP_INT();

    struct data_area_4_array *da = (struct data_area_4_array *)me.data->da;

    if( index >= da->used_slots )
        SYSCALL_THROW_STRING( "array get - index is out of bounds" );

    struct pvm_object o = pvm_get_ofield( da->page, index);
    SYSCALL_RETURN( ref_inc_o( o ) );
}


static int si_array_11_set(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 2);

    int index = POP_INT();

    struct pvm_object value = POP_ARG;

    pvm_set_array_ofield( me.data, index, value );

    // we increment refcount and return object back.
    // it will possibly be dropped and refcount will decrement again then.
    SYSCALL_RETURN( ref_inc_o( value) );
}

static int si_array_12_size(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 0);

    struct data_area_4_array *da = (struct data_area_4_array *)me.data->da;

    SYSCALL_RETURN(pvm_create_int_object( da->used_slots ) );
}




syscall_func_t	syscall_table_4_array[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_array_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_array_8_get_iterator,       &si_array_9_get_subarray,
    &si_array_10_get,               &si_array_11_set,
    &si_array_12_size,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(array);


// --------- mutex -------------------------------------------------------

static int si_mutex_5_tostring(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "mutex" ));
}


static int si_mutex_8_lock(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_mutex *da = pvm_object_da( me, mutex );

    // No locking in syscalls!!
    //pthread_mutex_lock(&(da->mutex));

    //SYSCALL_PUT_THIS_THREAD_ASLEEP()


    SYSCALL_RETURN_NOTHING;
}

static int si_mutex_9_unlock(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_mutex *da = pvm_object_da( me, mutex );

    // No locking in syscalls!!
    //pthread_mutex_unlock(&(da->mutex));

    SYSCALL_RETURN_NOTHING;
}

static int si_mutex_10_trylock(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_mutex *da = pvm_object_da( me, mutex );

    // No locking in syscalls!!
    //SYSCALL_RETURN(pvm_create_int_object( pthread_mutex_trylock(&(da->mutex)) ));

    SYSCALL_RETURN_NOTHING;
}


syscall_func_t	syscall_table_4_mutex[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_mutex_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_mutex_8_lock,     &si_mutex_9_unlock,
    &si_mutex_10_trylock, &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(mutex);



// --------- cond -------------------------------------------------------

static int si_cond_5_tostring(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( ".internal.cond" ));
}


static int si_cond_8_wait(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_cond *da = pvm_object_da( me, cond );

    // No locking in syscalls!!
    //pthread_cond_wait(&(da->cond));

    SYSCALL_PUT_THIS_THREAD_ASLEEP();

    SYSCALL_RETURN_NOTHING;
}

static int si_cond_9_twait(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_cond *da = pvm_object_da( me, cond );

    SYSCALL_THROW_STRING( "timed wait not impl" );

    // No locking in syscalls!!
    //pthread_cond_timedwait(&(da->cond));

    //SYSCALL_PUT_THIS_THREAD_ASLEEP();


    SYSCALL_RETURN_NOTHING;
}

static int si_cond_10_broadcast(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_cond *da = pvm_object_da( me, cond );

    // No locking in syscalls!!
    //pthread_cond_broadcast(&(da->cond));

    //SYSCALL_WAKE_THREAD_UP(thread)

    SYSCALL_RETURN_NOTHING;
}

static int si_cond_11_signal(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_cond *da = pvm_object_da( me, cond );

    // No locking in syscalls!!
    //pthread_cond_signal(&(da->cond));

    //SYSCALL_WAKE_THREAD_UP(thread)

    SYSCALL_RETURN_NOTHING;
}


syscall_func_t	syscall_table_4_cond[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_mutex_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_cond_8_wait,      &si_cond_9_twait,
    &si_cond_10_broadcast,&si_cond_11_signal,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(cond);


// --------- binary -------------------------------------------------------

static int si_binary_5_tostring(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    // TODO hexdump
    SYSCALL_RETURN(pvm_create_string_object( "(binary)" ));
}


static int si_binary_8_getbyte(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_binary *da = pvm_object_da( me, binary );

    unsigned int index = POP_INT();

    int size = me.data->_da_size - sizeof( struct data_area_4_binary );

    if( index < 0 || index >= size )
        SYSCALL_THROW_STRING( "binary index out of bounds" );

    SYSCALL_RETURN(pvm_create_int_object( da->data[index] ));
}

static int si_binary_9_setbyte(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_binary *da = pvm_object_da( me, binary );

    unsigned int byte = POP_INT();
    unsigned int index = POP_INT();

    int size = me.data->_da_size - sizeof( struct data_area_4_binary );

    if( index < 0 || index >= size )
        SYSCALL_THROW_STRING( "binary index out of bounds" );

    da->data[index] = byte;

    SYSCALL_RETURN_NOTHING;
}

// setrange( binary source, int from pos, int topos, int len )
static int si_binary_10_setrange(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_binary *da = pvm_object_da( me, binary );

    unsigned int len = POP_INT();
    unsigned int frompos = POP_INT();
    unsigned int topos = POP_INT();

    // TODO assert his class!!
    struct pvm_object _src = POP_ARG;
    struct data_area_4_binary *src = pvm_object_da( _src, binary );


    int size = me.data->_da_size - sizeof( struct data_area_4_binary );

    if( topos < 0 || topos+len > size )
        SYSCALL_THROW_STRING( "binary copy dest index/len out of bounds" );

    int src_size = _src.data->_da_size - sizeof( struct data_area_4_binary );

    if( frompos < 0 || frompos+len > src_size )
        SYSCALL_THROW_STRING( "binary copy src index/len out of bounds" );

    //da->data[index] = byte;
    memcpy( (da->data)+topos, (src->data)+frompos, len );

    SYS_FREE_O(_src);

    SYSCALL_RETURN_NOTHING;
}


syscall_func_t	syscall_table_4_binary[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_binary_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_binary_8_getbyte, &si_binary_9_setbyte,
    &si_binary_10_setrange, &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(binary);


// --------- closure -------------------------------------------------------



static int si_closure_9_getordinal(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_closure *da = pvm_object_da( me, closure );

    SYSCALL_RETURN(pvm_create_int_object( da->ordinal ));
}

static int si_closure_10_setordinal(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_closure *da = pvm_object_da( me, closure );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    da->ordinal = POP_INT();

    SYSCALL_RETURN_NOTHING;
}


static int si_closure_11_setobject(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_closure *da = pvm_object_da( me, closure );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    // We do not decrement its refcount, 'cause we store it.
    da->object = POP_ARG;

    SYSCALL_RETURN_NOTHING;
}


syscall_func_t	syscall_table_4_closure[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_binary_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &invalid_syscall, 	    	    &si_closure_9_getordinal,
    &si_closure_10_setordinal, 	    &si_closure_11_setobject,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(closure);

// --------- bitmap -------------------------------------------------------

static int si_bitmap_5_tostring(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    // TODO hexdump
    SYSCALL_RETURN(pvm_create_string_object( "(bitmap)" ));
}


static int si_bitmap_8_fromstring(struct pvm_object me, struct data_area_4_thread *tc )
{

    DEBUG_INFO;
    struct data_area_4_bitmap *da = pvm_object_da( me, bitmap );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

//printf("Load from string\n");

    pvm_object_t _s = POP_ARG;

    if( drv_video_string2bmp( da, pvm_object_da( _s, string)->data ) )
    	SYSCALL_THROW_STRING("can not parse graphics data");

    SYS_FREE_O(_s);

    SYSCALL_RETURN_NOTHING;
}

static int si_bitmap_9_paintto(struct pvm_object me, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    struct data_area_4_bitmap *da = pvm_object_da( me, bitmap );

    int n_param = POP_ISTACK;
    CHECK_PARAM_COUNT(n_param, 1);

    int y = POP_INT();
    int x = POP_INT();
    struct pvm_object _tty = POP_ARG;

    // TODO check class!
    struct data_area_4_tty *tty = pvm_object_da( _tty, tty );
    struct data_area_4_binary *pixels = pvm_object_da( da->image, binary );

    bitmap2bitmap(
    		tty->pixel, tty->w.xsize, tty->w.ysize, x, y,
    		(rgba_t *)pixels, da->xsize, da->ysize, 0, 0,
    		da->xsize, da->ysize
    );
    //drv_video_winblt( &(tty->w), tty->w.x, tty->w.y);
    drv_video_winblt( &(tty->w) );

    SYS_FREE_O(_tty);

    SYSCALL_RETURN_NOTHING;
}


syscall_func_t	syscall_table_4_bitmap[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_bitmap_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_bitmap_8_fromstring,	    &si_bitmap_9_paintto,
    &invalid_syscall, 	    	    &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(bitmap);


// --------- world -------------------------------------------------------

static int si_world_5_tostring(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;
    SYSCALL_RETURN(pvm_create_string_object( "(world)" ));
}

static struct pvm_object_storage	* thread_iface = 0;

static int si_world_8_getMyThread(struct pvm_object o, struct data_area_4_thread *tc )
{
    DEBUG_INFO;

    // TODO spinlock!
    if(thread_iface == 0 )
    {
        struct data_area_4_class *cda = pvm_object_da( pvm_get_thread_class(), class );
        thread_iface = cda->object_default_interface.data;
    }

    struct pvm_object out;

    //out.data = tc; //TODO: BUG: type mismatch!
    out.data =
        (pvm_object_storage_t *)
        (tc - DA_OFFSET()); // XXX HACK!
    out.interface = thread_iface;

    SYSCALL_RETURN( ref_inc_o( out ) );
}


syscall_func_t	syscall_table_4_world[16] =
{
    &si_void_0_construct,           &si_void_1_destruct,
    &si_void_2_class,               &si_void_3_clone,
    &si_void_4_equals,              &si_world_5_tostring,
    &si_void_6_toXML,               &si_void_7_fromXML,
    // 8
    &si_world_8_getMyThread,	    &invalid_syscall,
    &invalid_syscall, 	    	    &invalid_syscall,
    &invalid_syscall,               &invalid_syscall,
    &invalid_syscall,               &si_void_15_hashcode
    // 16

};
DECLARE_SIZE(world);

