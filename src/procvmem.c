#include <vmem.h>
#include <page.h>
#include <string.h>
#include <section.h>
#include <segment.h>
#include <gdt.h>
#include <asm.h>
#include <pmem.h>
#include <regs.h>
#include <debug.h>
#include <stdio.h>
#include <thread.h>

enum {
    // Address masks
    _PDE_MASK   = 0xFFC00000,
    _PTE_MASK   = 0x003FF000,
    //_OFF_MASK = 0x00000FFF,
    
    // Address shifts
    _PDE_SHIFT  = 22,
    _PTE_SHIFT  = 12,

    // Mappings of page directory and tables
    _PDE_START  = 0xFFFFF000,
    _PTE_START  = 0xFFC00000,
};

/*
 * Functions to access PDE, PTE and page tables
 *
 * PTEs: 0xFFC00000 - 0xFFFFEFFF
 * PDEs: 0xFFFFF000 - 0xFFFFFFFF
 */                     

// Get page directory entry by virtual address (points to page table)
static inline page_t* get_pde(uint32_t addr)
{
    return (page_t*)_PDE_START + (addr >> _PDE_SHIFT);
}

// Get page table entry by virtual address (points to page)
static inline page_t* get_pte(uint32_t addr)
{
    return (page_t*)_PTE_START + (addr >> _PTE_SHIFT);
}

// Get page table by virtual address (points to page)
// Similar to get_pte(), but it returns always the first pte!!!
static inline page_t* get_pgtable(uint32_t addr)
{
    return get_pte(addr & _PDE_MASK);
}

// Virtual address to page directory index
static inline int get_pde_index(uint32_t addr)
{
    return (addr & _PDE_MASK) >> _PDE_SHIFT;
}

// Virtual address to page table index
static inline int get_pte_index(uint32_t addr)
{
    return (addr & _PTE_MASK) >> _PTE_SHIFT;
}

/*
 * Prototypes
 */
 
static uint32_t copy_pgtable(uint32_t);
static void copy_page(uint32_t, int);
static bool alloc_pgtable(uint32_t, int);
static vmem_region_t* find_region(uint32_t);

/*
 * Initialize memory management
 */

void __init vmem_init()
{
    int n, tmp;
    uint32_t pgd_page, pgt_page;
    page_t *pgd, *pgt;
        
    /*
     * Create page directory
     */

    pgd_page = pmem_alloc_page();
    ASSERT(pgd_page != BAD_PAGE);
    pgd = (page_t*)PHYS_TO_VIRT(pgd_page);
    memset(pgd, 0, PAGE_SIZE);

    /* Map page directory into itself
     * to allow direct access to:
     *
     * PTEs: 0xFFC00000 - 0xFFFFEFFF
     * PDEs: 0xFFFFF000 - 0xFFFFFFFF
     */    

    pgd[PAGE_ENTRIES - 1] = page_init(pgd_page, PAGE_RW | PAGE_PRESENT);

    /*
     * Map kernel memory to 0xC0000000
     * vmem_map can not be used yet due to odd segmentation
     */
     
    pgt_page = pmem_alloc_page();
    ASSERT(pgt_page != BAD_PAGE);
    pgt = (page_t*)PHYS_TO_VIRT(pgt_page); 
    memset(pgt, 0, PAGE_SIZE);
    
    // Write page table entries
    tmp = get_pte_index(KERNEL_START);
    for (n = 0; n < pmem_get_stats()->kernel; ++n)
    {
        pgt[tmp + n] = page_init(KERNEL_START_PHYS + n * PAGE_SIZE, PAGE_PRESENT | PAGE_RW);
        pmem_inc_counter(pgt_page);
    }

    // .text read-only section
    tmp = get_pte_index(TEXT_START);
    for (n = 0; n < SIZE_TO_PAGES(TEXT_SIZE); ++n)
        pgt[tmp + n] &= ~PAGE_RW;

    // .init.text read-only section
    tmp = get_pte_index(INIT_TEXT_START);
    for (n = 0; n < SIZE_TO_PAGES(INIT_TEXT_SIZE); ++n)
        pgt[tmp + n] &= ~PAGE_RW;
 
    pgd[get_pde_index(VIRT_OFFSET)] = page_init(pgt_page, PAGE_RW | PAGE_PRESENT);

    /*
     * Identity mapping to set-up paging
     */
    pgd[0] = pgd[get_pde_index(VIRT_OFFSET)];    
    enable_paging(pgd_page);

    /* 
     * Reload gdt to set-up new flat segmentation
     * No console output until con_init() from now because
     * vga memory hasn't mapped yet.
     */

    gdt_setup_flat();
 
    // Remove identity mapping (pgd is invalid now, use mapped pde!)
    *get_pde(0) = 0;
    invalidate_tlb(0);
}

/*
 * Copy-on-write functions
 */
/*
// Copy complete page directory (only readable)
uint32_t vmem_copy_pgdir()
{
    uint32_t pgd_page, addr;
    page_t *pde, *new_pde, *new_pgd;
    
    pde = PDE_START;
    
    // Allocate new directory at address 0
    new_pde = new_pgd = alloc_temp(page_get_flags(*pde));
    pgd_page = page_get_address(*get_pte(new_pde));
    
    // Map directory into itself
    new_pde[PAGE_ENTRIES - 1] = page_init(pgd_page, PAGE_RW | PAGE_PRESENT);
    
    // Copy kernel space one-to-one (TODO: without temp)
    memcpy(new_pde + 768, pde + 768, PAGE_SIZE - 764);
    
    // Task address space
    for (addr = 0; addr < 0xC0000000; addr += PAGE_SIZE * PAGE_ENTRIES)
    {
        if (*pde & PAGE_PRESENT)
        {
            uint32_t pgt_page = vmem_copy_pgtable(addr);
            *new_pde = page_init(pgt_page, page_get_flags(*pde) & ~PAGE_RW);
        }
        else
            *new_pde = 0;
        
        ++new_pde;
        ++pde;
    }

    vmem_unmap_page(new_pgd);
    
    return pgd_page;
}

// TODO
// Copy page table (only readable)
static uint32_t copy_pgtable(uint32_t addr)
{
    uint32_t pgt_page;
    page_t *pgt, *new_pgt;
    
    pgt = get_pgtable(addr);
    
    // Allocate new table at address 0
    new_pgt = alloc_temp(page_get_flags(*pgt));
    pgt_page = page_get_address(*get_pte(new_pde));
        
    memcpy(new_pgt, pgt, PAGE_SIZE);
    
    for (pte = new_pgt; pte < new_pgt + PAGE_ENTRIES; ++pte)
        *pte &= ~PAGE_RW;

    return pgt_page;
}

// TODO
// Copy page
static void copy_page(uint32_t addr, int flags)
{
    uint32_t page;
    page_t* pte;
    
    ASSERT(is_page_aligned(addr));
     
    page = pmem_alloc_page();
    
    // Allocate new page temporarly at address 0
    vmem_alloc_page(0, PAGE_RW | PAGE_PRESENT);
    pte = get_pte(0);
    *pte = page_init(page, PAGE_RW | PAGE_PRESENT);
    invalidate_tlb(0);
    
    // Copy old page
    memcpy(0, (void*)addr, PAGE_SIZE);

    pte = get_pte(addr);
    *pte = page_init(page, flags | PAGE_PRESENT);
}

static page_t* alloc_temp(uint32_t flags)
{
    uint32_t addr;
    for (addr = 0xC03FF000; ; addr -= PAGE_SIZE)
    {
	page_t* pte = get_pte(addr);
        if (~*pte & PAGE_PRESENT)
	{
	    if (!vmem_alloc_page(addr, flags))
		return NULL;
	    break;
	}
    }
    return (page_t*)addr;
}
*/

/*
 * Allocate/free single page of virtual memory
 */

bool vmem_alloc(uint32_t start, uint32_t end, int flags)
{
    uint32_t addr;
    
    ASSERT(is_page_aligned(start));
    ASSERT(is_page_aligned(end));
   
    addr = start;
    while (addr < end)
    {
        if (!vmem_alloc_page(addr, flags))
	{
	    vmem_free(start, addr);
	    return false;
	}
        addr += PAGE_SIZE;
    }
    return true;
}

void vmem_free(uint32_t start, uint32_t end)
{
    uint32_t addr;
    
    ASSERT(is_page_aligned(start));
    ASSERT(is_page_aligned(end));
    
    addr = start;
    while (addr < end)
    {
        vmem_free_page(addr);
        addr += PAGE_SIZE;
    }
}

bool vmem_alloc_page(uint32_t addr, int flags)
{
    uint32_t page = pmem_alloc_page();
    
    if (page == BAD_PAGE)
        return false;

    if (!vmem_map_page(addr, page, flags))
    {
        pmem_free_page(page);
        return false;
    }
    
    pmem_inc_counter(page);
    
    memset((void*)addr, 0, PAGE_SIZE);
    return true;
}

void vmem_free_page(uint32_t addr)
{
    uint32_t page;
    
    ASSERT(is_page_aligned(addr)); 
    
    page = vmem_unmap_page(addr);
    if (pmem_dec_counter(page) == 0)
        pmem_free_page(page);
}

/*
 * Map physical memory directly. These functions are really low-level;
 * there's no checking done if the physical pages are free.
 * This can be used for VGA or DMA.
 * 
 * You should use:
 *    
 *    vmem_map(start, end, phys_start, PAGE_WRITE);
 *    pmem_reserve(phys_start, phys_start + (end - start));
 */

bool vmem_map(uint32_t start, uint32_t end, uint32_t phys_start, int flags)
{
    uint32_t addr, page;
    
    ASSERT(is_page_aligned(start));
    ASSERT(is_page_aligned(end));
    ASSERT(is_page_aligned(phys_start));
    
    addr = start;
    page = phys_start;
    while (addr < end)
    {
        if (!vmem_map_page(addr, page, flags))
        {
            vmem_unmap(start, addr);
            return false;
        }
        addr += PAGE_SIZE;
        page += PAGE_SIZE;
    }
    return true;
}

void vmem_unmap(uint32_t start, uint32_t end)
{
    uint32_t addr;
    
    ASSERT(is_page_aligned(start));
    ASSERT(is_page_aligned(end));
    
    addr = start;
    while (addr < end)
    {
        vmem_unmap_page(addr);
        addr += PAGE_SIZE;
    }
}

bool vmem_map_page(uint32_t addr, uint32_t page, int flags)
{
    page_t *pde, *pte;
   
    ASSERT(is_page_aligned(addr));
    ASSERT(is_page_aligned(page));

    pde = get_pde(addr);
    
    // Table not present?
    if (~page_get_flags(*pde) & PAGE_PRESENT)
    {
        if (!alloc_pgtable(addr, flags))
            return false;
    }
       
    // Get page table entry
    pte = get_pte(addr);
    if (page_get_flags(*pte) & PAGE_PRESENT)
        panic("Virtual address 0x%X already allocated", addr);
    
    // Set page table entry to physical page
    *pte = page_init(page, flags | PAGE_PRESENT);
    
    invalidate_tlb(addr);
    
    // Update page table usage counter
    pmem_inc_counter(page_get_address(*pde));

    return true;
}

uint32_t vmem_unmap_page(uint32_t addr)
{
    uint32_t page, pgt_page;
    page_t *pde, *pte;

    ASSERT(is_page_aligned(addr));
    
    // Page directory entry --> Page table
    pde = get_pde(addr);
    
    // Page table freed?
    if (~page_get_flags(*pde) & PAGE_PRESENT)
        panic("Page table for virtual address 0x%X already freed", addr);
    
    // Get page table entry which points to page
    pte = get_pte(addr);
    
    // Is virtual page freed?
    if (~page_get_flags(*pte) & PAGE_PRESENT)
        panic("Virtual address 0x%X already freed", addr);
    
    // Unmap page
    page = page_get_address(*pte);
    *pte = 0;
    invalidate_tlb(addr);

    // Free page table if unused
    pgt_page = page_get_address(*pde); 
    if (pmem_dec_counter(pgt_page) == 0)
    {
        *pde = 0;
        pmem_free_page(pgt_page);
        invalidate_tlb((uint32_t)pte);
    }
    
    return page;
}

static bool alloc_pgtable(uint32_t addr, int flags)
{
    page_t *pgt;
    uint32_t page;
     
    ASSERT(is_page_aligned(addr)); 
    
    page = pmem_alloc_page();
    if (page == BAD_PAGE)
        return false;

    // Put table in the directory
    *get_pde(addr) = page_init(page, flags | PAGE_RW | PAGE_PRESENT);
    
    // Clear page table
    pgt = get_pgtable(addr);
    invalidate_tlb((uint32_t)pgt);
    memset(pgt, 0, PAGE_SIZE);

    return true;
}

void sigsev_handler()
{
    thread_exit();
}

/*
 * Page fault handler
 */

void do_page_fault(const regs_t regs)
{  
    sigsev_handler();

    /*
    enum
    {
        ERROR_USER    = 0x1,
        ERROR_WRITE   = 0x2,
        ERROR_PROTECT = 0x4,
    };  
    
    uint32_t addr;
    vmem_region_t* region;
   
    *
    // Supervisor mode
    if (regs.error_code & ERROR_USER)
    {
        dump_regs(&regs);
        panic("PAGEFAULT IN KERNEL!!!");
    }
    *
    
    // Find region in which fault occured
    addr = get_reg(cr2);
    region = find_region(addr);
    
    // No memory region found
    if (!region)
    {
        printf("Segmentation violation: %s unallocated region\n",
               regs.error_code & ERROR_WRITE ? "Write to" : "Read from");
        dump_regs(&regs);
        sigsev_handler();
        return;
    }
    
    // Protection error
    if (regs.error_code & ERROR_PROTECT)
    {
        // Copy-on-write
        if ((regs.error_code & ERROR_WRITE) &&
            (region->flags & VMEM_WRITE))
	{
            if (!copy_page(addr, PAGE_RW | PAGE_USER))
		panic("Out of memory");
	}
	else
        {
            printf("Segmentation violation: %s protected page\n",
                   regs.error_code & ERROR_WRITE ? "Write to" : "Read from");
            dump_regs(&regs);
            sigsev_handler();
        }
    }
    // Page not present
    else
    {
        // Loading-on-demand
        if (((regs.error_code & ERROR_WRITE) && (region->flags & VMEM_WRITE)) ||
            (region->flags & VMEM_READ))
        {
	    int flags = PAGE_USER;
	    if (region->flags & VMEM_WRITE)
	        flags |= PAGE_RW;
            if (!vmem_alloc_page(addr, flags))
		panic("Out of memory");
        }
        else
        {
            printf("Segmentation violation: %s not-present page\n",
                   regs.error_code & ERROR_WRITE ? "Write to" : "Read from");
            dump_regs(&regs);
            sigsev_handler();
        }
    }
    */
}

static vmem_region_t* find_region(uint32_t addr)
{
    list_t* p;
    vmem_data_t* vmem = &curr_thread->vmem;

    // Check region in the cache
    if (vmem->region_cache &&
	addr >= vmem->region_cache->start && addr < vmem->region_cache->end)
        return vmem->region_cache;
    
    for (p = vmem->region_list.next; p != &vmem->region_list; p = p->next)
    {
        vmem_region_t* region = LIST_OBJECT(p, vmem_region_t, list_entry);
        if (addr >= region->start && addr < region->end)
        {
            vmem->region_cache = region;
            return region;
        }
    }
    
    return NULL;
}
