#include <console.h>
#include <types.h>
#include <ctype.h>
#include <string.h>
#include <vmem.h>
#include <io.h>
#include <math.h>
#include <page.h>
#include <thread.h>

#define VGA_ADDR (VGA_PHYS + 0xC0000000)

enum
{
    VGA_PHYS      = 0xB8000,
    VGA_SIZE      = 0x8000,
    VGA_CRTC      = 0x3D4,
    SCREEN_WIDTH  = 80,
    SCREEN_HEIGHT = 25,
    SCREEN_SIZE   = SCREEN_WIDTH * SCREEN_HEIGHT,
    NUM_CONSOLES  = 8,

    // Options
    MAX_PARAMS     = 4,
    MAX_SAVED      = 4,
    MAX_CONSOLES   = 12,

    // Ansi sequence "states"
    STATE_NORMAL = 0,
    STATE_ESCAPE = 1,
    STATE_CSI    = 2,

    // Colors
    COLOR_BLACK   = 0,
    COLOR_BLUE    = 1,
    COLOR_GREEN   = 2,
    COLOR_CYAN    = 3,
    COLOR_RED     = 4,
    COLOR_MAGENTA = 5,
    COLOR_YELLOW  = 6,
    COLOR_WHITE   = 7,

    // Default attribute
    DEFAULT_ATTR  = COLOR_BLACK << 4 | COLOR_WHITE,
};

// Cursor state structure
typedef struct cursor_s
{
    int x, y;
    int attr;
} cursor_t;

// Virtual console structure
typedef struct con_s
{
    // Start address in the frame buffer
    char*    addr;

    // Cursors
    cursor_t cur;
    cursor_t saved[MAX_SAVED];
    int      num_saved;

    // Parsing state
    int      state;
    int      last_param;
    int      param[MAX_PARAMS];
} con_t;

static con_t console[8] =
{
    {
        .addr  = (char*)VGA_ADDR,
        .cur   = { 0, 0, DEFAULT_ATTR },
        .state = STATE_NORMAL,
    },
};

static int num_consoles = 1;
static int curr_vc = 0;

static void normal(con_t* c, int ch);
static void escape(con_t* c, int ch);
static void csi(con_t* c, int ch);
static void csi_J(con_t* c);
static void csi_K(con_t* c);
static void csi_m(con_t* c);

static void set_cursor(con_t* c);
static void set_origin(con_t* c);
static void save_cursor(con_t* c);
static void restore_cursor(con_t* c);
static void line_feed(con_t* c);
static void erase(con_t* c, int, int);
static void insert(con_t* c, int, int);
static void delete(con_t* c, int, int);
static void scroll_up(con_t* c);
static void scroll_down(con_t* c);

/*
 * Very nice bochs e9 hack :-)
 *
 * Everything written to port e9 will
 * be shown on the bochs console!!!
 */
#ifdef E9_HACK
#  define e9_putchar(ch) (ch != '\033' ? outb(0xE9, ch) : (void)0)
#else
#  define e9_putchar(ch)
#endif

void __init con_init()
{
    int i;

    // Map complete vga memory
    vmem_map(VGA_ADDR, VGA_ADDR + VGA_SIZE, VGA_PHYS, PAGE_RW);

    // Initialize consoles
    num_consoles = NUM_CONSOLES;
    for (i = 1; i < num_consoles; ++i)
    {
        console[i].addr     = (char*)VGA_ADDR + i * 2 * SCREEN_SIZE;
        console[i].cur.attr = min(i, COLOR_WHITE);
        console[i].state    = STATE_NORMAL;
    }
}

// This function should interpret most ansi-escape-sequences
int con_putchar(int vc, int ch)
{
    con_t* c = console + clamp(vc, 0, num_consoles - 1);
    int oldx = c->cur.x, oldy = c->cur.y;
    bool irq_status;

    switch (c->state)
    {
    case STATE_NORMAL:
        e9_putchar(ch);
        normal(c, ch);
        break;

    case STATE_ESCAPE:
        irqs_save(&irq_status);
	escape(c, ch);
        irqs_restore(irq_status);
	break;

    case STATE_CSI:
	irqs_save(&irq_status);
        csi(c, ch);
        irqs_restore(irq_status);
	break;

    default: break;
    }

    if (vc == curr_vc && (oldx != c->cur.x || oldy != c->cur.y))
    {
        irqs_save(&irq_status);
        set_cursor(c);
	irqs_restore(irq_status);
    }

    return ch;
}

// Set virtual console
void con_setvc(int vc)
{
    curr_vc = clamp(vc, 0, num_consoles - 1);
    bool irq_status;
    irqs_save(&irq_status);
    set_origin(console + curr_vc);
    irqs_restore(irq_status);
}

// Get virtual console
int con_getvc()
{
    return curr_vc;
}

// Normal state
static void normal(con_t* c, int ch)
{
    char* p;
    bool irq_status;

    switch (ch)
    {
    // FIXME: Depends on erase character of tty
    case '\b':
    case 127:
	if (c->cur.x > 0)
	{
            int end = c->cur.y * SCREEN_WIDTH + c->cur.x;
	    --c->cur.x;
	    irqs_save(&irq_status);
	    erase(c, end - 1, end);
	    irqs_restore(irq_status);
	}
	break;

    case '\t':
        c->cur.x += min(8 - (c->cur.x % 8), SCREEN_WIDTH - 1);
        break;

    case '\n':
	irqs_save(&irq_status);
	line_feed(c);
	irqs_restore(irq_status);
        break;

    case '\r':
        c->cur.x = 0;
        break;

    case '\033':
        c->state = STATE_ESCAPE;
        break;

    default:
	irqs_save(&irq_status);
	p = c->addr + ((c->cur.y * SCREEN_WIDTH + c->cur.x) << 1);
        *p = ch;
        *(p + 1) = c->cur.attr;
	++c->cur.x;
        // Auto wrap
        if (c->cur.x == SCREEN_WIDTH)
            line_feed(c);
	irqs_restore(irq_status);
        break;
    }
}

/*
 * Escape state
 */

static void escape(con_t* c, int ch)
{
    switch (ch)
    {
    // ED - Cursor down one line
    case 'D':
	++c->cur.y;
        if (c->cur.y == SCREEN_HEIGHT)
            scroll_up(c);
        break;

    // EM - Cursor up one line
    case 'M':
        --c->cur.y;
        if (c->cur.y == -1)
            scroll_down(c);
        break;

    // EM - Line feed
    case 'N':
        line_feed(c);
        break;

    // E7 - Save cursor
    case '7':
        save_cursor(c);
        break;

    // E8 - Restore cursor
    case '8':
        restore_cursor(c);
        break;

    // Control sequence introducer
    case '[':
        c->state = STATE_CSI;
        memset(c->param, 0, sizeof (c->param));
	c->last_param = 0;
	return;

    default:
        break;
    }

    c->state = STATE_NORMAL;
}

/*
 * CSI state
 * Most of VT102 should be implemented
 */

static void csi(con_t* c, int ch)
{
    switch (ch)
    {
    // E[nF - Move cursor to start of line, n lines up
    case 'F':
        c->cur.x = 0;
        // fall through

    // E[nA - Move cursor up n lines
    case 'A':
        if (!c->param[0]) ++c->param[0];
	c->cur.y = max(c->cur.y - c->param[0], 0);
        break;

    // E[nE - move cursor to start of line, n lines down
    case 'E':
        c->cur.x = 0;
        // fall through

    // E[nB, E[ne - Move cursor down n lines
    case 'B':
    case 'e':
        if (!c->param[0]) ++c->param[0];
        c->cur.y = min(c->cur.y + c->param[0], SCREEN_HEIGHT - 1);
        break;

    // E[nC, E[na - Move cursor right n chars
    case 'C':
    case 'a':
        if (!c->param[0]) ++c->param[0];
        c->cur.x = min(c->cur.x + c->param[0], SCREEN_WIDTH - 1);
        break;

    // E[nD - Move cursor left n chars
    case 'D':
        if (!c->param[0]) ++c->param[0];
        c->cur.x = max(c->cur.x - c->param[0], 0);
        break;

    // E[nd - Move cursor to line n
    case 'd':
        c->cur.y = clamp(c->param[0] - 1, 0, SCREEN_HEIGHT - 1);
        break;

    // E[n`, E[nG - Move cursor to char position n
    case '`':
    case 'G':
        if (c->param[0]) --c->param[0];
        c->cur.x = clamp(c->param[0] - 1, 0, SCREEN_WIDTH - 1);
        break;

    // E[x;yH, E[x;yf - Move cursor to position y;x
    case 'H':
    case 'f':
        c->cur.y = clamp(c->param[0] - 1, 0, SCREEN_HEIGHT - 1);
        c->cur.x = clamp(c->param[1] - 1, 0, SCREEN_WIDTH - 1);
        break;

    // E[nL - Insert n blank lines
    case 'L':
        insert(c, (c->cur.y + 1) * SCREEN_WIDTH,
               (c->cur.y + c->param[0]) * SCREEN_WIDTH);
        break;

    // E[n@ - Insert n blank chars
    case '@':
        insert(c, c->cur.y * SCREEN_WIDTH + c->cur.x,
               c->cur.y * SCREEN_WIDTH + c->cur.x + c->param[0]);
        break;

    // E[nM - Delete n lines
    case 'M':
        delete(c, (c->cur.y + 1) * SCREEN_WIDTH,
               (c->cur.y + c->param[0]) * SCREEN_WIDTH);
        break;

    // E[n@ - Delete n characters
    case 'P':
        delete(c, c->cur.y * SCREEN_WIDTH + c->cur.x,
               c->cur.y * SCREEN_WIDTH + c->cur.x + c->param[0]);
        break;

    // E[nJ - Erase parts of display
    case 'J':
        csi_J(c);
        break;

    // E[nX - Erase n characters
    case 'X':
        erase(c, c->cur.y * SCREEN_WIDTH + c->cur.x,
              c->cur.y * SCREEN_WIDTH + c->cur.x + c->param[0]);
        break;

    // E[nK - Erase part or all of line
    case 'K':
        csi_K(c);
        break;

    // E[n;n;...m - Set character attributes
    case 'm':
        csi_m(c);
        break;

    // E[s - Push cursor position
    case 's':
        save_cursor(c);
        break;

    // E[u - Pop cursor position
    case 'u':
        restore_cursor(c);
        break;

    // Parameter separator
    case ';':
        if (c->last_param < MAX_PARAMS - 1)
            ++c->last_param;
        return;

    // Parameter
    default:
        if (isdigit(ch))
        {
            c->param[c->last_param] *= 10;
	    c->param[c->last_param] += todigit(ch);
	    return;
        }
        break;
    }

    c->state = STATE_NORMAL;
}

// E[nJ - Erase parts of display
static void csi_J(con_t* c)
{
    switch (c->param[0])
    {
    // Cursor to end of display
    case 0:
        erase(c, c->cur.y * SCREEN_WIDTH + c->cur.x, SCREEN_SIZE);
        break;

    // Top of display to cursor
    case 1:
        erase(c, 0, c->cur.y * SCREEN_WIDTH + c->cur.x);
        break;

    // Entire display
    case 2:
        c->cur.x = c->cur.y = 0;
	erase(c, 0, SCREEN_SIZE);
	break;

    default:
        break;
    }
}

// E[nK - Erase part or all of line
static void csi_K(con_t* c)
{
    switch (c->param[0])
    {
    // From cursor to end of line
    case 0:
        erase(c, c->cur.y * SCREEN_WIDTH + c->cur.x,
              c->cur.y * (SCREEN_WIDTH + 1) + c->cur.x);
        break;

    // From beginning of line to cursor
    case 1:
        erase(c, c->cur.y * SCREEN_WIDTH,
              c->cur.y * SCREEN_WIDTH + c->cur.x);
        break;

    // Entire line
    case 2:
        erase(c, c->cur.y * SCREEN_WIDTH,
              c->cur.y * (SCREEN_WIDTH + 1) + c->cur.x);
        break;

    default:
        break;
    }
}

// E[n;n;...m - Set character attributes
static void csi_m(con_t* c)
{
    int fg, bg, val;

    // Color translation
    static const int color[] =
    {
        COLOR_BLACK,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE,
    };

    while (c->last_param >= 0)
    {
        val = c->param[c->last_param--];
	switch (val)
        {
        // Default attrs
        case 0:
            c->cur.attr = DEFAULT_ATTR;
            break;

        // Bold (Intensity)
        case 1:
            c->cur.attr |= 0x08;
            break;

        // Blink
        case 5:
            c->cur.attr |= 0x80;
            break;

        // Reverse bg and fg
        case 7:
            fg = c->cur.attr & 7;
            bg = (c->cur.attr >> 4) & 7;
            c->cur.attr &= 0x88;
            c->cur.attr |= fg << 4 | bg;
            break;

        // Color
        default:
	    // Foreground
            if (val >= 30 && val <= 37)
            {
                c->cur.attr &= 0x78;
                c->cur.attr |= color[val - 30];
            }

            // Background
            else if (val >= 40 && val <= 47)
            {
                c->cur.attr &= 0x87;
                c->cur.attr |= color[val - 40] << 4;
            }
        }
    }
}

static void set_cursor(con_t* c)
{
    int offset = c->cur.y * SCREEN_WIDTH + c->cur.x +
                 ((int)(c->addr - VGA_ADDR) >> 1);
    outb(VGA_CRTC, 14);
    outb(VGA_CRTC + 1, offset >> 8);
    outb(VGA_CRTC, 15);
    outb(VGA_CRTC + 1, offset);
}

static void set_origin(con_t* c)
{
    int offset = (int)(c->addr - VGA_ADDR) >> 1;
    outb(VGA_CRTC, 12);
    outb(VGA_CRTC + 1, offset >> 8);
    outb(VGA_CRTC, 13);
    outb(VGA_CRTC + 1, offset);
    set_cursor(c);
}

static void save_cursor(con_t* c)
{
    if (c->num_saved < MAX_SAVED - 1)
        c->saved[c->num_saved++] = c->cur;
}

static void restore_cursor(con_t* c)
{
    if (c->num_saved > 0)
        c->cur = c->saved[--c->num_saved];
}

static void line_feed(con_t* c)
{
    ++c->cur.y;
    c->cur.x = 0;
    if (c->cur.y == SCREEN_HEIGHT)
        scroll_up(c);
}

static void erase(con_t* c, int start, int end)
{
    char* p = c->addr + (start << 1);
    char* q = c->addr + (end << 1);
    while (p < q)
    {
        *p++ = ' ';
        *p++ = c->cur.attr;
    }
}

static void insert(con_t* c, int start, int end)
{
    memmove(c->addr + (end << 1), c->addr + (start << 1),
            (end - start) << 1);
    erase(c, start, end);
}

static void delete(con_t* c, int start, int end)
{
    int size = SCREEN_SIZE - end;
    memmove(c->addr + (start << 1), c->addr + (end << 1),
            size << 1);
    size = end - start;
    erase(c, SCREEN_SIZE - size, SCREEN_SIZE);
}

static void scroll_up(con_t* c)
{
    delete(c, 0, SCREEN_WIDTH);
    --c->cur.y;
}

static void scroll_down(con_t* c)
{
    insert(c, 0, SCREEN_WIDTH);
    ++c->cur.y;
}
