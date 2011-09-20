/**
 *
 * Phantom OS multithreading library.
 *
 * Copyright (C) 2009-2010 Dmitry Zavalishin, dz@dz.ru
 *
 * Panic.
 *
 * Licensed under CPL 1.0, see LICENSE file.
 *
**/

#include <assert.h>
#include <stdarg.h>
#include <hal.h>
#include <phantom_libc.h>

#include <thread_private.h>

#include <phantom_libc.h>

#include <kernel/board.h>


int panic_reenter = 0;


void panic(const char *fmt, ...)
{
    if(panic_reenter)
        _exit(33);

	board_panic_stop_world();

    hal_cli();
    panic_reenter++;

    printf("Panic: ");
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    printf("\nPress any key");

    board_panic_wait_keypress();

    printf("\r             \r");

    stack_dump();

    dump_thread_stacks();

    printf("\nPress any key to reboot");
    board_panic_wait_keypress();

    exit(33);
}


