#ifndef _TYPES_H
#define _TYPES_H

#ifndef NULL
#  define NULL 0
#endif

#include <stdbool.h>

// Shortcuts
typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef long long          llong;
typedef unsigned long long ullong;

// Size types
typedef ullong uint64_t;
typedef uint   uint32_t;
typedef ushort uint16_t;
typedef uchar  uint8_t;
typedef int    int32_t;
typedef short  int16_t;
typedef char   int8_t;

// Standard types
typedef uint  size_t;
typedef ulong time_t;

// Useful for generic function pointers
typedef void (*func_t)();

// Callback function
typedef void (*callback_t)(void*);

#endif // _TYPES_H

