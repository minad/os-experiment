#ifndef _PIT_H
#define _PIT_H

#include <types.h>

enum
{
    // PIT channels
    PIT_CHANNEL_TIMER   = 0, // System timer (generates timer interrupt)
    PIT_CHANNEL_DMA     = 1, // DMA memory refreshing
    PIT_CHANNEL_SPEAKER = 2, // Connected to the speaker

    // PIT modes
    PIT_MODE_INTTC    = 0, // mode 0, intr on terminal cnt
    PIT_MODE_ONESHOT  = 1, // mode 1, one shot
    PIT_MODE_RATEGEN  = 2, // mode 2, rate generator
    PIT_MODE_SQWAVE   = 3, // mode 3, square wave
    PIT_MODE_SWSTROBE = 4, // mode 4, s/w triggered strobe
    PIT_MODE_HWSTROBE = 5, // mode 5, h/w triggered strobe
};

void pit_set(int channel, int mode, int value);
int  pit_get(int channel);

#endif // _PIT_H

