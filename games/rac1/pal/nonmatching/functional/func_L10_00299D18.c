/* functional: high. Repaired near miss: retail feeds D_0015EFA4 (gp-relative and absolute, the same word) as the argument of the second and third func_001F9850 (scale_ticks) calls of the counter block, where the near miss read D_00158FA4; the rest is the same reads, writes and calls with compiler-chosen order and registers. */
/* func_L10_00299D18 -- src/overlays/l10_orxon/vendor_00296BD8.c (functional C for the port, not a match)
 * Level 10 (Orxon) update of a moby: marks it (+0x30 = 0xFF), clears a pending flag, and when its pvars
 * (+0x78) name two linked mobys of class 0x325 sets flag 8 in one moby's pvar word +8 and clears it in the
 * other, depending on the per-planet byte table D_0014171B+0xAA35. If the moby's id (+0xB2) is already
 * flagged (D_L10_001BB9C0+0x454 byte or the per-planet bit table at D_0014171B+0xAB75): in one mode
 * (D_0013D5CA[4]) it hides the target moby; otherwise it bumps the counters at D_0014171B+0x32D/0x32F
 * (scale_ticks), sets the planet bit, shows help message 0x53F6, and returns while D_0013E633+0x2EC1 is 1.
 * Then it sets the id's bits in both tables and, if the pvars name a target, either calls func_L00_00217718 on
 * one of two cuboids (pvars +0x14 or +8) and then (func_L00_002110C0 / func_L00_002664B0(2,4)) or calls func_L00_002111E8,
 * func_L00_00217718 on cuboid +4, hides the target and deletes this moby. */
extern char D_0013E633[];
extern unsigned char D_0014171B[];
extern unsigned char D_0013D5CA[];
extern int D_0015EE84_m __asm__("D_0015EE84") MACRO_ADDR;
extern char *D_L10_00160058_c __asm__("D_L10_00160058") MACRO_ADDR;
extern int D_L10_0015F6A8 MACRO_ADDR;
extern int D_0015EFA4 MACRO_ADDR;
extern char *D_L10_0016016C MACRO_ADDR;
extern short D_L10_00161410;
extern int D_L10_001BAC60[];
extern unsigned char D_L10_001BB9C0_c[] __asm__("D_L10_001BB9C0");
extern int func_001F9850(int);
extern void func_00215F80(int, int);
extern void func_L00_00217718(void *, void *, int, int);
extern void func_L00_002110C0(int, int, char *);
extern void func_L00_002664B0(int, int);
extern void func_L00_002111E8(void);
extern void func_0020D678(void *);

void func_L10_00299D18(char *m) {
    char *d = *(char **)(m + 0x78);
    unsigned short id;
    ((unsigned char *)m)[0x30] = 0xFF;
    if (*(int *)&D_L10_00161410 == 1) {
        *(short *)(D_0014171B + 0x32D) = 0;
        *(int *)&D_L10_00161410 = 0;
    }
    if (*(int *)(d + 0x18) != -1) {
        int b = *(int *)(d + 0x1C);
        if (b != -1) {
            if (*(short *)(D_L10_00160058_c + (*(int *)(d + 0x18) << 8) + 0xA6) == 0x325 &&
                *(short *)(D_L10_00160058_c + (b << 8) + 0xA6) == 0x325) {
                char *f1 = *(char **)(D_L10_00160058_c + (*(int *)(d + 0x18) << 8) + 0x78);
                char *f2 = *(char **)(D_L10_00160058_c + (b << 8) + 0x78);
                if ((D_0014171B + 0xAA35)[((unsigned char *)m)[0xB0] + (D_0015EE84_m << 4)] == 0xFF) {
                    *(int *)(f1 + 8) |= 8;
                    *(int *)(f2 + 8) &= ~8;
                } else {
                    *(int *)(f1 + 8) &= ~8;
                    *(int *)(f2 + 8) |= 8;
                }
            }
        }
    }
    id = *(unsigned short *)(m + 0xB2);
    if (D_L10_001BB9C0_c[0x454 + (short)id] != 0 ||
        ((*(int *)(D_0014171B + 0xAB75 + ((((short)id >> 5) * 4) + (D_0015EE84_m << 8))) >> (id & 0x1F)) & 1) != 0) {
        if (D_0013D5CA[4] != 0) {
            char *bs = D_L10_00160058_c;
            *(unsigned short *)(bs + (*(int *)d << 8) + 0x34) |= 1;
            *(unsigned short *)(bs + (*(int *)d << 8) + 0x34) |= 2;
            *(int *)(bs + (*(int *)d << 8) + 0x94) = 0;
        } else {
            if (*(int *)&D_L10_0015F6A8 == 0) {
                unsigned char *T = D_0014171B + 0x22D;
                unsigned short t0 = *(unsigned short *)(T + 0x100);
                int r = func_001F9850(0x12C);
                if (t0 < r) {
                    unsigned short t1 = *(unsigned short *)(T + 0x100);
                    int x;
                    int r2;
                    if (t1 > 0xFFFE) {
                        x = D_0015EFA4;
                    } else {
                        *(short *)(T + 0x100) = t1 + 1;
                        x = D_0015EFA4;
                    }
                    r2 = func_001F9850(x);
                    if (*(unsigned short *)(T + 0x102) < r2 / 600) {
                        *(short *)(T + 0x102) = func_001F9850(D_0015EFA4) / 600;
                    }
                    {
                        int w = *(int *)(T + 0x104);
                        w |= 1 << D_0015EE84_m;
                        w |= 0x80000000;
                        *(int *)(T + 0x104) = w;
                    }
                    func_00215F80(1, 0x53F6);
                }
            }
            if (((unsigned char *)D_0013E633)[0x2EC1] == 1) return;
        }
    }
    *(int *)(D_0014171B + 0xAB75 + ((((short)*(unsigned short *)(m + 0xB2) >> 5) * 4) + (D_0015EE84_m << 8)))
        |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
    D_L10_001BAC60[(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
    if (*(int *)(d + 8) == -1 || *(int *)d == -1) return;
    if (D_0013D5CA[4] == 0) {
        char *e = D_L10_00160058_c + (*(int *)d << 8);
        int eo = D_0015EE84_m << 4;
        unsigned char byte;
        int k;
        char *P;
        *(int *)(e + 0x94) = *(int *)(*(char **)(e + 0x24) + 0x10);
        byte = (D_0014171B + 0xAA35)[((unsigned char *)m)[0xB0] + eo];
        if (byte == 0xFF || *(int *)(d + 0x14) == -1) {
            k = *(int *)(d + 0x14);
            P = D_L10_0016016C + (k << 7);
            func_L00_00217718(P + 0x30, P + 0x70, 0, 1);
        } else {
            k = *(int *)(d + 8);
            P = D_L10_0016016C + (k << 7);
            func_L00_00217718(P + 0x30, P + 0x70, 0, 1);
        }
        func_L00_002110C0(1, 0x43, D_L10_00160058_c + (*(int *)d << 8));
        func_L00_002664B0(2, 4);
    } else {
        unsigned char *h = (unsigned char *)D_0013E633 + 0xE1D;
        if (h[0x20A4] != 1) {
            h[0x22CB] = 1;
            func_0020D678(m);
            return;
        }
        func_L00_002111E8();
        {
            char *P = D_L10_0016016C + (*(int *)(d + 4) << 7);
            func_L00_00217718(P + 0x30, P + 0x70, 0, 1);
        }
        func_L00_002664B0(0, 5);
        *(unsigned short *)(D_L10_00160058_c + (*(int *)d << 8) + 0x34) |= 1;
        *(unsigned short *)(D_L10_00160058_c + (*(int *)d << 8) + 0x34) |= 2;
        *(int *)(D_L10_00160058_c + (*(int *)d << 8) + 0x94) = 0;
        h[0x22CB] = 1;
        func_0020D678(m);
    }
}
