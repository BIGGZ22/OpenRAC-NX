/* Near miss from src/levels/01_oozla/L01_003DA740.c: the build refuses this body at its
   place — 312 bytes, expected 304. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/01_oozla/L01_003DA740.c. */
typedef unsigned char      u8;

typedef signed int         s32;

typedef float              f32;

extern s32 func_310648();

extern f32 func_310580(f32 x, f32 y);

extern s32 func_310528(s32 a, s32 b);

extern f32 func_2E3A48(f32 x);

extern f32 func_2E3A60(f32 x);

extern void func_2E3470(void *a, void *b, void *c);

extern void func_32EF38(void *buf, s32 a1, s32 a2, s32 a3, f32 x, f32 y);

void func_L01_003DBB88(u8 *a0, s32 a1) {
    u8 *state = *(u8 **)(a0 + 0x68);
    f32 buf[3];
    s32 i;

    for (i = a1; i > 0; i--) {
        f32 angle = func_310648();
        f32 span = func_310580(0.0f, 0.6f);
        f32 near = func_310580(0.0f, 1.5f);
        f32 far = func_310580(1.5f, 5.5f);
        s32 handle = func_310528(0x64, 0xA0);

        buf[0] = func_2E3A48(angle) * span;
        buf[1] = func_2E3A60(angle) * span;
        buf[2] = 0.0f;
        func_2E3470(buf, buf, state + 0x380);
        func_32EF38(buf, 0x30305020, 0x182810, handle, near, far);
    }
}
