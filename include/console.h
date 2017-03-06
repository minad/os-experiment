#ifndef _CONSOLE_H
#define _CONSOLE_H

/*
 * Console interface
 * Everything is done with ANSI-Escape sequences!
 */

void con_init() __init;
int  con_putchar(int vc, int ch);
void con_setvc(int vc);
int  con_getvc();

#endif // _CONSOLE_H
