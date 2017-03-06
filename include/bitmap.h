#ifndef _BITMAP_H
#define _BITMAP_H

#include <types.h>
#include <math.h>

enum
{
    BITS_SHIFT    = 5,
    BITS_PER_LONG = 1 << BITS_SHIFT,
    BITS_MASK     = BITS_PER_LONG - 1,
};

#define BITS_TO_LONGS(bits) \
    ceil(bits, BITS_PER_LONG)

/*
 * Get/set/clear single bit
 */
    
static inline bool bitmap_getbit(const ulong* map, int bit)
{
    return (*(map + (bit >> BITS_SHIFT)) >> (bit & BITS_MASK)) & 1;
}

static inline void bitmap_setbit(ulong* map, int bit)
{
    *(map + (bit >> BITS_SHIFT)) |= (1 << (bit & BITS_MASK)); 
}

static inline void bitmap_clearbit(ulong* map, int bit)
{
    *(map + (bit >> BITS_SHIFT)) &= ~(1 << (bit & BITS_MASK));
}

/*
 * Set/clear bit range
 */
void bitmap_setbits(ulong* map, int first, int count);
void bitmap_clearbits(ulong* map, int first, int count);

/*
 * Find bits
 */
int bitmap_find1(const ulong* map, int bits);
int bitmap_find0(const ulong* map, int bits);

/*
 * Bitrange only 1 or 0?
 */
bool bitmap_range1(const ulong* map, int first, int count);
bool bitmap_range0(const ulong* map, int first, int count);

#endif // _BITMAP_H
