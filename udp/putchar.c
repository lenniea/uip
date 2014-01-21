#include <stdio.h>
#undef putchar

#define TRACE_BUF_SIZE		4096
extern unsigned char trace_buf[TRACE_BUF_SIZE];

short trace_index = 0;

int putchar(int c)
{
	trace_buf[trace_index++ & (TRACE_BUF_SIZE - 1)] = c;
	return c;
}
