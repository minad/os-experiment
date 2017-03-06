#include <bitmap.h>
#include <string.h>

void bitmap_setbits(ulong* map, int first, int count)
{
    int last = first + count;
    int begin = first >> BITS_SHIFT;
    int end = last >> BITS_SHIFT;
    
    if (begin < end)
    {
        if (begin + 1 < end)
            memset(map + begin + 1, 0xFF, (end - begin - 1) * sizeof (ulong));
        *(map + begin) |= ~((1 << (first & BITS_MASK)) - 1);
        *(map + end)   |=   (1 << (last  & BITS_MASK)) - 1;
    }
    else
    {
        *(map + begin) |= ~((1 << (first & BITS_MASK)) - 1) &
                           ((1 << (last  & BITS_MASK)) - 1);
    }
}

void bitmap_clearbits(ulong* map, int first, int count)
{
    int last = first + count;
    int begin = first >> BITS_SHIFT;
    int end = last >> BITS_SHIFT;
    
    if (begin < end)
    {
        if (begin + 1 < end)
            memset(map + begin + 1, 0, (end - begin - 1) * sizeof (ulong));
        *(map + begin) &=   (1 << (first & BITS_MASK)) - 1;
        *(map + end)   &= ~((1 << (last  & BITS_MASK)) - 1);
    }
    else
    {
        *(map + begin) &= ((1 << (first & BITS_MASK)) - 1) |
                         ~((1 << (last  & BITS_MASK)) - 1);
    }
}

int bitmap_find1(const ulong* map, int bits)
{
    int n;
    const ulong* p = map;
    const ulong* end = p + BITS_TO_LONGS(bits);
    
    while (p < end && *p == 0)
        ++p;
    
    if (p == end)
        return -1;

    for (n = 0; n < BITS_PER_LONG; ++n)
    {
        if (*p & (1 << n))
        {
            n += (p - map) * BITS_PER_LONG;
            if (n >= bits)
                return -1;
            return n;
        }
    }
            
    return -1;
}

int bitmap_find0(const ulong* map, int bits)
{
    int n;
    const ulong* p = map;
    const ulong* end = p + BITS_TO_LONGS(bits);
    
    while (p < end && *p == 0xFFFFFFFF)
        ++p;
    
    if (p < end)
        return -1;

    for (n = 0; n < BITS_PER_LONG; ++n)
    {
        if (~*p & (1 << n))
        {
            n += (p - map) * BITS_PER_LONG;
            if (n >= bits)
                return -1;
            return n;
        }
    }
            
    return -1;
}

bool bitmap_range1(const ulong* map, int first, int count)
{
/*
    const ulong* p = map + BITS_TO_LONGS(first);
    const ulong* end = p + (count >> BITS_SHIFT);
    ulong tmp;
    int i;
    
    while (p < end)
    {
        if (*p++ != 0xFFFFFFFF) 
             return false;
    }

    tmp = ~*(map + (first >> BITS_SHIFT));
    for (i = first & BITS_MASK; i < BITS_PER_LONG; ++i)
    {
        if (tmp & (1 << i))
             return false;
    }
    
    first += count;
    tmp = ~*(map + BITS_TO_LONGS(first) - 1);
    for (i = first & BITS_MASK; i < BITS_PER_LONG; ++i)
    {
        if (tmp & (1 << i))
             return false;
    }
*/
    return true;
}

bool bitmap_range0(const ulong* map, int first, int count)
{
/*
    const ulong* p = map + BITS_TO_LONGS(first);
    const ulong* end = p + (count >> BITS_SHIFT);
    ulong tmp;
    int i;
    
    while (p < end)
    {
        if (*p++ != 0) 
             return false;
    }

    tmp = *(map + (first >> BITS_SHIFT));
    for (i = first & BITS_MASK; i < BITS_PER_LONG; ++i)
    {
        if (tmp & (1 << i))
             return false;
    }
    
    first += count;
    tmp = *(map + BITS_TO_LONGS(first) - 1);
    for (i = first & BITS_MASK; i < BITS_PER_LONG; ++i)
    {
        if (tmp & (1 << i))
             return false;
    }
*/
    return true;
}
