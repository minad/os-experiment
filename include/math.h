#ifndef _MATH_H
#define _MATH_H

static inline int max(int a, int b)
{
    return (a > b ? a : b);
}

static inline int min(int a, int b)
{
    return (a < b ? a : b);
}

static inline int clamp(int x, int a, int b)
{
    return max(min(x, b), a);
}

static inline uint32_t floor(uint32_t val, uint32_t size)
{
    return ((val / size) * size);
}

static inline uint32_t ceil(uint32_t val, uint32_t size)
{
    return floor(val + size - 1, size);
}

#endif // _MATH_H

