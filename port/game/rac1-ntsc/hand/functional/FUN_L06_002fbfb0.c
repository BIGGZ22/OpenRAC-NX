/* Carried from the PAL decompilation's nonmatching/l06_blarg/func_L06_002FD3E0.c (games/rac1) through the version
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
/* What the PAL file this was written in declares before it. */
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_00250DF8(void *);
extern int func_00214720(void *, int);
extern char D_0013E533[];
extern int func_001F9688(int);
extern void func_0022DA68(int, int, int);
extern float func_001F9B48(void *, void *);
extern float func_00213F38(float *, float *, float, float, float, float);
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern float func_002132A8(float, float);
extern int func_001F9740(void *);

/* NON_MATCHING FUN_L06_002fbfb0 -- src/overlays/l06_blarg/vendor_002B5990.c
 * Best so far: SIZE ours 1628 / retail 1664, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 6 blarg-class moby update (class 1054): spawns a child moby (func_0020C4F8) and steers it through a six-
 *   Still off: the case 4 dispatch (retail loads f20/f21/f23 from the gp block and the 10/20 constants in the stat
 *   Stopped for effort, not at a listed wall: 4 of 10 runs used. A next pass would start from the case 4 arm order
 */
extern short D_L06_00161F1C;
extern short D_L06_00161F20;
extern short D_L06_00161F24;
extern short D_L06_00161F28;
extern short D_L06_00161F2C;
extern short D_L06_00161F30;
extern short D_L06_00161F34;
extern short D_L06_00161F38;
extern short D_L06_00161F3C;
extern short D_L06_00161F40;
extern short D_L06_00161F44;
extern short D_L06_00161F48;
extern short D_L06_00161F4C;
extern short D_L06_00161F50;
extern short D_L06_00161F54;
extern short D_L06_00161F58;
extern short D_L06_00161F5C;
extern struct Moby *func_0020D348_m(int) __asm__("func_0020C4F8");
extern float AbsoluteFloat(float input) __asm__("func_001F99C0");
extern int func_001FA898_r(float) __asm__("func_001FA6D0");

/* Update for moby class 1054 on level 6: spawns a child moby and steers it through a six-state machine. */
void FUN_L06_002fbfb0(unsigned char *moby)
{
    char *data, *c;
    float vec[4], d1, d2, f0, f12, f13, f14, f15, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, k;
    int cnt, st, nb;

    data = *(char **)(moby + 0x78);
    switch (moby[0x20]) {
    case 0:
        qcopy(data + 0x10, moby + 0x10);
        qcopy(data + 0x20, moby + 0x40);
        qcopy(data + 0x30, moby + 0xE0);
        c = (char *)func_0020D348_m(0x41F);
        *(char **)(data + 0x40) = c;
        *(short *)(c + 0x32) = 0x40;
        c[0x31] = 1;
        *(s64 *)(c + 0x38) = *(s64 *)(moby + 0x38);
        *(short *)(*(char **)(data + 0x40) + 0x34) = *(unsigned short *)(moby + 0x34);
        qcopy(*(char **)(data + 0x40) + 0x10, moby + 0x10);
        qcopy(*(char **)(data + 0x40) + 0x40, moby + 0x40);
        if (*(int *)data >= 0) {
            moby[0x20] = 1;
            return;
        }
        st = *(int *)(data + 4);
        if (st == 2) {
            moby[0x20] = 2;
            return;
        }
        func_L00_001FF500(vec, data + 0x30, *(float *)&D_L06_00161F1C);
        func_001F9A10(moby + 0x10, data + 0x10, vec);
        f12 = *(float *)&D_L06_00161F20;
        func_L00_001FF500(vec, data + 0x30, -f12);
        func_001F9A10(*(char **)(data + 0x40) + 0x10, data + 0x10, vec);
        func_L00_00250DF8(*(char **)(data + 0x40));
        moby[0x20] = 5;
        return;
    case 1:
        if (func_00214720(D_0013E533 + 0xE9D, *(int *)data) == 0) return;
        st = *(int *)(data + 4);
        if (st == 1) {
            moby[0x20] = 4;
            *(int *)(data + 0x4C) = func_001F9688(0x14);
        } else if (moby[0x21] == 0xFF) {
            moby[0x20] = 3;
        } else {
            func_L01_0026E0E0(moby[0x21], 3);
        }
        func_0022DA68(0, 0, *(int *)(data + 0x40));
        return;
    case 3:
        d1 = func_001F9B48(moby + 0x10, data + 0x10);
        d2 = func_001F9B48(*(char **)(data + 0x40) + 0x10, data + 0x10);
        func_00213F38(&d1, (float *)(data + 0x8), *(float *)&D_L06_00161F1C, *(float *)&D_0015ED70 * 10.0f, *(float *)&D_0015ED70 * 20.0f, *(float *)&D_0015ED6C * 5.0f);
        func_00213F38(&d2, (float *)(data + 0x44), *(float *)&D_L06_00161F20, *(float *)&D_0015ED70 * 10.0f, *(float *)&D_0015ED70 * 20.0f, *(float *)&D_0015ED6C * 5.0f);
        func_L00_001FF500(vec, data + 0x30, d1);
        func_001F9A10(moby + 0x10, data + 0x10, vec);
        func_L00_001FF500(vec, data + 0x30, -d2);
        func_001F9A10(*(char **)(data + 0x40) + 0x10, data + 0x10, vec);
        func_L00_00250DF8(*(char **)(data + 0x40));
        k = 0.01f;
        f0 = AbsoluteFloat(d1 - *(float *)&D_L06_00161F1C);
        if (!(f0 < k)) return;
        f0 = AbsoluteFloat(d2 - *(float *)&D_L06_00161F20);
        if (!(f0 < k)) return;
        moby[0x20] = 5;
        return;
    case 4:
        d1 = func_001F9B48(moby + 0x10, data + 0x10);
        d2 = func_001F9B48(*(char **)(data + 0x40) + 0x10, data + 0x10);
        st = moby[0xBC];
        nb = 1;
        if (st == 1) {
            nb = 2;
            f12 = (float)*(int *)&D_L06_00161F54;
            f13 = (float)*(int *)&D_L06_00161F58;
            f20 = d1;
            f21 = d2;
            f23 = *(float *)(*(char **)(data + 0x40) + 0x44);
            f22 = *(float *)(moby + 0x44);
            cnt = 0;
            goto l7e4;
        }
        if (st == 2) {
            f12 = (float)*(int *)&D_L06_00161F4C;
            f13 = (float)*(int *)&D_L06_00161F50;
            f21 = *(float *)&D_L06_00161F28;
            f22 = *(float *)&D_L06_00161F2C;
            f23 = *(float *)&D_L06_00161F30;
            cnt = 1;
            goto l7e4;
        }
        f26 = 10.0f;
        f23 = *(float *)(moby + 0x44);
        f24 = 20.0f;
        f20 = *(float *)&D_L06_00161F24;
        f22 = f23;
        f21 = *(float *)&D_L06_00161F28;
        f29 = f26;
        f25 = 20.0f;
        f27 = f24;
        cnt = 30;
        goto l80c;
    l7e4:
        f26 = *(float *)&D_L06_00161F34;
        f25 = *(float *)&D_L06_00161F38;
        f24 = *(float *)&D_L06_00161F3C;
        f29 = *(float *)&D_L06_00161F40;
        f28 = *(float *)&D_L06_00161F44;
        f27 = *(float *)&D_L06_00161F48;
        f0 = func_002132A8(f12, f13);
        cnt = func_001FA898_r(f0);
    l80c:
        func_00213F38(&d1, (float *)(data + 0x8), f20, f26 * *(float *)&D_0015ED70, f25 * *(float *)&D_0015ED70, f24 * *(float *)&D_0015ED6C);
        func_00213F38(&d2, (float *)(data + 0x44), f21, f26 * *(float *)&D_0015ED70, f25 * *(float *)&D_0015ED70, f24 * *(float *)&D_0015ED6C);
        k = 0.017453292f;
        f12 = f22;
        f22 = f27 * k;
        f21 = f29 * k;
        f20 = f28 * k;
        func_00213F38((float *)(moby + 0x44), (float *)(data + 0xC), f12, f21 * *(float *)&D_0015ED70, f20 * *(float *)&D_0015ED70, f22 * *(float *)&D_0015ED6C);
        func_00213F38((float *)(*(char **)(data + 0x40) + 0x44), (float *)(data + 0x48), f23, f21 * *(float *)&D_0015ED70, f20 * *(float *)&D_0015ED70, f22 * *(float *)&D_0015ED6C);
        func_L00_001FF500(vec, data + 0x10, d1);
        func_001F9A10(moby + 0x10, data + 0x10, vec);
        func_L00_001FF500(vec, data + 0x10, -d2);
        func_001F9A10(*(char **)(data + 0x40) + 0x10, data + 0x10, vec);
        func_L00_00250DF8(*(char **)(data + 0x40));
        if (func_001F9740(data + 0x4C) == 0) return;
        moby[0xBC] = (unsigned char)nb;
        *(int *)(data + 0x4C) = func_001F9688(cnt);
        f0 = AbsoluteFloat(d1 - *(float *)&D_L06_00161F24);
        if (!(f0 < 0.01f)) return;
        f0 = AbsoluteFloat(d2 - *(float *)&D_L06_00161F28);
        if (!(f0 < 0.01f)) return;
        moby[0x20] = 5;
        return;
    case 5:
        if (*(float *)&D_L06_00161F5C == 0.0f) return;
        moby[0xBC] = 0;
        moby[0x20] = 1;
        qcopy(moby + 0x10, data + 0x10);
        qcopy(moby + 0x40, data + 0x20);
        qcopy(*(char **)(data + 0x40) + 0x10, moby + 0x10);
        qcopy(*(char **)(data + 0x40) + 0x40, moby + 0x40);
        *(float *)(data + 0x48) = 0.0f;
        *(float *)(data + 0x8) = 0.0f;
        *(float *)(data + 0xC) = 0.0f;
        *(float *)(data + 0x44) = 0.0f;
        return;
    default:
        return;
    }
}
