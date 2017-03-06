#include <pmem.h>
#include <bitmap.h>
#include <multiboot.h>
#include <stdio.h>
#include <section.h>
#include <page.h>
#include <debug.h>
#include <ansicode.h>
#include <string.h>

enum
{
    // 1024 pages make one superpage
    SUPER_SIZE = 1024,
    
    // Start of upper memory
    UPPER_START = 0x100000,
};

// Memory statistics
static pmem_stats_t stats;

/*
 * Superpage map
 *    - One bit for 1024 pages
 *    - 1 if one or more pages in this
 *      superpage are free
 *    - 0 if superpage is used completely
 */
 
static ulong* super_map;
static int    super_map_size;

/*
 * Page map
 *    - One bit for each page
 *    - 1 if page is free
 *    - 0 if page is used
 */
 
static ulong* page_map;
static int    page_map_size;

/*
 * Page usage counter
 */
static ushort* page_counter;
static int     page_counter_size;

static void init_bitmap();
static void dump_bitmap(ulong*, int);

// Index in memory map to page address
static inline uint32_t index_to_page(int index)
{
    if (index < stats.lower)
        return (index * PAGE_SIZE);
    return ((index - stats.lower) * PAGE_SIZE + UPPER_START);
}

// Page address to index in memory map
static inline int page_to_index(uint32_t page)
{
    if (page / PAGE_SIZE < stats.lower)
        return (page / PAGE_SIZE);
    return ((page - UPPER_START) / PAGE_SIZE + stats.lower);
}

// Initialize physical memory management
void __init pmem_init()
{
    if (~multiboot_get()->flags & MULTIBOOT_MEM)
        panic("No memory information from boot manager");

    stats.upper = (multiboot_get()->mem_upper << 10) / PAGE_SIZE;
    stats.lower = (multiboot_get()->mem_lower << 10) / PAGE_SIZE;
    stats.total = stats.upper + stats.lower;
    
    stats.free = stats.total;
    stats.used = 0;
    stats.kernel = SIZE_TO_PAGES(KERNEL_SIZE);

    init_bitmap();
}

const pmem_stats_t* pmem_get_stats()
{
    return &stats;
}

// Memory statistics
void pmem_dump_stats(const pmem_stats_t* stats)
{
    printf("Memory Statistics:\n"
           " Lower: %dK\n"
           " Upper: %dK\n"
           " Total: %dK\n"
           " Free:  %dK\n"
           " Used:  %dK\n",
           stats->lower << 2, stats->upper << 2,
           stats->total << 2, stats->free << 2,
           stats->used << 2);
}

// Dump memory map
void pmem_dump_map()
{
    printf("Superpage bitmap at 0x%p (4M per bit, 256M per line):\n\n", super_map);
    dump_bitmap(super_map, super_map_size);
    
    printf("Page bitmap at 0x%p (4K per bit, 256K per line):\n\n", page_map);
    dump_bitmap(page_map, page_map_size);
}

// Allocate physical page
uint32_t pmem_alloc_page()
{
    int offset, index;
    
    if (stats.free == 0)
    {
        puts(FG_RED "Out of physical memory" NOCOLOR);
        return BAD_PAGE;
    }

    // Find (partially) free super-page
    index = bitmap_find1(super_map, super_map_size);
    ASSERT(index >= 0);

    // Find page
    offset = index * SUPER_SIZE;
    index = bitmap_find1(page_map + offset / BITS_PER_LONG,
                        page_map_size - offset / BITS_PER_LONG);
    ASSERT(index >= 0);
    
    // Offset from super-page
    index += offset;
    
    // Mark as used
    bitmap_clearbit(page_map, index);
    
    // Last page in a superpage used? --> Superpage is full
    if (index % SUPER_SIZE == SUPER_SIZE - 1)
        bitmap_clearbit(super_map, index / SUPER_SIZE);
    
    ++stats.used;
    --stats.free;

    return index_to_page(index);
}

// Free physical page
void pmem_free_page(uint32_t page)
{
    int index;

    ASSERT(is_page_aligned(page));

    index = page_to_index(page);

#ifndef NDEBUG
    if (bitmap_getbit(page_map, index))
        panic("Physical page at 0x%X already freed", page);
#endif
    
    bitmap_setbit(page_map, index);
    bitmap_setbit(super_map, index / SUPER_SIZE);

    --stats.used;
    ++stats.free;
}

void pmem_alloc_region(uint32_t start, uint32_t end)
{
    int si, ei, ssi, sei;
    
    ASSERT(is_page_aligned(start));
    ASSERT(is_page_aligned(end));
    
    si = page_to_index(start);
    ei = page_to_index(end);
    
#ifndef NDEBUG
    if (!bitmap_range1(page_map, si, ei - si)) 
        panic("Can't allocate physical memory region: 0x%X - 0x%X", start, end);  
#endif

    bitmap_clearbits(page_map, si, ei - si);
    
    ssi = (si + SUPER_SIZE - 1) / SUPER_SIZE;
    sei = ei / SUPER_SIZE;
    bitmap_clearbits(super_map, ssi, sei - ssi);

    stats.free -= ei - si;
    stats.used += ei - si;    
}

void pmem_free_region(uint32_t start, uint32_t end)
{
    int si, ei, ssi, sei;
    
    ASSERT(is_page_aligned(start));
    ASSERT(is_page_aligned(end));
    
    si = page_to_index(start);
    ei = page_to_index(end);
    
#ifndef NDEBUG
    if (!bitmap_range0(page_map, si, ei - si)) 
        panic("Can't free physical memory region: 0x%X - 0x%X", start, end);  
#endif

    bitmap_setbits(page_map, si, ei - si);
    
    ssi = si / SUPER_SIZE;
    sei = (ei + SUPER_SIZE - 1) / SUPER_SIZE;
    bitmap_setbits(super_map, ssi, sei - ssi);

    stats.free += ei - si;
    stats.used -= ei - si; 
}

int pmem_inc(uint32_t page)
{
    int i = page_to_index(page);
    ASSERT(i < page_counter_size);
    return ++page_counter[i];
}

int pmem_dec(uint32_t page)
{ 
    int i = page_to_index(page);
    ASSERT(i < page_counter_size);
    return --page_counter[i];
}

// Initialize memory bitmap
static void __init init_bitmap()
{
    int size = 0;
    char* start = (char*)(KERNEL_START + KERNEL_SIZE);

    // Allocate page map
    page_map = (ulong*)(start + size);
    page_map_size  = stats.total;
    size += ceil(page_map_size >> 3, sizeof (ulong));
   
    // Allocate super map
    super_map = (ulong*)(start + size);
    super_map_size = (page_map_size + SUPER_SIZE - 1) / SUPER_SIZE;
    size += ceil(super_map_size >> 3, sizeof (ulong));

    // Allocate page usage counters
    page_counter = (ushort*)(start + size);
    page_counter_size = stats.total;
    size += page_counter_size * sizeof (ushort);
    memset(page_counter, 0, page_counter_size * sizeof (ushort));
    
    // Add to kernel memory
    stats.kernel += SIZE_TO_PAGES(size);
    
    // All pages are free
    bitmap_setbits(page_map, 0, page_map_size);
    bitmap_setbits(super_map, 0, super_map_size);
    
    // Reserve kernel memory
    pmem_alloc_region(KERNEL_START_PHYS,
                      KERNEL_START_PHYS + stats.kernel * PAGE_SIZE);
}

static void dump_bitmap(ulong* map, int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        putchar(bitmap_getbit(map, i) ? '-' : 'U');
        if (i % 65 == 64)
            putchar('\n');
    }
    putchar('\n');
    putchar('\n');
}
