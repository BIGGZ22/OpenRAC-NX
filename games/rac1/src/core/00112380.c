#include "common.h"
#include "structs.h"

/*
 * core_text object 0x112380-0x112468. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * newlib (the SDK's libc.a): atoi.o (atoi) and callocr.o (_calloc_r),
 * back to back. Built with Sony's 2.9-ee (Makefile.sn, EE29_CORE), like
 * libc.a.
 */

extern long func_00116F68(int arg0, int arg1, int arg2);

int func_00112380(int arg0) {
    return (int)func_00116F68(arg0, 0, 10);
}

extern void *func_00114920(void *, unsigned int);          /* _malloc_r */
extern void *func_001153FC_c(void *, int, unsigned int) __asm__("func_001153FC"); /* memset */

/* newlib's MALLOC_ZERO (mallocr.c, dlmalloc 2.6.5, public domain),
   verbatim: its do/while(0) is the library's own macro wrapper. */
#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
    unsigned int mzsz = (nbytes);                                             \
    if (mzsz <= 9 * 4) {                                                      \
        unsigned int *mz = (unsigned int *)(charp);                           \
        if (mzsz >= 5 * 4) {                                                  \
            *mz++ = 0;                                                        \
            *mz++ = 0;                                                        \
            if (mzsz >= 7 * 4) {                                              \
                *mz++ = 0;                                                    \
                *mz++ = 0;                                                    \
                if (mzsz >= 9 * 4) {                                          \
                    *mz++ = 0;                                                \
                    *mz++ = 0;                                                \
                }                                                             \
            }                                                                 \
        }                                                                     \
        *mz++ = 0;                                                            \
        *mz++ = 0;                                                            \
        *mz = 0;                                                              \
    } else {                                                                  \
        func_001153FC_c((charp), 0, mzsz);                                    \
    }                                                                         \
} while (0)

/* _calloc_r (newlib): allocate, then zero the chunk's usable size.
   Adapted from Lombyte (MIT) for PAL. */
void *func_001123A8(void *ptr, unsigned int n, unsigned int elem_size) {
    unsigned int sz = n * elem_size;
    void *mem;

    mem = func_00114920(ptr, sz);
    if (mem == 0) {
        return 0;
    } else {
        unsigned int *p = (unsigned int *)((char *)mem - 4);
        unsigned int csz = *p & ~(unsigned int)3;

        MALLOC_ZERO(mem, csz - 4);
        return mem;
    }
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00112464);
