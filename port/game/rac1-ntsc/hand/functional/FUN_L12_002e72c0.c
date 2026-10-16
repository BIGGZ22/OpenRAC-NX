/* Carried from the PAL decompilation's nonmatching/functional/func_L12_002E8640.c (games/rac1) through the version
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
/* functional: high. Same reads, writes, calls and arguments as retail; the jump table is a switch, the shared exits are gotos, quad copies are four-float copies, and the two func_0020C828 calls pass the pointer that retail leaves in $a0. No COP2/VU0 instructions in this function. */
/* Level 12 (Hoven) moby update, one call per frame per moby. data = moby+0x78 is its state block, hero = the target
 * moby found by func_L00_0025FCB8 (or the hero global when none). Runs func_L00_00263B38 on data+0x120, hides or shows
 * the moby (flags +0x34 |= 0x41 or &= 0xFFBE) from the near test against the hero and the clip volume, runs the
 * shared update func_L12_002E81E0, then a state machine on moby+0x20: 0 setup, 1 walk along a waypoint row of
 * D_L12_001B0930, 2 wait/spawn check, 3 wander and face the hero, 4 and 9 dying/explode checks, 5 to 8 idle,
 * attack-ready and chase steering. The tail (state-independent) animates the two bone controllers. */
extern char D_0013E633_2E8640[] __asm__("D_0013E533");
extern int D_L12_0015F6A8_2E8640 __asm__("D_L12_0015F5C4") MACRO_ADDR;
extern char D_L12_001672C0_2E8640[] __asm__("D_L12_00167240");
extern float D_L12_0015F660_2E8640[] __asm__("D_L12_0015F580") MACRO_ADDR;
extern char *D_L12_00178400_2E8640[] __asm__("D_L12_00178380");
extern char *D_L12_001B0C30_2E8640[] __asm__("D_L12_001B0930");
extern float D_0015EE6C_2E8640 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015EE64_2E8640 __asm__("D_0015ED64") MACRO_ADDR;
extern void func_L00_00264BB0_2E8640(void *, float) __asm__("func_L00_00263B38");
extern int func_00215570_2E8640(void *, int) __asm__("func_00214720");
extern void func_L12_002E9560_2E8640(void *) __asm__("func_L12_002E81E0");
extern int func_L00_00260D30_2E8640(void *, void *, float) __asm__("func_L00_0025FCB8");
extern float func_001F9D48_2E8640(void *, void *) __asm__("func_001F9B80");
extern float func_001F9D10_2E8640(void *, void *) __asm__("func_001F9B48");
extern void func_L01_0026E8E0_2E8640(void *) __asm__("func_L01_0026D930");
extern int func_001160D8_2E8640(void) __asm__("func_001160D8");
extern int func_L00_00258BC8_2E8640(int, int) __asm__("func_L00_00257B90");
extern int func_001F9850_2E8640(int) __asm__("func_001F9688");
extern float func_L00_00258C80_2E8640(float, float) __asm__("func_L00_00257C48");
extern float func_00214358_2E8640(void *, int, float) __asm__("func_00213508");
extern int func_001F9908_2E8640(void *) __asm__("func_001F9740");
extern void func_00213F28_2E8640(void *, int, int, int, int) __asm__("func_002130D8");
extern int func_L12_0027C8D8_2E8640(void *, void *, float, float, float) __asm__("func_L12_0027B8E0");
extern int func_L00_001EFFF0_2E8640(void *, void *, int, void *, void *) __asm__("func_L00_001EFC70");
extern void func_0020D678_2E8640(void *) __asm__("func_0020C828");
extern void func_L01_0026F040_2E8640(int, int) __asm__("func_L01_0026E090");
extern int func_L01_0028C2D8_2E8640(void *, void *, float) __asm__("func_L01_0028B510");
extern void func_L12_002E9B88_2E8640(void *, void *, float, float) __asm__("func_L12_002E8808");
extern void func_L12_002E9988_2E8640(void *, void *) __asm__("func_L12_002E8608");
extern void func_L12_002E9B30_2E8640(void *) __asm__("func_L12_002E87B0");
extern void func_L12_002E9A50_2E8640(void *) __asm__("func_L12_002E86D0");
extern void func_L12_002E9AA0_2E8640(void *) __asm__("func_L12_002E8720");
extern void func_L12_002E8610_2E8640(void *, void *) __asm__("func_L12_002E7290");
extern float func_001F9F90_2E8640(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_2E8640(float) __asm__("func_001F9DE0");
extern int func_L00_001F2BE8_2E8640(void *, int, void *, int, float) __asm__("func_L00_001F2868");
extern int func_L00_0025F410_2E8640(void *) __asm__("func_L00_0025E3B8");
extern float func_001F9B88_2E8640(float) __asm__("func_001F99C0");
extern void func_L00_0025AC00_2E8640(void *, void *, int, void *, void *, float) __asm__("func_L00_00259BC8");
extern float func_L00_001FF860_2E8640(float, float) __asm__("func_L00_001FF8B0");
extern float func_001FA790_2E8640(float, float) __asm__("func_001FA5C8");
extern float func_001FA850_2E8640(float, float) __asm__("func_001FA688");
extern float func_001FA748_2E8640(float, float) __asm__("func_001FA580");
extern int func_00215B18_2E8640(void *, float) __asm__("func_00214CC8");
extern void func_L00_00250800_2E8640(void *, int, void *) __asm__("func_L00_0024F7C8");
extern void func_001F9BF0_2E8640(void *, void *, void *) __asm__("func_001F9A28");
extern float func_001F9CE8_2E8640(void *) __asm__("func_001F9B20");
extern void func_00215C00_2E8640(void *, float, float, float) __asm__("func_00214DB0");
extern int func_0022ED80_2E8640(int, int, void *) __asm__("func_0022DA68");
extern int func_001FA898_2E8640(float) __asm__("func_001FA6D0");
extern void func_L04_002B0068_2E8640(void *, void *, void *, int) __asm__("func_L04_002AEE30");
extern int func_002140B0_2E8640(int) __asm__("func_00213260");
extern int func_L00_0025D6F0_2E8640(void *, void *) __asm__("func_L00_0025C698");
extern void func_L00_002584A8_2E8640(void *, int, int) __asm__("func_L00_00257470");
extern void func_L00_00263950_2E8640(void *, void *, int, float, float) __asm__("func_L00_002628D8");

#define M_U8(p, o) (*(unsigned char *)((char *)(p) + (o)))
#define M_U16(p, o) (*(unsigned short *)((char *)(p) + (o)))
#define M_I(p, o) (*(int *)((char *)(p) + (o)))
#define M_F(p, o) (*(float *)((char *)(p) + (o)))
#define M_P(p, o) (*(char **)((char *)(p) + (o)))

typedef int u128_2E8640_c2e72c0 __attribute__((mode(TI)));

#define COPY4(dst, src) (*(u128_2E8640_c2e72c0 *)(dst) = *(u128_2E8640_c2e72c0 *)(src))

void FUN_L12_002e72c0(char *moby) {
    char *data;
    char *hero;
    char *B;
    char *obj;
    char *row;
    char *arr;
    int *q;
    float buf[20] __attribute__((aligned(16)));
    float v50[4] __attribute__((aligned(16)));
    float v60[4] __attribute__((aligned(16)));
    float f20;
    float f21;
    float f;
    float g;
    int st;
    int n;
    int r;
    int i;

    data = M_P(moby, 0x78);
    func_L00_00264BB0_2E8640(data + 0x120, 3.0f);

    if (D_L12_0015F6A8_2E8640 == 2) {
        goto hide;
    }
    if (M_U8(moby, 0x20) >= 3) {
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        obj = M_P(data, 0x248);
        if (obj != 0) {
            M_U16(obj, 0x34) = M_U16(obj, 0x34) & 0xFFBE;
        }
    }
    r = M_I(data, 0x270);
    if (M_U8(data, 0x269) != 0) {
        B = D_0013E633_2E8640 + 0xE1D;
        if (M_I(B, 0x2084) == 0 && M_I(B, 0x2090) == 0x67) {
            if (func_001F9D48_2E8640(moby + 0x10, B + 0x80) < 30.0f) {
                goto finish;
            }
        }
    }
    if (r != -1) {
        if (func_00215570_2E8640(D_L12_001672C0_2E8640, r) == 0) {
        hide:
            M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x41;
            obj = M_P(data, 0x248);
            if (obj != 0) {
                M_U16(obj, 0x34) = M_U16(obj, 0x34) | 0x41;
            }
            return;
        }
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        obj = M_P(data, 0x248);
        if (obj != 0) {
            M_U16(obj, 0x34) = M_U16(obj, 0x34) & 0xFFBE;
        }
    }
    if (M_U8(data, 0x2E) == 2) {
        goto finish;
    }

    func_L12_002E9560_2E8640(moby);
    func_L00_00260D30_2E8640(moby, buf, M_F(data, 0x230));
    hero = M_P(buf, 0x40);
    if (hero == 0) {
        B = D_0013E633_2E8640 + 0xE1D;
        hero = M_P(B, 0x2080);
        COPY4(buf + 8, (float *)(B + 0xD0));
    }

    st = M_U8(moby, 0x20);
    if (st >= 10) {
        goto tail;
    }
    switch (st) {
    case 0:
        f = M_F(data, 0x230);
        M_F(data, 0x20) = 2.0f;
        *(short *)(data + 0x24) = 2;
        M_U8(data, 0x5A) = 8;
        M_F(data, 0x28C) = f;
        M_I(data, 0x90) = 0x200;
        M_F(data, 0x98) = 0.5f;
        M_U8(data, 0x58) = 8;
        M_U8(data, 0x2E) = 1;
        M_U8(data, 0x29) = 1;
        func_L01_0026E8E0_2E8640(data + 0xD0);
        f = D_0015EE6C_2E8640 * 5.5f;
        M_I(data, 0xD0) = 0x38D;
        M_F(data, 0xD8) = 1.0f;
        M_I(data, 0x108) = 0x18;
        M_F(data, 0xF4) = f;
        M_F(data, 0xDC) = 1.0f;
        COPY4((float *)(data + 0x220), (float *)(moby + 0x10));
        if (M_U8(moby, 0x21) != 0xFF) {
            q = *(int **)(data + 0x24C);
            if (*q == 1) {
                M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x8000;
            }
            q = *(int **)(data + 0x24C);
            *q = (*q == 0);
        } else {
            if (func_001160D8_2E8640() & 1) {
                M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x8000;
            }
        }
        if (M_I(data, 0x274) != 0) {
            r = func_L00_00258BC8_2E8640(0, 300);
            M_I(data, 0x288) = func_001F9850_2E8640(r);
        }
        M_I(moby, 0x40) = 0;
        M_I(moby, 0x44) = 0;
        if (M_F(data, 0x244) == M_F(moby, 0x40)) {
            if (M_U8(moby, 0x21) != 0xFF) {
                q = *(int **)(data + 0x250);
                M_F(data, 0x244) = (float)*q;
                r = *q;
                if (r > 0) {
                    *q = -r;
                } else {
                    *q = r + 0x14;
                }
            } else {
                M_F(data, 0x244) = func_L00_00258C80_2E8640(15.0f, 35.0f);
            }
        }
        M_F(data, 0x244) = M_F(data, 0x244) * 0.0174532924f;
        M_F(moby, 0x18) = func_00214358_2E8640(moby + 0x10, 0, 0.5f);
        if (M_U8(data, 0x268) != 0) {
            M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x41;
            obj = M_P(data, 0x248);
            if (obj != 0) {
                M_U16(obj, 0x34) = M_U16(obj, 0x34) | 0x41;
            }
        }
        M_U8(moby, 0x20) = 2;
        break;

    case 1:
        if (func_001F9908_2E8640(data + 0x258) == 0) {
            break;
        }
        if (M_U8(moby, 0x53) != 8 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
            func_00213F28_2E8640(moby, 8, 0, func_001F9850_2E8640(11), 5);
        }
        row = D_L12_001B0C30_2E8640[M_I(data, 0x254)];
        if (func_001F9D48_2E8640(moby + 0x10, row + (M_I(data, 0x260) << 4) + 0x10) < 3.0f) {
            M_I(data, 0x260) = M_I(data, 0x260) + 1;
        }
        n = *(int *)row - 1;
        if (n < M_I(data, 0x260)) {
            M_I(data, 0x260) = n;
        }
        func_L12_002E9B88_2E8640(moby, row + (M_I(data, 0x260) << 4) + 0x10, 0.0f, D_0015EE6C_2E8640 * 8.0f);
        if (M_I(data, 0x260) == *(int *)row - 1) {
            M_U8(moby, 0x20) = 3;
            break;
        }
        if (func_001F9D48_2E8640(moby + 0x10, hero + 0x10) < M_F(data, 0x264)) {
            M_U8(moby, 0x20) = 3;
        }
        break;

    case 2:
        if (func_001F9D10_2E8640(moby + 0x10, D_0013E633_2E8640 + 0xE9D) > 64.0f) {
            return;
        }
        if (M_U8(moby, 0x53) != 0) {
            if (M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_00213F28_2E8640(moby, 0, 0, func_001F9850_2E8640(8), 5);
            }
        }
        r = func_L12_0027C8D8_2E8640(moby, hero + 0x10, M_F(data, 0x230), 3.0f, 0.0f);
        if (r == 0) {
            if (M_I(data, 0x25C) != -1) {
                if (func_00215570_2E8640(hero + 0x10, M_I(data, 0x25C)) == 0) {
                    if (M_U8(moby, 0xBC) == 0) {
                        goto tail;
                    }
                }
            } else if (M_U8(moby, 0xBC) == 0) {
                goto tail;
            }
        }
        if (M_U16(moby, 0x34) & 1) {
            COPY4(v50, (float *)(moby + 0x10));
            v50[2] = v50[2] + 0.5f;
            if (func_L00_001EFFF0_2E8640(v50, D_L12_001672C0_2E8640, 2, moby, 0) != 0) {
                M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
                obj = M_P(data, 0x248);
                if (obj != 0) {
                    M_U16(obj, 0x34) = M_U16(obj, 0x34) & 0xFFBE;
                }
            } else {
                obj = M_P(data, 0x248);
                if (obj != 0) {
                    func_0020D678_2E8640(obj);
                }
                func_0020D678_2E8640(moby);
                return;
            }
        }
        if (M_U8(moby, 0x21) != 0xFF) {
            func_L01_0026F040_2E8640(M_U8(moby, 0x21), 1);
        }
        if (M_I(data, 0x254) == -1) {
            M_U8(moby, 0x20) = 3;
            break;
        }
        M_I(data, 0x260) = func_L01_0028C2D8_2E8640(moby + 0x10, D_L12_001B0C30_2E8640[M_I(data, 0x254)], 0.0f);
        M_U8(moby, 0x20) = 1;
        break;

    case 3:
        if (M_U8(data, 0x268) == 1) {
            COPY4(v50, (float *)(moby + 0x10));
            v50[0] = v50[0] + func_001F9F90_2E8640(M_F(moby, 0x48));
            v50[1] = v50[1] + func_001F9FA8_2E8640(M_F(moby, 0x48));
            n = func_L00_001F2BE8_2E8640(v50, 0, moby, 0, 1.5f);
            if (n != 0) {
                COPY4(v60, D_L12_0015F660_2E8640);
                if (n > 0) {
                    B = D_0013E633_2E8640 + 0xE1D;
                    i = 0;
                    do {
                        if (func_L00_0025F410_2E8640(D_L12_00178400_2E8640[i]) != 0) {
                            obj = D_L12_00178400_2E8640[i];
                            f = func_001F9B88_2E8640(M_F(moby, 0x18) - M_F(obj, 0x18));
                            if (f < 0.7f) {
                                func_L00_0025AC00_2E8640(D_L12_00178400_2E8640[i], M_P(B, 0x2080), 0x10000, moby + 0x10, v60, 1.0f);
                            }
                        }
                        i++;
                    } while (i < n);
                }
            }
        }
        f = func_001FA790_2E8640(func_L00_001FF860_2E8640(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)), M_F(moby, 0x48));
        if (f > 0.959931076f) {
            f = 0.959931076f;
        } else if (f < -0.959931076f) {
            f = -0.959931076f;
        }
        f = f * 0.5f;
        M_F(data, 0x208) = f;
        M_F(data, 0x188) = f;
        func_L12_002E9B88_2E8640(moby, hero + 0x10, M_F(data, 0x244), 0.0f);
        if (func_001F9D48_2E8640(moby + 0x10, hero + 0x10) < 5.5f || M_U8(data, 0x242) != 0) {
            f = func_001FA850_2E8640(M_F(moby, 0x48), func_L00_001FF860_2E8640(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)));
            if (f < 0.0872664601f) {
                M_U8(moby, 0x20) = 5;
                break;
            }
        }
        if (M_U8(moby, 0x53) != 8 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
            func_00213F28_2E8640(moby, 8, 0, func_001F9850_2E8640(11), 5);
        }
        goto tail;

    case 5:
        if (M_F(data, 0x28C) < 8.0f) {
            M_F(data, 0x28C) = 8.0f;
        }
        func_L12_002E9988_2E8640(moby, hero + 0x10);
        func_L12_002E9B30_2E8640(moby);
        if (M_U8(moby, 0x53) != 2 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
            func_00213F28_2E8640(moby, 2, 0, func_001F9850_2E8640(8), 5);
        }
        if (M_U8(moby, 0x70) & 2) {
            if (M_U8(moby, 0x53) != 10 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_00213F28_2E8640(moby, 10, 0, func_001F9850_2E8640(5), 5);
            }
            M_U8(moby, 0x20) = 6;
        }
        break;

    case 6:
        func_L12_002E9988_2E8640(moby, hero + 0x10);
        func_L12_002E9B30_2E8640(moby);
        if (M_P(data, 0x248) != 0) {
            if (func_00215B18_2E8640(moby, 0.0f) != 0) {
                func_L00_00250800_2E8640(M_P(data, 0x248), 0, v50);
                func_001F9BF0_2E8640(v60, buf + 8, v50);
                f21 = func_L00_001FF860_2E8640(v60[0], v60[1]);
                f20 = func_001FA748_2E8640(M_F(moby, 0x48), M_F(data, 0x208));
                if (func_001FA850_2E8640(f21, f20) > 0.226892799f) {
                    f21 = f20;
                }
                g = func_001F9CE8_2E8640(v60);
                g = func_L00_001FF860_2E8640(g, v60[2]);
                func_00215C00_2E8640(v60, D_0015EE6C_2E8640 * 15.0f, f21, g);
                func_0022ED80_2E8640(0, 0, moby);
                n = func_001FA898_2E8640(18.0f / (D_0015EE6C_2E8640 * 15.0f));
                func_L04_002B0068_2E8640(moby, v50, v60, n);
            }
        }
        if (M_U8(moby, 0x70) & 2) {
            if (M_U8(moby, 0x53) != 5 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_00213F28_2E8640(moby, 5, 0, func_001F9850_2E8640(8), 5);
            }
            M_U8(moby, 0x20) = 7;
        }
        break;

    case 7:
        func_L12_002E9988_2E8640(moby, hero + 0x10);
        func_L12_002E9B30_2E8640(moby);
        if (M_U8(moby, 0x53) != 5 || (M_U8(moby, 0x70) & 2)) {
            if (M_U8(moby, 0x53) != 0 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_00213F28_2E8640(moby, 0, 0, func_001F9850_2E8640(8), 5);
            }
        }
        if (func_002140B0_2E8640(9) == 0) {
            M_U8(moby, 0x20) = 5;
            break;
        }
        if (func_001F9D48_2E8640(moby + 0x10, hero + 0x10) > 10.5f && M_U8(data, 0x242) == 0) {
            M_U8(moby, 0x20) = 3;
            break;
        }
        f = func_001FA850_2E8640(M_F(moby, 0x48), func_L00_001FF860_2E8640(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)));
        if (f > 0.488692194f) {
            M_U8(moby, 0x20) = 3;
        }
        break;

    case 8:
        func_L12_002E9B88_2E8640(moby, data + 0x220, 0.0f, 0.0f);
        f = M_F(data, 0x23C) * 30.0f;
        if (f > 0.785398185f) {
            f = 0.785398185f;
        } else if (f < -0.785398185f) {
            f = -0.785398185f;
        }
        M_F(data, 0x188) = f * 0.699999988f;
        M_F(data, 0x208) = f * 0.400000006f;
        if (func_001F9D48_2E8640(moby + 0x10, data + 0x220) < 1.0f) {
            M_U8(moby, 0x20) = 2;
        }
        goto tail;

    case 4:
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        r = func_L00_0025D6F0_2E8640(moby, data + 0x70);
        if (r & 0x60) {
            if (func_001F9D48_2E8640(moby + 0x10, hero + 0x10) < 6.0f || M_U8(data, 0x242) != 0) {
                f = func_001FA850_2E8640(M_F(moby, 0x48), func_L00_001FF860_2E8640(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)));
                if (f < 0.122173049f) {
                    M_U8(moby, 0x20) = 5;
                    break;
                }
            }
            M_U8(moby, 0x20) = 3;
            break;
        }
        if (M_F(moby, 0x18) < 2.0f) {
            goto finish;
        }
        goto tail;

    case 9:
        r = func_L00_0025D6F0_2E8640(moby, data + 0x70);
        if (r & 0x60) {
            func_L00_002584A8_2E8640(moby, 0, -1);
            func_L12_002E8610_2E8640(moby, moby + 0x10);
            goto finish;
        }
        if (M_F(moby, 0x18) < 2.0f) {
            func_L00_002584A8_2E8640(moby, 0, -1);
            goto finish;
        }
        break;
    }

tail:
    if (M_U8(moby, 0x31) == 0) {
        obj = M_P(data, 0x248);
        if (obj == 0) {
            return;
        }
        if (M_U8(obj, 0x31) == 0) {
            return;
        }
    }
    func_L12_002E9AA0_2E8640(moby);
    if (M_U16(moby, 0x34) & 0x8000) {
        M_F(data, 0x188) = -M_F(data, 0x188);
        M_F(data, 0x208) = -M_F(data, 0x208);
    }
    func_L00_00263950_2E8640(moby, data + 0x120, 5, D_0015EE64_2E8640 * 0.0179999992f, D_0015EE64_2E8640 * 0.300000012f);
    func_L00_00263950_2E8640(moby, data + 0x1A0, 6, D_0015EE64_2E8640 * 0.0240000002f, D_0015EE64_2E8640 * 0.300000012f);
    return;

finish:
    func_L12_002E9A50_2E8640(moby);
}

#undef M_U8
#undef M_U16
#undef M_I
#undef M_F
#undef M_P
#undef COPY4
