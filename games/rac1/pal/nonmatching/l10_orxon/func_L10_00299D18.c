/* NON_MATCHING func_L10_00299D18 -- src/overlays/l10_orxon/vendor_00296BD8.c
 * Best so far: SIZE ours 1220 / retail 1232, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 10 update for moby class 22: links two Ent records (word at 0x18/0x1C), sets table bits, then spawns thr
 *   Left: (1) the flag test after the T-block sits in retail at a different place (lui/lbu of D_0013E633 then the 
 *   Budget used: 10 runs. Would unblock: the flag-test placement; the 0x100 store order.
 */
extern char D_0013E633[];
extern unsigned char D_0014171B[];
extern unsigned char D_0013D5CA[];
extern int D_0015EE84_m __asm__("D_0015EE84") MACRO_ADDR;
extern char *D_L10_00160058_c __asm__("D_L10_00160058") MACRO_ADDR;
extern int D_L10_0015F6A8 MACRO_ADDR;
extern int D_0015EFA4 MACRO_ADDR;
extern char *D_L10_0016016C MACRO_ADDR;
extern short D_L10_00161410;
extern short D_00158FA4;
extern int D_L10_001BAC60[];
extern unsigned char D_L10_001BB9C0_c[] __asm__("D_L10_001BB9C0");
extern int func_001F9850(int);
extern void func_00215F80(int, int);
extern void func_L00_00217718(void *, void *, int, int);
extern void func_L00_002110C0(int, int, char *);
extern void func_L00_002664B0(int, int);
extern void func_L00_002111E8(void);
extern void func_0020D678(void *);

// Level 10 update for moby class 22: links two Ent records, sets table bits, calls the spawn helpers.
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
                        x = *(int *)&D_00158FA4;
                    } else {
                        *(short *)(T + 0x100) = t1 + 1;
                        x = D_0015EFA4;
                    }
                    r2 = func_001F9850(x);
                    if (*(unsigned short *)(T + 0x102) < r2 / 600) {
                        *(short *)(T + 0x102) = func_001F9850(*(int *)&D_00158FA4) / 600;
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
