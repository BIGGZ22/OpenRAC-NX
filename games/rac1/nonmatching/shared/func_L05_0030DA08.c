/* NON_MATCHING func_L05_0030DA08 -- src/overlays/shared/vendor_002CF2C0.c
 * Best so far: SIZE ours 876 / retail 888, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Floor button update (state byte 0x20): state 0 eases the press timer and tests the level's pressed bits, state
 *   Left: the list-scan loops (retail keeps the index in $6, the table base in $7 and w0 in $5, with the back-edge
 */
extern char D_0013E633[];
extern char D_0014171B[];
extern char *D_L05_001B0CB0[];
extern char D_L05_001BBA40[];
extern char D_L05_001BADE0[];
extern char D_L05_0015FD48[];
extern float D_0015EE6C MACRO_ADDR;
extern short D_L05_0015FE84;
extern float func_00214158(void);
extern float func_001FA748(float, float);
extern float func_001F9FA8(float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_0022ED80(int, int, int);

/* Floor button update: on state 0 it eases its press timer and checks the level's pressed table, on state 1 it recomputes its trigger and reports to the table. */
void func_L05_0030DA08(unsigned char *m) {
    int i;
    char *d = *(char **)(m + 0x78);
    unsigned short u;
    short s;
    short h;
    unsigned char b0;
    int v;
    int bit;
    int w;
    float t;
    float r;
    char *X;

    switch (m[0x20]) {
    case 0:
        *(float *)(d + 0xC) = func_00214158();
        *(float *)(m + 0x18) = *(float *)(m + 0x18) - 0.35f;
        u = *(unsigned short *)(m + 0xB2);
        w = (int)(u << 16);
        s = w >> 16;
        bit = (*(int *)(D_0014171B + 0xAB75 + ((w >> 21) << 2) + ((*(int *)&D_L05_0015FE84) << 8)) >> (u & 0x1F)) & 1;
        if (*(unsigned char *)(D_L05_001BBA40 + s + 0x454) == 0 && bit == 0
            && (*(int *)(d + 8) == 0 || *(unsigned char *)(D_0014171B + 0xAA35 + m[0xB0] + ((*(int *)&D_L05_0015FE84) << 4)) != 0xFF)) {
            m[0x20] = 1;
            return;
        }
        m[0x20] = 2;
        *(int *)(m + 0x90) = 0x80208020;
        m[0xBC] = 2;
        if (*(int *)d == -1) {
            return;
        }
        if (*(int *)D_L05_001B0CB0[*(int *)d] > 0) {
            i = 0;
            do {
                if (*(float *)(D_L05_001B0CB0[*(int *)d] + (i << 4) + 0x1C) == *(float *)(d + 4)) {
                    *(int *)(D_L05_001B0CB0[*(int *)d] + (i << 4) + 0x1C) = 0;
                }
                i++;
            } while (i < *(int *)D_L05_001B0CB0[*(int *)d]);
        }
        break;
    case 1:
        t = func_001FA748(*(float *)(d + 0xC), D_0015EE6C * 6.28318548f);
        *(float *)(d + 0xC) = t;
        r = func_001F9FA8(t);
        v = func_001FA898_r((r * 4.0f - 3.0f) * 128.0f);
        if (v < 0x81) {
            v = (0x1F < v) ? v : 0x20;
        } else {
            v = 0x80;
        }
        *(int *)(m + 0x90) = (v << 16) | (v << 8) | 0x80000000 | v;
        X = D_0013E633 + 0xE1D;
        if (*(int *)(X + 0x2FC) == (int)m && *(short *)(X + 0x30E) == 0) {
            h = *(short *)(m + 0xB2);
            b0 = m[0xB0];
            *(unsigned char *)(D_L05_001BADE0 + h + 0x454) = b0 + 2;
            if (b0 == 0xFF || (D_L05_0015FD48[b0] != 0xFF && *(unsigned char *)(D_0014171B + 0xAA35 + b0 + ((*(int *)&D_L05_0015FE84) << 4)) == 0xFF)) {
                *(unsigned char *)(D_L05_001BBA40 + *(short *)(m + 0xB2) + 0x454) = b0 + 2;
            }
            m[0xBC] = 1;
            m[0x20] = 2;
            *(int *)(m + 0x90) = 0x80208020;
            func_0022ED80(0, 0, (int)m);
            if (*(int *)d == -1) {
                return;
            }
            if (*(int *)D_L05_001B0CB0[*(int *)d] > 0) {
                i = 0;
                do {
                    if (*(float *)(D_L05_001B0CB0[*(int *)d] + (i << 4) + 0x1C) == *(float *)(d + 4)) {
                        *(int *)(D_L05_001B0CB0[*(int *)d] + (i << 4) + 0x1C) = 0;
                    }
                    i++;
                } while (i < *(int *)D_L05_001B0CB0[*(int *)d]);
            }
        }
            break;
    }
}
