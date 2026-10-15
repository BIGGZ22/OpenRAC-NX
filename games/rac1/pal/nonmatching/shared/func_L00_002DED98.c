/* NON_MATCHING func_L00_002DED98 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: BYTES 4/948 (99.6% of the bytes match), checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: BYTES 4/948. Three changes got here: the 0.3f step in a local (`step = 0.3f;` right before `a
 *   += step`, so the pre-loop add and the loop share one register as in retail), the colour loop as an index
 *   loop `for (k = 0; k < 2; k++)` over s.col[k * 2] (GCC derives the pointer and the down counter itself,
 *   after the hoisted constants, as retail does), and the row reads in retail's order (y = D_L00_001E6358[i -
 *   1] before v = [i]). Left: the spill slots of the two row pointers are swapped (ours [i-1] at 0xD0, retail
 *   at 0xD4). Reading v first, other spellings of [i - 1] and declaration orders do not swap them. Retail
 *   computes &D[i-1] first at the top of the row yet gives it the higher slot.
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
    float step;
    int i, j, k, v;
    unsigned short y;
    s.t1 = func_001F4868(0x15);
    s.t2 = 0x0000FF9000000260L; /* FIX: retail is ori 0xFF90; dsll32 0; ori 0x260 = 0xFF90<<32 | 0x260 (was 0xFF90<<48) */
    s.t3 = 0x8000000048L;
    s.t0 = 0;
    a = *(float *)&D_L00_00161C18;
    *(float *)&D_L00_00161C18 = a + 0.001f;
    if (*(float *)&D_L00_00161C18 > 7.0f) *(float *)&D_L00_00161C18 -= 7.0f;
    step = 0.3f;
    a += step;
    for (i = 1; i < 10; i++) {
        b = *(float *)&D_L00_00161C1C;
        *(float *)&D_L00_00161C1C = b + *(float *)&D_L00_00161C20;
        if (*(float *)&D_L00_00161C1C > 7.0f) *(float *)&D_L00_00161C1C -= 7.0f;
        a2 = a + step;
        for (j = 0; j < 20; j++) {
            qcopy(s.q[0], D_L00_001E6370 + (j * 16 + i * 0x140));
            qcopy(s.q[1], D_L00_001E6370 + ((j + 1) % 20 * 16 + i * 0x140));
            qcopy(s.q[2], D_L00_001E6230 + (j * 16 + i * 0x140));
            qcopy(s.q[3], D_L00_001E6230 + ((j + 1) % 20 * 16 + i * 0x140));
            func_001F9BF0(vc, s.q[0], s.q[1]);
            func_001F9BF0(vd, s.q[2], s.q[1]);
            func_001F9CA0(ve, vd, vc);
            func_001F9BF0(vf, s.q[1], D_L00_00166EC0);
            dot = func_001F9C78(ve, vf);
            if (0.0f <= dot) {
                b += 0.05f;
            } else {
                b2 = b + 0.05f;
                am = a - step;
                y = D_L00_001E6358[i - 1];
                v = D_L00_001E6358[i];
                for (k = 0; k < 2; k++) {
                    if (v < 0) v = 0;
                    if (v > 0x40) v = 0x40;
                    v = (v << 24) | 0x404040;
                    s.col[k * 2] = v;
                    s.col[k * 2 + 1] = v;
                    v = (short)y;
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
