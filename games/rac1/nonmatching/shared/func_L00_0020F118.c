/* NON_MATCHING func_L00_0020F118 -- src/overlays/shared/help_0020CDF0.c
 * Best so far: SIZE ours 1584 / retail 1592, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Cannot land as written (#define in a candidate): rewrite that in plain C first.
 * What the last attempts found:
 *   func_L00_0020F118 (HeroItemsCreate): creates the hero's missing item mobys, one block per slot at hero block (
 *   Best try p10.c/p11.c: control flow and instruction sequence identical to retail, but 1584 vs 1592 bytes and re
 *   What would help: the original almost surely used one shared spawn helper/macro (same 0x32/0x31/ld-sd/0x73 patt
 *   q27/s02: best.c needed D_0014171B declared `extern char D_0014171B[] NOT_SDA;` (file now declares char[], not 
 */
#define REC(i) ((char *)D_L00_00179BC0 + (i) * 0x4C)
extern void func_00205270(int, int);
extern void func_001F99B0(void *, int, int);
extern char *func_0020D348(int);
extern void func_00213DE0(void *, int, int, int);
extern char *func_L00_002B5428(int);
extern int D_L00_0015F4F8 MACRO_ADDR;
extern int D_L00_00179C1C;
extern unsigned char D_0013D5E9 NOT_SDA;
extern int D_L00_0017A59C;
extern int D_L00_0017A5E8;
extern unsigned char D_0013D5EB NOT_SDA;
extern int D_L00_0017A634;
extern char D_0014171B[] NOT_SDA;
extern unsigned char D_0015EE88[] NOT_SDA;

/* Creates the hero's item mobys (the slots at +0x1090..+0x1220 of the hero block) that are missing. */
void func_L00_0020F118(void) {
    char *g = D_0013E633 + 0xE1D;
    char *g1;
    char *g2;
    char *g3;
    char *g4;
    char *g5;
    char *g6;
    char *g7;
    char *g8;
    char *g9;
    char *g10;
    char *g11;
    char *m;
    char *r;
    unsigned char *d;
    int idx;
    int o;

    if (*(int *)(g + 0x1090) == 0) {
        if (D_L00_0015F4F8 > *(int *)(g + 0x10A4)) {
            int t = *(int *)(g + 0x20B8);
            if (t != 0 && t != 0x24) goto next;
            idx = *(int *)(g + 0x20D4);
            if (idx == 0) {
                int x;
                if (*(int *)(D_0015EE88 + 8) != 0 || (x = *(int *)(D_0014171B + 0x45)) == 0) {
                    idx = 8;
                } else {
                    idx = x;
                }
            }
            {
                int p, q, e;
                int k;
                char *s;
                r = (char *)D_L00_00179BC0 - (-((idx) * 0x4C));
                func_00205270(*(int *)(r + 0x10), -1);
                p = (int)(D_0013E633 + 0x270D);
                e = p + 0x210;
                q = p + 0xA0;
                do {
                    func_001F99B0((void *)p, 0, 0xB0);
                    p += 0xB0;
                    *(short *)q = -1;
                    q += 0xB0;
                } while (p < e);
                g1 = D_0013E633 + 0xE1D;
                *(int *)(g1 + 0x10B8) = idx;
                r = (char *)D_L00_00179BC0 - (-((idx) * 0x4C));
                k = *(int *)(r + 8);
                s = g1 + k * 0x50;
                r = (char *)D_L00_00179BC0 - (-((*(int *)(s + 0x10B8)) * 0x4C));
                m = func_0020D348(*(int *)(r + 0x10));
                if (m != 0) {
                    *(short *)(m + 0x32) = 0x20;
                    m[0x31] = 1;
                    d = *(unsigned char **)(m + 0x24);
                    *(long *)(m + 0x38) = *(long *)(*(char **)(g1 + 0x2080) + 0x38);
                    if (d[6] != 0) m[0x73] = 0x18;
                    *(char **)(s + 0x1090) = m;
                    *(int *)(s + 0x10B4) = 2;
                    if (idx == 8) {
                        *(int *)(s + 0x10A0) = 0x80;
                        func_00213DE0(m, 1, 0, 1);
                    } else {
                        *(int *)(s + 0x10A0) = 0x20;
                    }
                    s = D_0013E633 + 0xE1D + k * 0x50;
                    r = (char *)D_L00_00179BC0 - (-((*(int *)(s + 0x10B8)) * 0x4C));
                    D_0013E633[0xE1D + 0x20AB] = *(unsigned char *)(r + 0x18);
                }
            }
        }
    }
next:
    g2 = D_0013E633 + 0xE1D;
    if (*(int *)(g2 + 0x1180) == 0) {
        int k = *(int *)(g2 + 0x20E0);
        if (k == 0) {
            k = *(int *)(D_0014171B + 0x51);
            if (k == 0) {
                k = 2;
                if (*(int *)(D_0015EE88 + 0xC) != 0) k = 3;
            }
        }
        g3 = D_0013E633 + 0xE1D;
        *(int *)(g3 + 0x11A8) = k;
        r = (char *)D_L00_00179BC0 - (-((k) * 0x4C));
        m = func_0020D348(*(int *)(r + 0x10));
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            m[0x31] = 1;
            d = *(unsigned char **)(m + 0x24);
            *(long *)(m + 0x38) = *(long *)(*(char **)(g3 + 0x2080) + 0x38);
            if (d[6] != 0) m[0x73] = 0x18;
            *(int *)(g3 + 0x11A4) = 2;
            *(char **)(g3 + 0x1180) = m;
            if (*(short *)(g3 + 0x22D8) != 0) {
                *(unsigned short *)(m + 0x34) |= 0x41;
            }
            if (*(short *)(g3 + 0x22D8) == 0 && *(int *)(g3 + 0x11A8) == 3) {
                func_L00_002B5428(0);
                func_L00_002B5428(1);
            }
        }
    }
    g4 = D_0013E633 + 0xE1D;
    if (*(int *)(g4 + 0x1184) == 0) {
        m = func_0020D348(D_L00_00179C1C);
        if (m != 0) {
            *(short *)(m + 0x32) = 0x20;
            m[0x31] = 1;
            d = *(unsigned char **)(m + 0x24);
            *(long *)(m + 0x38) = *(long *)(*(char **)(g4 + 0x2080) + 0x38);
            if (d[6] != 0) m[0x73] = 0x18;
            *(int *)(g4 + 0x11A4) = 2;
            *(char **)(g4 + 0x1184) = m;
            if (*(short *)(g4 + 0x22D8) != 0) {
                *(unsigned short *)(m + 0x34) |= 0x41;
            }
        }
    }
    g5 = D_0013E633 + 0xE1D;
    if (*(int *)(g5 + 0x1130) == 0) {
        int a = *(int *)(D_0014171B + 0x4D);
        int b = *(int *)(g5 + 0x20DC);
        if (a != 0 || b != 0) {
            if (b == 0) {
                *(int *)(g5 + 0x1158) = a;
            } else {
                *(int *)(g5 + 0x1158) = b;
            }
            g6 = D_0013E633 + 0xE1D;
            r = (char *)D_L00_00179BC0 - (-((*(int *)(g6 + 0x1158)) * 0x4C));
            m = func_0020D348(*(int *)(r + 0x10));
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                m[0x31] = 1;
                *(unsigned short *)(m + 0x34) |= 2;
                d = *(unsigned char **)(m + 0x24);
                *(long *)(m + 0x38) = *(long *)(*(char **)(g6 + 0x2080) + 0x38);
                *(int *)(m + 0x74) = 0;
                if (d[6] != 0) m[0x73] = 0x18;
                *(unsigned short *)(m + 0x34) |= 0x800;
                *(int *)(g6 + 0x1154) = 2;
                *(char **)(g6 + 0x1130) = m;
            }
        }
    }
    g7 = D_0013E633 + 0xE1D;
    if (*(int *)(g7 + 0x10E0) == 0) {
        int v = ((int *)(D_0014171B + 0x45))[1];
        if (v != 0) {
            *(int *)(g7 + 0x1108) = v;
            r = (char *)D_L00_00179BC0 - (-((v) * 0x4C));
            m = func_0020D348(*(int *)(r + 0x10));
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                m[0x31] = 1;
                d = *(unsigned char **)(m + 0x24);
                *(long *)(m + 0x38) = *(long *)(*(char **)(g7 + 0x2080) + 0x38);
                if (d[6] != 0) m[0x73] = 0x18;
                *(char **)(g7 + 0x10E0) = m;
                *(int *)(g7 + 0x1104) = 2;
            }
            r = (char *)D_L00_00179BC0 - (-((((int *)(D_0014171B + 0x45))[1]) * 0x4C));
            m = func_0020D348(*(int *)(r + 0x14));
            if (m != 0) {
                g8 = D_0013E633 + 0xE1D;
                *(short *)(m + 0x32) = 0x20;
                m[0x31] = 1;
                d = *(unsigned char **)(m + 0x24);
                *(long *)(m + 0x38) = *(long *)(*(char **)(g8 + 0x2080) + 0x38);
                if (d[6] != 0) m[0x73] = 0x18;
                *(char **)(g8 + 0x10E4) = m;
                *(int *)(g8 + 0x1104) = 2;
            }
        }
    }
    g9 = D_0013E633 + 0xE1D;
    if (*(int *)(g9 + 0x11D0) == 0) {
        if (D_0013D5E9 != 0) {
            m = func_0020D348(D_L00_0017A59C);
            *(int *)(g9 + 0x11F8) = 0x21;
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                m[0x31] = 1;
                d = *(unsigned char **)(m + 0x24);
                *(long *)(m + 0x38) = *(long *)(*(char **)(g9 + 0x2080) + 0x38);
                if (d[6] != 0) m[0x73] = 0x18;
                *(char **)(g9 + 0x11D0) = m;
                *(int *)(g9 + 0x11F4) = 2;
            }
        }
    }
    g10 = D_0013E633 + 0xE1D;
    if (*(int *)(g10 + 0x11D4) == 0) {
        if (*(&D_0013D5E9 + 1) != 0) {
            m = func_0020D348(D_L00_0017A5E8);
            *(int *)(g10 + 0x11F8) = 0x22;
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                m[0x31] = 1;
                *(long *)(m + 0x38) = *(long *)(*(char **)(g10 + 0x2080) + 0x38);
                *(int *)(g10 + 0x11F4) = 2;
                *(char **)(g10 + 0x11D4) = m;
            }
        }
    }
    g11 = D_0013E633 + 0xE1D;
    if (*(int *)(g11 + 0x1220) == 0) {
        if (D_0013D5EB != 0) {
            m = func_0020D348(D_L00_0017A634);
            *(int *)(g11 + 0x1248) = 0x23;
            if (m != 0) {
                *(short *)(m + 0x32) = 0x20;
                m[0x31] = 1;
                *(long *)(m + 0x38) = *(long *)(*(char **)(g11 + 0x2080) + 0x38);
                *(int *)(g11 + 0x1244) = 2;
                *(char **)(g11 + 0x1220) = m;
            }
        }
    }
}
