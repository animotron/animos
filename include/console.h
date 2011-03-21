/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * Console IO.
 *
 *
**/

#ifndef _PHANTOM_CONSOLE_H
#define _PHANTOM_CONSOLE_H

#include <drv_video_screen.h>

struct console_ops
{
    int 	(*getchar)(void);
    int 	(*putchar)(int);
    int 	(*puts)(const char *);

    // TODO remove - ANSI is used instead
    int 	(*set_fg_color)(struct rgba_t);
    int 	(*set_bg_color)(struct rgba_t);
};

void 	phantom_set_console_ops( struct console_ops *ops );

// Obsolete
void 	phantom_set_console_getchar( int (*_getchar_impl)(void) );
void 	phantom_set_console_putchar( int (*putchar_impl)(int) );
void 	phantom_set_console_puts( int (*puts_impl)(const char *) );

// TODO machdep
int 	driver_isa_vga_putc(int c);

// non-interrupt keyb
int 	phantom_scan_console_getc(void);

void    console_set_fg_color( struct rgba_t );

void    console_set_message_color(void);
void    console_set_error_color(void);
void    console_set_warning_color(void);
void    console_set_normal_color(void);

// TODO remove it from here, it is used temp. in syscalls!
int phantom_dev_keyboard_getc(void);


#endif // _PHANTOM_CONSOLE_H
