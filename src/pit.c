#include <pit.h>
#include <debug.h>
#include <io.h>

enum
{
    PIT_CONTROL  = 0x43,    // Control port
    PIT_COUNTER  = 0x40,    // Counter base port
    PIT_BOTH     = 0x30,    // Counter 16 bits, LSB first
    PIT_FREQUENY = 1193180, // Frequency
};

void pit_set(int channel, int mode, int value)
{
    ASSERT(channel < 3);
    ASSERT(mode < 6);
    ASSERT(value > 0);
    
    uint16_t divisor = PIT_FREQUENY / value;

    // Set channel mode
    outb(PIT_CONTROL, (channel << 6) | PIT_BOTH | (mode << 1));

    // Divisor
    outb(PIT_COUNTER + channel, divisor);
    outb(PIT_COUNTER + channel, divisor >> 8);
}

int pit_get(int channel)
{
    uint16_t val;

    ASSERT(channel < 3);

    outb(PIT_CONTROL, channel << 6);
    
    val = inb(PIT_COUNTER + channel);
    val |= inb(PIT_COUNTER + channel) << 8;
    
    return val;
}
