#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef NULL
#  define NULL 0
#endif

#define offsetof(type, member) (uint)&(((type *)0)->member)

#endif // _STDDEF_H

