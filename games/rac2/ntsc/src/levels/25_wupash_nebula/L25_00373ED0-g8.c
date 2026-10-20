#include "include_asm.h"

typedef unsigned char u8;

typedef signed int s32;

typedef float f32;

/* The level's own live-object count lives in the small-data area; the list it walks
   is far larger than the eight-byte threshold, so it keeps absolute addressing. */
extern s32 D_L25_001AC4D8 __attribute__((sda));

extern u8 *D_L25_00225910[] __attribute__((nosda));

extern void func_L05_0037B280(void *, void *);

extern void func_L05_0037CEE0(void *, void *, f32);

INCLUDE_ASM("asm/levels", func_L05_0037D608);

/* Offers every live entry to the handler that takes one pointer through the loop. */
void func_L25_003752D8(u8 *a0) {
    s32 i;
    for (i = 0; i < D_L25_001AC4D8; i++) {
        func_L05_0037B280(D_L25_00225910[i], a0);
    }
}

/* Offers every live entry to the handler that carries one float through the loop. */
void func_L25_00376FE0(u8 *a0, f32 f12) {
    s32 i;
    for (i = 0; i < D_L25_001AC4D8; i++) {
        func_L05_0037CEE0(D_L25_00225910[i], a0, f12);
    }
}

extern void func_L05_0037D608(void *, void *, f32);

/* Offers every live entry to the handler that carries one float through the loop. */
void func_L25_00377608(u8 *a0, f32 f12) {
    s32 i;
    for (i = 0; i < D_L25_001AC4D8; i++) {
        func_L05_0037D608(D_L25_00225910[i], a0, f12);
    }
}
