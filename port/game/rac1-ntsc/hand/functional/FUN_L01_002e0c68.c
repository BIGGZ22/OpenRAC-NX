/* Carried from the PAL decompilation's nonmatching/functional/func_L01_002E1FE0.c (games/rac1/pal) through the version
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
/* functional: medium. Same reads, writes, calls and arguments as retail; differs only in compiler structure (branch layout, register use), and the uninitialised y/z of one 3-float argument vector are zeroed on the host (retail leaves stack garbage there). */
/* Level 1 (Novalis) moby update/state machine, jump table of 6 states on moby[0x20], state record at
 * moby[0x78]. State 0 stores the moby's z-ish float (0x18) as the record's rest value and goes to 1; 1 is the
 * idle/"wait for Ratchet" state (when the save bit of the moby's id or the table byte says it was used, or
 * when Ratchet is near, facing it, grounded in state 8 with the hit probe and no other moby of flag 0x1000
 * within 3.25, it starts the 3 sequence: camera set-up, level sound, record fields; else it eases the rise,
 * and plays the table sound when the table entry is set); 2 does nothing; 3 is the "carry" state: it pulls
 * Ratchet's position toward the moby (eased velocity along record+0x1C, hero animation states 0x3F/0x40/0x41
 * sounds, gravity on the record's +0x24, yaw easing of moby+0x48) and advances progress record[0] from the
 * rotation; 4 lowers the moby toward the rest height then goes to 5; 5 sets progress to 1. Every path except
 * the "used" early exits ends with func_L01_002E0BD8(moby). With no state record it only turns the yaw. */
extern float D_0015EE6C_2E1FE0 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_0015EE64_2E1FE0 __asm__("D_0015ED64") MACRO_ADDR;
extern float D_0015EE70_2E1FE0 __asm__("D_0015ED70") MACRO_ADDR;
extern int D_0015EE84_2E1FE0 __asm__("D_0015ED84") MACRO_ADDR;
extern unsigned char D_0015EEB4_2E1FE0[] __asm__("D_0015EDB4") MACRO_ADDR;
extern char D_0014171B_2E1FE0[] __asm__("D_0014161B");
extern int D_L01_00167300_2E1FE0 __asm__("D_L01_00167280");
extern float D_L01_001672D8_2E1FE0 __asm__("D_L01_00167258");
extern char D_L01_001672C0_2E1FE0[] __asm__("D_L01_00167240");
extern char *D_L01_00160064_2E1FE0 __asm__("D_L01_0015FFE4");
extern char *D_L01_0016016C_2E1FE0 __asm__("D_L01_001600EC") MACRO_ADDR;
extern unsigned char D_L01_001BB9C0_2E1FE0[] __asm__("D_L01_001BB6B0");
extern char D_L01_001BADB0_2E1FE0[] __asm__("D_L01_001BAAA0");
extern char D_L01_001BAC60_2E1FE0[] __asm__("D_L01_001BA950");
extern float D_L01_00161930_2E1FE0 __asm__("D_L01_001618A0") MACRO_ADDR;

extern float func_001FA748_2E1FE0(float, float) __asm__("func_001FA580");
extern float func_001FA790_2E1FE0(float, float) __asm__("func_001FA5C8");
extern float func_001FA850_2E1FE0(float, float) __asm__("func_001FA688");
extern float func_001F9D48_2E1FE0(void *, void *) __asm__("func_001F9B80");
extern float func_L00_001FF860_2E1FE0(float, float) __asm__("func_L00_001FF8B0");
extern float func_001F9B88_2E1FE0(float) __asm__("func_001F99C0");
extern int func_L00_00267BA8_2E1FE0(int, int, int) __asm__("func_L00_00266D60");
extern int func_001F9850_2E1FE0(int) __asm__("func_001F9688");
extern int func_L01_0023D688_2E1FE0(int, int) __asm__("func_L01_0023CF98");
extern void func_L00_002EBF50_2E1FE0(void *, void *, int, int, int) __asm__("func_L00_002EAAA0");
extern void func_L00_002EBEE0_2E1FE0(void *) __asm__("func_L00_002EAA30");
extern void func_L00_002EBE88_2E1FE0(void *) __asm__("func_L00_002EA9D8");
extern void func_L01_00318260_2E1FE0(float, float) __asm__("func_L01_00316E88");
extern float func_00214D28_2E1FE0(float *, float, float) __asm__("func_00213ED8");
extern void func_L01_0029C2A0_2E1FE0(void *, int, void *, int, void *) __asm__("func_L01_0029B0A0");
extern void func_L00_002512D8_2E1FE0(int) __asm__("func_L00_002502A0");
extern void func_L00_00286128_2E1FE0(void *, void *) __asm__("func_L00_00284E50");
extern void func_L00_002EC0C8_2E1FE0(int) __asm__("func_L00_002EAC18");
extern float func_001F9CE8_2E1FE0(void *) __asm__("func_001F9B20");
extern void func_L00_001FF328_2E1FE0(void *, void *, float) __asm__("func_L00_001FF378");
extern void func_001FA218_2E1FE0(void *, void *) __asm__("func_001FA050");
extern void func_001F9EE8_2E1FE0(void *, void *, void *) __asm__("func_001F9D20");
extern float func_001F9F90_2E1FE0(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_2E1FE0(float) __asm__("func_001F9DE0");
extern float func_001F9C78_2E1FE0(void *, void *) __asm__("func_001F9AB0");
extern void func_L00_001FF4B0_2E1FE0(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_L00_00232C10_2E1FE0(int, int, float) __asm__("func_L00_002323B8");
extern void func_001F9BD8_2E1FE0(void *, void *, void *) __asm__("func_001F9A10");
extern void func_001F9BF0_2E1FE0(void *, void *, void *) __asm__("func_001F9A28");
extern void func_L00_0025C918_2E1FE0(float *, float *, float, float, float, float) __asm__("func_L00_0025B8C0");
extern void func_L00_0025CC58_2E1FE0(float, float, void *, int) __asm__("func_L00_0025BC00");
extern float func_00214358_2E1FE0(void *, int, float) __asm__("func_00213508");
extern void func_001F99D8_2E1FE0(void *, int) __asm__("func_001F9810");
extern int func_0022ED80_2E1FE0(int, int, void *) __asm__("func_0022DA68");
extern void func_L01_002E0BD8(char *);

#define F(p, o) (*(float *)((p) + (o)))
#define I(p, o) (*(int *)((p) + (o)))

void FUN_L01_002e0c68(char *m) {
    char *s = *(char **)(m + 0x78);
    char *H = (char *)0x13F350;
    char *HP = H + 0x80;
    char *HO;
    float V40[4], V50[4], V60[4], V70[4], V80[4], V90[4], VA0[4], W0[4];
    float d, a, f, f22, f23, g, ang, lim;
    int state, h, ok, i;
    char *p;

    if (s == 0) {
        F(m, 0x48) = func_001FA748_2E1FE0(F(m, 0x48), D_0015EE6C_2E1FE0 * 6.2831855f);
        return;
    }

    state = (unsigned char)m[0x20];
    if (I(H, 0x2084) != 0x3B) {
        if (state == 3) {
            m[0x20] = 1;
            if (D_L01_00167300_2E1FE0 == 0) {
                return;
            }
            func_L00_002EC0C8_2E1FE0(4);
            return;
        }
    }
    if (state >= 6) {
        goto done;
    }

    switch (state) {
    case 0:
        F(s, 0x3C) = F(m, 0x18);
        m[0x20] = 1;
        goto done;
    case 2:
        goto done;
    case 5:
        F(s, 0x0) = 1.0f;
        goto done;
    case 4:
        F(m, 0x18) = F(m, 0x18) - D_0015EE6C_2E1FE0 * 0.5f;
        g = F(s, 0x3C) - 0.5f;
        if (F(m, 0x18) <= g) {
            F(m, 0x18) = g;
            m[0x20] = 5;
            F(s, 0x0) = 1.0f;
        }
        goto done;
    case 1:
        h = *(unsigned short *)(m + 0xB2);
        if (D_L01_001BB9C0_2E1FE0[(short)h + 0x454] != 0 ||
            ((I(D_0014171B_2E1FE0, 0xAB75 + ((short)h >> 5) * 4 + (D_0015EE84_2E1FE0 << 8)) >> (h & 0x1F)) & 1)) {
            F(m, 0x18) = F(m, 0x18) - 0.5f;
            m[0x20] = 5;
            F(s, 0x0) = 1.0f;
            goto done;
        }
        d = func_001F9D48_2E1FE0(m + 0x10, HP);
        if (d < 1.75f) {
            if (H[0x20A4] == 0) {
                f = func_L00_001FF860_2E1FE0(F(m, 0x10) - F(H, 0x80), F(m, 0x14) - F(H, 0x84));
                a = func_001FA850_2E1FE0(F(H, 0x98), f);
                if (a < 1.5707964f) {
                    g = func_001F9B88_2E1FE0((F(H, 0x88) - 0.3f) - F(m, 0x18));
                    if (g < 0.2f && I(H, 0x10B8) == 8 && I(H, 0x1090) != 0) {
                        int r = func_L00_00267BA8_2E1FE0(0x80, func_001F9850_2E1FE0(7), 0);
                        if (r != 0 && (unsigned int)func_001F9850_2E1FE0(0x1E) < (unsigned int)I(s, 0x14) &&
                            F(m, 0x18) == F(s, 0x3C)) {
                            if (F(s, 0x0) < 1.0f) {
                                ok = 1;
                                I(s, 0x14) = 0;
                                D_L01_00161930_2E1FE0 = 0.0f;
                                for (p = D_L01_00160064_2E1FE0; p != 0; p = *(char **)(p + 0x28)) {
                                    if (*(unsigned short *)(p + 0x34) & 0x1000) {
                                        if (func_001F9D48_2E1FE0(m + 0x10, p + 0x10) < 3.25f) {
                                            ok = 0;
                                            break;
                                        }
                                    }
                                }
                                if (ok == 0) {
                                    goto done;
                                }
                                func_L01_0023D688_2E1FE0(0x3B, 1);
                                func_L00_002EBF50_2E1FE0(D_L01_001672C0_2E1FE0, D_L01_001672C0_2E1FE0 + 0x10, 3,
                                                         func_001F9850_2E1FE0(0xB4), 0);
                                func_L00_002EBEE0_2E1FE0(D_L01_0016016C_2E1FE0 + (I(s, 0x20) << 7) + 0x70);
                                func_L00_002EBE88_2E1FE0(D_L01_0016016C_2E1FE0 + (I(s, 0x20) << 7) + 0x30);
                                func_L01_00318260_2E1FE0(F(s, 0x28), F(s, 0x2C));
                                m[0x20] = 3;
                                I(s, 0x24) = 0;
                                F(s, 0x18) = F(s, 0x0) * (F(s, 0x30) * 6.2831855f);
                                goto done;
                            }
                        }
                    }
                }
            }
        }
        if (F(s, 0x0) < 1.0f) {
            I(s, 0x14) = I(s, 0x14) + 1;
            func_00214D28_2E1FE0((float *)s, 0.0f, 1.0f / (float)func_001F9850_2E1FE0(0x3C));
            if (0.0f < F(s, 0x0)) {
                F(m, 0x48) = func_001FA790_2E1FE0(F(m, 0x48),
                                                  F(s, 0x30) * 6.2831855f / (float)func_001F9850_2E1FE0(0x3C));
            }
            F(m, 0x18) = F(m, 0x18) + D_0015EE6C_2E1FE0 * 0.5f;
            if (F(s, 0x3C) <= F(m, 0x18)) {
                F(m, 0x18) = F(s, 0x3C);
            }
        } else {
            if (I(s, 0x38) == 0) {
                goto done;
            }
            if (I(s, 0x34) != -1 &&
                (unsigned char)(D_0014171B_2E1FE0 + 0xAA35)[(unsigned char)m[0xB0] + (D_0015EE84_2E1FE0 << 4)] != 0xFF) {
                func_L01_0029C2A0_2E1FE0(s, 4, m, 2, D_L01_001BADB0_2E1FE0);
                func_L00_002512D8_2E1FE0((unsigned char)m[0xB0]);
                if (I(s, 0x34) != -1) {
                    char *b = D_L01_0016016C_2E1FE0 + (I(s, 0x34) << 7);
                    func_L00_00286128_2E1FE0(b + 0x30, b + 0x70);
                }
            }
            F(m, 0x18) = F(m, 0x18) - D_0015EE6C_2E1FE0 * 0.5f;
            g = F(s, 0x3C) - 0.5f;
            if (F(m, 0x18) <= g) {
                F(m, 0x18) = g;
            }
        }
        goto done;
    case 3:
        V80[0] = V80[1] = V80[2] = V80[3] = 0.0f;
        V80[2] = D_L01_001672D8_2E1FE0;
        V70[0] = V80[0]; V70[1] = V80[1]; V70[2] = V80[2]; V70[3] = V80[3];
        f23 = func_L00_001FF860_2E1FE0(F(H, 0x80) - F(m, 0x10), F(H, 0x84) - F(m, 0x14));
        {
            int r = func_L00_00267BA8_2E1FE0(0x80, func_001F9850_2E1FE0(7), 0);
            int go = 0;
            if (r != 0 && func_001F9850_2E1FE0(0x3C) < I(H, 0x198)) {
                go = 1;
            } else if (F(s, 0x0) == 1.0f) {
                go = 1;
            }
            if (go) {
                func_L01_0023D688_2E1FE0(0, 1);
                if (I(s, 0x38) != 0) {
                    m[0x20] = 1;
                } else if (F(s, 0x0) == 1.0f) {
                    func_L01_0029C2A0_2E1FE0(s, 4, m, 2, D_L01_001BADB0_2E1FE0);
                    m[0x20] = 4;
                    h = *(unsigned short *)(m + 0xB2);
                    I(D_0014171B_2E1FE0, 0xAB75 + ((short)h >> 5) * 4 + (D_0015EE84_2E1FE0 << 8)) |= 1 << (h & 0x1F);
                    h = *(unsigned short *)(m + 0xB2);
                    I(D_L01_001BAC60_2E1FE0, ((short)h >> 5) * 4) |= 1 << (h & 0x1F);
                    func_0022ED80_2E1FE0(0, 0, m);
                    if (I(s, 0x34) != -1 &&
                        (unsigned char)(D_0014171B_2E1FE0 + 0xAA35)[(unsigned char)m[0xB0] + (D_0015EE84_2E1FE0 << 4)] != 0xFF) {
                        func_L01_0029C2A0_2E1FE0(s, 4, m, 2, D_L01_001BADB0_2E1FE0);
                        func_L00_002512D8_2E1FE0((unsigned char)m[0xB0]);
                        if (I(s, 0x34) != -1) {
                            char *b = D_L01_0016016C_2E1FE0 + (I(s, 0x34) << 7);
                            func_L00_00286128_2E1FE0(b + 0x30, b + 0x70);
                        }
                    }
                }
                func_L00_002EC0C8_2E1FE0(4);
                return;
            }
        }
        V40[0] = -F(H, 0x1D24);
        V40[1] = -F(H, 0x1D20);
        d = func_001F9CE8_2E1FE0(V40);
        if (0.0f < d) {
            func_L00_001FF328_2E1FE0(V40, V40, d);
        }
        func_001FA218_2E1FE0(W0, V70);
        func_001F9EE8_2E1FE0(V40, V40, W0);
        HO = *(char **)(H + 0x2080);
        V60[0] = func_001F9F90_2E1FE0(F(HO, 0x48));
        HO = *(char **)(H + 0x2080);
        V60[1] = func_001F9FA8_2E1FE0(F(HO, 0x48));
        V60[2] = 0.0f;
        d = func_001F9C78_2E1FE0(V60, V40);
        f = D_0015EE70_2E1FE0 * 5.5f;
        g = d - F(s, 0x1C);
        if (f < g) g = f;
        if (g < -f) g = -f;
        F(s, 0x1C) = F(s, 0x1C) + g;
        lim = D_0015EE6C_2E1FE0 * 3.7f;
        if (lim < F(s, 0x1C)) F(s, 0x1C) = lim;
        if (F(s, 0x1C) < 0.0f) F(s, 0x1C) = 0.0f;
        if (func_001F9B88_2E1FE0(F(s, 0x1C)) < D_0015EE70_2E1FE0 * 5.5f * 0.25f) F(s, 0x1C) = 0.0f;
        func_L00_001FF4B0_2E1FE0(V50, V60, F(s, 0x1C));

        HO = *(char **)(H + 0x2080);
        if (I(H, 0xA98) & 2) {
            if ((unsigned char)HO[0x53] == 0x44) {
                func_L00_00232C10_2E1FE0(0x3F, 0, (float)func_001F9850_2E1FE0(7));
            }
        }
        HO = *(char **)(H + 0x2080);
        if ((unsigned char)HO[0x53] != 0x44) {
            if (I(H, 0xA9C) == 0) {
                HO = *(char **)(H + 0x2080);
                if ((unsigned char)HO[0x52] == 0x40 && func_001F9B88_2E1FE0(F(s, 0x1C)) == 0.0f) {
                    func_L00_00232C10_2E1FE0(0x3F, 0, (float)func_001F9850_2E1FE0(0xE));
                } else {
                    HO = *(char **)(H + 0x2080);
                    if ((unsigned char)HO[0x52] == 0x3F) {
                        if (0.0f < F(s, 0x1C)) {
                            func_L00_00232C10_2E1FE0(0x40, 0, (float)func_001F9850_2E1FE0(0xE));
                        }
                    } else if ((unsigned char)HO[0x52] == 0x40) {
                        if (D_0015EE6C_2E1FE0 * 3.7f * 0.6f < F(s, 0x1C)) {
                            func_L00_00232C10_2E1FE0(0x41, 0, (float)func_001F9850_2E1FE0(0xE));
                        }
                    } else if ((unsigned char)HO[0x52] == 0x41) {
                        if (F(s, 0x1C) < D_0015EE6C_2E1FE0 * 3.7f * 0.4f) {
                            func_L00_00232C10_2E1FE0(0x40, 0, (float)func_001F9850_2E1FE0(0x10));
                        }
                    }
                }
            }
        }

        g = F(H, 0x88);
        func_001F9BD8_2E1FE0(HP, HP, V50);
        f22 = func_L00_001FF860_2E1FE0(F(H, 0x80) - F(m, 0x10), F(H, 0x84) - F(m, 0x14));
        V50[0] = func_001F9F90_2E1FE0(f22) * 1.05f;
        V50[1] = func_001F9FA8_2E1FE0(f22) * 1.05f;
        V50[2] = 0.0f;
        func_001F9BD8_2E1FE0(V80, m + 0x10, V50);
        func_001F9BF0_2E1FE0(V90, V80, HP);
        V90[2] = 0.0f;
        d = func_001F9CE8_2E1FE0(V90);
        VA0[0] = -d;
        VA0[1] = 0.0f;
        VA0[2] = 0.0f;
        VA0[3] = 0.0f;
        func_L00_0025C918_2E1FE0(VA0, &D_L01_00161930_2E1FE0, 0.0f, D_0015EE64_2E1FE0 * 0.015f,
                                 D_0015EE64_2E1FE0 * 0.3f, D_0015EE6C_2E1FE0 * 7.0f);
        func_L00_001FF4B0_2E1FE0(V90, V90, D_L01_00161930_2E1FE0);
        func_001F9BD8_2E1FE0(HP, HP, V90);
        F(H, 0x88) = g;
        ang = D_0015EEB4_2E1FE0[1] ? -1.5707964f : 1.5707964f;
        F(H, 0x180) = func_001FA748_2E1FE0(ang, f22);
        func_L00_0025CC58_2E1FE0(F(H, 0x180), D_0015EE6C_2E1FE0 * 6.2831855f, H + 0x98, 0);
        F(s, 0x24) = F(s, 0x24) + D_0015EE70_2E1FE0 * 9.8f;
        F(H, 0x88) = F(H, 0x88) - (F(s, 0x24) - 2.0f);
        g = func_00214358_2E1FE0(HP, 0, 0.5f);
        F(H, 0x88) = F(H, 0x88) - 2.0f;
        if (F(H, 0x88) < g) {
            F(H, 0x88) = g;
            I(s, 0x24) = 0;
        }
        HO = *(char **)(H + 0x2080);
        if ((unsigned char)HO[0x53] != 0x44) {
            float hi = 1.0471976f;
            float lo = func_001FA748_2E1FE0(0.2617994f, f22);
            func_L00_0025CC58_2E1FE0(lo, D_0015EE6C_2E1FE0 * 3.1415927f, m + 0x48, 0);
            if (hi < func_001FA790_2E1FE0(F(m, 0x48), lo)) {
                F(m, 0x48) = func_001FA790_2E1FE0(F(m, 0x48), hi);
            } else if (func_001FA790_2E1FE0(F(m, 0x48), lo) < -1.0471976f) {
                F(m, 0x48) = func_001FA748_2E1FE0(F(m, 0x48), hi);
            }
        }
        f = func_001FA850_2E1FE0(f22, f23);
        F(s, 0x18) = F(s, 0x18) + f;
        F(s, 0x0) = F(s, 0x18) / (F(s, 0x30) * 6.2831855f);
        if (1.0f < F(s, 0x0)) F(s, 0x0) = 1.0f;
        if (F(s, 0x0) < 0.0f) F(s, 0x0) = 0.0f;
        func_001F99D8_2E1FE0(H + 0xE0, 0x90);
        goto done;
    }
done:
    func_L01_002E0BD8(m);
}
