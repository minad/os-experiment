#ifndef _RINGBUF_H
#define _RINGBUF_H

typedef struct ringbuf_s {
    size_t available;
    size_t size;
    size_t __head;
    size_t __tail;
    char*  __data;
} ringbuf_t:

void ringbuf_init(ringbuf_t* buf, size_t size);
bool ringbuf_write(ringbuf_t* buf, int ch);
int  ringbuf_read(ringbuf_t* buf);

#endif
