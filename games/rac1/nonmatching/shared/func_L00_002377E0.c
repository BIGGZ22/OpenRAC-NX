/* NON_MATCHING func_L00_002377E0 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 840 / retail 832, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_002377E0: draws a HUD element: sets e->w/h from globals, places it (00236400/00236468), draws the bac
 *   p5.c/p6.c is the best shape (same size 832, BYTES ~570): float math, call order and table loop are right (t re
 *   Remaining gaps: (1) loop.c strength-reduces `D + i*16` (arg to 1FF2F8) together with the float read, retail re
 *   (2) resulting saved-register order (retail: $19 float ptr, $20 i, $21 size, $22 tex, $23 v2, $30 hi) and early
 */
extern int func_00200198(int, int);
extern void func_L00_0023C458(int, int, int, int, int, int);
extern void func_001F9BC0(void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF2F8(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_002008B8(int, int, int, int, int, int);
extern float D_L00_0017E764[];
extern short D_L00_0015F840;
extern short D_L00_0015F844;
extern short D_L00_0015F848;
extern short D_L00_0015F84C;
extern short D_L00_0015F830;
extern short D_L00_0015F834;
extern short D_L00_0015F838;

/* Draws a HUD element's backdrop and the 100 entries of its particle table that are due */
int func_L00_002377E0(HudElem *e) {
    float v0[4], v1[4], v2[4], v3[4], v4[4], v5[4];
    int x, y;
    float u, t;
    int tex, sz, i;

    x = e->unk50;
    y = e->unk54;
    e->w = *(int *)&D_L00_0015F840;
    e->h = *(int *)&D_L00_0015F844;
    func_L00_00236400(e, &x, &y);
    func_L00_00236468(e, &x, &y, e->unk6C, 0);
    tex = func_00200198(*(int *)e, 0);
    func_L00_0023C458(tex, x + e->unk48, y + e->unk4A, *(int *)&D_L00_0015F840, *(int *)&D_L00_0015F844, 0x80);
    u = (10000 - e->unk74) / 10000.0f;
    v0[0] = (*(int *)&D_L00_0015F840 * *(float *)&D_L00_0015F848 * 0.5f) / 17.0f * 16.0f;
    v0[1] = -(*(int *)&D_L00_0015F844 * *(float *)&D_L00_0015F84C * 0.5f) / 116.0f * 16.0f;
    v0[2] = 1.0f;
    v0[3] = 1.0f;
    v1[0] = (*(int *)&D_L00_0015F840 * *(float *)&D_L00_0015F848 * 0.5f + (x + e->unk48)
             + (1.0f - *(float *)&D_L00_0015F848) * 0.5f * *(int *)&D_L00_0015F840) * 16.0f;
    v1[1] = (*(int *)&D_L00_0015F844 * *(float *)&D_L00_0015F84C * 0.5f + (y + e->unk4A)
             + (1.0f - *(float *)&D_L00_0015F84C) * 0.5f * *(int *)&D_L00_0015F844) * 16.0f;
    v1[2] = 0.0f;
    v1[3] = 0.0f;
    t = 116.0f - (u + u) * 116.0f;
    tex = func_00200198(*(int *)e, 1);
    func_001F9BC0(v2);
    func_001F9BC0(v3);
    func_001F9BC0(v4);
    v2[0] = v2[1] = *(float *)&D_L00_0015F830 * 16.0f;
    v3[0] = v3[1] = (*(float *)&D_L00_0015F834 - *(float *)&D_L00_0015F830) * 16.0f;
    v4[0] = v4[1] = (*(float *)&D_L00_0015F838 - *(float *)&D_L00_0015F834) * 16.0f;
    sz = 0;
    for (i = 0; i < 100; i++) {
        if (i == 0) {
            func_001F9BF0(v1, v1, v2);
            sz = (int)(*(float *)&D_L00_0015F830 * 32.0f);
            t += *(float *)&D_L00_0015F830;
        }
        if (t < D_L00_0017E764[i * 4]) continue;
        func_L00_001FF2F8(v5, D_L00_0017E760 + i * 16, v0);
        func_001F9BD8(v5, v5, v1);
        func_002008B8(tex, (int)v5[0], (int)v5[1], sz, sz, 0x80);
    }
    return e->w;
}
