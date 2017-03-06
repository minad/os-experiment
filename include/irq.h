#ifndef _IRQ_H
#define _IRQ_H

enum
{
    IRQ_TIMER       =  0,
    IRQ_KEYBOARD    =  1,
    IRQ_CASCADE     =  2,
    IRQ_SERIAL2     =  3,
    IRQ_SERIAL1     =  4,
    IRQ_LPT         =  5,
    IRQ_FLOPPY      =  6,
    IRQ_RTCLOCK     =  8,
    IRQ_MOUSE       = 12,
    IRQ_COPROCESSOR = 13,
    IRQ_IDE1        = 14,
    IRQ_IDE2        = 15,
};

#endif // _IRQ_H

