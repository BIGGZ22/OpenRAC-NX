/* Carried from the PAL decompilation's src/overlays/shared/help_002274A8.c (matched C there) (games/rac1) through the version
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
extern unsigned char D_0013E533[];
/* functional: medium. Help-menu frame update (state g[0x20A4] 3 = the panel): runs the update chain, picks a random-based colour from sin/cos of D_L01_0015F5CC mod rand and applies it to the hero's tint (w+0x90), updates the 3 slots, the fade colour (g+0x22E4) and the sound/timer trigger (22DC/22DE). Float/clamp order hand-traced; not run. */
extern unsigned char D_0013D4D5[];
extern unsigned char D_0015EDB0[] MACRO_ADDR;
extern int D_L01_0015F5CC MACRO_ADDR;
extern int func_001F9688(int);
extern float D_L01_0017C04C;
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern float func_001F9DE0(float);
extern int func_001FA6E0(int, int, float);
extern void func_L00_00232670(void);
extern void func_L00_00211670(void);
extern void func_L01_0022CD48(void);
extern void func_L01_00242930(void);
extern void func_L00_00216F10(void);
extern void func_L00_002054E8(void);
extern void func_L00_00208A10(void);
extern void func_L00_00207C48(void);
extern void func_L00_00207A58(void);
extern void func_L00_002484E0(void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_L00_00207D00(void);
extern void func_L00_00216DE8(int, int);
extern void func_L00_00208B60(void);
extern void func_L00_00257024(void *);
typedef struct {
    char pad0[0x80];
    char a80[1];
    char pad1[0x300 - 0x81];
    int i300;
    char pad2[0x12E7 - 0x304];
    unsigned char b12E7;
    char pad3[0x1D50 - 0x12E8];
    char a1D50[0x30];
    char pad4[0x2080 - 0x1D80];
    char *p2080;
    char pad5[0x20A4 - 0x2084];
    unsigned char b20A4;
    char pad6[0x20B8 - 0x20A5];
    int i20B8;
    char pad7[0x22DC - 0x20BC];
    short s22DC;
    short s22DE;
    short s22E0;
    short pad22E2;
    int c22E4;
} GS_28598_c228000;
#define GG ((GS_28598_c228000 *)(D_0013E533 + 0xE1D))



void FUN_L01_00228000(void) {
    int h2, v2, t;
    int v, h, i, r16, r17, r18, c20, color;
    float f0, f20, ang;

    func_L00_00232670();
    func_L00_00211670();
    func_L01_0022CD48();
    func_L01_00242930();
    if (GG->b20A4 == 0) return;
    func_L00_00216F10();
    func_L00_002054E8();
    if (GG->b20A4 == 0) return;
    func_L00_00208A10();
    func_L00_00207C48();
    func_L00_00207A58();
    func_L00_002484E0(GG->a80);
    if (GG->b20A4 == 3) {
        if (D_0015EDB0[1] != 0) {
            D_L01_0017C04C = 1.9f;
        }
        c20 = 0x80;
        v = func_001F9688(0xAA);
        h = GG->s22E0;
        if (h == 2) {
            v = func_001F9688(0x32);
        } else if (h == 1) {
            v = func_001F9688(0x5A);
        }
        f0 = func_001FA6C0(v);
        ang = (float)(D_L01_0015F5CC % v) / f0;
        ang = ang + ang;
        ang = ang * 3.14159274f;
        f20 = func_001F9DE0(ang + -3.14159274f);
        r16 = func_001FA6D0(f20 * 20.0f) + 0x32;
        r17 = func_001FA6D0(f20 * 70.0f) + 0xB4;
        t = func_001FA6D0(f20 * 10.0f);
        *(unsigned short *)(GG->p2080 + 0x34) |= 0x10;
        if (r16 >= 0x100) r16 = 0xFF;
        if (r17 >= 0x100) r17 = 0xFF;
        r18 = t + 0x14;
        if (GG->s22E0 == 2) {
            color = (c20 << 24) | (r18 << 16) | (((r16 + (int)((unsigned int)r16 >> 31)) >> 1) << 8) | r17;
        } else {
            color = (c20 << 24) | (r18 << 16) | (r17 << 8) | r16;
        }
        *(int *)(GG->p2080 + 0x90) = func_001FA6E0(*(int *)(GG->p2080 + 0x90), color, 0.1f);
        for (i = 0; i < 3; i++) {
            func_L00_0024F7C8(GG->p2080, i, GG->a1D50 + i * 0x10);
        }
        c20 = 0x30;
        r18 = *(unsigned char *)(GG->p2080 + 0x92);
        r17 = *(unsigned char *)(GG->p2080 + 0x91);
        r16 = *(unsigned char *)(GG->p2080 + 0x90);
        h2 = GG->s22DE;
        if (h2 != 0) {
            f20 = func_001FA6C0(h2);
            f20 = f20 / func_001FA6C0(0x12);
            c20 = func_001FA6D0(f20 * (float)c20);
        }
        GG->c22E4 = (c20 << 24) | (r18 << 16) | (r17 << 8) | r16;
        func_L00_00207D00();
        GG->s22E0 = 0;
        if (GG->b20A4 == 3) {
            v2 = GG->i20B8;
            if (v2 != 0 && v2 != 0x1F) {
                if (GG->s22DE == 0) {
                    func_L00_00216DE8(0x19, 0);
                    GG->s22DE = 0x12;
                    GG->s22DC = 1;
                }
            } else {
                GS_28598_c228000 *g = GG;
                int e = g->b12E7;
                g->i20B8 = 0;
                if (e != 0 && g->i300 != 0 && *(unsigned char *)(D_0013D4D5 + 7) != 0 &&
                    g->s22DE == 0) {
                    func_L00_00216DE8(0x19, 0);
                    g->s22DE = 0x12;
                    g->s22DC = 1;
                }
            }
        }
    }
    func_L00_00208B60();
    func_L00_00257024(GG->p2080);
}
