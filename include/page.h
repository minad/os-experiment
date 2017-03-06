#ifndef _PAGE_H
#define _PAGE_H

enum
{
    // Page flags
    PAGE_PRESENT      = 0x001,  // Page is present
    PAGE_RW           = 0x002,  // Read/Write (all entries)
    PAGE_USER         = 0x004,  // User (all entries)
    PAGE_PWT          = 0x008,  // Page Write-Through Caching (all entries)
    PAGE_PCD          = 0x010,  // Page Cache Disable (all entries)
    PAGE_ACCESSED     = 0x020,  // Accessed (all entries)
    PAGE_DIRTY        = 0x040,  // Dirty Page (PTE only)

    // Page size
    PAGE_SIZE         = 0x1000, // Page size
    PAGE_ENTRIES      = 0x400,  // Entries per page
};

#define BAD_PAGE ~0

#define SIZE_TO_PAGES(size) \
    (((size) + PAGE_SIZE - 1) / PAGE_SIZE)

typedef uint32_t page_t;

static inline page_t page_init(uint32_t address, int flags)
{
    return ((address & 0xFFFFF000) | (flags & 0x1FF));
}
    
static inline uint32_t page_get_address(page_t page)
{
    return (page & 0xFFFFF000);
}
 
static inline int page_get_flags(page_t page)
{
    return (page & 0x1FF);
}

// Utility to check if address is page aligned
static inline bool is_page_aligned(uint32_t addr)
{
    return ((addr & 0xFFFFF000) == addr);
}

#endif // _PAGE_H
