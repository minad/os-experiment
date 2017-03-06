/*
 * CPU Detection from Linux
 */
#include <cpu.h>
#include <string.h>
#include <stdio.h>

static cpu_info_t cpu;

// Get CPUID
static inline void cpuid(uint32_t op, uint32_t* a, uint32_t* b,
                         uint32_t* c, uint32_t* d)
{
     __asm__ __volatile__ ("cpuid" : "=a" (*a), "=b" (*b),
                           "=c" (*c), "=d" (*d) : "a" (op));
}

// Standard macro to see if a specific flag is changeable
static bool __init flag_is_changeable(uint32_t flag)
{
    uint32_t f1, f2;
    __asm__ __volatile__ (
        "pushfl\n"
        "pushfl\n"
        "popl %0\n"
        "movl %0,%1\n"
        "xorl %2,%0\n"
        "pushl %0\n"
        "popfl\n"
        "pushfl\n"
        "popl %0\n"
        "popfl\n" : "=&r" (f1), "=&r" (f2) : "ir" (flag));
    return ((f1 ^ f2) & flag);
}

// Detect a NexGen CPU running without BIOS hypercode new enough
// to have CPUID. (Thanks to Herbert Oppmann)
static bool __init deep_magic_nexgen_probe()
{
    bool ret;
    __asm__ __volatile__ (
        "movw   $0x5555, %%ax\n"
        "xorw   %%dx,%%dx\n"
        "movw   $2, %%cx\n"
        "divw   %%cx\n"
        "movl   $0, %%eax\n"
        "jnz    1f\n"
        "movl   $1, %%eax\n"
        "1:\n"
        : "=a" (ret) : : "cx", "dx" );
    return ret;
}

// Perform the Cyrix 5/2 test. A Cyrix won't change
// the flags, while other 486 chips will.
static bool __init test_cyrix_52div()
{
    uint32_t test;
    __asm__ __volatile__(
        "sahf\n\t"    // clear flags (%eax = 0x0005)
        "div %b2\n\t" // divide 5 by 2
        "lahf"        // store flags into %ah
        : "=a" (test)
        : "0" (5), "q" (2)
        : "cc");
    // AH is 0x02 on Cyrix after the divide..
    return ((test >> 8) == 0x02);
}

static int __init get_vendor(const char* v)
{
    if (!strcmp(v, "GenuineIntel"))
        return CPU_VENDOR_INTEL;
    if (!strcmp(v, "AuthenticAMD"))
        return CPU_VENDOR_AMD; 
    if (!strcmp(v, "CyrixInstead"))
        return CPU_VENDOR_CYRIX;
    if (!strcmp(v, "UMC UMC UMC "))
        return CPU_VENDOR_UMC;
    if (!strcmp(v, "CentaurHauls"))
        return CPU_VENDOR_CENTAUR;
    if (!strcmp(v, "NexGenDriven"))
        return CPU_VENDOR_NEXGEN;
    if (!strcmp(v, "RiseRiseRise"))
        return CPU_VENDOR_RISE;
    if (!strcmp(v, "GenuineTMx86") || !strcmp(v, "TransmetaCPU"))
        return CPU_VENDOR_TRANSMETA;
    return CPU_VENDOR_UNKNOWN;
}

static void __init get_model_id()
{
    char *p, *q;
    uint32_t* m;

    // Read model name 
    m = (uint32_t*)cpu.model_id;
    cpuid(0x80000002, &m[0], &m[1], &m[2],  &m[3]);
    cpuid(0x80000003, &m[4], &m[5], &m[6],  &m[7]);
    cpuid(0x80000004, &m[8], &m[9], &m[10], &m[11]);
    cpu.model_id[48] = 0;

    // Intel chips right-justify this string for some dumb reason;
    // undo that brain damage
    p = q = cpu.model_id;    
    while (*p++ == ' ');
    if (p != q)
    {
        while (*p) *q++ = *p++;
        while (q <= &cpu.model_id[48]) *q++ = 0;    
    }
}

typedef struct model_info_s
{
    int   vendor;
    int   family;
    char* names[16];
} model_info_t;

static model_info_t cpu_models[] __initdata = {
    {
        CPU_VENDOR_INTEL, 4,
        { "486 DX-25/33", "486 DX-50", "486 SX", "486 DX/2", "486 SL", 
          "486 SX/2", NULL, "486 DX/2-WB", "486 DX/4", "486 DX/4-WB", NULL,
          NULL, NULL, NULL, NULL, NULL }
    },
    { 
        CPU_VENDOR_INTEL, 5,
        { "Pentium 60/66 A-step", "Pentium 60/66", "Pentium 75 - 200",
          "OverDrive PODP5V83", "Pentium MMX", NULL, NULL,
          "Mobile Pentium 75 - 200", "Mobile Pentium MMX", NULL, NULL, NULL, 
          NULL, NULL, NULL, NULL }
    },
    {
        CPU_VENDOR_INTEL, 6,
        { "Pentium Pro A-step", "Pentium Pro", NULL, "Pentium II (Klamath)",
          NULL, "Pentium II (Deschutes)", "Mobile Pentium II",
          "Pentium III (Katmai)", "Pentium III (Coppermine)", NULL,
          "Pentium III (Cascades)", NULL, NULL, NULL, NULL }
    },
    {
        CPU_VENDOR_AMD, 4,
        { NULL, NULL, NULL, "486 DX/2", NULL, NULL, NULL, "486 DX/2-WB",
          "486 DX/4", "486 DX/4-WB", NULL, NULL, NULL, NULL, "Am5x86-WT",
          "Am5x86-WB" }
    },
    {
        CPU_VENDOR_AMD, 5, // Is this this really necessary??
        { "K5/SSA5", "K5", "K5", "K5", NULL, NULL, "K6", "K6", "K6-2", "K6-3",
          NULL, NULL, NULL, NULL, NULL, NULL }
    },
    { 
        CPU_VENDOR_AMD, 6, // Is this this really necessary??
        { "Athlon", "Athlon", "Athlon", NULL, "Athlon", NULL,
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        CPU_VENDOR_UMC, 4,
        { NULL, "U5D", "U5S", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
          NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        CPU_VENDOR_NEXGEN, 5,
        { "Nx586", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
          NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
    {
        CPU_VENDOR_RISE, 5,
        { "mP6", "mP6", NULL, NULL, NULL, NULL, NULL,
          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
    },
};

// Look up CPU names by table lookup.
static const char* __init lookup_model_id()
{
     const model_info_t* m = cpu_models;
     int i;

     if (cpu.model >= 16)
        return NULL;
     
     for (i = 0; i < sizeof(cpu_models) /sizeof(model_info_t); ++i)
     {
         if (m->vendor == cpu.vendor && m->family == cpu.family)
             return m->names[cpu.model];
         ++m;
     }
     return NULL;
}

void __init cpu_detect()
{
    memset(&cpu, 0, sizeof (cpu));
    
    // First of all, decide if this is a 486 or higher
    // It's a 486 if we can modify the AC flag
    if (flag_is_changeable(0x40000))
        cpu.family = 4;
    else
        cpu.family = 3;

    // Detect Cyrix with disabled CPUID */
    if (cpu.family == 4 && test_cyrix_52div())
    {
        strcpy(cpu.vendor_id, "CyrixInstead");
        cpu.vendor = CPU_VENDOR_CYRIX;
    } 
    // Detect NexGen with old hypercode
    else if (deep_magic_nexgen_probe())
        strcpy(cpu.vendor_id, "NexGenDriven");
    
    // Has CPUID?
    if (flag_is_changeable(0x200000))
    {
        uint32_t level, junk;
       
        // Get vendor id
        cpuid(0x00000000, &level,
              (uint32_t*)&cpu.vendor_id[0],
              (uint32_t*)&cpu.vendor_id[8],
              (uint32_t*)&cpu.vendor_id[4]);
        cpu.vendor = get_vendor(cpu.vendor_id);
        
        // Standard level
        if (level >= 0x00000001)
        {
            uint32_t eax;
            cpuid(0x00000001, &eax, &junk, &junk, &cpu.feature[0]);
            cpu.family = (eax >> 8) & 15;
            cpu.model  = (eax >> 4) & 15;
        }
        
        // Extended level
        cpuid(0x80000000, &level, &junk, &junk, &junk);
        if ((level & 0xFFFF0000) == 0x80000000)
        {
            if (level >= 0x80000001)
                cpuid(0x80000001, &junk, &junk, &junk, &cpu.feature[1]);
            
            if (level >= 0x80000004)
                get_model_id();
        }
                
        // Transmeta level
        cpuid(0x80860000, &level, &junk, &junk, &junk);
        if ((level & 0xFFFF0000) == 0x80860000)
        {
            if (level >= 0x80860001)
                cpuid(0x80000001, &junk, &junk, &junk, &cpu.feature[2]);
        }
    }
    
    // If the model name is still unset, do table lookup.
    if (!cpu.model_id[0])
    {
        const char* p = lookup_model_id(cpu);
        if (p)
            strcpy(cpu.model_id, p);
        else
            snprintf(cpu.model_id, sizeof (cpu.model_id), "%02x/%02x", cpu.vendor, cpu.model);
    }
}

static const char* feature_str[] =
{
    "FPU", "VME", "DE",       "PSE",     "TSC",      "MSR",  "PAE",  "MCE",   "CX8", "APIC", 
    NULL,  "SEP", "MTRR",     "PGE",     "MCA",      "CMOV", "PAT",  "PSE36", "PSN", "CLFSH", 
    NULL,  "DS",  "ACPI",     "MMX",     "FXSR",     "SSE",  "SSE2", "SS",    "HTT", "TM",  
    NULL,  "PBE", NULL,       NULL,      NULL,       NULL,   NULL,   NULL,    NULL,  NULL,
    NULL,  NULL,  NULL,       "SYSCALL", NULL,       NULL,   NULL,   NULL,    NULL,  NULL,
    NULL,  NULL,  NULL,       NULL,      "MMXEXT",   NULL,   NULL,   NULL,    NULL,  NULL,
    NULL,  "LM",  "3DNOWEXT", "3DNOW",   "RECOVERY", "LR",   NULL,   "LRTI",
};

const cpu_info_t* cpu_get_info()
{
    return &cpu;
}

void cpu_dump_info(const cpu_info_t* cpu)
{
    int i;

    printf("CPU Detection:\n"
           " Family:   %d86\n"
           " Vendor:   %s\n"
           " Model:    %s\n"
           " Features: ", cpu->family, cpu->vendor_id, cpu->model_id);
           
    for (i = 0; i < sizeof (feature_str) / sizeof (feature_str[0]); ++i)
    {
        if (cpu_has_feature(cpu, i) && feature_str[i])
            printf("%s ", feature_str[i]);
    }
    
    putchar('\n');
}

