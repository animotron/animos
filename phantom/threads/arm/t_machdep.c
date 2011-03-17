/**
 *
 * Phantom OS multithreading library.
 *
 * Copyright (C) 2009-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * Machine dependent C code, arm.
 *
 * Licensed under CPL 1.0, see LICENSE file.
 *
**/

#if 1
// Not done at all


#include "thread_private.h"
#include <phantom_libc.h>
#include <cpu_state.h>
#include <arm/psr.h>


// asm code
void phantom_thread_trampoline(void);




/**
 * Initializes what is needed for thread to be ready to
 * switch to. Thread structure must be already filled.
 */
void phantom_thread_state_init(phantom_thread_t *t)
{
    t->cpu.sp = (int)(t->stack + t->stack_size);
    t->cpu.fp = 0;
    t->cpu.ip = (int)phantom_thread_trampoline;
    t->cpu.lr = 0;
    t->cpu.cpsr = PSR_SYS32_MODE; // We will change it to user mode later


    int *sp = (int *)(t->cpu.sp);

    // Simulate prev func's stack frame, of all zeroes, for stack trace to stop here
    STACK_PUSH(sp,0);
    STACK_PUSH(sp,0);
    STACK_PUSH(sp,0);
    STACK_PUSH(sp,0);
    STACK_PUSH(sp,0);


    // --- Will be popped by thread switch code ---

    // Now contents for "pop {r4-r12}" in context stwitch, 9 registers

    // I don't know regisrers order for "pop {r4-r12}", so I do it twice :)


    STACK_PUSH(sp,t->start_func); 	// R4
    STACK_PUSH(sp,t->start_func_arg);	// R5
    STACK_PUSH(sp,t);			// R6

    STACK_PUSH(sp,0);
    STACK_PUSH(sp,0);
    STACK_PUSH(sp,0);

    STACK_PUSH(sp,t);			// R6
    STACK_PUSH(sp,t->start_func_arg);	// R5
    STACK_PUSH(sp,t->start_func); 	// R4

    t->cpu.sp = (int)sp;
/*
	// God knows why it works just once for importing main thread,
	// then causes 'no fpu' exception 7 err 0
#if 0 && FXSAVE
    //char mystate[512];
    //char his_state[512];

	// We need two 16 bytes alinged buffers
	char state_buffer[512*2 + 16];  

	char  *my_state = state_buffer;

	while( ((unsigned)my_state) & 0xFu )
		my_state++;

	char *his_state = my_state+512;

    //phantom_thread_fp_init( mystate, t->cpu.fxstate );
//FXDEBUG(double x = 0.0 + 1.0);

    //asm volatile("fxsave %0" : : "m" (my_state));
    i386_fxsave(my_state);
//FXDEBUG(hexdump( &mystate, 512, "FXSTATE our", 0));
    asm volatile("finit " : : );

    //asm volatile("fxsave %0" : : "m" (his_state));
    i386_fxsave(his_state);
//FXDEBUG(hexdump( &mystate, 512, "FXSTATE init", 0));

    i386_fxrstor(my_state);
    //asm volatile("fxrstor %0" : : "m" (my_state));

#endif
*/
}

void switch_to_user_mode()
{
    //asm("ljmp %0, $0" : : "i" (USER_CS));

}

void
phantom_thread_c_starter(void (*func)(void *), void *arg, phantom_thread_t *t)
{
    SET_CURRENT_THREAD(t);
    // Thread switch locked it befor switching into us, we have to unlock
    hal_spin_unlock(&schedlock);

    // TODO all arch thread starters and context

#if DEBUG
    printf("---- !! phantom_thread_c_starter !! ---\n");
#endif

    hal_sti(); // Make sure new thread is started with interrupts on

#if 1
    if( THREAD_FLAG_USER & t->thread_flags )
    {
        //switch_to_user_mode();
    }
#endif

    func(arg);
    t_kill_thread( t->tid );
    panic("thread %d returned from t_kill_thread", t->tid );
}


#endif

