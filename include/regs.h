#ifndef _REGS_H
#define _REGS_H

#include <types.h>

/* 
 * The layout of this structure must match the order of registers
 * pushed and popped by the interrupt handlers (see interrupt_stub).
 */
typedef struct regs_s {
    // Pushed by "interrupt_stub"
    uint16_t  gs, _pad0;
    uint16_t  fs, _pad1;
    uint16_t  es, _pad2;
    uint16_t  ds, _pad3;    
    uint32_t  edi;
    uint32_t  esi;
    uint32_t  ebp;
    uint32_t  edx;
    uint32_t  ecx;
    uint32_t  ebx;
    uint32_t  eax;
    uint32_t  int_nr;
    // Pushed by exception or stub
    uint32_t  error_code;
    // Pushed by exception
    // user_esp and user_ss are only available if a privilege change occurs
    uint32_t  eip;
    uint16_t  cs, _pad4;
    uint32_t  eflags;
    uint32_t  user_esp;
    uint32_t  user_ss;
} regs_t __packed;

#endif // _REGS_H
