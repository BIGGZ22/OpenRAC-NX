/* Carried from the PAL decompilation's nonmatching/functional/func_L01_002F0040.c (games/rac1/pal) through the version
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
/* functional: high. Same reads, writes, calls and arguments in the same order as retail (no COP2/VU0 code; the lq/sq pairs are plain 16-byte copies); differs only in compiler choices (registers, stack slots, folded offsets, branch layout), and the gp-relative globals are named by their addresses (_gp = 0x166C00). */
/* Level 1 / 11 (Novalis) shared moby helper, called at the end of func_L01_002EDCA0's update.
 * m = the moby, rec = m[0x78] its state record.
 *  1. Target: t = func_L00_0025A420(m, 0x330000, 0). When found, V = m's position + 0.5 in z, and the aim point W
 *     is the hero block (0x13F420) when t's moby class t[0x20]->[0xA6] is 0x47, V itself when that moby's
 *     pvar[0x46] is 5 or its class is 0xBA, else that moby's position (t's own position when t[0x20] is 0).
 *     If V-W is under 24 and the line test func_L00_001EFC70(V, W, 2, m, 0) hits, the target is dropped.
 *  2. func_L00_0025A478(m, t, rec+0x20, 0, &flag, &amount, 0, 4) applies the hit. Unless flag is 1, amount is 0
 *     or the state byte m[0x20] is 0xB/0xA/8: the moby is hurt (func_0022DA68, rec[0x20] -= 1, a flash with
 *     timer func_001FA6D0(rec[0x250]*0.8*1024)), knocked back (func_L01_002EF770 toward t's position), and
 *     while health rec[0x20] stays above 0 it spawns two debris mobys (func_L01_002EF350) whose angle from
 *     func_L00_0025AB48 is turned by pi/4 (func_001FA5C8, then func_001FA580), then calls
 *     func_L00_00257470(m, 0x800 or 0, -1) and
 *     func_0020C828(m); at 0 health it clears bit 0x1000 of m[0x34] and calls func_0020C828(m).
 *  3. Otherwise m[0xA4] = 0xFF; if rec[0x258] (a path) is set, the state byte is non-zero and m's z is below
 *     that path's height, it snaps back (func_001F99F8 + func_L01_002EF770) and calls func_0020C828.
 *     Else the per-frame search: the rec[0x22C] timer (reset to 240 when rec[0x38] asks), the search radius
 *     rec[0x228], the path test func_L00_0025FF38 against the path table D_L01_001B0930[rec[0x24C]] (mode
 *     rec[0x204] = 2 when lost), the target choice func_L01_002EF478(m, rec[0x248]) kept at rec[0x200] with
 *     mode 1 (default: the hero block's pointer at 0x1413D0), and the bob angle rec[0x254] whose func_001F9DE0 value sets
 *     m[0x90] through func_001FA6E0. */
typedef int Q_2F0040_c2eec68 __attribute__((mode(TI)));
typedef union { Q_2F0040_c2eec68 q; float f[4]; } U_2F0040_c2eec68;

extern char D_0013E633_2F0040[] __asm__("D_0013E533");
extern char *D_L01_001B0C30_2F0040[] __asm__("D_L01_001B0930");
extern int D_L01_00161350_2F0040 __asm__("D_L01_001612D0");
extern float D_0015EE6C_2F0040 __asm__("D_0015ED6C") MACRO_ADDR;
extern int D_L01_00160058_2F0040 __asm__("D_L01_0015FFD8") MACRO_ADDR;
extern int D_L01_00161A28_2F0040 __asm__("D_L01_00161998") MACRO_ADDR;
extern int D_L01_00161A2C_2F0040 __asm__("D_L01_0016199C") MACRO_ADDR;
extern float D_L01_00161A30_2F0040 __asm__("D_L01_001619A0") MACRO_ADDR;
extern float D_L01_00161A34_2F0040 __asm__("D_L01_001619A4") MACRO_ADDR;

extern char *func_L00_0025B478_2F0040(void *, int, int) __asm__("func_L00_0025A420");
extern float func_001F9D10_2F0040(void *, void *) __asm__("func_001F9B48");
extern int func_L00_001EFFF0_2F0040(void *, void *, int, void *, int) __asm__("func_L00_001EFC70");
extern void func_L00_0025B4D0_2F0040(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025A478");
extern void func_0022ED80_2F0040(int, int, void *) __asm__("func_0022DA68");
extern int func_001FA898_2F0040(float) __asm__("func_001FA6D0");
extern void func_L01_002F0B48_2F0040(void *, void *) __asm__("func_L01_002EF770");
extern char *func_L01_002F0728_2F0040(void *) __asm__("func_L01_002EF350");
extern void func_L00_0025BBA0_2F0040(void *, float *, void *, void *) __asm__("func_L00_0025AB48");
extern float func_001FA790_2F0040(float, float) __asm__("func_001FA5C8");
extern float func_001FA748_2F0040(float, float) __asm__("func_001FA580");
extern void func_L00_0025D5B0_2F0040(void *, void *, float, int, int, int) __asm__("func_L00_0025C558");
extern void func_L00_002584A8_2F0040(void *, int, int) __asm__("func_L00_00257470");
extern void func_0020D678_2F0040(void *) __asm__("func_0020C828");
extern void func_001F9BC0_2F0040(void *) __asm__("func_001F99F8");
extern int func_001F9850_2F0040(int) __asm__("func_001F9688");
extern int func_001F9908_2F0040(void *) __asm__("func_001F9740");
extern int func_L00_00260FB0_2F0040(float, void *, void *, int, int, void *, int) __asm__("func_L00_0025FF38");
extern float func_001F9D48_2F0040(void *, void *) __asm__("func_001F9B80");
extern float func_001F9B88_2F0040(float) __asm__("func_001F99C0");
extern char *func_L01_002F0850_2F0040(void *, int) __asm__("func_L01_002EF478");
extern float func_001F9FA8_2F0040(float) __asm__("func_001F9DE0");
extern int func_001FA8A8_2F0040(int, int, float) __asm__("func_001FA6E0");

void FUN_L01_002eec68(void *mp) {
    char *m = (char *)mp;
    char *d = *(char **)(m + 0x78);
    char *t;
    char *p;
    char *x;
    char *w;
    char *t2;
    char *p16;
    char *e;
    char *path;
    int cls;
    float dt;
    int c5;
    float near;
    float q;
    U_2F0040_c2eec68 V;
    U_2F0040_c2eec68 W;
    int flag;
    float amount = 0.0f;
    float ang28;
    float ang2c;

    t = func_L00_0025B478_2F0040(m, 0x330000, 0);
    if (t != 0) {
        V.q = *(Q_2F0040_c2eec68 *)(m + 0x10);
        V.f[2] = V.f[2] + 0.5f;
        p = *(char **)(t + 0x20);
        if (p == 0) {
            W.q = *(Q_2F0040_c2eec68 *)t;
        } else {
            cls = *(short *)(p + 0xA6);
            if (cls == 0x47) {
                W.q = *(Q_2F0040_c2eec68 *)(D_0013E633_2F0040 + 0xEED); /* hero block 0x13F420 */
            } else {
                x = *(char **)(p + 0x24);
                if ((x != 0 && *(short *)(x + 0x46) == 5) || cls == 0xBA) {
                    W.q = V.q;
                } else {
                    W.q = *(Q_2F0040_c2eec68 *)(p + 0x10);
                }
            }
        }
        if (func_001F9D10_2F0040(&V, &W) < 24.0f) {
            if (func_L00_001EFFF0_2F0040(&V, &W, 2, m, 0) != 0) {
                t = 0;
            }
        }
    }

    func_L00_0025B4D0_2F0040(m, t, d + 0x20, 0, &flag, &amount, 0, 4);
    if (flag != 1 && amount != 0.0f
        && *(unsigned char *)(m + 0x20) != 0xB
        && *(unsigned char *)(m + 0x20) != 0xA
        && *(unsigned char *)(m + 0x20) != 8) {
        float f12;

        func_0022ED80_2F0040(0, 0, m);
        f12 = *(float *)(d + 0x250) * 0.8f * 1024.0f;
        *(unsigned char *)(d + 0x9D) = 0;
        *(float *)(d + 0x70) = 0.008f; /* 0x3C03126F */
        *(int *)(d + 0x84) = 8;
        *(float *)(d + 0x20) = *(float *)(d + 0x20) - 1.0f;
        *(int *)(d + 0x80) = func_001FA898_2F0040(f12);
        *(float *)(d + 0x88) = *(float *)(d + 0x250) * 0.8f;
        func_L01_002F0B48_2F0040(m, t + 0x10);
        if (!(0.0f < *(float *)(d + 0x20))) {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
            func_0020D678_2F0040(m);
            return;
        }

        /* first debris moby (func_L01_002EF350), angle turned by func_001FA5C8 with pi/4 */
        t2 = func_L01_002F0728_2F0040(m);
        if (t2 != 0) {
            w = *(char **)(t2 + 0x78);
            *(unsigned char *)(w + 0x9D) = 3;
            *(float *)(w + 0x70) = 0.008f;
            *(int *)(w + 0x84) = 8;
            *(int *)(w + 0x80) = func_001FA898_2F0040(*(float *)(d + 0x250) * 0.8f * 1024.0f);
            *(float *)(w + 0x88) = *(float *)(d + 0x250) * 0.8f;
            dt = D_0015EE6C_2F0040; /* read once, before the stores below */
            *(float *)(w + 0x78) = D_L01_00161A30_2F0040 * dt;
            *(float *)(w + 0xB0) = 7.0f;
            *(float *)(w + 0xB4) = 13.0f;
            *(float *)(w + 0x7C) = D_L01_00161A34_2F0040 * dt;
            *(int *)(w + 0x84) = *(int *)(w + 0x84) | 4;
            V.q = *(Q_2F0040_c2eec68 *)(t + 0x10);
            func_L00_0025BBA0_2F0040(&V, &ang28, w + 0x78, w + 0x7C);
            ang28 = func_001FA790_2F0040(ang28, 0.7853982f); /* 0x3F490FDB */
            func_L00_0025D5B0_2F0040(t2, w + 0x60, ang28, 4, 8, 0);
            *(float *)(t2 + 0x58) = 1.0f;
            *(unsigned char *)(t2 + 0x20) = 8;
        }
        /* second debris moby, angle turned by func_001FA580 with pi/4 */
        t2 = func_L01_002F0728_2F0040(m);
        if (t2 != 0) {
            w = *(char **)(t2 + 0x78);
            *(unsigned char *)(w + 0x9D) = 3;
            *(float *)(w + 0x70) = 0.008f;
            *(int *)(w + 0x84) = 8;
            *(int *)(w + 0x80) = func_001FA898_2F0040(*(float *)(d + 0x250) * 0.8f * 1024.0f);
            *(float *)(w + 0x88) = *(float *)(d + 0x250) * 0.8f;
            dt = D_0015EE6C_2F0040; /* read once, before the stores below */
            *(float *)(w + 0x78) = D_L01_00161A30_2F0040 * dt;
            *(float *)(w + 0xB0) = 7.0f;
            *(float *)(w + 0xB4) = 13.0f;
            *(float *)(w + 0x7C) = D_L01_00161A34_2F0040 * dt;
            *(int *)(w + 0x84) = *(int *)(w + 0x84) | 4;
            V.q = *(Q_2F0040_c2eec68 *)(t + 0x10);
            func_L00_0025BBA0_2F0040(&V, &ang2c, w + 0x78, w + 0x7C);
            ang2c = func_001FA748_2F0040(ang2c, 0.7853982f); /* 0x3F490FDB */
            func_L00_0025D5B0_2F0040(t2, w + 0x60, ang2c, 4, 8, 0);
            *(float *)(t2 + 0x58) = 1.0f;
            *(unsigned char *)(t2 + 0x20) = 8;
        }

        p = *(char **)(t + 0x20);
        x = p != 0 ? *(char **)(p + 0x24) : 0;
        if (x != 0 && *(short *)(x + 0x46) == 5) {
            func_L00_002584A8_2F0040(m, 0x800, -1);
        } else {
            func_L00_002584A8_2F0040(m, 0, -1);
        }
        func_0020D678_2F0040(m);
        return;
    }

    *(unsigned char *)(m + 0xA4) = 0xFF;
    if (*(int *)(d + 0x258) != 0 && *(unsigned char *)(m + 0x20) != 0) {
        path = (char *)(*(int *)(d + 0x258) * 0x1190 + D_L01_00161350_2F0040);
        if (*(float *)(m + 0x18) < *(float *)(path + 8)) {
            func_001F9BC0_2F0040(&V);
            func_L01_002F0B48_2F0040(m, &V);
            func_0020D678_2F0040(m);
            return;
        }
    }

    if (*(int *)(d + 0x38) != 0) {
        *(int *)(d + 0x22C) = func_001F9850_2F0040(0xF0);
        *(int *)(d + 0x38) = 0;
    }
    if (func_001F9908_2F0040(d + 0x22C) != 0) {
        *(float *)(d + 0x228) = *(float *)(d + 0x224);
    } else {
        *(float *)(d + 0x228) = *(float *)(d + 0x224) + 20.0f;
    }

    if (*(int *)(d + 0x260) <= 0
        || *(unsigned char *)((*(int *)(d + 0x260) << 8) + D_L01_00160058_2F0040 + 0xBC) != 0) {
        e = D_L01_001B0C30_2F0040[*(int *)(d + 0x24C)];
        if (func_L00_00260FB0_2F0040(*(float *)(d + 0x228), m, d + 0x1C0, 0, 0, e + 0x10, *(int *)e) != 2) {
            if (*(float *)(d + 0x228) < func_001F9D48_2F0040(d + 0x210, d + 0x1C0)) {
                *(int *)(d + 0x204) = 2;
            } else if (3.0f < func_001F9B88_2F0040(*(float *)(m + 0x18) - *(float *)(d + 0x1C8))) {
                *(int *)(d + 0x204) = 2;
            }
        }
    }

    c5 = *(int *)(d + 0x248);
    if (c5 >= 0) {
        p16 = func_L01_002F0850_2F0040(m, c5);
        if (p16 != 0) {
            near = func_001F9D48_2F0040(m + 0x10, p16 + 0x10);
            if (near < *(float *)(d + 0x228)) {
                if (*(int *)(d + 0x204) == 2
                    || near < func_001F9D48_2F0040(m + 0x10, *(char **)(d + 0x200) + 0x10)) {
                    *(char **)(d + 0x200) = p16;
                    *(int *)(d + 0x204) = 1;
                }
            }
        }
    }

    if (*(int *)(d + 0x200) == 0) {
        *(int *)(d + 0x200) = *(int *)(D_0013E633_2F0040 + 0x2E9D); /* 0x1413D0 */
    }
    q = func_001FA748_2F0040(*(float *)(d + 0x254), D_0015EE6C_2F0040 * 9.424778f); /* 0x4116CBE4 */
    *(float *)(d + 0x254) = q;
    *(int *)(m + 0x90) = func_001FA8A8_2F0040(D_L01_00161A28_2F0040, D_L01_00161A2C_2F0040,
                                              (func_001F9FA8_2F0040(q) + 1.0f) * 0.5f);
}
