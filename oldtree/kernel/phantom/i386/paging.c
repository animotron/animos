#include "../config.h"
#include <malloc.h>
#include <phantom_libc.h>

#include <x86/phantom_pmap.h>
#include <x86/phantom_page.h>
#include <i386/proc_reg.h>
#include <hal.h>

#include "misc.h"
#include "cpu.h"

static pd_entry_t *pdir;
static pt_entry_t *ptabs;



void phantom_paging_init(void)
{
    // On ia32 - allocate all the possible page tables, for simplicity
#if OWN_PHYSMEMALLOC
    extern char phantom_pdir_mem[];
    extern char phantom_ptab_mem[];

    pdir = (pd_entry_t *)&phantom_pdir_mem;
    ptabs = (pt_entry_t *)&phantom_ptab_mem;
#else
    pdir = (pd_entry_t *)smemalign( PAGE_SIZE, PAGE_SIZE );

    // 1024 page tables
    ptabs = (pt_entry_t *)smemalign( PAGE_SIZE, PAGE_SIZE*NPDE );
#endif

    assert((((unsigned) pdir) & (PAGE_SIZE-1)) == 0);
    assert((((unsigned)ptabs) & (PAGE_SIZE-1)) == 0);
    assert( ((unsigned) pdir) != 0);
    assert( ((unsigned)ptabs) != 0);

    memset( ptabs, 0, PAGE_SIZE*NPDE );

    //pd_entry_t *pde = pdir;
    unsigned int *pde = pdir;
    void *ptep = ptabs;
    int i;

    for( i = 0; i < NPDE; i++, pde++ )
    {
        *pde = pa_to_pde((int)ptep) | INTEL_PDE_VALID | INTEL_PDE_USER | INTEL_PDE_WRITE;
        ptep += PAGE_SIZE;
    }


    // Now map all mem equally
    //hal_pages_control( 0, 0, NPDE*NPTE, page_map, page_rw );

#if OWN_PHYSMEMALLOC
    phantom_map_mem_equally();
#else
    // Now map all mem equally - JUST 1/4
    //- upper 2GBs are handled by virt mem, and 0x40000000 - 0x80000000 is for addr space allocator
    hal_pages_control( 0, 0, NPDE*NPTE/4, page_map, page_rw );
#endif

#if 0
    // If we have superpages support, enable 'em in CPU
    if( cpuid.feature_flags & CPUF_4MB_PAGES )
    {
        set_cr4(get_cr4() | CR4_PSE);
    }
#endif


    // Load page directory phys addr to CR3
    // Note that pa = va now
    set_cr3((int)pdir);

    // Tell CPU to start paging
    asm volatile("                      \
                 movl	%0,%%cr0       ;\
                 jmp	1f             ;\
                 1:                     \
                 " : : "r" (get_cr0() | CR0_PG));


}








//static pt_entry_t *get_pte( linaddr_t la );

// Find PDE for given lin addr (usually == virt addr in Phantom)
//#define get_pde( la ) (pdir + lin2pdenum(la))

//#define lin2ptenum(a) ( ((a) >> PTESHIFT) & PTEMASK )
// No mask as we are indexing array of PTs!
#define lin2lin_ptenum(a) ( ((a) >> PTESHIFT) )

//#define ptenum2lin(a) ((a) << PTESHIFT )

// Find PTE for given lin addr
// Since our map is in one piece (all page tables are following each other),
// we can do it remarkably easy!
#define get_pte( la ) (ptabs + lin2lin_ptenum(la))


void phantom_map_page(linaddr_t la, pt_entry_t mapping )
{
    assert(PAGE_ALIGNED(la));
    *get_pte(la) = mapping;
}


void phantom_unmap_page(linaddr_t la )
{
    assert(PAGE_ALIGNED(la));
    *get_pte(la) = 0;
}


int phantom_is_page_accessed(linaddr_t la )
{
    assert(PAGE_ALIGNED(la));
    return (*get_pte(la)) & INTEL_PTE_REF;
}

int phantom_is_page_modified(linaddr_t la )
{
    assert(PAGE_ALIGNED(la));
    return (*get_pte(la)) & INTEL_PTE_MOD;
}



void
phantom_dump_pdir()
{
    pt_entry_t *e = ptabs;
    int was_invalid = 0;
    //phyasddr_t range_start = ptenum2lin(0);

    printf("------------ PageTable\n");

    int i;
    for( i = 0; i < NPDE*NPTE; i++, e++ )
    {

        physaddr_t lin = ptenum2lin(i);

        if(!was_invalid && !( *e & INTEL_PTE_VALID))
        {
            //printf("INVALID\n");
            printf("%08x ", lin);
            was_invalid = 1;
            continue;
        }

        if(was_invalid)
        {
            if(!( *e & INTEL_PTE_VALID))
                continue;

            was_invalid = 0;
            physaddr_t elin = ptenum2lin(i-1);
            printf("-%08x INVALID\n", elin);
        }

        printf("%08x ", lin);
        if(pte_to_pa(*e) == lin)
            printf(" (EQ) ");
        else
            printf( " -> %08X", pte_to_pa(*e) );

        printf( " %s%s%s%s%s%s%s ", 
			*e & INTEL_PTE_GLOBAL ? " GLOBAL" : "",
			*e & INTEL_PTE_MOD ? " MOD" : "",
			*e & INTEL_PTE_REF ? " REF" : "",
			*e & INTEL_PTE_NCACHE ? " NCACHE" : "",
			*e & INTEL_PTE_WTHRU ? " WTHRU" : "",
			*e & INTEL_PTE_USER ? " USER" : "",
			*e & INTEL_PTE_WRITE ? " WRITE" : ""
              );

        printf("\n");

    }

    printf("\n");
}

