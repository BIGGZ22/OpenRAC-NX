/* NON_MATCHING func_L00_002CD7B0 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: SIZE ours 772 / retail 776, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_002CD7B0: picks the best target moby from the D_L00_001ABD80 list (skip list of 7, range/yaw/pitch te
 *   Difference: near the end retail re-tests `u == 0` (`beqz $3`, redundant after the earlier null test of u = *(D
 *   Would unblock: a source form in which the second null test is not dominated by the first (unknown); stopped af
 */
extern int D_L00_00173F40[];
extern int D_L00_001ABD80[];
extern int func_L00_0025D390(char *);
extern float func_001F9D48(void *, void *);
extern float func_L00_001FF860(float, float);
extern float func_001FA850(float, float);
extern int func_L00_001EFFF0(void *, void *, int, int, int);

/* Finds the best-scoring target moby in the target list within range and angle limits. */
char *func_L00_002CD7B0(float *pos, float *lim, int *skip, float maxy, float maxp, float maxd) {
    char *best = 0;
    float bestScore = 1e9f;
    int i = 0;
    char *m = (char *)D_L00_001ABD80[0];
    while (m != 0) {
        int n = i + 1;
        if (*(short *)(m + 0x32) != 0) {
            int found = 0;
            if (skip != 0) {
                int k;
                for (k = 0; k < 7; k++) {
                    if (skip[k] == (int)m) { found = 1; break; }
                }
            }
            if (skip != 0 ? found == 0 : *(unsigned char *)(m + 0x31) != 0) {
                float *r = (float *)func_L00_0025D390(m);
                if (r != 0 && !(r[0] < 0.0f) && (*(unsigned short *)(m + 0x34) & 0x1000) && m != 0
                    && *(int *)(m + 0x24) != 0 && *(short *)(*(int *)(m + 0x24) + 0x46) == 5) {
                    float *p = (float *)(m + 0x10);
                    float d = func_001F9D48(pos, p);
                    if (d < maxd) {
                        float v[4];
                        float a, b, e, g, sc;
                        int t, u;
                        qcopy(v, p);
                        v[2] = v[2] + (r[4] + 0.1f);
                        a = func_L00_001FF860(v[0] - pos[0], v[1] - pos[1]);
                        a = func_001FA850(lim[2], a);
                        e = a * a;
                        if (e < maxy * maxy) {
                            b = func_L00_001FF860(d, v[2] - pos[2]);
                            b = func_001FA850(lim[1], b);
                            g = b * b;
                            if (g < maxp * maxp) {
                                if (d > 5.0f) sc = e * g * d + d;
                                else sc = d / 5.0f;
                                if (sc < bestScore
                                    && (func_L00_001EFFF0(pos, v, 2, (int)m, 0) == 0
                                        || ((t = D_L00_00173F40[6]) != 0
                                            && (u = *(int *)(t + 0x24)) != 0
                                            && (*(short *)(u + 0x46) == 5
                                                || (u != 0 && *(short *)(u + 0x46) == 0xF))))) {
                                    bestScore = sc;
                                    best = m;
                                }
                            }
                        }
                    }
                }
            }
        }
        i = n;
        m = (char *)D_L00_001ABD80[i];
    }
    return best;
}
