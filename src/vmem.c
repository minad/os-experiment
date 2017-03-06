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
 
static bool alloc_pgtable(uint32_t, int);

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
        pmem_inc(pgt_page);
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
    
    pmem_inc(page);
    
    memset((void*)addr, 0, PAGE_SIZE);
    return true;
}

void vmem_free_page(uint32_t addr)
{
    uint32_t page;
    
    ASSERT(is_page_aligned(addr)); 
    
    page = vmem_unmap_page(addr);
    if (pmem_dec(page) == 0)
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
    pmem_inc(page_get_address(*pde));

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
    if (pmem_dec(pgt_page) == 0)
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

