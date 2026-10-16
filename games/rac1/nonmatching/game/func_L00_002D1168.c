/* The crates' stacking init (US level 01 FUN_002eac18), for the crate m and its group (byte +0x21; 0xFF:
   none). Every crate: flag 4 in its pvar +0xAC; a class-500 crate may turn a quarter (rotation z) on a
   random draw when it is not tilted; a class-501 crate whose mission byte (+0xB0, not 0xFF) has a death
   count (D_0014171B + 0xD875 table) below pvar +0xF8 is deleted. Then it settles: pvar +0xC4 = uid,
   +0xB0 = 1.0, flag 1, and a line from half a unit above it down to z = 0.1 (CollLine flags 2) finds
   what is below: the ground (z = the hit), a crate of its group (it sits on it: its position = that
   crate's + its up row, below/above links pvar +0xA4 / +0xA0), or a carrier (it records its pose in the
   carrier's frame, func_L00_002616E0, pvar +0xD0 / +0xE0 / +0xF0 / +0xF4). Loop 2 does that for the
   bottom crates of the group and passes the carrier and the ammo-crate path counter (D_L00_001B0830) up
   each stack; loop 3 measures distances whose results are not used. Written from the PAL assembly;
   ReRAC (crates/rc-game/src/moby_update/classes/crate_.rs stack_init; ISC License, Copyright (c) 2026
   ReRAC contributors) as the reference. In the lone-crate branch retail passes a stale stack word as
   func_L00_002616E0's first argument; this passes 0. */
extern int func_002140B0_D1168(int) __asm__("func_002140B0");
extern void func_0020D678_D1168(void *) __asm__("func_0020D678");
extern int func_L00_0025F410_D1168(void *) __asm__("func_L00_0025F410");
extern int func_L00_0025D390_D1168(void *) __asm__("func_L00_0025D390");
extern int func_L00_002616E0_D1168(void *, void *, void *, void *, void *, void *) __asm__("func_L00_002616E0");
extern int func_L00_0025A208_D1168(void *, int, int, int) __asm__("func_L00_0025A208");
extern int func_L00_0025A2F0_D1168(void *, void *, int, int) __asm__("func_L00_0025A2F0");
extern int func_L00_001EFFF0_D1168(void *, void *, int, void *, int) __asm__("func_L00_001EFFF0");
extern void func_001F9978_D1168(void) __asm__("func_001F9978");
extern void func_001F9BD8_D1168(void *, void *, void *) __asm__("func_001F9BD8");
extern float func_001F9D10_D1168(void *, void *) __asm__("func_001F9D10");
extern float func_001FA748_D1168(float, float) __asm__("func_001FA748");
extern char D_L00_00173F40_D1168[] __asm__("D_L00_00173F40");
extern char D_0014171B_D1168[] __asm__("D_0014171B");
extern char *D_L00_001B0830_D1168[] __asm__("D_L00_001B0830");

void func_L00_002D1168(char *m0) {
    float a[4] __attribute__((aligned(16)));
    float b[4] __attribute__((aligned(16)));
    char *w0;
    char *w1;
    char *w2;
    char *c;
    char *pv;
    char *hm;
    char *bp;
    char *path;
    char *out = D_L00_00173F40_D1168;
    int *table = (int *)(D_0014171B_D1168 + 0xD875);
    int group;
    int lone;
    int k;
    int n;
    int i;
    union { unsigned int u; float f; } h;

    w0 = m0;
    pv = *(char **)(m0 + 0x78);
    group = *(unsigned char *)(m0 + 0x21);
    *(int *)(pv + 0xAC) |= 4;
    lone = group == 0xFF;
    if (!lone) {
        func_L00_0025A208_D1168(&w0, group, 0, 0);
    }
    while (w0 != 0) {
        c = w0;
        pv = *(char **)(c + 0x78);
        if (!lone && !func_L00_0025F410_D1168(c)) {
            goto next1;
        }
        if (*(short *)(c + 0xA6) == 0x1F4 && func_002140B0_D1168(2) != 0 && *(float *)(c + 0x40) == 0.0f &&
            *(float *)(c + 0x44) == 0.0f) {
            h.u = 0x3FC90FDB;
            *(float *)(c + 0x48) = func_001FA748_D1168(*(float *)(c + 0x48), h.f);
        }
        if (*(short *)(c + 0xA6) == 0x1F5 && *(unsigned char *)(c + 0xB0) != 0xFF &&
            table[*(unsigned char *)(c + 0xB0)] < *(int *)(pv + 0xF8)) {
            func_0020D678_D1168(c);
            if (lone) {
                return;
            }
            goto next1;
        }
        *(unsigned short *)(pv + 0xC4) = *(unsigned short *)(c + 0xA8);
        *(float *)(pv + 0xB0) = 1.0f;
        *(int *)(pv + 0xAC) |= 1;
        for (i = 0; i < 4; i++) {
            a[i] = ((float *)(c + 0x10))[i];
            b[i] = ((float *)(c + 0x10))[i];
        }
        h.u = 0x3DCCCCCD;
        b[2] = h.f;
        a[2] = a[2] + 0.5f;
        if (func_L00_001EFFF0_D1168(a, b, 2, 0, 0) == 0) {
            *(int *)(pv + 0xA4) = 0;
            if (lone) {
                return;
            }
            goto next1;
        }
        hm = *(char **)(out + 0x18);
        if (hm == 0) {
            *(float *)(c + 0x18) = *(float *)(out + 0x28);
            *(int *)(pv + 0xA4) = 0;
            if (lone) {
                return;
            }
            goto next1;
        }
        if (lone) {
            if (func_L00_0025F410_D1168(hm)) {
                return;
            }
            *(int *)(pv + 0xA4) = 0;
            *(int *)(pv + 0xAC) |= 8;
            if (!func_L00_0025D390_D1168(*(char **)(out + 0x18))) {
                return;
            }
            if (w0 != 0) {
                *(short *)(w0 + 0x36) = 0x7F80;
            }
            func_L00_002616E0_D1168(0, *(char **)(out + 0x18), c + 0x10, c + 0x40, pv + 0xD0, pv + 0xE0);
            *(int *)(pv + 0xF4) = 0;
            *(char **)(pv + 0xF0) = *(char **)(out + 0x18);
            return;
        }
        if (!func_L00_0025F410_D1168(hm) &&
            (unsigned short)(*(unsigned short *)(*(char **)(out + 0x18) + 0xA6) - 0x128) < 2 == 0) {
            *(int *)(pv + 0xA4) = 0;
            *(unsigned char *)(c + 0x30) = 0xFF;
            *(int *)(pv + 0xAC) |= 8;
            goto next1;
        }
        hm = *(char **)(out + 0x18);
        if (*(char **)(hm + 0x78) == 0 || *(unsigned char *)(hm + 0x21) != *(unsigned char *)(c + 0x21)) {
            *(int *)(pv + 0xA4) = 0;
            goto next1;
        }
        if (!func_L00_0025F410_D1168(hm)) {
            *(int *)(pv + 0xCC) = 0;
            *(int *)(pv + 0xA4) = 0;
            *(float *)(c + 0x18) = *(float *)(out + 0x28);
            goto next1;
        }
        *(char **)(pv + 0xA4) = *(char **)(out + 0x18);
        hm = *(char **)(out + 0x18);
        for (i = 0; i < 4; i++) {
            ((float *)(c + 0x10))[i] = ((float *)(hm + 0x10))[i];
        }
        func_001F9BD8_D1168(c + 0x10, c + 0x10, hm + 0xE0);
        bp = *(char **)(*(char **)(out + 0x18) + 0x78);
        if (func_L00_0025F410_D1168(w0)) {
            *(char **)(bp + 0xA0) = w0;
        } else {
            func_001F9978_D1168();
            *(int *)(bp + 0xA0) = 0;
        }
    next1:
        func_L00_0025A2F0_D1168(&w0, w0, 0, 0);
    }

    /* Loop 2: the bottom crates, and up each stack. */
    func_L00_0025A208_D1168(&w1, group, 0, 0);
    while (w1 != 0) {
        pv = *(char **)(w1 + 0x78);
        if (!func_L00_0025F410_D1168(w1) || *(int *)(pv + 0xA4) != 0) {
            goto next2;
        }
        for (i = 0; i < 4; i++) {
            a[i] = ((float *)(w1 + 0x10))[i];
            b[i] = ((float *)(w1 + 0x10))[i];
        }
        h.u = 0x3DCCCCCD;
        b[2] = h.f;
        a[2] = a[2] + *(float *)(pv + 0xB0) * 0.5f;
        if (func_L00_001EFFF0_D1168(a, b, 2, w0, 0) != 0 && *(char **)(out + 0x18) != 0 &&
            func_L00_0025D390_D1168(*(char **)(out + 0x18))) {
            if (w0 != 0) {
                *(short *)(w0 + 0x36) = 0x7F80;
            }
            func_L00_002616E0_D1168(w1, *(char **)(out + 0x18), w1 + 0x10, w1 + 0x40, pv + 0xD0, pv + 0xE0);
            *(int *)(pv + 0xF4) = 0;
            *(int *)(pv + 0xAC) |= 8;
            *(char **)(pv + 0xF0) = *(char **)(out + 0x18);
        }
        w0 = w1;
        while (w0 != 0) {
            pv = *(char **)(w0 + 0x78);
            bp = 0;
            if (*(char **)(pv + 0xA4) != 0) {
                bp = *(char **)(*(char **)(pv + 0xA4) + 0x78);
            }
            if (bp != 0) {
                *(int *)(pv + 0xCC) = *(int *)(bp + 0xCC);
                if (*(int *)(bp + 0xF0) != 0) {
                    *(unsigned char *)(w0 + 0x30) = 0xFF;
                    *(int *)(pv + 0xAC) |= 8;
                    for (i = 0; i < 4; i++) {
                        ((int *)(pv + 0xD0))[i] = ((int *)(bp + 0xD0))[i];
                        ((int *)(pv + 0xE0))[i] = ((int *)(bp + 0xE0))[i];
                    }
                    *(int *)(pv + 0xF0) = *(int *)(bp + 0xF0);
                    *(float *)(pv + 0xF4) = *(float *)(bp + 0xF4) + 1.0f;
                }
            }
            if ((*(short *)(pv + 0xC6) != 0 || *(short *)(w0 + 0xA6) == 0x1F5) && *(int *)(pv + 0xA0) != 0 &&
                *(int *)(pv + 0xC0) != -1) {
                path = D_L00_001B0830_D1168[*(int *)(pv + 0xC0)];
                n = *(int *)path;
                k = *(int *)(path + 0x1C);
                if (n == 0 || k == 1 || k == 3 || k == 0x111) {
                    *(int *)(pv + 0xC0) = -1;
                } else if ((unsigned int)(k - 0x11) < 0x100) {
                    if (k - 0x10 < n) {
                        *(int *)(path + 0x1C) = k + 1;
                    } else {
                        *(int *)(pv + 0xC0) = -1;
                    }
                } else {
                    *(int *)(path + 0x1C) = 0x11;
                    for (i = 0; i < *(int *)D_L00_001B0830_D1168[*(int *)(pv + 0xC0)]; i++) {
                        *(int *)(D_L00_001B0830_D1168[*(int *)(pv + 0xC0)] + i * 16 + 0x1C) = 0;
                    }
                }
            }
            w0 = *(char **)(pv + 0xA0);
        }
    next2:
        func_L00_0025A2F0_D1168(&w1, w1, 0, 0);
    }

    /* Loop 3: distances between the group's crates (not used). */
    func_L00_0025A208_D1168(&w0, group, 0, 0);
    while (w0 != 0) {
        if (func_L00_0025F410_D1168(w0)) {
            func_L00_0025A2F0_D1168(&w2, w0, 0, 0);
            while (w2 != 0) {
                if (func_L00_0025F410_D1168(w2)) {
                    func_001F9D10_D1168(w0 + 0x10, w2 + 0x10);
                }
                func_L00_0025A2F0_D1168(&w2, w2, 0, 0);
            }
        }
        func_L00_0025A2F0_D1168(&w0, w0, 0, 0);
    }
}
