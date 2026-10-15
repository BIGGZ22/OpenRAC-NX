#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

typedef struct {
    u32 field00;
    u32 field04;
    f32 field08;
    f32 field0C;
} NativeCallbackTimer;

extern NativeCallbackTimer D_L04_001B98E0;
extern f32 D_L04_001B1C88 __attribute__((sda));

typedef struct { u8 b[0x10]; } Local16;

extern f32 D_L03_001B0344 __attribute__((sda));

extern f32 D_L03_001B0CA0 __attribute__((sda));

extern u8 D_L03_001B9BB0[] __attribute__((nosda));

extern void func_00282F20(u8 *, u8 *, f32);

extern void func_00282D30(u8 *, u8 *, f32);

extern void func_00282CC0(u8 *, u8 *, u8 *);

extern void func_002A0F48(u8 *);

void func_L03_003CAAF0(u8 *a0, u8 *a1, u8 *a2) {
    Local16 buf0;
    Local16 buf1;

    func_00282F20(buf0.b, a0 + 0xC0, D_L03_001B0344);
    func_00282D30(buf1.b, buf0.b, -1.0f);
    func_00282CC0(a1, buf0.b, a0 + 0x10);
    func_00282CC0(a2, buf1.b, a0 + 0x10);
}

void func_L03_003E50E0(u8 *a0) {
    u8 *q = *(u8 **)(a0 + 0x68);
    u8 *p = *(u8 **)(q + 0x58);

    func_00282F20(p + 0x10, D_L03_001B9BB0, D_L03_001B0CA0);
    func_00282CC0(p + 0x10, p + 0x10, D_L03_001B9BB0 - 0x230);
    if (*(u32 *)(p + 0x64) != 0)
        (*(void (**)(u8 *))(*(u8 **)(q + 0x58) + 0x64))(p);
    func_002A0F48(*(u8 **)(q + 0x58));
}

/* updates the registered countdown state and clears its original completion flags */
void func_L03_0043ACD8(void) {
    NativeCallbackTimer *state = &D_L04_001B98E0;
    u32 mode = state->field04;
    state->field08 = 10000.0f;
    if (mode == 0) {
        state->field0C -= D_L04_001B1C88;
        if (state->field0C < 0.0f) state->field0C = 0.0f;
    } else {
        state->field04 = 0;
    }
    state->field00 = 0;
}

/* The $gp float each of the two mutual twins reads (3.0f and 1.0f). */
extern f32 D_L03_001B02F0 __attribute__((sda));

extern f32 D_L03_001B1A90 __attribute__((sda));

void func_L03_003CA230(u8 *a0, u8 *a1) {
    func_00282D30(a1, a0 + 0xC0, D_L03_001B02F0);
    func_00282CC0(a1, a1, a0 + 0x10);
}

/* The same body with func_00282F20 and 1.0f. */
void func_L03_004197C8(u8 *a0, u8 *a1) {
    func_00282F20(a1, a0 + 0xC0, D_L03_001B1A90);
    func_00282CC0(a1, a1, a0 + 0x10);
}
