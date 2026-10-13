/* Carried from the PAL decompilation's nonmatching/functional/func_L05_002F9BA0.c (games/rac1/pal) through the version
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
extern f32 func_001F9D10_1edff8(void *, void *) __asm__("func_001F9B48");
extern void func_L05_0024CEE8(int, int);
extern int func_001F9688(int);
extern int func_L00_00203908(int a, int b);
extern void func_001F9A28(void *, void *, void *);
extern float func_001F9AF0(void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern float func_00213308(void);
extern f32 func_001F9DC8(f32);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_00257B90(int, int);
extern void func_L00_00269958(void *, void *, int, int, int, int, int, int);
extern void func_001F9A68(void *, void *, float);
extern void func_L00_0026CBB0(void *, void *, int, int, int, int, float);
extern int D_L05_0015F5CC MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern int D_L05_0015F6A8 MACRO_ADDR;
extern int D_L05_00161928;
extern int D_L05_0016190C;
extern int D_L05_00161910;
extern char D_0013E533[];
typedef struct {
    unsigned char pad0[0x19C];
    int f19C;
    unsigned char pad1A0[0x86C - 0x1A0];
    void *board;
    unsigned char pad870[0x208C - 0x870];
    int group;
} Hero_2F9BA0_c2f87a8;
extern Hero_2F9BA0_c2f87a8 G_2F9BA0 __asm__("D_0013F350");
extern unsigned char D_0013D255[];
extern unsigned char D_0014161B[];

/* FUN_L05_002f87a8 -- src/overlays/shared/vendor_002CF2C0.c (functional C for the port, not a match)
 * Hoverboard moby update (class 439, levels 05 and 16). State 0: when Ratchet comes within 5.5 (and
 * the story flag D_L05_0015F5CC is at least 4, and he is not in movement group 0x15 or 0x16) it
 * becomes his board (hero +0x86C), plays sound 0x6B, saves its position as home (pvars +0x10) and
 * goes to state 1. State 1: may play the 0x138F hint line; when Ratchet leaves group 0x16 it goes
 * back to 0, returning home unless pvars +0x50. Every frame it then works out its velocity from the
 * last position (pvars +0x00) and, when slow and not suppressed (+0xBC == 2), emits its hover
 * exhaust particles (two kinds while moving, +0xBC != 0).
 * From the staged near miss (nonmatching/shared/FUN_L05_002f87a8.c); fixed: state 0 saves the
 * position into pvars +0x10 (the near miss copied the other way), and the state tests.
 * equiv: NEAR (all copies: retail rebuilds moby + 0x10 where this keeps it). */
void FUN_L05_002f87a8(unsigned char *moby) {
    char *pos;
    char *data;
    char *g;
    char *gq;
    char v0[16];
    char v10[16];
    char v20[16];
    char v30[16];
    char v40[16];
    float f1;
    int one = 1;
    int st;

    pos = (char *)moby + 0x10;
    data = *(char **)(moby + 0x78);
    st = moby[0x20];
    if (st != 1) {
        if (st < 2) {
            if (st == 0) {
                g = D_0013E533 + 0xE9D;
                if (func_001F9D10_1edff8(g, pos) < 5.5f) {
                    if (D_L05_0015F5CC >= 4) {
                        int grp;
                        gq = g - 0x80;
                        grp = *(int *)(gq + 0x208C);
                        if (grp != 0x15) if (grp != 0x16) {
                            *(unsigned char **)(gq + 0x86C) = moby;
                            func_L05_0024CEE8(0x6B, 1);
                            if (*(int *)(data + 0x50) == 0) {
                                qcopy(data + 0x10, pos);
                            }
                            moby[0x20] = one;
                            moby[0x30] = 0xFF;
                        }
                    }
                }
            }
        }
    } else {
        if (*(int *)(data + 0x50) == 0) {
            if (D_0013D255[0x13B] != 0) {
                if (*(unsigned short *)(D_0014161B + 0x31D) == 0) {
                    if (D_L05_0015F6A8 != 0) {
                        int ret = func_001F9688(0x4B0);
                        if (ret < G_2F9BA0.f19C) {
                            if (*(unsigned short *)(D_0014161B + 0x645) == 0) {
                                func_L00_00203908(0x138F, 0x5F);
                            }
                        }
                    }
                }
            }
        }
        if (G_2F9BA0.group != 0x16) {
            moby[0x30] = 0x10;
            if (*(int *)(data + 0x50) != 0) {
                moby[0x20] = 0;
            } else {
                qcopy(pos, data + 0x10);
                moby[0x20] = 0;
            }
        }
    }

    func_001F9A28(v0, pos, data);
    qcopy(data, pos);
    f1 = func_001F9AF0(v0);
    if (moby[0xBC] != 2 && f1 < 2.0f) {
        float f20;
        float f21;
        func_L00_0024F7C8(moby, D_L05_0015F5CC & 1, v10);
        f21 = func_00213308();
        f20 = D_0015ED6C * 0.25f;
        func_L00_001FF500(v20, moby + 0xE0, f20 * func_001F9DC8(f21));
        func_L00_001FF500(v30, moby + 0xD0, f20 * func_001F9DE0(f21));
        func_001F9A10(v20, v20, v0);
        func_001F9A10(v20, v20, v30);
        qcopy(v20, v0);
        *(float *)(v20 + 0xC) = (float)D_L05_00161928;
        if (moby[0xBC] == 0) {
            int a;
            int b;
            int c;
            a = func_001F9688(10);
            b = func_001F9688(13);
            c = func_L00_00257B90(a, b);
            func_L00_00269958(v10, v20, D_L05_0016190C, D_L05_00161910, c, 15, 15, 1);
        } else {
            int a;
            int b;
            int c;
            func_001F9A68(v40, v0, 0.92f);
            a = func_001F9688(0xF);
            b = func_001F9688(0x12);
            c = func_L00_00257B90(a, b);
            func_L00_00269958(v10, v40, 0x5032F0D2, D_L05_00161910, c, 0x1E, 0x14, 1);
            a = func_001F9688(8);
            b = func_001F9688(0x11);
            c = func_L00_00257B90(a, b);
            func_L00_0026CBB0(v10, v20, D_L05_0016190C, D_L05_00161910, c, 1, 15000.0f);
        }
    }
    moby[0xBC] = 0;
}
