#include <ringbuf.h>

void ringbuf_init(ringbuf_t* buf, size_t size)
{
    buf->__data = (char*)malloc(size);
    buf->__head = buf->__tail = buf->available = 0;
    buf->size = size;
}

bool ringbuf_write(ringbuf_t* buf, int ch)
{
    if (buf->available == buf->size)
	return false;
    buf->__data[buf->__head] = ch;
    ++buf->available;
    if (++buf->__head == buf->size)
	buf->__head = 0;
}

int ringbuf_read(ringbuf_t* buf)
{
    if (buf->available == 0)
	return -1;
    char ch = buf->__data[buf->__tail];
    --buf->available;
    if (++buf->__tail == buf->size)
	buf->__tail = 0;
    return ch;
}

