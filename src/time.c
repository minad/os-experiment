#include <time.h>
#include <io.h>

// Read value and decode bcd
static inline uint8_t cmos_read(uint8_t addr)
{
    uint8_t val;
    outb_p(0x70, addr);
    val = inb_p(0x71);
    return (10 * (val >> 4) + (val & 15));
}

struct tm* gettime(struct tm* tm)
{
    static const int month[12] =
    { 
	0,
	31,
	31+28,
	31+28+31,
	31+28+31+30,
	31+28+31+30+31,
	31+28+31+30+31+30,
	31+28+31+30+31+30+31,
	31+28+31+30+31+30+31+31,
	31+28+31+30+31+30+31+31+30,
	31+28+31+30+31+30+31+31+30+31,
	31+28+31+30+31+30+31+31+30+31+30,
    };
    
    tm->tm_sec  = cmos_read(0);
    tm->tm_min  = cmos_read(2);
    tm->tm_hour = cmos_read(4);
    tm->tm_wday = cmos_read(6);
    tm->tm_mday = cmos_read(7);
    tm->tm_mon  = cmos_read(8) - 1;
    tm->tm_year = cmos_read(9);
    
    // Year 2000 and later
    if (tm->tm_year < 70)
	tm->tm_year += 100;

    // Day in the year (0 - 365)
    tm->tm_yday = month[tm->tm_mon] + tm->tm_mday - 1;
    
    // Leap years
    if (tm->tm_mon > 1 && tm->tm_year % 4 == 0 && tm->tm_year % 100 != 0)
	++tm->tm_yday;
    
    return tm;
}

time_t time(time_t* tp)
{
    struct tm tm;
    time_t t = mktime(gettime(&tm));
    if (tp)
	*tp = t;
    return t;
}

time_t mktime(const struct tm* tm)
{
    enum
    {
        MINUTE = 60,
        HOUR   = 60 * MINUTE,
        DAY    = 24 * HOUR,
        YEAR   = 365 * DAY,
    };

    return (tm->tm_sec +
            MINUTE * tm->tm_min +
	    HOUR * tm->tm_hour +
	    DAY * tm->tm_yday +
	    YEAR * (tm->tm_year - 70) +
	    DAY * ((tm->tm_year - 69) / 4) + // Leap years
	    -DAY * (tm->tm_year / 100) +     // 100 is no leap year
	    DAY * (tm->tm_year / 400));      // 400 is a leap year
}
