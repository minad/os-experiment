#ifndef _CTYPE_H
#define _CTYPE_H

#include <types.h>

extern const uchar _ctype[];

enum
{
    _C_BLANK = 0x01,
    _C_CTRL  = 0x02,
    _C_DIGIT = 0x04,
    _C_HEX   = 0x08,
    _C_LOWER = 0x10,
    _C_PUNCT = 0x20,
    _C_SPACE = 0x40,
    _C_UPPER = 0x80,
    _C_ALPHA = _C_UPPER | _C_LOWER,
    _C_ALNUM = _C_ALPHA | _C_DIGIT,
    _C_GRAPH = _C_PUNCT | _C_ALNUM,
    _C_PRINT = _C_BLANK | _C_GRAPH,
};

static inline int isalnum(int c)  { return (_ctype[c] & _C_ALNUM); }
static inline int isalpha(int c)  { return (_ctype[c] & _C_ALPHA); }
static inline int iscntrl(int c)  { return (_ctype[c] & _C_CTRL);  }
static inline int isdigit(int c)  { return (_ctype[c] & _C_DIGIT); }
static inline int isgraph(int c)  { return (_ctype[c] & _C_GRAPH); }
static inline int islower(int c)  { return (_ctype[c] & _C_LOWER); }
static inline int isprint(int c)  { return (_ctype[c] & _C_PRINT); }
static inline int ispunct(int c)  { return (_ctype[c] & _C_PUNCT); }
static inline int isspace(int c)  { return (_ctype[c] & _C_SPACE); }
static inline int isupper(int c)  { return (_ctype[c] & _C_UPPER); }
static inline int isxdigit(int c) { return (_ctype[c] & _C_HEX);   }

static inline int isascii(int c)  { return (c <= 127); }
static inline int toascii(int c)  { return (c & 127);  }
static inline int tolower(int c)  { return (isupper(c) ? c - 'A' + 'a' : c); }
static inline int toupper(int c)  { return (islower(c) ? c - 'a' + 'A' : c); }
static inline int todigit(int c)  { return (isdigit(c) ? c - '0' : 0); } // non std

#endif // _CTYPE_H

