/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2009 Dmitry Zavalishin, dz@dz.ru
 *
 * Protected mode VESA driver, incomplete, untested.
 *
 *
**/

#define DEBUG_MSG_PREFIX "video"
#include <debug_ext.h>
#define debug_level_flow 6
#define debug_level_error 10
#define debug_level_info 10

#include "hal.h"
#include "video.h"

#include <sys/cdefs.h>
#include <sys/types.h>
#include <errno.h>

#include <i386/pio.h>
#include <i386/seg.h>
#include <i386/trap.h>
#include <x86/phantom_page.h>
#include <phantom_libc.h>

#include <kernel/vm.h>


static int direct_vesa_probe();
static errno_t load_pm_vesa( void *ROM_va, size_t ROM_size, size_t hdr_offset );
errno_t call_16bit_code( u_int16_t cs, u_int16_t ss, u_int16_t entry, struct trap_state *ts );

struct drv_video_screen_t        video_driver_direct_vesa =
{
    "Protected mode Vesa",
    // size
    0, 0,
    // mouse x y flags
    0, 0, 0,

    // screen
    0,

probe: 	direct_vesa_probe,
start: 	(void *)drv_video_null,
stop:  	(void *)drv_video_null,

update:	drv_video_null,
bitblt: (void *)drv_video_null,
winblt: (void *)drv_video_null,
readblt: (void *)drv_video_null,

mouse:  drv_video_null,
redraw_mouse_cursor: 	drv_video_draw_mouse_deflt,
set_mouse_cursor: 	drv_video_set_mouse_cursor_deflt,
mouse_disable:          drv_video_mouse_off_deflt,
mouse_enable:          	drv_video_mouse_on_deflt,
};



struct pm_vesa_header
{
    char         signature[4];
    u_int16_t    entryPoint;
    u_int16_t    entryInit;      // offset of init func
    u_int16_t    biosDataSel;
    u_int16_t    A0000Sel;
    u_int16_t    B0000Sel;
    u_int16_t    B8000Sel;
    u_int16_t    codeAsDataSel;
    u_int8_t     protectedMode;
    u_int8_t     checksum;
} __packed;


static int direct_vesa_probe()
{
    const int ROM_pa = 0xC0000;
    const int ROM_size = 32*1024;
    const int ROM_pages = ROM_size/PAGE_SIZE;

    char *ROM_va = (char *)phystokv(ROM_pa);

    hal_pages_control( ROM_pa, ROM_va, ROM_pages, page_map, page_ro );

    char *p = ROM_va;
    SHOW_FLOW( 0, "Look for VESA PM entry starting at 0x%X", p);

    char *entry = 0;
    int cnt = ROM_size;
    while(cnt--)
    {
        if(
           p[0] == 'P' && p[1] == 'M' &&
           p[2] == 'I' && p[3] == 'D' )
        {
            SHOW_FLOW( 0, "Found VESA PM entry at 0x%X", p);
            entry = p;
            break;
        }
        p++;
    }

    if(entry == NULL)
        SHOW_FLOW0( 0, "no VESA PM entry");
    else
    {
        //struct pm_vesa_header hdr = *((struct pm_vesa_header *)entry);
        int hdr_offset = entry - ROM_va;

        int c = sizeof(struct pm_vesa_header);

        char csum = 0;

        while(c--)
            csum += *entry++;

        if(csum)
            SHOW_ERROR0( 0, "VESA PM entry checksum is wrong");
        else
        {
#if 0
            SHOW_FLOW0( 1, "gettig VESA PM BIOS copy");
            load_pm_vesa( ROM_va, ROM_size, hdr_offset );
#endif
        }
    }

    hal_pages_control( ROM_pa, ROM_va, ROM_pages, page_unmap, page_ro );
pressEnter("PM VESA done");

    // Experimental code
    return 0;
}


// TODO this is X86-specific code, move to i386 dir

#define BDA_size 0x600
#define STK_size 1024
#define DBA_size 128*1024

static void *data_buffer;

static errno_t load_pm_vesa( void *in_ROM_va, size_t ROM_size, size_t hdr_offset )
{
    // ROM copy
    physaddr_t	ROM_pa;
    void *	ROM_va;

    // Bios Data Area
    physaddr_t	BDA_pa;
    void *	BDA_va;

    // Stack
    physaddr_t	STK_pa;
    void *	STK_va;

    // Data Buffer Area (for 32-16 bit data exchange)
    physaddr_t	DBA_pa;
    void *	DBA_va;

    hal_pv_alloc( &ROM_pa, &ROM_va, ROM_size );
    hal_pv_alloc( &BDA_pa, &BDA_va, BDA_size );
    hal_pv_alloc( &STK_pa, &STK_va, STK_size );
    hal_pv_alloc( &DBA_pa, &DBA_va, DBA_size );

    data_buffer = DBA_va;

    struct pm_vesa_header *hdr = ROM_va+hdr_offset;

    assert(hdr->signature[0] == 'P' && hdr->signature[1] == 'M' &&
           hdr->signature[2] == 'I' && hdr->signature[3] == 'D' );


    memcpy( ROM_va, in_ROM_va, ROM_size );
    memset( BDA_va, 0, BDA_size );

    hdr->biosDataSel = VBE3_BD_16;
    hdr->A0000Sel = VBE3_A0_16;
    hdr->B0000Sel = VBE3_B0_16;
    hdr->B8000Sel = VBE3_B8_16;
    hdr->codeAsDataSel = VBE3_DS_16;
    hdr->protectedMode = 0xFF;

    // BUG include something
    extern struct real_descriptor 	gdt[GDTSZ];

    // load segment descriptors!

    make_descriptor(gdt, VBE3_CS_16, ROM_pa, ROM_size-1, ACC_PL_K | ACC_CODE_R, SZ_16 );
    make_descriptor(gdt, VBE3_DS_16, ROM_pa, ROM_size-1, ACC_PL_K | ACC_DATA_W, SZ_16 );
    make_descriptor(gdt, VBE3_BD_16, BDA_pa, BDA_size-1, ACC_PL_K | ACC_DATA_W, SZ_16 );
    make_descriptor(gdt, VBE3_ST_16, STK_pa, STK_size-1, ACC_PL_K | ACC_DATA_W, SZ_16 );
    make_descriptor(gdt, VBE3_DB_16, DBA_pa, DBA_size-1, ACC_PL_K | ACC_DATA_W, SZ_16 );

    make_descriptor(gdt, VBE3_A0_16, 0xA0000, 0xffff, ACC_PL_K | ACC_DATA_W, SZ_16 );
    make_descriptor(gdt, VBE3_B0_16, 0xB0000, 0xffff, ACC_PL_K | ACC_DATA_W, SZ_16 );
    make_descriptor(gdt, VBE3_B8_16, 0xB8000, 0xffff, ACC_PL_K | ACC_DATA_W, SZ_16 );


    struct trap_state ts;

#if 0
    errno_t ret;
    if( (ret = call_16bit_code( VBE3_CS_16, VBE3_ST_16, hdr->entryInit, &ts )) )
        return ret;

    if( (ret = call_16bit_code( VBE3_CS_16, VBE3_ST_16, hdr->entryPoint, &ts )) )
        return ret;
#endif

    return 0;
}




#if 0
#ifdef HAVE_CODE16GCC
#define CODE16_STRING ".code16gcc"
#else
#define CODE16_STRING ".code16"
#endif


/* Switch GAS into 16-bit mode.  */
#define CODE16 asm(CODE16_STRING);

/* Switch back to 32-bit mode.  */
#define CODE32 asm(".code32");


/*
 * Macros to switch between 16-bit and 32-bit code
 * in the middle of a C function.
 * Be careful with these!
 * If you accidentally leave the assembler in the wrong mode
 * at the end of a function, following functions will be assembled wrong
 * and you'll get very, very strange results...
 * It's safer to use the higher-level macros below when possible.
 */
#define i16_switch_to_32bit(cs32) asm volatile("\
	ljmp	%0,$1f\
        .code32\
	1:\
	" : : "i" (cs32));
#define switch_to_16bit(cs16) asm volatile("\
		ljmp	%0,$1f\
		"CODE16_STRING"\
	1:\
	" : : "i" (cs16));
#endif




errno_t call_16bit_code( u_int16_t cs, u_int16_t ss, u_int16_t entry, struct trap_state *ts )
{
    // need AX, BX, CX, DX, ES, DI

    // load regs
    // set stack

    //asm volatile("lcall	%0,%1" : : "i" (cs), "i" (entry));

    // return to old stack
    // store regs

    return ENXIO;
}
