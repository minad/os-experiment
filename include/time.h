#ifndef _TIME_H
#define _TIME_H

#include <types.h>

struct tm
{
    int tm_sec;   // seconds
    int tm_min;   // minutes
    int tm_hour;  // hours
    int tm_mday;  // day of the month
    int tm_mon;   // month
    int tm_year;  // year
    int tm_wday;  // day of the week
    int tm_yday;  // day in the year
};

struct tm* gettime(struct tm*);
time_t time(time_t*);
time_t mktime(const struct tm*);

#endif

