/* NON_MATCHING func_L00_002D3B20 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: SIZE ours 1040 / retail 1052, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Decoy glove update (moby class 562): a state machine on moby[0x20] with the data block at moby[0x78]; the call
 *   Left: ours saves $fp and $s7 as well as retail's seven saved regs (retail keeps d+0x40 in $18 then $21, and G'
 *   Unblock: a second form for the data block and the G base that gives retail's saved-register set, then the sche
 */
extern void func_L00_00250800(void *, int, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_00260878(void *, void *);
extern int func_L00_002C48C8(void *, void *, void *);
extern void func_L00_00222B80(int, int);
extern int func_001F9908(int *);
extern int func_001F9850(int);
extern int func_L00_00234718(int);
extern int func_L00_00217570(int, int);
extern int func_L00_00234638(int, int);
extern void func_L00_002D3608(void *, void *, void *);
extern void func_L00_002C4B90(float *, char *, void *);
extern short D_L00_001B0BB0[];
extern int D_0015EFA4 MACRO_ADDR;
extern int D_0015EE84 MACRO_ADDR;
extern char D_0013E633[];
extern char D_0013A5E0[];
extern char D_0014171B[];
typedef int u128_D3B20 __attribute__((mode(TI)));

/* Decoy glove update: state machine on moby[0x20], per-object data block at moby[0x78]. Adapted from Lombyte (MIT) for PAL: none. */
void func_L00_002D3B20(unsigned char *moby) {
    char *d;
    char *p;
    char *g;
    char *e;
    float vb[4];
    float va[4];
    float vc[4];
    float vd[4];
    u128_D3B20 loc;

    if (moby == 0) return;
    d = *(char **)(moby + 0x78);
    if (d == 0) return;
    *(u128_D3B20 *)va = 0;
    va[1] = -0.19f;
    va[2] = -0.11f;
    func_L00_00250800(moby, 0, vb);
    e = d + 0x40;
    func_0020DAF8_2d3330(moby, 0, vd);
    func_001F9EE8(vc, va, vd);
    func_001F9BD8(e, vb, vc);

    p = *(char **)(d + 0x50);
    if (p) {
        *(int *)(p + 0x98) = 1;
        if (*(unsigned char *)(D_0013E633 + 6) && *(short *)(*(char **)(d + 0x50) + 0xA6) != 0x76C) {
            qzero(&loc);
            func_L00_00260878(*(char **)(d + 0x50), D_L00_001B0BB0);
            func_0020D678_D83D8(*(char **)(d + 0x50));
            *(int *)(d + 0x50) = func_L00_002C48C8(moby, e, &loc);
        }
    }
    if (*(char **)(d + 0x50)) {
        if (*(unsigned char *)(*(char **)(d + 0x50) + 0x20) == 0xFE || *(unsigned char *)(*(char **)(d + 0x50) + 0x20) == 0xFD) *(int *)(d + 0x50) = 0;
    }

    g = D_0013E633 + 0xE1D;
    if (*(int *)(g + 0x2084) == 1) func_L00_00222B80(0x1E, 1);
    if (func_001F9908((int *)(d + 0x54))) {
        if (*(unsigned char *)(g + 0x20AC) == 0) {
            if (*(unsigned char *)(g + 0x20A8) && *(int *)(g + 0x1BC) == func_001F9850(0x11)) goto L3D10;
            if (*(int *)(D_0013A5E0 + 0x2604) & *(int *)(g + 0x10A0)) {
                if (*(int *)(g + 0x2084) == 0x1E && func_L00_00234718(-1)) goto L3D10;
            }
            if (*(int *)(g + 0x2084) == 0x23 && *(int *)(g + 0x198) == func_001F9850(0x10)) goto L3D10;
        }
    }
    goto L3D34;
L3D10:
    func_L00_00217570(0x1A, 0);
    func_L00_00234638(-1, 1);
    moby[0x20] = 3;
L3D34:
    *(u128_D3B20 *)&loc = *(u128_D3B20 *)(d + 0x40);
    func_L00_002D3608(moby, d, &loc);

    switch (moby[0x20]) {
    case 0:
        *(int *)(d + 0x50) = 0;
        moby[0x20] = (moby[0x70] & 2) ? 2 : 1;
    case 1:
        if (moby[0x70] & 2) moby[0x20] = 2;
        break;
    case 2:
        break;
    case 3:
        p = *(char **)(d + 0x50);
        if (p) {
            char *P = D_0014171B + 0x65;
            if (*(unsigned short *)(P + 0xC8) <= 0xFFFE) *(unsigned short *)(P + 0xC8) = *(unsigned short *)(P + 0xC8) + 1;
            if ((int)*(unsigned short *)(P + 0xCA) < func_001F9850(D_0015EFA4) / 600)
                *(unsigned short *)(P + 0xCA) = func_001F9850(D_0015EFA4) / 600;
            *(unsigned int *)(P + 0xCC) = (*(unsigned int *)(P + 0xCC) | (1 << D_0015EE84)) | 0x80000000;
            func_L00_002C4B90((float *)e, p, *(char **)(p + 0x78) + 0x10);
            *(int *)(d + 0x50) = 0;
        } else {
            func_0022ED80(0, 0, (int)moby);
        }
        moby[0x20] = 4;
        *(int *)(d + 0x54) = func_001F9850(0x14);
        break;
    case 4:
        if (*(int *)(g + 0x2084) != 0x23 && *(unsigned char *)(g + 0x20A8) == 0) moby[0x20] = 2;
        break;
    case 5:
        moby[0x20] = 6;
        break;
    case 6:
        return;
    default:
        break;
    }

    p = *(char **)(d + 0x50);
    if (p) {
        qcopy(p + 0x10, e);
        return;
    }
    if (func_L00_00234718(-1) || *(unsigned char *)(g + 0x20A8)) {
        qzero(&loc);
        *(int *)(d + 0x50) = func_L00_002C48C8(moby, e, &loc);
    }
}
