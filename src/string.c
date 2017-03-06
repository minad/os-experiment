#include <string.h>
#include <ctype.h>
#include <types.h>
#include <bitmap.h>
#include <malloc.h>

static inline void map_init(ulong* map, const char* delim)
{
    memset(map, 0, 32);
    while (*delim)
    {
        bitmap_setbit(map, *delim);
        ++delim;
    }
}

/*
 * POSIX buffer routines
 */

void *memccpy(void *dest, const void *src, int c, size_t count)
{
    char *p = (char *)dest;
    const char *q = (const char *)src;
    while (count--)
    {
        *p++ = *q;
        if (*q == (char)c)
            return p;
        ++q;
    }
    return NULL;
}

int memcmp(const void *a, const void *b, size_t count)
{
    const char *p = (const char *)a, *q = (const char *)b;
    int res;
    while (count--)
        if (res = (*p++ - *q++))
            return res;
    return 0;
}

void *memcpy(void *dest, const void *src, size_t count)
{
    char *p = (char *)dest;
    const char *q = (const char *)src;
    while (count--)
        *p++ = *q++;
    return dest;
}

void *memset(void *buf, int c, size_t count)
{
    char *p = (char *)buf;
    while (count--)
        *p++ = c;
    return buf;
}

void *memchr(const void *buf, int c, size_t count)
{
    const char *p = (const char *)buf;
    while (count--)
    {
        if (*p == (char)c)
            return ((void*)p);
        ++p;
    }
    return NULL;
}

void *memmove(void *dest, const void *src, size_t count)
{
    if (dest < src)
        return memcpy(dest, src, count);
    char *p = (char *)dest + count - 1;
    const char *q = (const char *)src + count - 1;
    while (count--)
        *p-- = *q--;
    return dest;
}

/*
 * POSIX string routines
 */

char *strcat(char *dest, const char *src)
{
    char *p = dest;
    while (*p)
        ++p;
    while (*p++ = *src++);
    return dest;
}

char *strchr(const char *str, int c)
{
    while (*str)
    {
        if (*str == (char)c)
            return ((char *)str);
        ++str;
    }
    return NULL;
}

int strcmp(const char *a, const char *b)
{
    int res = -1;
    while (*a)
        if (res = (*a++ - *b++))
            break;
    return res;
}

char *strcpy(char *dest, const char *src)
{
    char *p = dest;
    while (*p++ = *src++);
    return dest;
}

size_t strcspn(const char *str, const char *delim)
{
    ulong map[BITS_TO_LONGS(256)];
    int len = 0;
    map_init(map, delim);
    while (*str)
    {
        if (bitmap_getbit(map, *str))
            return len;
        ++str;
        ++len;
    }
    return 0;
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (*str++)
        ++len;
    return len;
}

char *strncat(char *dest, const char *src, size_t count)
{
    char *p = dest;
    while (*p)
        ++p;
    while (count--)
        if ((*p++ = *src++) != '\0')
            return dest;
    *p = '\0';
    return dest;
}

int strncmp(const char *a, const char *b, size_t count)
{
    int res = -1;
    while (--count && *a)
        if (res = *a++ - *b++)
            break;
    return res;
}

// Warning!!! Non-standard strncpy which always adds '\0'
char *strncpy(char *dest, const char *src, size_t count)
{
    char *p = dest;
    while (--count > 1 && (*p++ = *src++));
    if (count >= 0)
        *p = '\0';
    return dest;
}

char *strpbrk(const char *str, const char *delim)
{
    ulong map[BITS_TO_LONGS(256)];
    map_init(map, delim);
    while (*str)
    {
        if (bitmap_getbit(map, *str))
            return ((char *)str);
        ++str;
    }
    return NULL;
}

char *strrchr(const char *str, int c)
{
    while (*str)
        ++str;
    do
    {
        if (*str == (char)c)
            return ((char *)str);
        --str;
    } while (*str);
    return NULL;
}

size_t strspn(const char *str, const char *delim)
{
    ulong map[BITS_TO_LONGS(256)];
    int len = 0;
    map_init(map, delim);
    while (*str)
    {
        if (!bitmap_getbit(map, *str))
            return len;
        ++str;
        ++len;
    }
    return 0;
}

char *strstr(const char *a, const char *b)
{
    // FIXME: Replace this fucking slow brute-force version with boyer-more
    while (*a)
    {
        const char *p = a, *q = b;
        while (*p && *q && *p == *q)
            ++p, ++q;
        if (*q == '\0')
            return ((char *)a);
        ++a;
    }
    return NULL;
}

char* strdup(const char* s)
{
    char* s2 = malloc(strlen(s));
    strcpy(s2, s);
    return s2;
}

/*
 * Non-POSIX string routines
 */

int stricmp(const char *a, const char *b)
{
    int res = -1;
    while (*a)
        if (res = tolower(*a++) - tolower(*b++))
            break;
    return res;
}

char *strlwr(char *str)
{
    char *p = str;
    while (*p)
    {
        *p = tolower(*p);
        ++p;
    }
    return str;
}

int strnicmp(const char *a, const char *b, size_t count)
{
    int res = -1;
    while (--count && *a)
        if (res = tolower(*a++) - tolower(*b++))
            break;
    return res;
}

char *strsep(char **str, const char *delim)
{
    ulong map[BITS_TO_LONGS(256)];
    char *token = *str;
    map_init(map, delim);
    for (;;)
    {
        if (*token == '\0')
            return NULL;
        if (!bitmap_getbit(map, *token))
            break;
        ++token;
    }
    *str = token;
    for (;;)
    {
        if (**str == '\0')
            return token;
        if (bitmap_getbit(map, **str))
            break;
        ++(*str);
    }
    *(*str)++ = '\0';
    return token;
}

char *strupr(char *str)
{
    char *p = str;
    while (*p)
    {
        *p = toupper(*p);
        ++p;
    }
    return str;
}
