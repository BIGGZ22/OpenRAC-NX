/* functional: high. Checked against the US assembly: same calls and arguments (the jump table at 0x1E8AD0 orders the six screens as here), constants, globals and stack vectors. */
/* Carried from the PAL decompilation's src/game/vendor.c (matched C there) (games/rac1) through the version
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
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9BF8(void *, void *, float);
extern void func_001F9A28(void *, void *, void *);
extern void func_0020CD48(char *arg0, int arg1, void *arg2, char *arg3);
extern f32 func_001FA6C0(s32);
extern void func_001F55D8(f32, f32, f32, f32, s32, s32, s32, s32, u64, s64);
void func_00239328(s32 pass_index, f32 capture_width,
                                                 f32 capture_height);
extern void func_00234C98_l(int, long) __asm__("func_00233980");
void func_00239690(int a, int b, float f);
void func_00239750(void);
typedef int u128_3AA38_c239780 __attribute__((mode(TI)));
extern float D_001E6218[][4];
extern int D_00161298 MACRO_ADDR;
extern short D_001612A0;
extern short D_0016129C;
extern short D_00161294;
extern int D_0015ED80 MACRO_ADDR;
extern long D_0015EED0 MACRO_ADDR;
extern float func_001F9AF0(void *);
extern void func_00237C80(void *, void *, float *, float *, float *, float *);
extern void func_001FB740(int, int);
extern void func_00233B68(void);
extern int func_001FA898_3AA38(float) __asm__("func_001FA6D0");
extern void func_00239838_3AA38(char *) __asm__("func_00238520");
extern void func_00239CF8_3AA38(char *) __asm__("func_002389E0");
extern void func_00239948_3AA38(char *) __asm__("func_00238630");
extern void func_00238F08(char *, int, int);
extern void func_0023A478_3AA38(char *) __asm__("func_00239160");
extern void func_00239A00_3AA38(char *) __asm__("func_002386E8");



/* Renders the six vendor monitor screens: projects each screen's joint box, renders its view off-screen and draws it back. */
void FUN_00239780(char *m) {
    float len;
    float f;
    float g;
    int i;
    long tex;

    for (i = 0; i < 6; i++) {
        u128_3AA38_c239780 d1[1];
        u128_3AA38_c239780 d2[1];
        u128_3AA38_c239780 base[1];
        u128_3AA38_c239780 out[3];
        int ids[3] = { i * 4, i * 4 + 1, i * 4 + 2 };
        u128_3AA38_c239780 tmp[1];
        u128_3AA38_c239780 c[4];
        u128_3AA38_c239780 pad[5];
        u128_3AA38_c239780 a[1];
        u128_3AA38_c239780 b[1];
        float sx, sy, sw, sh;
        float x, y, w, h;
        func_0020CD48(m, 3, ids, (char *)out);
        qcopy(base, out);
        func_001F9A28(d1, &out[1], out);
        func_001F9A28(d2, &out[2], out);
        len = func_001F9AF0(d1);
        func_001F9BF8(tmp, d1, D_001E6218[i][0]);
        func_001F9A10(base, base, tmp);
        func_001F9BF8(d1, d1, len - 2.0f * D_001E6218[i][2]);
        len = func_001F9AF0(d2);
        func_001F9BF8(tmp, d2, D_001E6218[i][1]);
        func_001F9A10(base, base, tmp);
        func_001F9BF8(d2, d2, len - 2.0f * D_001E6218[i][3]);
        func_001F9A10(tmp, base, d1);
        func_001F9A10(tmp, tmp, d2);
        func_00237C80(base, tmp, &sx, &sy, &sw, &sh);
        if (D_00161298 != 0 || *(int *)&D_001612A0 != 0) {
            f = 1.0f;
            if (*(int *)&D_001612A0 != 0) {
                f = func_001FA6C0(8 - *(int *)&D_0016129C) * 0.125f;
            }
            if (D_00161298 != 0) {
                f = func_001FA6C0(*(int *)&D_00161294) * 0.125f;
            }
            len = func_001F9AF0(d1);
            g = 1.0f - f;
            func_001F9BF8(tmp, d1, len * g * 0.5f);
            func_001F9BF8(d1, d1, len * f);
            func_001F9A10(base, base, tmp);
            len = func_001F9AF0(d2);
            func_001F9BF8(tmp, d2, len * g * 0.5f);
            func_001F9BF8(d2, d2, len * f);
            func_001F9A10(base, base, tmp);
        }
        qcopy(&c[0], base);
        func_001F9A10(&c[1], base, d1);
        func_001F9A10(&c[2], base, d2);
        func_001F9A10(&c[3], &c[2], d1);
        a[0] = c[0];
        b[0] = c[3];
        func_00237C80(a, b, &x, &y, &w, &h);
        func_00239690(9, 7, 1.0f);
        func_001FB740(0x200, 0x200);
        func_00234C98_l(0x42, 0x8000000064L);
        switch (i) {
        case 0:
            func_00239838_3AA38(m);
            break;
        case 1:
            func_00239CF8_3AA38(m);
            break;
        case 2:
            func_00239948_3AA38(m);
            break;
        case 3:
            func_00238F08(m, (int)x, (int)y);
            break;
        case 4:
            func_0023A478_3AA38(m);
            break;
        case 5:
            func_00239A00_3AA38(m);
            break;
        }
        func_00239328(i, x, y);
        func_00239750();
        func_001FB740(0x200, 0x200);
        func_00234C98_l(0x42, 0x8000000064L);
        func_00234C98_l(8, 5);
        func_00233B68();
        tex = D_0015EED0;
        if (D_0015ED80 != 0) {
            func_001F55D8(w, h, x, y, 0, 0, (int)(sx - 1.0f),
                          func_001FA898_3AA38(sy * 0.92857146f - 1.0f),
                          0x80808080, tex);
        } else {
            func_001F55D8(w, h, x, y, 0, 0, (int)(sx - 1.0f), (int)(sy - 1.0f),
                          0x80808080, tex);
        }
        func_00234C98_l(8, 0);
    }
}
