/* NON_MATCHING func_L00_002AE278 -- src/overlays/shared/vendor_002AB910.c
 * Best so far: BYTES 682/1076 (36.6% of the bytes match), checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: the hero through Hero (state.control_mode, ground_moby, unk670/674, motion.pos, motion.unk100,
 *   moby, state.current) and the first test turned round as in retail: `if (control_mode == 0xF ||
 *   (ground_moby && func_L00_0025D390(ground_moby)))` takes the short unk100 branch first, and p60 is read
 *   only in the else branch. Size 1076 = retail, BYTES 682/1076 (registers). Left: retail re-reads d + 0x60
 *   before the first call and computes e's %hi in both branches, and keeps speed in f20 and the dot product in
 *   f21 (ours swaps them and saves f22). Re-reading d + 0x60 everywhere gives 1092, dropping e gives 1084.
 */
#include "hero.h"
extern Hero hero_2AE278 __asm__("D_0013F450");
extern char D_0013E15A[];
extern char D_0013A5E0[];
extern char D_L00_00173F60[];
extern float D_0015EE70 MACRO_ADDR;

extern int func_001F9850(int);
extern unsigned char *func_L00_0025D390(unsigned char *);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9CE8(void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_001F9C78(void *, void *);
extern float func_001F9D48(void *, void *);
extern void func_001F9C30(void *, void *, float);
extern float func_L00_001FF860(float, float);
extern float func_001FA850(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001FA888(int);
extern void func_L00_002C3790(float *, float *, float *, float, float, float);
extern int func_L00_001EFFF0(void *, void *, int, int, int);

typedef union {
    u128 q;
    float f[4];
} V2AE278 __attribute__((aligned(16)));

/* Steers moby m toward pos and vel, blending its data at 0x78 with the level's global state. */
void func_L00_002AE278(char *pos, char *m, char *vel) {
    char *d;
    char *e;
    char *p;
    char *p60;
    char *p64;
    V2AE278 A, B, C, D, E;
    float f20, f21, t, z;
    short h;
    int ok;

    d = *(char **)(m + 0x78);
    *(short *)(d + 0x6A) = func_001F9850(0x1E);
    m[0x20] = 1;
    qcopy(d, vel);
    e = D_0013E15A + 0x4C6;

    if (hero_2AE278.state.control_mode == 0xF
        || (hero_2AE278.ground_moby != 0 && func_L00_0025D390((unsigned char *)hero_2AE278.ground_moby) != 0)) {
        func_001F9BD8(d, d, &hero_2AE278.motion.displacement);
    } else {
        p60 = *(char **)(d + 0x60);
        if (p60 != 0) {
            h = *(short *)(p60 + 0xA6);
            if (h == 0x56E || h == 0x58E) {
                func_001F9BD8(d, d, d + 0x40);
            } else {
                p64 = *(char **)(d + 0x64);
                if (p64 != 0) {
                    f20 = func_001F9CE8(vel);
                    func_001F9BF0(&A, &hero_2AE278.motion.pos, p60 + 0x10);
                    func_L00_001FF4B0(&A, &A, 1.0f);
                    f21 = func_001F9C78(&A, d + 0x40);
                    f20 = f21 + f20;
                    t = func_001F9D48(pos, p60 + 0x10) / f20;
                    if (0.0f < f21) {
                        t = t * f21;
                        func_001F9C30(&B, &A, t);
                    } else {
                        func_001F9C30(&B, d + 0x40, t);
                    }
                    func_001F9BD8(&D, &B, p60 + 0x10);
                    B.q = D.q;
                    t = func_001FA850(func_L00_001FF860(hero_2AE278.unk670, hero_2AE278.unk674),
                                      func_L00_001FF860(B.f[0] - hero_2AE278.motion.pos.f[0], B.f[1] - hero_2AE278.motion.pos.f[1]));
                    if (1.5707964f < t) {
                        f20 = func_001F9F90(func_L00_001FF860(hero_2AE278.unk670, hero_2AE278.unk674));
                        f20 = f20 * func_001F9D48(&hero_2AE278.motion.pos, &B);
                        f20 = f20 * 0.5f;
                        D.f[0] = f20;
                        f20 = func_001F9FA8(func_L00_001FF860(hero_2AE278.unk670, hero_2AE278.unk674));
                        f20 = f20 * func_001F9D48(&hero_2AE278.motion.pos, &B);
                        f20 = f20 * 0.5f;
                        D.f[1] = f20;
                        *(int *)&D.f[2] = 0;
                        func_001F9BD8(&E, &D, &hero_2AE278.motion.pos);
                        B.q = E.q;
                    }
                    f20 = D_0015EE70 * 11.0f;
                    if ((*(int *)(D_0013A5E0 + 0x2600) & 5) != 0) {
                        f21 = func_001FA888(*(unsigned char *)(e + 0xA));
                        z = f21 * 2.5f + 8.5f;
                    } else {
                        z = 8.5f;
                    }
                    func_L00_002C3790((float *)pos, B.f, C.f, f20, 1e-5f, z);
                    func_001F9C30(&C, &C, 0.65f);
                    func_001F9C30(d, d, 0.35f);
                    func_001F9BD8(&D, d, &C);
                    *(u128 *)d = D.q;
                }
            }
        }
    }
    *(short *)(d + 0x54) = func_001F9850(0x12C);
    qcopy(m + 0x10, pos);
    p = (char *)hero_2AE278.moby;
    A.q = *(u128 *)(p + 0x10);
    A.f[2] = *(float *)(pos + 8);
    ok = func_L00_001EFFF0(&A, pos, 0, (int)hero_2AE278.moby, 0);
    if (ok) {
        m[0xBC] = 1;
        qcopy(m + 0x10, D_L00_00173F60);
    }
    if (*(unsigned char *)(e + 0xA)) {
        if (hero_2AE278.state.current == 1) {
            z = *(float *)(d + 8);
            func_001F9C30(d, d, 1.5f);
            *(float *)(d + 8) = z / 1.5f;
        }
    }
}
