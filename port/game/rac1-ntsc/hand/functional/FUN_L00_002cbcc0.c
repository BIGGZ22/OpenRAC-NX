/* Carried from the PAL decompilation's src/overlays/shared/vendor_002C96D0.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c2cbcc0 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2cbcc0 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2cbcc0;
extern void func_L00_00267A08(void *);
extern f32 func_002132A8(f32, f32);
extern void func_L00_001FF500(void *, void *, f32);
extern s32 func_001F9688(s32);
extern s32 func_L00_00257B90(s32, s32);
extern void *func_L00_0026DD20(void *, void *, u32, s32, f32);
extern s32 func_00213260(s32);
extern u8 *func_L00_0026D000(void *, s32, f32, f32, void *, u32, f32, f32);
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;



void FUN_L00_002cbcc0(u8 *m, s32 enabled)
{
    s32 i;
    s32 j;
    void **handle;
    u8 *pos;
    Vec4_c2cbcc0 vectors[2];
    s32 lifetime;
    s32 color;
    s32 sign;
    u8 *effect;
    s32 alpha;

    handle = (void **)(*(u8 **)(m + 0x78) + 0x2c);
    for (i = 16; i >= 0; i--) {
        if (*handle != 0)
            func_L00_00267A08(*handle);
        *handle++ = 0;
    }
    pos = m + 0x10;

    if (enabled != 0) {
        Vec4_c2cbcc0 *vector = &vectors[1];
        for (j = 4; j >= 0; j--) {
            vectors[1].q = 0;
            vectors[1].f[0] = func_002132A8(-1.0f, 1.0f);
            vectors[1].f[1] = func_002132A8(-1.0f, 1.0f);
            vectors[1].f[2] = func_002132A8(-1.0f, 1.0f);
            vectors[0].q = vectors[1].q;
            func_L00_001FF500(vector, &vectors[0],
                func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f));
            lifetime = func_L00_00257B90(func_001F9688(10), func_001F9688(15));
            func_L00_0026DD20(pos, vector, 0x7f2f4f6f, lifetime, 35000.0f);
        }
    }

    {
        s32 j;
        for (j = 2; j >= 0; j--) {
            color = ((u32)func_L00_00257B90(0x20, 0x80) << 24) | 0x606060;
            sign = func_00213260(2) ? -1 : 1;
            vectors[0].q = 0;
            vectors[0].f[2] = 1.0f;
            vectors[0].f[3] = 1.0f;
            vectors[0].f[2] = func_002132A8(0.01f, 0.025f) * D_0015ED60;
            effect = func_L00_0026D000(pos, sign, 0.2f, 1.0f, &vectors[0], color, 1.02f, 140000.0f);
            if (effect != 0) {
                *(s16 *)(effect + 0xa) = func_001F9688(func_L00_00257B90(50, 80));
                i = (s32)(effect + 0x20);
                *(s32 *)(i + 4) = 2;
                ((u8 *)i)[0xa] = color >> 24;
                ((u8 *)i)[0xb] = effect[0xa];
                alpha = func_00213260(2);
                if (alpha) {
                    alpha = 0x44;
                    effect[3] = alpha;
                }
            }
        }
    }
}
