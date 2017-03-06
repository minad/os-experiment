#ifndef _KEYBOARD_H
#define _KEYBOARD_H

void kbd_init() __init;

// Delay 0 = 250 msec ... 3 = 1000msec
// Repeat rate (0 = 2 Hz ... 31 = 30 Hz)
void kbd_setrate(int delay, int repeat);

void kbd_reboot();

#endif // _KEYBOARD_H

