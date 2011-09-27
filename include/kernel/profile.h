/**
 *
 * Phantom OS
 *
 * Copyright (C) 2005-2011 Dmitry Zavalishin, dz@dz.ru
 *
 * Kernel profiler.
 *
 *
**/

#ifndef KERNEL_PROFILER_H
#define KERNEL_PROFILER_H

#include <sys/types.h>
#include <kernel/smp.h>

typedef unsigned profiler_entry_t;

#define PROFILER_SKIP_0_PERCENT 1


// We distinguish hit addresses which differ more than this
#define PROFILER_MAP_DIVIDER 8


// size of hit map - must be > ( kernel code size / PROFILER_MAP_DIVIDER )
// curr kernel size 0x100000, reserve twice for future growth
#define PROFILER_MAP_SIZE (0x200000/PROFILER_MAP_DIVIDER)


void profiler_register_interrupt_hit( addr_t ip );
void profiler_dump_map( void );

int percpu_idle_status[MAX_CPUS];
int percpu_idle_count[MAX_CPUS][2]; // [cpu][1] increments in idle, [cpu][0] in not idle
int percpu_cpu_load[MAX_CPUS]; // load percentage


#endif // KERNEL_PROFILER_H

