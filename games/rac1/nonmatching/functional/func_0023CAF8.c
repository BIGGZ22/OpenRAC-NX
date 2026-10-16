/* functional: high. pcmCallback: skips the four-byte prefix of the entry, wraps the source cursor at the ring capacity, splits the copy into the two available spans of the decoder, and returns whether anything was copied; every read, write and call with its arguments is in the retail order. */
#include "common.h"
extern char *D_0016130C MACRO_ADDR;
extern void func_0023C128(void *, char **, int *, char **, int *);
extern void func_0023C1F8(void *, int);
extern int func_0023CBE0(char *, int, char *, int, char *, int, char *, int);

int func_0023CAF8(void *unused, char *entry, char *buffer) {
    struct { char *p1; int n1; char *p2; int n2; } spans;
    int total = *(int *)(entry + 0xC) - 4;
    char *source = *(char **)(entry + 8) + 4;
    int capacity = *(int *)(buffer + 0x50008);
    char *end = buffer + capacity;
    int first;
    int copied;
    int remaining;
    if (source >= end) source -= capacity;
    first = end - source;
    if (total < first) first = total;
    remaining = total - first;
    func_0023C128(D_0016130C + 0xD9100, &spans.p1, &spans.n1, &spans.p2, &spans.n2);
    copied = func_0023CBE0(spans.p1, spans.n1, spans.p2, spans.n2, source, first, buffer, remaining);
    func_0023C1F8(D_0016130C + 0xD9100, copied);
    return copied > 0;
}
