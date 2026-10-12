/* NON_MATCHING func_L06_0030D3E8 -- src/overlays/shared/vendor_002FF000.c
 * Best so far: BYTES 171/472 (63.8% of the bytes match), checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Draws a fading round marker (4-vertex packed longs, K=0x00FFFFF080008000) at a moby's projected position. Need
 *   q28/t06: p8.c is the best this round (468 vs retail 472; earlier best 476). Changes that helped: P/Q locals fo
 *   Left: retail copies the first func_001FA898 result into $s0 (`daddu $s0,$v0,$zero`, then `addiu $s0,$s0,-0x100
 *   s03 (hq1): draws a fading marker: func_00234C98 and func_001F2418, func_001FA898, func_001FA8A8, then 64-bit p
 *   hq13 n01 (p14-p21, budget spent): closest is p21.c (p8 with the Q row `(y - r) << 16` computed before the P ro
 */
extern void func_00234C98(int, long);
extern void func_001F2418(float *, void *);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_001FA8A8(int, int, float);
extern void func_L02_0020BF88(void *, void *, void *, int, int);
extern char D_0013E15A[];
extern short D_L06_001624AC;
extern short D_L06_001624B0;
extern short D_L06_001624B4;
extern short D_L06_001624B8;
extern short D_L06_001624A8;
extern short D_L06_001624A4;

// Draws a fading round marker at the moby's screen position.
extern void func_L06_0030D3E8_x(char *) __asm__("func_L06_0030D3E8");
void func_L06_0030D3E8_x(char *moby) {
    char *data = *(char **)(moby + 0x78);
    float pos[4];
    float scr[4];
    int col[4];
    int fill[4];
    long q[4];
    long p;
    long qq;
    float a;
    float b;
    int x;
    int y;
    int r;
    int fv;
    int z;
    int *bp;
    qcopy(pos, moby + 0x10);
    a = *(float *)data;
    if (a > 1.0f) a = 1.0f;
    b = *(float *)data - 1.0f;
    if (b > 1.0f) {
        b = 1.0f;
    } else if (b < 0.0f) {
        b = 0.0f;
    }
    func_00234C98(0x47, 0x5380B);
    func_001F2418(scr, pos);
    bp = (int *)(D_0013E15A + 0x4A6);
    x = func_001FA898_r(scr[0] - (float)bp[4]);
    x -= 0x1000;
    y = func_001FA898_r(scr[1] - (float)bp[4]) - 0x1000;
    col[0] = *(int *)&D_L06_001624AC;
    col[1] = *(int *)&D_L06_001624B0;
    col[2] = *(int *)&D_L06_001624B4;
    col[3] = *(int *)&D_L06_001624B8;
    fv = func_001FA8A8(*(int *)(data + 8), *(int *)(data + 0xC), b);
    fill[0] = fv;
    fill[3] = fv;
    fill[2] = fv;
    fill[1] = fv;
    r = func_001FA898_r((float)(*(int *)&D_L06_001624A8 << 4) * a);
    qq = (long)((y - r) << 16) + (long)0x00FFFFF080008000L;
    p = (long)((y + r) << 16) + (long)0x00FFFFF080008000L;
    q[3] = p + (x - r);
    q[1] = qq + (x - r);
    q[0] = qq + (x + r);
    q[2] = p + (x + r);
    z = func_001F4868(*(int *)&D_L06_001624A4);
    func_L02_0020BF88(q, col, fill, z, 1);
    func_00234C98(0x47, 0x5360B);
}
