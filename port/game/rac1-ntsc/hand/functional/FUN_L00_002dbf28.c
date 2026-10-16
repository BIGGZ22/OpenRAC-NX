/* Carried from the PAL decompilation's src/overlays/shared/vendor_002D9438.c (matched C there) (games/rac1) through the version
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
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern char D_L00_00166DC0[] NOT_SDA;
extern float D_0015ED6C MACRO_ADDR;
extern float func_002132A8(float, float);
extern void func_001F9A68(void *, void *, float);
extern int func_00213260(int);
void func_001F9A28(void *, void *, void *);
f32 func_001FA6C0(s32);
extern void func_L00_001FF500(void *, void *, float);
typedef struct { u8 p[9]; u8 b9; } G1T_c2dbf28;
extern G1T_c2dbf28 D_0013E520;
extern void func_001F9A10(void *,void *,void *);
extern int func_001F9688(int);
typedef int u128_DD3D8_c2dbf28 __attribute__((mode(TI)));
typedef struct { u128_DD3D8_c2dbf28 v[5]; } B50_DD3D8_c2dbf28;
typedef struct { int v[6]; } I6_DD3D8_c2dbf28;
extern B50_DD3D8_c2dbf28 D_L00_001EA0B0;
extern I6_DD3D8_c2dbf28 D_L00_001EA100;
extern I6_DD3D8_c2dbf28 D_L00_001EA118;
extern int *D_L00_00177F00[];
extern float func_001F9AB0(void *, void *);
extern float func_001F9AF0(void *);
extern int func_L00_00257B90(int, int);
extern char *func_L00_002AF450(char *, char *, int, int, int);
extern void func_L00_001FF598(void *, void *, float);
extern unsigned func_L00_0025C0E8(unsigned, int);
extern void func_L00_0026A9F0(void *, void *, int, int, float, int, int, int, int, float);
extern char *func_L00_002E0CB8_DD3D8(float, void *, void *, void *, int, unsigned char, unsigned char, unsigned char, unsigned char) __asm__("func_L00_002DF808");
typedef struct { char p0[0x160]; float f160; int p164; int i168; } Cam_DD3D8_c2dbf28;
extern Cam_DD3D8_c2dbf28 D_L00_00166D80_DD3D8 __asm__("D_L00_00166C80");
extern void func_L00_0025C088(int *, int *, int *, int);
extern char *func_L00_002D3838(char *, char *, int, char *);
extern int func_L00_001F2BE8_DD3D8(float, void *, int, void *, void *) __asm__("func_L00_001F2868");
extern void func_L00_0025A9F8(void *, void *, void *, int, int, int, int, int, float, float, float);



/* Explosion burst for a moby: debris sprays, sparks, coloured flashes, camera shake and an optional damage area. */
void FUN_L00_002dbf28(char *self, char *q) {
    B50_DD3D8_c2dbf28 blk;
    float a[4];
    float b[4];
    float c[4];
    float d[4];
    float e[4];
    float g[4];
    I6_DD3D8_c2dbf28 t1;
    I6_DD3D8_c2dbf28 t2;
    int r0, g0, b0;
    int r1, g1, b1;
    float f22, f23, f24, sc, dt;
    int step, i, j, k;

    blk = D_L00_001EA0B0;
    *(u128_DD3D8_c2dbf28 *)c = 0;
    c[2] = D_0015ED6C + D_0015ED6C;
    i = 0;
    *(u128_DD3D8_c2dbf28 *)b = *(u128_DD3D8_c2dbf28 *)c;
    *(u128_DD3D8_c2dbf28 *)c = 0;
    c[2] = 1.0f;
    f22 = func_001FA6C0((&D_0013E520)->b9) + 1.0f;
    step = 1;
    if ((&D_0013E520)->b9 == 0) step = 2;
    func_001F9A68(b, b, f22);
    func_001F9A28(a, D_L00_00166DC0, self + 0x10);
    f23 = func_001F9AF0(a);
    for (; i < 10 / step; i++) {
        *(u128_DD3D8_c2dbf28 *)g = 0;
        g[0] = func_002132A8(-1.0f, 1.0f);
        g[1] = func_002132A8(-1.0f, 1.0f);
        *(u128_DD3D8_c2dbf28 *)e = *(u128_DD3D8_c2dbf28 *)g;
        dt = func_001F9AB0(e, c);
        func_L00_001FF500(d, c, dt);
        func_001F9A28(e, e, d);
        func_L00_001FF500(d, c, func_002132A8(0.0f, 1.0f));
        func_001F9A10(e, e, d);
        func_L00_001FF500(e, e, f22 * func_002132A8(3.5f, 6.5f) * D_0015ED6C);
        func_001F9A10(e, e, b);
        {
            int h = func_001F9688(60);
            func_L00_002AF450(self + 0x10, (char *)e, func_L00_00257B90(h, func_001F9688(120)), 0, (&D_0013E520)->b9);
        }
    }
    for (j = 0; j < 4 / step; j++) {
        *(u128_DD3D8_c2dbf28 *)g = 0;
        g[0] = func_002132A8(-1.0f, 1.0f);
        g[1] = func_002132A8(-1.0f, 1.0f);
        *(u128_DD3D8_c2dbf28 *)e = *(u128_DD3D8_c2dbf28 *)g;
        dt = func_001F9AB0(e, c);
        func_L00_001FF500(d, c, dt);
        func_001F9A28(e, e, d);
        func_L00_001FF500(d, c, func_002132A8(0.0f, 1.0f));
        func_001F9A10(e, e, d);
        func_L00_001FF500(e, e, f22 * func_002132A8(6.5f, 10.0f) * D_0015ED6C);
        func_001F9A10(e, e, b);
        {
            int h = func_001F9688(60);
            func_L00_002AF450(self + 0x10, (char *)e, func_L00_00257B90(h, func_001F9688(90)), 1, (&D_0013E520)->b9);
        }
    }
    *(u128_DD3D8_c2dbf28 *)g = 0;
    g[0] = func_002132A8(-1.0f, 1.0f);
    g[1] = func_002132A8(-1.0f, 1.0f);
    g[2] = func_002132A8(-1.0f, 1.0f);
    *(u128_DD3D8_c2dbf28 *)e = *(u128_DD3D8_c2dbf28 *)g;
    a[2] += f23 * 0.5f;
    func_L00_001FF500(e, e, f23 / 5.0f * D_0015ED6C);
    func_L00_001FF500(a, a, (f23 + f23) * D_0015ED6C);
    func_001F9A10(e, e, a);
    func_L00_001FF598(e, e, D_0015ED6C * 10.0f);
    {
        int h = func_001F9688(60);
        func_L00_002AF450(self + 0x10, (char *)e, func_L00_00257B90(h, func_001F9688(90)), 1, (&D_0013E520)->b9);
    }
    if (f23 < 6.0f) {
        k = func_001FA898_r(f23) + 1;
    } else {
        k = 4;
    }
    f24 = 0.0f;
    if (f23 < 7.0f) {
        f24 = 7.0f - f23;
    }
    if (k > 0) {
        int cnt = k;
        do {
            float sh = f22 * func_002132A8(8.0f, 10.0f) * D_0015ED6C - f24 * D_0015ED6C;
            int A, B, r1, r2;
            float big;
            t1 = D_L00_001EA100;
            t2 = D_L00_001EA118;
            A = func_L00_0025C0E8(t1.v[func_00213260(6)], (&D_0013E520)->b9);
            B = func_L00_0025C0E8(t2.v[func_00213260(6)], (&D_0013E520)->b9);
            big = f22 * 400000.0f;
            r1 = func_L00_00257B90(func_001F9688(15), func_001F9688(20));
            r2 = func_L00_00257B90(func_001F9688(25), func_001F9688(30));
            func_L00_0026A9F0(self + 0x10, b, A, B, big, r1, r2, 0, 0, sh);
        } while (--cnt);
    }
    if (f23 > 9.0f) {
        float s4 = f22 * 4.0f;
        func_L00_002E0CB8_DD3D8(s4, self, self + 0x10, b, func_001F9688(15), 0x7F, 0x7F, 0x7F, 0x20);
        r0 = 0x7F;
        g0 = 0x20;
        b0 = 0;
        func_L00_0025C088(&r0, &g0, &b0, (&D_0013E520)->b9);
        func_L00_002E0CB8_DD3D8(s4, self, self + 0x10, b, func_001F9688(24), r0, g0, b0, 0x20);
    }
    r1 = 0x7F;
    g1 = 0x3F;
    b1 = 0;
    func_L00_0025C088(&r1, &g1, &b1, (&D_0013E520)->b9);
    {
        float s = f22 * 4.0f;
        func_L00_002E0CB8_DD3D8(s, self, self + 0x10, b, func_001F9688(20), r1, g1, b1, 0x30);
    }
    r1 = 0x60;
    g1 = 0x10;
    b1 = 0;
    func_L00_0025C088(&r1, &g1, &b1, (&D_0013E520)->b9);
    {
        float s = f22 * 3.5f;
        func_L00_002E0CB8_DD3D8(s, self, self + 0x10, b, func_001F9688(27), r1, g1, b1, 0x40);
    }
    r1 = 0x20;
    g1 = 0;
    b1 = 0;
    func_L00_0025C088(&r1, &g1, &b1, (&D_0013E520)->b9);
    {
        float s = f22 * 3.0f;
        func_L00_002E0CB8_DD3D8(s, self, self + 0x10, b, func_001F9688(29), r1, g1, b1, 0x20);
    }
    D_L00_00166D80_DD3D8.f160 = (f23 < 20.0f) ? 0.4f - f23 * 0.0175f : 0.050000012f;
    D_L00_00166D80_DD3D8.i168 = func_001F9688(25);
    func_L00_002D3838((char *)&blk, self + 0x10, 0, 0);
    if ((&D_0013E520)->b9) {
        k = func_L00_001F2BE8_DD3D8(3.0f, self + 0x10, 0x10, self, 0);
        *(u128_DD3D8_c2dbf28 *)e = *(u128_DD3D8_c2dbf28 *)(self + 0x10);
        func_L00_0025A9F8(self, e, D_L00_00177F00, k, 0, 0x830000, 2, 1, 2.0f, 1.0f, 1.0f);
    }
}
