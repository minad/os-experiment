#include <malloc.h>
#include <vmem.h>
#include <page.h>
#include <string.h>
#include <math.h>
#include <debug.h>
#include <thread.h>

//---------------------------------------------------------------
// Heap implementation

// Max. 100M kernel heap
#define HEAP_START 0xF9400000 
#define HEAP_END   0xFF800000

static uint32_t heap_page_end = HEAP_START,
		heap_end      = HEAP_START;

static int brk(void* end)
{
    uint32_t new_page_end = ceil((uint32_t)end, PAGE_SIZE);
	
    if (new_page_end > HEAP_END || new_page_end < HEAP_START)
        return -1;

    // New pages needed?
    if (new_page_end > heap_page_end)
    {
        vmem_alloc(heap_page_end, new_page_end, PAGE_RW);
	heap_page_end = new_page_end;
    }
    // Too many pages allocated?
    else if (new_page_end < heap_page_end)
    {
	vmem_free(new_page_end, heap_page_end);
        heap_page_end = new_page_end;
    }
   
    heap_end = (uint32_t)end;
    
    TRACE("Kernel heap changed: end=0x%X, num_pages=%d\n",
           heap_end, SIZE_TO_PAGES(heap_end - HEAP_START));
    
    return 0;
}

static void* sbrk(int incr)
{
    char* end = (char*)heap_end;
    if (brk(end + incr) < 0)
        return NULL;
    return end;
}

//---------------------------------------------------------------
// malloc implementation

typedef struct block_s
{
    struct block_s* prev;
    struct block_s* next;
    uint            size : 31;
    uint            free : 1;    
} block_t __packed;

enum
{
    /* Minimum block size (with header)
     * to avoid very small fragments.
     */
    BLOCK_MINSIZE = 64,
    BLOCK_HDRSIZE = sizeof (block_t),
};

static block_t block_list;

static inline void block_insert(block_t* a, block_t* b)
{
    b->prev = a;
    b->next = a->next;
    if (a->next)
        a->next->prev = b;
    a->next = b;
}

// Concatenate two coalescing blocks
static inline void blocks_concat(block_t* a, block_t* b)
{
    a->size += b->size;
    a->next = b->next;
    if (b->next)
        b->next->prev = a;
}

void* malloc(size_t size)
{
    block_t *block, *prev;
    size_t newsize;
    
    size += BLOCK_HDRSIZE;
    if (size < BLOCK_MINSIZE)
        size = BLOCK_MINSIZE;
   
    TRACE("malloc: size=%d\n", size);
    
    critical_enter();
    
    // Find first free block
    for (prev = &block_list; block = prev->next; prev = block)
    {
        if (block->free && block->size >= size)
            break;
    }
    
    // Heap full? --> Increase heap size
    if (!block)
    {
        TRACE(" Heap is full\n");
        
	block_t* newb = (block_t*)sbrk(size);
        if (!newb)
            return NULL;
        newb->size = size;
        newb->free = false;
        block_insert(prev, newb);
        
        critical_leave();	
	return ((char*)newb + BLOCK_HDRSIZE);
    }
    
    // Split block?
    newsize = block->size - size;
    if (newsize >= BLOCK_MINSIZE)
    {
        TRACE(" Splitting free block\n");
        
	block_t* newb = (block_t*)((char*)block + size);
        newb->size = newsize;
        newb->free = true;
        block_insert(block, newb);
        block->size = size;
    }
    
    TRACE(" User address: %p\n", (char*)block + BLOCK_HDRSIZE);
    
    block->free = false;

    critical_leave(); 
    return ((char*)block + BLOCK_HDRSIZE);
}

void* calloc(size_t size)
{
    void* block = malloc(size);
    if (!block)
        return NULL;
    memset(block, 0, size);
    return block;
}

void* realloc(void* mem, size_t size)
{
    ASSERT(mem);
    block_t* block = (block_t*)((char*)mem - BLOCK_HDRSIZE);
    void* newmem = malloc(size);
    if (newmem)
        memcpy(newmem, mem, block->size - BLOCK_HDRSIZE);
    free(mem);
    return newmem;
}

void free(void* mem)
{
    block_t *block, *prev, *next;
    ASSERT(mem);
    
    TRACE("free: address=%p\n", (char*)mem - BLOCK_HDRSIZE);
   
    critical_enter();
    
    block = (block_t*)((char*)mem - BLOCK_HDRSIZE);
    prev = block->prev;
    next = block->next;
    
    block->free = true;
    
    // Concatenate with previous
    if (prev != &block_list && prev->free)
    {
        TRACE(" Concatenating with previous\n");
        
	blocks_concat(prev, block);

        // important!!!
        block = prev;
        prev  = prev->prev;
    }
    
    // Decrease heap size
    if (!next)
    {
        TRACE(" Last block; Decreasing heap size\n");
        
	sbrk(-block->size);
        prev->next = NULL;
    }
    // Concatenate with next
    else if (next->free)
    {
        blocks_concat(block, next);
        
	TRACE("Concatenating with next\n");
    }

    critical_leave(); 
}
