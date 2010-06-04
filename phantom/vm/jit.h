/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * JIT header
 *
 *
 *
**/

#ifndef JIT_H
#define JIT_H

#include <sys/types.h>

void jit_init(void);

// =========================================================================
//                                        XX
//                  X                      X
// XX XXX   XXXX   XXXXX    XXXXX          X XXX    XXXX   XX XXX   XXXX
//  XX  X  X    X   X           X          XX   X  XXXXXX   XX  X  XXXXXX
//  X   X  X    X   X   X  XXXXXX          X    X  X        X   X  X
// XXX XXX  XXXX    XXXX   XXXX XX        XXXXXX    XXXXX  XXX XXX  XXXXX
//
// Don't you EVER remove or change there funcs. Each instance of OS keeps
// these ordinals in JITted code.
//
// Or we have to find way to force OS instance to reJIT on some serious
// change. Any kernel change, for example?
//
// =========================================================================



enum jit_callable
{
    JIT_F_LOAD_F_ACC, 			// pvm_exec_load_fast_acc
    JIT_F_THREAD_SLEEP_WORKER,          // phantom_thread_sleep_worker
    JIT_F_WAIT_SNAP, 			// phantom_thread_wait_4_snap
    JIT_F_CODE_GET_BYTE, 		// pvm_code_get_byte
    JIT_F_CREATE_OBJ, 			// pvm_create_object
    JIT_F_COPY_OBJ,                     // pvm_copy_object
    JIT_F_CREATE_INT_OBJ,               // pvm_create_int_object

    // LAST!
    JIT_FUNC_TABLE_SIZE
};

//void jit_kernel_func_table_init(void);


enum jit_register
{
    JIT_R_AX, JIT_R_BX, JIT_R_CX, JIT_R_DX, JIT_R_SI, JIT_R_DI
};


struct jit_out
{
    //! Next label id
    int 	nextLabel;

    // Code buffer
    char *	buf;
    char *	bufp; // Current put pos
    int   	bufsize;

    // map of interpreted IP to asm instr offset

    // map of callable kernel funcs - id to func
};


typedef struct jit_out jit_out_t;





void copy_jit_code( jit_out_t *j, void *start, size_t size );




// --------------------------------------------------------------------------
// Direct instructions
// --------------------------------------------------------------------------

void jit_gen_mov( jit_out_t *j, int src, int dst ); // src reg -> dst reg
void jit_gen_cmp( jit_out_t *j, int src, int dst ); // dst = (dst == src)
void jit_gen_neg( jit_out_t *j, int dst ); // dst = !dst

void jit_jz( jit_out_t *, int jlabel );

void jit_gen_push( jit_out_t *j, int src ); // Push reg
void jit_gen_call( jit_out_t *j, enum jit_callable funcId );


// --------------------------------------------------------------------------
// Misc
// --------------------------------------------------------------------------


void jit_o2int( jit_out_t *j );
void jit_gen_isnull( jit_out_t *j, int dst ); // dst = (dst == 0)
void jit_check_snap_request( jit_out_t *j );


// --------------------------------------------------------------------------
// I Stack
// --------------------------------------------------------------------------


void jit_is_pop( jit_out_t *j ); // pop AX
void jit_is_push( jit_out_t *j ); // push AX
void jit_is_top( jit_out_t *j );
void jit_iconst( jit_out_t *j, int const); // mov 0, %ax


// --------------------------------------------------------------------------
// O Stack
// --------------------------------------------------------------------------

void jit_os_push( jit_out_t *j );
void jit_os_pop( jit_out_t *j );
void jit_os_top( jit_out_t *j );

// --------------------------------------------------------------------------
// Ref inc/dec
// --------------------------------------------------------------------------


void jit_refinc( jit_out_t *j, int reg );
void jit_refdec( jit_out_t *j, int reg );


// --------------------------------------------------------------------------
// Summon
// --------------------------------------------------------------------------

void jit_get_null( jit_out_t *j ); // AX = 0 DX = 0
void jit_get_thread( jit_out_t *j ); // AX, DX = thread ptr
void jit_get_this( jit_out_t *j ); // AX, DX = this ptr
void jit_get_class_class( jit_out_t *j ); // AX, DX = ptr
void jit_get_iface_class( jit_out_t *j ); // AX, DX = ptr
void jit_get_code_class( jit_out_t *j ); // AX, DX = ptr
void jit_get_int_class( jit_out_t *j ); // AX, DX = ptr
void jit_get_strting_class( jit_out_t *j ); // AX, DX = ptr
void jit_get_array_class( jit_out_t *j ); // AX, DX = ptr






#endif // JIT_H


