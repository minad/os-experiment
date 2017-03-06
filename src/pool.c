#include <pool.h>
#include <malloc.h>

static void pool_grow(pool_t* pool, size_t count)
{
    char* memory = (char*)malloc(pool->element_size * count + sizeof (list_t));
    list_add(&(pool->blocks), (list_t*)memory);
    memory += sizeof (list_t);
    for (int i = 0; i < count; ++i)
	list_add(&(pool->free_elements), (list_t*)(memory + i * pool->element_size));
}

void pool_init(pool_t* pool, size_t count, size_t size)
{
    if (size < sizeof (list_t))
	size = sizeof (list_t);
    list_init(&(pool->free_elements));
    list_init(&(pool->blocks));
    pool->element_size = size;
    pool->element_count = count;
    pool_grow(pool, count);
}

void pool_free(pool_t* pool)
{
    list_t* block;
    for (block = pool->blocks.next; block != &(pool->blocks);)
    {
        void* mem = block;
	block = block->next;
	free(mem);
    }
}

void* pool_get(pool_t* pool)
{
    list_t* head = &(pool->free_elements);
    list_t* next;
    if (list_empty(head))
	pool_grow(pool, pool->element_count * 3 / 2);
    next = pool->free_elements.next;
    list_delete(next);
    return next;
}

void* pool_release(pool_t* pool, void* obj)
{
    list_add(&(pool->free_elements), (list_t*)obj);
}
