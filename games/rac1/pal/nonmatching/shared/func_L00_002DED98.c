/* NON_MATCHING func_L00_002DED98 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 956 / retail 948, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_002DED98 (6 of 10 runs used; best p5.c, 956 vs 948 bytes): draws a 9x20 grid of quads (func_L00_001FD
 *   scroll angles D_L00_00161C18/1C/20; per quad: 4 qcopies from tables D_L00_001E6370/6230, vector math, dot-prod
 *   D_L00_001E6358. Packet struct is 0x90 bytes at sp+0 (note: sizeof(long)==8 and long long is 128-bit here, so t
 *   Structure, loop shape, delay slots and store orders all match. Two differences remain: (1) retail has no `mov.
 *   0.3f used by the pre-loop `a += 0.3f` and by the loop is one pseudo in $f24, ours keeps a separate temp and co
 *   stack-address spill slots (D8..F8) come out in a different order (ours rotates [ve,q1] to the end).
 *   Unblock: whatever source shape makes the pre-loop add share the loop-invariant 0.3f pseudo; then slot order ma
 */
extern short D_L00_001E6358[];
extern short D_L00_00161C18;
extern short D_L00_00161C1C;
extern short D_L00_00161C20;
extern char D_L00_001E6370[];
extern char D_L00_001E6230[];
extern int func_001F4868(int);
extern void func_001F9CA0(void *, void *, void *);
extern float func_001F9C78(void *, void *);
extern void func_L00_001FD1D8(void *, void *, int);
extern void func_00234C98(int, long);

typedef struct {
    float q[4][4];
    unsigned int col[4];
    float x0, y0, x1, y1, x2, y2, x3, y3;
    long t0, t1, t2, t3;
} Pkt;

// Draws a grid of ten rows by twenty strips, stepping the two scroll angles and coloring each strip from a per-row table.
void func_L00_002DED98(void) {
    Pkt s;
    float vc[4];
    float vd[4];
    float ve[4];
    float vf[4];
    float a, a2, am, b, b2, dot;
    int i, j, k, v;
    unsigned short y;
    unsigned int *o;
    s.t1 = func_001F4868(0x15);
    s.t2 = 0x0000FF9000000260L; /* FIX: retail is ori 0xFF90; dsll32 0; ori 0x260 = 0xFF90<<32 | 0x260 (was 0xFF90<<48) */
    s.t3 = 0x8000000048L;
    s.t0 = 0;
    a = *(float *)&D_L00_00161C18;
    *(float *)&D_L00_00161C18 = a + 0.001f;
    if (*(float *)&D_L00_00161C18 > 7.0f) *(float *)&D_L00_00161C18 -= 7.0f;
    a += 0.3f;
    for (i = 1; i < 10; i++) {
        b = *(float *)&D_L00_00161C1C;
        *(float *)&D_L00_00161C1C = b + *(float *)&D_L00_00161C20;
        if (*(float *)&D_L00_00161C1C > 7.0f) *(float *)&D_L00_00161C1C -= 7.0f;
        a2 = a + 0.3f;
        for (j = 0; j < 20; j++) {
            qcopy(s.q[0], D_L00_001E6370 + (i * 0x140 + j * 16));
            qcopy(s.q[1], D_L00_001E6370 + (i * 0x140 + (j + 1) % 20 * 16));
            qcopy(s.q[2], D_L00_001E6230 + (i * 0x140 + j * 16));
            qcopy(s.q[3], D_L00_001E6230 + (i * 0x140 + (j + 1) % 20 * 16));
            func_001F9BF0(vc, s.q[0], s.q[1]);
            func_001F9BF0(vd, s.q[2], s.q[1]);
            func_001F9CA0(ve, vd, vc);
            func_001F9BF0(vf, s.q[1], D_L00_00166EC0);
            dot = func_001F9C78(ve, vf);
            if (0.0f <= dot) {
                b += 0.05f;
            } else {
                b2 = b + 0.05f;
                am = a - 0.3f;
                v = D_L00_001E6358[i];
                y = D_L00_001E6358[i - 1];
                o = s.col;
                for (k = 1; k >= 0; k--) {
                    if (v < 0) v = 0;
                    if (v > 0x40) v = 0x40;
                    v = (v << 24) | 0x404040;
                    o[0] = v;
                    o[1] = v;
                    v = (short)y;
                    o += 2;
                }
                if (i == 1) {
                    s.col[3] = 0x20404040;
                    s.col[2] = 0x20404040;
                } else if (i == 9) {
                    s.col[1] = 0x404040;
                    s.col[0] = 0x404040;
                }
                s.x0 = a;
                s.y0 = b;
                s.x1 = a;
                s.y1 = b2;
                s.x2 = am;
                s.y2 = b;
                s.x3 = am;
                s.y3 = b2;
                b = b2;
                func_L00_001FD1D8(&s, 0, 0);
            }
        }
        a = a2;
    }
    func_00234C98(0x47, 0x5360B);
}
