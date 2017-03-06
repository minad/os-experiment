#ifndef _ANSICODE_H
#define _ANSICODE_H

// Foreground colors
#define FG_BLACK   "\033[30m"
#define FG_RED     "\033[31m"
#define FG_GREEN   "\033[32m"
#define FG_YELLOW  "\033[33m"
#define FG_BLUE    "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN    "\033[36m"
#define FG_WHITE   "\033[37m"

// Light foreground colors
#define FG_LTBLACK   "\033[30;1m"
#define FG_LTRED     "\033[31;1m"
#define FG_LTGREEN   "\033[32;1m"
#define FG_LTYELLOW  "\033[33;1m"
#define FG_LTBLUE    "\033[34;1m"
#define FG_LTMAGENTA "\033[35;1m"
#define FG_LTCYAN    "\033[36;1m"
#define FG_LTWHITE   "\033[37;1m"

// Background colors
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

// Reset to default attr
#define NOCOLOR "\033[0m"

// Clear screen
#define CLRSCR "\033[2J"

#endif // _ANSICODE_H
