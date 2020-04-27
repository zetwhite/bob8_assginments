#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "printf.h"

/* Comment out to disable debug prints (e.g. for performance testing) */
#define DEBUG 1

unsigned int replaybuf[1024];
void * allocations[8192 * 8];

/* Exports from alloc.c */
void *myalloc(size_t size);
void *myrealloc(void *ptr, size_t size);
void myfree(void *ptr);

int main()
{
    char buf[128];
    struct rusage usage;
    unsigned int rem = 0, idx = 0, sz;

    while (1)
    {
        if (rem == 0)
        {
            int rv = read(0, replaybuf, sizeof(replaybuf));
            if (rv <= 0)
                break;
            rem = rv / sizeof(unsigned int);
            idx = 0;
        }

        unsigned int arg1 = replaybuf[idx++];
        unsigned int arg2 = replaybuf[idx++];
        unsigned int op = arg1 >> 24;
        unsigned int size = arg1 & 0xffffff;
        rem -= 2;

        if (op == 0)
        {
            unsigned char *p;
            p = myalloc(size);
            allocations[arg2] = p;

            if (size) memset(p, 0x41, size);
        }
        else if (op == 1)
        {
            unsigned char *p = allocations[arg2];

            /* TODO validate memory has no been corrupted */

            p = myrealloc(p, size);
            allocations[arg2] = p;

            if (size) memset(p, 0x41, size);
        }
        else if (op == 2)
        {
            unsigned char *p = allocations[arg2];

            /* TODO validate memory has no been corrupted */

            myfree(p);
            allocations[arg2] = NULL;
        }
    }

    getrusage(RUSAGE_SELF, &usage);
    sz = sprintf(buf, "\tUser time: %f seconds\n\tMax RSS: %ld kilobytes\n",
            usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0f,
            usage.ru_maxrss);
    write(STDOUT_FILENO, buf, sz);

    return 0;
}

/* Debug printf support */
void _putchar(char character)
{
    write(STDERR_FILENO, &character, 1);
}

void debug(const char *fmt, ...)
{
#if DEBUG
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
#endif
}
