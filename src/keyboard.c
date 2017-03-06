#include <keyboard.h>
#include <pic.h>
#include <irq.h>
#include <idt.h>
#include <desc.h>
#include <asm.h>
#include <regs.h>
#include <keymap.h>
#include <stdio.h>

enum
{
    MAX_WAIT_TRIES  = 32,
    /*
    // Keys codes
    KEY_CAPS_LOCK   = 0x3A,
    KEY_NUM_LOCK    = 0x45,
    KEY_SCROLL_LOCK = 0x46,
    KEY_LSHIFT      = 0x2A,
    KEY_RSHIFT      = 0x36,
    KEY_CTRL        = 0x1D,
    KEY_ALT         = 0x38,

    // Key states
    SCROLL_LOCK = 0x001,
    NUM_LOCK    = 0x002,
    CAPS_LOCK   = 0x004,
    LEFT_SHIFT  = 0x008,
    RIGHT_SHIFT = 0x010,
    LEFT_CTRL   = 0x020,
    RIGHT_CTRL  = 0x040,
    LEFT_ALT    = 0x080,
    RIGHT_ALT   = 0x100,
    SHIFT       = LEFT_SHIFT | RIGHT_SHIFT,
    CTRL        = LEFT_CTRL | RIGHT_CTRL,
    */
    
    // Led mask of key state
    //LED_MASK    = 0x5,
};

//static int key_status = NUM_LOCK;

static int shift_state = 0;

void irq_keyboard();
static void kbd_setleds();
static void kbd_wait();
static void kbd_outb(ushort, uchar);
static void kbd_flush();

static void key_self(int, bool);
static void key_self(int, bool);
static void key_fn(int, bool);
static void key_spec(int, bool);
static void key_pad(int, bool);
static void key_dead(int, bool);
static void key_cons(int, bool);
static void key_cur(int, bool);
static void key_shift(int, bool);
static void key_meta(int, bool);
static void key_ascii(int, bool);
static void key_lock(int, bool);
static void key_lowercase(int, bool);
static void key_ignore(int key, bool released) {}
    
typedef void (*key_t)(int, bool);

static key_t key_handler[16] =
{
    key_self,
    key_fn,
    key_spec,
    key_pad,
    key_dead,
    key_cons,
    key_cur,
    key_shift,
    key_meta,
    key_ascii,
    key_lock,
    key_lowercase,
    key_ignore,
    key_ignore,
    key_ignore,
    key_ignore,
};

void __init kbd_init()
{
    idt_set(PIC_INTBASE + IRQ_KEYBOARD, irq_keyboard, DESC_TYPE_INT | DESC_PRESENT);
    pic_irq_enable(IRQ_KEYBOARD);

    kbd_outb(0x60, 0xFF);
    kbd_outb(0x60, 0xF4);
    kbd_outb(0x64, 0xAE); // Enables keyboard

    kbd_setrate(1, 20);

    kbd_flush();
}

void kbd_setrate(int delay, int rate)
{
    kbd_outb(0x60, 0xF3);
    kbd_outb(0x60, ((delay & 3) << 5) | (~rate & 31));
}

void kbd_reboot()
{
    int i;
    irqs_disable();
    kbd_flush();
    for (i = 0; i < 100; ++i)
        outb(0x64, 0xFE);
    force_reboot();
}

// Keyboard handler
void do_irq_keyboard(const regs_t regs)
{
    pic_irq_end(IRQ_KEYBOARD);
    while (inb(0x64) & 1)
    {
        ushort* map;
        int key, sym;
        
        key = inb(0x60);
        
        // TODO: "Gray" key
        if (key == 0xE0 || key == 0xE1)
            continue;
  
        // Index in the key map
        /*
        if ((key_status & CAPS_LOCK) && !(key_status & SHIFT) || (key_status & SHIFT))
            i |= 1;
        if (key_status & LEFT_ALT)
            i |= 2;
        if (key_status & CTRL)
            i |= 4;        
        if (key_status & RIGHT_ALT)
            i |= 4;
        */
             
        if (shift_state >= keymap_count)
            continue;
        map = key_maps[shift_state];
        if (!map)
            continue;
            
        sym = map[key & 0x7F] & 0xFFF;

	con_printf(6, "KEY: handler=%d key=%d (%c)\n", sym >> 8, sym & 0xFF, sym & 0xFF);
	
        if (key_handler[sym >> 8])
            (key_handler[sym >> 8])(sym & 0xFF, key & 0x80);
    }
}

static void kbd_outb(ushort port, uchar b)
{
    kbd_wait();
    outb(port, b);
}

static void kbd_wait()
{
    int i;
    for (i = 0; i < MAX_WAIT_TRIES && (inb(0x64) & 2); ++i);
}

static void kbd_flush()
{
    int status, i = 0;
    do
    {
        status = inb(0x64);
        if (status & 1)
        {
            inb(0x60);
            continue;
        }
    } while (++i < MAX_WAIT_TRIES && (status & 2));
}

static void kbd_setleds()
{
    /*
    kbd_wait();
    // Command "Set leds"
    outb(0x60, 0xED);
    kbd_wait();
    outb(0x64, key_status & LED_MASK);
    */
}

static void key_self(int key, bool released)
{
    if (released)
        return;
    putchar(key);
}

static void key_fn(int key, bool released)
{ 
    const char* p = func_table[key];
    while (*p)
	putchar(*p++);
}

static void key_spec(int key, bool released)
{
    if (released)
	return;

    switch (key)
    {
    // Enter
    case 1:
        putchar('\n');
	break;
    }
}

static void key_pad(int key, bool released)
{
    static const char ch[] = { 0, 0, 'B', 0, 'D', 0, 'C', 0, 'A', 0 };
    if (released)
	return;
    putchar('\033');
    putchar('[');
    putchar(ch[key]);
}

static void key_dead(int key, bool released)
{
    printf("key_dead: key=%d, released=%d\n", key, released);
}

static void key_cons(int key, bool released)
{
    con_setvc(key);
}

static void key_cur(int key, bool released)
{
    printf("%d\n", key);
    putchar('\033');
    putchar('[');
    putchar(key + 'A' - 1);
}

static void key_shift(int key, bool released)
{
    if (released)
        shift_state &= ~(1 << key);
    else
	shift_state |= (1 << key);
    /*
    if (released)
    {
        switch (key)
        {
        case KEY_LSHIFT:
            key_status &= ~LEFT_SHIFT;
            break;
           
        case KEY_RSHIFT:
            key_status &= ~RIGHT_SHIFT;
            break;
    
        case KEY_CTRL:
            key_status &= ~LEFT_CTRL;
            break;
    
        case KEY_ALT:
            key_status &= ~LEFT_ALT;
            break;
    
        default:
            break;
        }   
    }
    else
    {
        switch (key)
        {
        case KEY_CAPS_LOCK:
            key_status ^= CAPS_LOCK;
            kbd_setleds();
            break;
    
        case KEY_NUM_LOCK:
            key_status ^= NUM_LOCK;
            kbd_setleds();
            break;
    
        case KEY_SCROLL_LOCK:
            key_status ^= SCROLL_LOCK;
            kbd_setleds();
            break;
    
        case KEY_LSHIFT:
            key_status |= LEFT_SHIFT;
            break;
                
        case KEY_RSHIFT:
            key_status |= RIGHT_SHIFT;
            break;
    
        case KEY_CTRL:
            key_status |= LEFT_CTRL;
            break;
    
        case KEY_ALT:
            key_status |= LEFT_ALT;
            break;
    
        default:
            break;
        }  
    }
    */
}

static void key_meta(int key, bool released)
{
    printf("key_meta: key=%d, released=%d\n", key, released);
}

static void key_ascii(int key, bool released)
{
    printf("key_ascii: key=%d, released=%d\n", key, released);
}

static void key_lock(int key, bool released)
{
    printf("key_lock: key=%d, released=%d\n", key, released);
}

static void key_lowercase(int key, bool released)
{
    if (released)
	return;
    putchar(key);
}
