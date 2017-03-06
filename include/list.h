#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>

typedef struct list_s
{
    struct list_s *next, *prev;
} list_t;

/*
 * List head initializer
 */

#define LIST_INIT(head) { &head, &head }

static inline void list_init(list_t* head)
{
    head->next = head->prev = head;
}

/*
 * Add new entry before head (at the end)
 */

static inline void list_add(list_t *head, list_t *entry)
{
    entry->next = head;
    entry->prev = head->prev;
    head->prev->next = entry;
    head->prev       = entry;
}

/*
 * Delete list entry
 */

static inline void list_delete(list_t *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

/*
 * Check if list is empty
 */

static inline int list_empty(const list_t *head)
{
    return (head->next == head);
}

/*
 * Get outer object
 */

#define LIST_OBJECT(entry, type, member) \
(type *)((char *)(entry) - offsetof(type, member))

#endif
