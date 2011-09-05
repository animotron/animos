/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * machdep cpu/system init
 *
**/

#define DEBUG_MSG_PREFIX "arch.init"
#include <debug_ext.h>
#define debug_level_flow 6
#define debug_level_error 10
#define debug_level_info 10

#include <kernel/init.h>
#include <kernel/trap.h>
#include <device.h>
#include <ia32/pio.h>
#include <threads.h>


static u_int32_t		isa_read32(u_int32_t addr)                      { return inl(addr); }
static void			isa_write32(u_int32_t addr, u_int32_t value)    { outl(addr,value); }

static u_int16_t		isa_read16(u_int32_t addr)                      { return inw(addr); }
static void			isa_write16(u_int32_t addr, u_int16_t value)    { outw(addr,value); }

static u_int8_t			isa_read8(u_int32_t addr)               	{ return inb(addr); }
static void			isa_write8(u_int32_t addr, u_int8_t value)      { outb(addr,value); }



void arch_init_early(void)
{
    isa_bus.read32 	= isa_read32;
    isa_bus.write32 	= isa_write32;

    isa_bus.read16      = isa_read16;
    isa_bus.write16     = isa_write16;

    isa_bus.read8   	= isa_read8;
    isa_bus.write8      = isa_write8;

#if 0
    // TODO Enable superpage support if we have it.
    if (cpu.feature_flags & CPUF_4MB_PAGES)
    {
        set_cr4(get_cr4() | CR4_PSE);
    }
#endif
}


static int ignore_handler(struct trap_state *ts)
{
    (void) ts;

    //hal_sti(); // It works in open interrupts - NOOO! We carry user spinlock here, so we have to be in closed interrupts up to unlock!
    phantom_scheduler_soft_interrupt();
    // it returns with soft irqs disabled
    hal_enable_softirq();

    return 0;
}


void arch_threads_init()
{
    // HACK!!! Used to give away CPU in thread switch
    // Replace with?
    phantom_trap_handlers[15] = ignore_handler;
}

