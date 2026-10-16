/* HeroItemsAttach: every frame, places Ratchet's attached item mobys on his joints. For each
   item slot i (0..6, records of 0x50 bytes at hero + i * 0x50; only slots 4 and 5 in the
   modes 2 and 6 of D_L00_0015F6A8) and each moby of the slot (+0x1090, and +0x1094 for the
   slots 1, 3 and 4): the moby takes Ratchet's light words, then its attach matrix (hero
   +0xAC0 + list * 0x40, the list from the item record +0xC, list 3 for slot 1's second moby):
   - a detached item (slot +0x10AA) only updates the slot's hand point (+0x1070) and angles
     (+0x1080), the moby's update and mode;
   - otherwise position = the matrix's row 3 and rows = the matrix (normalised and advanced
     unless the item is a glove, a head item or a boot), the yaw/pitch of its x axis to
     +0x48/+0x44, and the glove/head/boot items copy Ratchet's pose through
     func_L00_00209940 from their joint tables; items with record +0x18 set take the
     Euler-angle route instead when D_L00_0015F770 is set.
   Slot 3's second moby (Clank) also carries the antenna glow moby (class 0x4B4, hero
   +0x118C, created on demand) on its joint 6, with a pulsing colour.
   Adapted from ReRAC (crates/rc-game/src/hero/items.rs attach_hand, crates/rc-engine/src/
   moby_attach.rs place_antenna; ISC License, Copyright (c) 2026 ReRAC contributors). */
typedef struct {
    int w[4];
} __attribute__((aligned(16))) HIA_Q;
typedef struct {
    char pad0[0xC];
    int list;
    char pad10[0x8];
    int euler;
    char pad1C[0x30];
} HIA_Rec;
extern char D_0013E633_hia[] __asm__("D_0013E633");
extern HIA_Rec D_L00_00179BC0_hia[] __asm__("D_L00_00179BC0");
extern int D_L00_0015F6A8_hia[] __asm__("D_L00_0015F6A8");
extern int D_L00_0015F770_hia[] __asm__("D_L00_0015F770");
extern int D_L00_0015F6B0_hia[] __asm__("D_L00_0015F6B0");
extern unsigned char D_0015EEB4_hia[] __asm__("D_0015EEB4");
extern char D_L00_0017A6C0_hia[] __asm__("D_L00_0017A6C0");
extern char D_L00_0017C580_hia[] __asm__("D_L00_0017C580");
extern char D_L00_0017A708_hia[] __asm__("D_L00_0017A708");
extern char D_L00_0017C780_hia[] __asm__("D_L00_0017C780");
extern char D_L00_0015F788_hia[] __asm__("D_L00_0015F788");
extern char D_L00_0017AA60_hia[] __asm__("D_L00_0017AA60");
extern char D_L00_0017A780_hia[] __asm__("D_L00_0017A780");
extern char D_L00_0017A750_hia[] __asm__("D_L00_0017A750");
extern char D_L00_0017C980_hia[] __asm__("D_L00_0017C980");
extern char D_L00_0017A768_hia[] __asm__("D_L00_0017A768");
extern char D_L00_0017CA80_hia[] __asm__("D_L00_0017CA80");
extern void func_002153E8_hia(void *, void *) __asm__("func_002153E8");
extern void func_L00_002514B8_hia(void *) __asm__("func_L00_002514B8");
extern void func_L00_00251E30_hia(void *) __asm__("func_L00_00251E30");
extern int func_L00_0020DB68_hia(int) __asm__("func_L00_0020DB68");
extern int func_L00_0020DBB0_hia(int) __asm__("func_L00_0020DBB0");
extern int func_L00_0020DBD8_hia(int) __asm__("func_L00_0020DBD8");
extern void func_001FA480_hia(void *, void *) __asm__("func_001FA480");
extern void func_00214F78_hia(void *) __asm__("func_00214F78");
extern void func_001F9EC0_hia(void *, void *, void *) __asm__("func_001F9EC0");
extern float func_001F9CE8_hia(void *) __asm__("func_001F9CE8");
extern float func_L00_001FF860_hia(float, float) __asm__("func_L00_001FF860");
extern void func_0020EEE8_hia(void *) __asm__("func_0020EEE8");
extern void func_L00_00209940_hia(void *, void *, void *, int, int) __asm__("func_L00_00209940");
extern void func_L00_0020F7F8_hia(void *, void *) __asm__("func_L00_0020F7F8");
extern void func_001F9CA0_hia(void *, void *, void *) __asm__("func_001F9CA0");
extern char *func_0020D348_hia(int) __asm__("func_0020D348");
extern void func_0020DAF8_hia(void *, int, void *) __asm__("func_0020DAF8");
extern void func_L00_00208318_hia(void) __asm__("func_L00_00208318");
extern int func_001F9850_hia(int) __asm__("func_001F9850");
extern float func_001FA888_hia(int) __asm__("func_001FA888");
extern float func_001F9FA8_hia(float) __asm__("func_001F9FA8");
extern int func_001FA898_hia(float) __asm__("func_001FA898");

void func_L00_0020FC18(void) {
    char *h = D_0013E633_hia + 0xE1D;
    float mtx[16] __attribute__((aligned(16)));
    float v[4] __attribute__((aligned(16)));
    HIA_Q save0;
    HIA_Q save1;
    char *s;
    char *m;
    char *g;
    char *mat;
    int i;
    int j;
    int off;
    int lst;
    int isGlove;
    int isHead;
    int isBoot;
    int mode;
    int n;
    int r;
    int gr;
    int b;
    float f;

    for (i = 0; i < 7; i++) {
        mode = D_L00_0015F6A8_hia[0];
        if ((mode == 2 || mode == 6) && i != 4 && i != 5) {
            continue;
        }
        off = i * 0x50;
        s = h + off;
        j = 0;
        do {
            if (j == 1) {
                m = *(char **)(s + 0x1094);
            } else {
                m = *(char **)(s + 0x1090);
            }
            if (m != 0) {
                *(long *)(m + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
                lst = D_L00_00179BC0_hia[*(int *)(s + 0x10B8)].list;
                if (i == 1 && j == 1) {
                    lst = 3;
                }
                mat = h + 0xAC0 + lst * 0x40;
                if (*(unsigned char *)(s + 0x10AA) != 0) {
                    *(HIA_Q *)(h + 0x1070 + off) = *(HIA_Q *)(h + 0xAF0 + lst * 0x40);
                    func_002153E8_hia(mat, h + 0x1080 + off);
                    func_L00_002514B8_hia(m);
                    func_L00_00251E30_hia(m);
                    *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xFFFB;
                } else {
                    *(HIA_Q *)(m + 0x10) = *(HIA_Q *)(h + 0xAF0 + lst * 0x40);
                    if (D_L00_0015F770_hia[0] != 0 &&
                        D_L00_00179BC0_hia[*(int *)(s + 0x10B8)].euler != 0) {
                        func_L00_002514B8_hia(m);
                        if (D_L00_0015F770_hia[0] != 0 && i == 0) {
                            func_L00_0020F7F8_hia(m, mat);
                        } else {
                            func_002153E8_hia(mat, m + 0x40);
                        }
                        func_L00_00251E30_hia(m);
                        if (D_0015EEB4_hia[1] != 0) {
                            func_001F9CA0_hia(m + 0xE0, m + 0xC0, m + 0xD0);
                        }
                        func_0020EEE8_hia(m);
                        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 6;
                    } else {
                        isGlove = func_L00_0020DB68_hia(i);
                        isHead = func_L00_0020DBB0_hia(i);
                        isBoot = func_L00_0020DBD8_hia(i);
                        if (isGlove == 0 && isHead == 0 && isBoot == 0) {
                            func_L00_002514B8_hia(m);
                        }
                        func_L00_00251E30_hia(m);
                        func_001FA480_hia(m + 0xC0, mat);
                        if (isGlove == 0 && isHead == 0 && isBoot == 0) {
                            func_00214F78_hia(m + 0xC0);
                        }
                        v[1] = 0.0f;
                        v[0] = 1.0f;
                        v[2] = 0.0f;
                        func_001F9EC0_hia(v, v, m + 0xC0);
                        f = func_001F9CE8_hia(v);
                        *(float *)(m + 0x44) = -func_L00_001FF860_hia(f, v[2]);
                        *(float *)(m + 0x48) = func_L00_001FF860_hia(v[0], v[1]);
                        func_0020EEE8_hia(m);
                        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 6;
                        if (isGlove != 0) {
                            func_L00_00209940_hia(D_L00_0017A6C0_hia, D_L00_0017C580_hia,
                                                  *(void **)(m + 0x24), 0, 0);
                            *(int *)(m + 0x54) = 0;
                            *(int *)(m + 0x50) = 0;
                            *(char **)(m + 0x68) = D_L00_0017C580_hia;
                            *(char **)(m + 0x6C) = D_L00_0017C580_hia;
                        } else if (isHead != 0) {
                            if (*(short *)(m + 0xA6) == 0x1B1) {
                                func_L00_00209940_hia(D_L00_0017A708_hia, D_L00_0017C780_hia,
                                                      *(void **)(m + 0x24), 6, 0);
                            } else {
                                func_L00_00209940_hia(D_L00_0015F788_hia, D_L00_0017C780_hia,
                                                      *(void **)(m + 0x24), 0, 0);
                            }
                            *(int *)(m + 0x54) = 0;
                            *(char **)(m + 0x6C) = D_L00_0017C780_hia;
                            *(char **)(m + 0x68) = D_L00_0017C780_hia;
                            *(int *)(m + 0x50) = 0;
                        } else if (isBoot != 0) {
                            save0 = *(HIA_Q *)(D_L00_0017AA60_hia + 0x0);
                            save1 = *(HIA_Q *)(D_L00_0017AA60_hia + 0xB0);
                            *(float *)(D_L00_0017A780_hia + 0x398) = 1.0f;
                            *(float *)(D_L00_0017A780_hia + 0x2E0) = 1.0f;
                            *(float *)(D_L00_0017A780_hia + 0x2E4) = 1.0f;
                            *(float *)(D_L00_0017A780_hia + 0x2E8) = 1.0f;
                            *(float *)(D_L00_0017A780_hia + 0x390) = 1.0f;
                            *(float *)(D_L00_0017A780_hia + 0x394) = 1.0f;
                            if (j == 0) {
                                func_L00_00209940_hia(D_L00_0017A750_hia, D_L00_0017C980_hia,
                                                      *(void **)(m + 0x24), 0, 0);
                                *(char **)(m + 0x68) = D_L00_0017C980_hia;
                                *(char **)(m + 0x6C) = D_L00_0017C980_hia;
                            } else {
                                func_L00_00209940_hia(D_L00_0017A768_hia, D_L00_0017CA80_hia,
                                                      *(void **)(m + 0x24), 0, 0);
                                *(char **)(m + 0x68) = D_L00_0017CA80_hia;
                                *(char **)(m + 0x6C) = D_L00_0017CA80_hia;
                            }
                            *(int *)(m + 0x54) = 0;
                            *(int *)(m + 0x50) = 0;
                            *(HIA_Q *)(D_L00_0017AA60_hia + 0x0) = save0;
                            *(HIA_Q *)(D_L00_0017AA60_hia + 0xB0) = save1;
                        }
                    }
                }
            }

            /* Clank (slot 3, second moby) carries the antenna glow moby on his joint 6. */
            if (i == 3 && j == 1 && m != 0) {
                g = *(char **)(h + 0x118C);
                if (g == 0) {
                    g = func_0020D348_hia(0x4B4);
                    *(char **)(h + 0x118C) = g;
                    if (g == 0) {
                        goto next;
                    }
                    *(unsigned int *)(g + 0x90) = 0x801432D7;
                    g = *(char **)(h + 0x118C);
                    *(HIA_Q *)(g + 0x10) = *(HIA_Q *)(h + 0x80);
                    *(HIA_Q *)(g + 0x40) = *(HIA_Q *)(h + 0x90);
                    *(short *)(g + 0x32) = 0x40;
                    *(unsigned char *)(*(char **)(h + 0x118C) + 0x31) = 1;
                    *(unsigned char *)(*(char **)(h + 0x118C) + 0x30) = 0;
                    *(short *)(*(char **)(h + 0x118C) + 0x34) = 0;
                    g = *(char **)(h + 0x118C);
                    *(float *)(g + 0x2C) = *(float *)(g + 0x2C) * 1.3f;
                    func_L00_00251E30_hia(*(char **)(h + 0x118C));
                    if (*(char **)(h + 0x118C) == 0) {
                        goto next;
                    }
                }
                g = *(char **)(h + 0x118C);
                if ((*(unsigned short *)(g + 0x34) & 1) == 0) {
                    func_0020DAF8_hia(m, 6, mtx);
                    *(HIA_Q *)(h + 0x1D80) = *(HIA_Q *)(mtx + 12);
                    func_L00_00208318_hia();
                    *(HIA_Q *)(g + 0x10) = *(HIA_Q *)(mtx + 12);
                    func_001FA480_hia(g + 0xC0, mtx);
                    func_L00_00251E30_hia(g);
                    *(long *)(g + 0x38) = *(long *)(*(char **)(h + 0x2080) + 0x38);
                    n = func_001F9850_hia(0x78);
                    f = func_001FA888_hia(n);
                    f = (float)(D_L00_0015F6B0_hia[0] % n) / f;
                    f = f + f;
                    f = f * 3.1415927f;
                    f = func_001F9FA8_hia(f + -3.1415927f);
                    r = func_001FA898_hia(f * 90.0f);
                    gr = func_001FA898_hia(f * 50.0f);
                    b = func_001FA898_hia(f * 10.0f);
                    r = r + 0xD7;
                    if (r >= 0x100) {
                        r = 0xFF;
                    }
                    *(unsigned int *)(g + 0x90) = (unsigned int)((b + 0x14) << 16) | 0x80000000u |
                                                  (unsigned int)((gr + 0x32) << 8) | (unsigned int)r;
                    *(unsigned short *)(g + 0x34) = *(unsigned short *)(g + 0x34) | 0x10;
                }
            }
        next:
            j++;
        } while (j < 2 && (i == 1 || i == 3 || i == 4));
    }
}
