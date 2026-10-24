#include "common.h"
#include "structs.h"

extern char *D_0016130C MACRO_ADDR;
extern char D_001E8E38[];
extern void func_0023DFC0(void *, char **, int *, char **, int *);
extern int func_0023CBE0(char *, int, char *, int, char *, int, char *, int);
extern int func_0023E068(void *, long, long, void *, int);
extern void func_0023BF48(void *);
extern void func_0023DFE0(void *, int);

/* Copies wrapped input into free decoder spans, records timestamps, then submits the copied bytes. */
int videoCallback(void *unused, char *entry, char *buffer) {
    struct { char *p1; int n1; char *p2; int n2; } spans;
    char *source = *(char **)(entry + 8);
    int total = *(int *)(entry + 0xC);
    int first = buffer + *(int *)(buffer + 0x50008) - source;
    int copied;
    int remaining;
    if (total < first) first = total;
    remaining = total - first;
    videoDecBeginPut(decoder_state + 0xD9048, &spans.p1, &spans.n1, &spans.p2, &spans.n2);
    copied = cpy2area((char *)(((unsigned int)spans.p1 & 0xFFFFFFF) | 0x20000000), spans.n1,
                          (char *)(((unsigned int)spans.p2 & 0xFFFFFFF) | 0x20000000), spans.n2,
                          source, first, buffer, remaining);
    if (copied > 0) {
        if (!videoDecPutTs(decoder_state + 0xD9048, *(long *)(entry + 0x10), *(long *)(entry + 0x18), spans.p1, copied)) {
            ErrMessage(D_001E8E38);
        }
    }
    videoDecEndPut(decoder_state + 0xD9048, copied);
    return copied > 0;
}
extern void audioDecBeginPut(void *, char **, int *, char **, int *);
extern void audioDecEndPut(void *, int);

/* Skips the entry's 4-byte prefix, wraps the source at the ring size (+0x50008),
 * copies into the decoder's two free spans and reports whether anything was copied. */
int pcmCallback(void *unused, char *entry, char *buffer) {
    char *dst0, *dst1;
    int len0, len1;
    char *src;
    char *ring;
    int first, rest;
    int total;
    int copied;

    total = *(int *)(entry + 0xC) - 4;
    src = *(char **)(entry + 8) + 4;
    if (src >= buffer + *(int *)(buffer + 0x50008)) src -= *(int *)(buffer + 0x50008);
    first = buffer + *(int *)(buffer + 0x50008) - src;
    if (total < first) first = total;
    ring = buffer;
    rest = total - first;
    audioDecBeginPut(decoder_state + 0xD9100, &dst0, &len0, &dst1, &len1);
    copied = cpy2area(dst0, len0, dst1, len1, src, first, ring, rest);
    audioDecEndPut(decoder_state + 0xD9100, copied);
    return copied > 0;
}
extern void *func_00115248(void *dest, const void *src, int n);

/* cpy2area(unsigned char *, int, unsigned char *, int, unsigned char *, int, unsigned char *, int) */
int cpy2area(char *dst1, int size1, char *dst2, int size2,
                  char *src1, int len1, char *src2, int len2) {
    if (size1 + size2 < len1 + len2) {
        return 0;
    }
    if (len1 >= size1) {
        func_00115248(dst1, src1, size1);
        func_00115248(dst2, src1 + size1, len1 - size1);
        func_00115248(dst2 + len1 - size1, src2, len2);
    } else {
        int rem = size1 - len1;
        if (len2 >= rem) {
            func_00115248(dst1, src1, len1);
            func_00115248(dst1 + len1, src2, rem);
            func_00115248(dst2, src2 + size1 - len1, len2 - rem);
        } else {
            func_00115248(dst1, src1, len1);
            func_00115248(dst1 + len1, src2, len2);
        }
    }
    return len1 + len2;
}
