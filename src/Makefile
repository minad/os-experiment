CC = cc
CPP = cpp
LD = ld
NM = nm
STRIP = strip

OBJECTS =\
asm.o\
bitmap.o\
console.o\
cpu.o\
ctype.o\
debug.o\
gdt.o\
idt.o\
keyboard.o\
keymap.o\
main.o\
malloc.o\
multiboot.o\
pic.o\
pit.o\
pmem.o\
pool.o\
stdio.o\
string.o\
syscall.o\
thread.o\
time.o\
timer.o\
vmem.o

KERNEL     = myos.elf
KERNEL_MAP = myos.map
LDS        = myos.lds
INCLUDES   = -I../include -include compiler.h
DEBUGFLAGS = -DE9_HACK -g
CFLAGS     = $(DEBUGFLAGS) -pipe -pedantic -Wall -std=gnu99 -nostdlib\
             -ffreestanding -m32 -march=i486
ASM_FLAGS  = $(DEBUGFLAGS) -pipe -m32 -march=i486
LDFLAGS    = -m elf_i386

$(KERNEL): $(OBJECTS)
	$(CPP) -D__ASM__ -I../include -P -E $(LDS) | $(LD) $(LDFLAGS) -o $@ $(OBJECTS) -T /dev/stdin
	$(NM) $(KERNEL) > $(KERNEL_MAP)
	#$(STRIP) $(KERNEL)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

.S.o:
	$(CC) $(ASM_FLAGS) -D__ASM__ -Wall $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(KERNEL) $(KERNEL_MAP) *.s *.i
