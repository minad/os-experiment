#ifndef _KEYMAP_H
#define _KEYMAP_H

#include <types.h>

enum
{
    MAX_DIACR      = 256,
    MAX_NR_FUNC    = 256,
    MAX_NR_KEYMAPS = 256,
    NR_KEYS        = 256,
};

typedef struct kbdiacr
{
    uchar diacr, base, result;
} kbd_diacr_t;

typedef ushort u_short;

extern ushort      plain_map[];
extern ushort*     key_maps[];
extern uint        keymap_count;
extern char*       func_table[];
extern kbd_diacr_t accent_table[];
extern uint        accent_table_size;

#endif
