#include <stdio.h>
#include <debug.h>
#include <ctype.h>
#include <string.h>
#include <console.h>

// printf states
enum
{
    STATE_NORMAL,    
    STATE_FLAG,      
    STATE_WIDTH,
    STATE_MODIFIER,
    STATE_TYPE,
};

// printf length modifier 
enum
{
    MOD_INT,
    MOD_CHAR,
    MOD_SHORT,
    MOD_LONG,
};

// printf flags
enum
{
    // Real flags
    FLAG_LEFT      = 0x01,
    FLAG_SIGN      = 0x04,
    FLAG_SIGNBLANK = 0x08,
    FLAG_ZEROPAD   = 0x10,
    // Internal flags
    FLAG_UNSIGNED  = 0x20,
    FLAG_UPPERCASE = 0x40,
};

static char* append_number(char* p, char* end, uint value, int flags, int width, int radix)
{
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    int len = 0, sign = 0;
    char buf[32], *bufend = buf;

    ASSERT(radix >= 2 && radix <= 36);
   
    if (p >= end)
        return p;
    
    if (flags & FLAG_UNSIGNED)
    {
        do
        {
	    *bufend++ = digits[value % radix];
            ++len;
            value /= radix;
        }
        while (value);

        if (flags & FLAG_SIGN)
            sign = '+', ++len;
        else if (flags & FLAG_SIGNBLANK)
            sign = ' ', ++len;
    }
    else
    {
        if ((int)value < 0)
            sign = '-', ++len, value = -value;
        else if (flags & FLAG_SIGN)
            sign = '+', ++len;
        else if (flags & FLAG_SIGNBLANK)
            sign = ' ', ++len;
        
        do
        {
            *bufend++ = digits[value % radix];
            ++len;
            value /= radix;
        }
        while (value);
    }
 
    if ((flags & FLAG_ZEROPAD) && sign)
        *p++ = sign;
          
    if (~flags & FLAG_LEFT)
    {
        int ch = (flags & FLAG_ZEROPAD) ? '0' : ' ';
        while (len++ < width && p < end)
            *p++ = ch;
    }
    
    if ((~flags & FLAG_ZEROPAD) && sign && p < end)
        *p++ = sign;
    
    if (flags & FLAG_UPPERCASE)
    {
        while (p < end && --bufend >= buf)
            *p++ = toupper(*bufend);
    }
    else
    {
        while (p < end && --bufend >= buf)
            *p++ = *bufend;    
    }

    if (flags & FLAG_LEFT)
    {
        while (len++ < width && p < end)
            *p++ = ' ';
    }
    
    return p;
}

static char* append_string(char* p, char* end, const char* str, int flags, int width)
{
    size_t len;
    if (!str)
        str = "<NULL>";
    len = strlen(str);
    
    if (~flags & FLAG_LEFT)
        while (len++ < width && p < end)
            *p++ = ' ';
    
    while (p < end && *str)
        *p++ = *str++;
    
    if (flags & FLAG_LEFT)
        while (len++ < width && p < end)
            *p++ = ' ';   
    
    return p;
}

static char* append_char(char* p, char* end, int ch, int flags, int width)
{
    if (~flags & FLAG_LEFT)
        while (--width > 0 && p < end)
            *p++ = ' ';
    
    if (p < end)
        *p++ = ch;
    
    if (flags & FLAG_LEFT)
        while (--width > 0 && p < end)
            *p++ = ' ';
            
    return p;
}

/*
 * Simple vsnprintf implementation
 *
 * Supported types: d, i, x, X, p, c, s, o (octal int) and b (binary int)
 * Supported flags: -, +, _blank_, 0
 * Width and length qualifiers are also supported. That's everything.
 */

int vsnprintf(char *buffer, size_t size, const char *format, va_list argptr)
{
    char *p = buffer, *end = buffer + size;
    int state = STATE_NORMAL;
    int flags = 0, width = 0, mod = MOD_INT;
    char ch;
    
    while ((ch = *format++) && p < end)
    {
        switch (state)
        {
        /*
         * Normal string processing
         */
        case STATE_NORMAL:
            if (ch != '%')
            {
                *p++ = ch;
                break;
            }
            flags = width = 0;
            mod = MOD_INT;
            state = STATE_FLAG;
            break;
        
        /*
         * Process flags
         */
        case STATE_FLAG:
            switch (ch)
            {
            case '-': flags |= FLAG_LEFT; continue;
            case '+': flags |= FLAG_SIGN; continue;
            case ' ': flags |= FLAG_SIGNBLANK; continue;
            case '0': flags |= FLAG_ZEROPAD; continue;
            default: break;
            }
            state = STATE_WIDTH;
            // fall through

        /*
         * Process width field
         */
        case STATE_WIDTH:
            if (isdigit(ch))
            {
                width = width * 10 + todigit(ch);
                break;
            }
            if (ch == '*')
            {
                width = va_arg(argptr, int);
                if (width < 0)
                {
                    width = -width;
                    flags |= FLAG_LEFT;
                }
                state = STATE_TYPE;
                break;
            }
            state = STATE_MODIFIER;
            // fall through

        /*
         * Process length modifier
         */
        case STATE_MODIFIER:
            switch (ch)
            {
            case 'l':
                mod = MOD_LONG;
                continue;

            case 'h':
                mod = mod == MOD_SHORT ? MOD_CHAR : MOD_SHORT;
                continue;

            default:
                break;
            }       
            state = STATE_TYPE;
            // fall through     
     
        /*
         * Process type
         */       
        case STATE_TYPE:
        {
            int value;
            int radix;

            state = STATE_NORMAL;
            
            /*
             * All types
             */
            
            switch (ch)
            {
            case 'd':
            case 'i':
                radix = 10;
                break;
                
            case 'b':
                radix = 2;
                flags |= FLAG_UNSIGNED;
                break;

            case 'o':
                radix = 8;
                flags |= FLAG_UNSIGNED;
                break;
                
            case 'u':
                radix = 10;
                flags |= FLAG_UNSIGNED;
                break;
                
            case 'p':
                flags |= FLAG_ZEROPAD;
                mod = MOD_INT;
                // fall through
            
            case 'X':
                flags |= FLAG_UPPERCASE;
                
            case 'x':
                radix = 16;
                flags |= FLAG_UNSIGNED;
                break;
                
            case 's':
                p = append_string(p, end, va_arg(argptr, const char*), flags, width); 
                continue;
               
            case 'c':
                p = append_char(p, end, va_arg(argptr, int), flags, width);
                continue;
            
            case '%':
                *p++ = '%';
                continue;
            
            default:
                continue;
            }
            
            /*
             * Numeric type
             */
             
            switch (mod)
            {
            case MOD_CHAR:
                value = (char)va_arg(argptr, int);
                break;
                
            case MOD_SHORT:
                value = (short)va_arg(argptr, int);
                break;
                
            default:
                value = va_arg(argptr, int);
                break;
            }
            
            p = append_number(p, end, value, flags, width, radix);
            break;
        }

        default:
            break;
        }
    }
    
    if (p < end)
        *p = 0;
        
    return (p - buffer);
}

int snprintf(char *buffer, size_t size, const char *format, ...)
{
    va_list argptr;
    int count;
    va_start(argptr, format);
    count = vsnprintf(buffer, size, format, argptr);
    va_end(argptr);
    return count;
}

int vprintf(const char* format, va_list argptr)
{
    return con_vprintf(con_getvc(), format, argptr);
}

int printf(const char* format, ...)
{
    int count;
    va_list argptr;
    va_start(argptr, format);
    count = vprintf(format, argptr);
    va_end(argptr);
    return count;
}

int puts(const char* str)
{
    return con_puts(con_getvc(), str);
}

int putchar(int ch)
{
    return con_putchar(con_getvc(), ch);
}

int con_vprintf(int vc, const char* format, va_list argptr)
{
    char buffer[1024], *p;
    int n = vsnprintf(buffer, sizeof(buffer), format, argptr);
    buffer[n >= sizeof (buffer) ? sizeof (buffer) - 1 : n] = 0;
    for (p = buffer; p < buffer + n; ++p)
        con_putchar(vc, *p);
    return n;
}

int con_printf(int vc, const char* format, ...)
{
    int count;
    va_list argptr;
    va_start(argptr, format);
    count = con_vprintf(vc, format, argptr);
    va_end(argptr);
    return count;
}

int con_puts(int vc, const char* str)
{
    const char*p = str;
    while (*p) 
        con_putchar(vc, *p++);
    con_putchar(vc, '\n');
    return (p - str);
}
