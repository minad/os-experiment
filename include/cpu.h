#ifndef _CPU_H
#define _CPU_H

#include <types.h>

enum
{
    // Cpu vendor (cpuid level 0x00000000)
    CPU_VENDOR_UNKNOWN   = 0,  
    CPU_VENDOR_INTEL     = 1,
    CPU_VENDOR_CYRIX     = 2,    
    CPU_VENDOR_AMD       = 3,      
    CPU_VENDOR_UMC       = 4,    
    CPU_VENDOR_NEXGEN    = 5,    
    CPU_VENDOR_CENTAUR   = 6,   
    CPU_VENDOR_RISE      = 7,      
    CPU_VENDOR_TRANSMETA = 8,
        
    // Intel cpu features (cpuid level 0x00000001, first dword)
    CPU_FEATURE_FPU      =  0, // Floating Point Unit
    CPU_FEATURE_VME      =  1, // Virtual 8086 Mode
    CPU_FEATURE_DE       =  2, // Debugging Extensions
    CPU_FEATURE_PSE      =  3, // Page Size Extension
    CPU_FEATURE_TSC      =  4, // Time Stamp Counter
    CPU_FEATURE_MSR      =  5, // Model Specific Registers
    CPU_FEATURE_PAE      =  6, // Physical Address Extension
    CPU_FEATURE_MCE      =  7, // Machine Check Exception
    CPU_FEATURE_CX8      =  8, // CMPXCHG8B Instruction
    CPU_FEATURE_APIC     =  9, // APIC On-Chip
    CPU_FEATURE_SEP      = 11, // SYSENTER and SYSEXIT Instructions
    CPU_FEATURE_MTRR     = 12, // Memory Type Range Registers
    CPU_FEATURE_PGE      = 13, // PTE Global Bit
    CPU_FEATURE_MCA      = 14, // Machine Check Architecture
    CPU_FEATURE_CMOV     = 15, // Conditional Move Instructions
    CPU_FEATURE_PAT      = 16, // Page Attribute Table
    CPU_FEATURE_PSE36    = 17, // 36-Bit Page Size Extension
    CPU_FEATURE_PSN      = 18, // Processor Serial Number
    CPU_FEATURE_CLFSH    = 19, // CLFLUSH Instruction
    CPU_FEATURE_DS       = 21, // Debug Store
    CPU_FEATURE_ACPI     = 22, // ACPI Processor Performance Modulation Registers
    CPU_FEATURE_MMX      = 23, // MMX Technology
    CPU_FEATURE_FXSR     = 24, // FXSAVE and FXSTOR Instructions
    CPU_FEATURE_SSE      = 25, // SSE Extensions
    CPU_FEATURE_SSE2     = 26, // SSE2 Extensions
    CPU_FEATURE_SS       = 27, // Self Snoop
    CPU_FEATURE_HTT      = 28, // Hyper-Threading Technology
    CPU_FEATURE_TM       = 29, // Thermal Monitor
    CPU_FEATURE_PBE      = 31, // Pending Break Enable
    
    // AMD cpu features (cpuid level 0x80000001, second dword)
    CPU_FEATURE_SYSCALL  = 43, // SYSCALL and SYSRET Instructions
    CPU_FEATURE_MMXEXT   = 54, // AMD MMX extensions
    CPU_FEATURE_LM       = 61, // Long Mode (x86-64)
    CPU_FEATURE_3DNOWEXT = 62, // AMD 3DNow! extensions
    CPU_FEATURE_3DNOW    = 63, // 3DNow!    
    
    // Transmeta CPU features (cpuid level 0x80860001, third dword)
    CPU_FEATURE_RECOVERY = 64, // Recovery CMS active
    CPU_FEATURE_LR       = 65, // Longrun power control
    CPU_FEATURE_LRTI     = 67, // Longrun table interface
};

typedef struct cpu_info_s
{
    int      family;
    int      vendor;
    int      model;
    char     vendor_id[16];
    char     model_id[48];
    uint32_t feature[3];
} cpu_info_t;

void cpu_detect() __init;
const cpu_info_t* cpu_get_info();
void cpu_dump_info(const cpu_info_t*);

static inline bool cpu_has_feature(const cpu_info_t* cpu, int f)
{
    return (cpu->feature[f >> 5] & f & 15);
}

// Check CPU feature
#define CPU_HAS_FEATURE(f) \
cpu_has_feature(get_cpu_info(), CPU_FEATURE_##f)

#endif // _CPU_H
