/* NON_MATCHING func_L00_00236F38 -- src/overlays/shared/hud_00235960.c
 * Best so far: BYTES 99/1384 (92.8% of the bytes match), checked 2026-10-16.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Gauge draw: frame rect, a blink below a third (sound 0xB on its rising edge), then 100 particles in bands.
 *   The two blink tests are one static inline helper (retail reloads 0.75 and 60.0 and the frame inside each).
 *   SDATA floats for 0x15F830..38 (the file's -G2 made the neighbours declare them short).
 *   Scale before position and step[1] before step[0] each brought the score down (209 -> 99).
 *   Still differing: 160*H/208*W swap a2/a3 in the rect, and four float regs in the setup (1.0, 0.5, b, 1-a).
 *   Tried: term and statement orders, locals or none, constant stores moved, half sizes kept in scale[].
 *   fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18).
 */
extern void func_00201640(int, int, int, int, long, long);
extern int func_001F9850(int);
extern float func_001F9878(float);
extern int func_L00_00217570(int, int);
extern int func_00200198(int, int);
extern void func_001F9BC0(void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF2F8(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_002008B8(int, int, int, int, int, int);
extern void func_L00_0023C458(int, int, int, int, int, int);
extern int D_L00_0015F840 SDATA(D_L00_0015F840);
extern int D_L00_0015F844 SDATA(D_L00_0015F844);
extern float D_L00_0015F848 SDATA(D_L00_0015F848);
extern float D_L00_0015F84C SDATA(D_L00_0015F84C);
extern float D_L00_0015F830_f SDATA(D_L00_0015F830);
extern float D_L00_0015F834_f SDATA(D_L00_0015F834);
extern float D_L00_0015F838_f SDATA(D_L00_0015F838);

/* whether a value below a third blinks on, ago frames back */
static inline int hud_blink(HudElem *e, int ago) {
    if (e->unk74 * 3 <= 10000) {
        return (D_L00_0015F4F8 - ago) % (int)(func_001F9850(1) * 60.0f) < (int)(func_001F9878(0.75f) * 60.0f);
    }
    return 0;
}

/* draws a gauge: its frame, an icon that blinks while the value (+0x74, out of 10000) is below a
   third, and the particles of func_L00_00236BF8 up to the value's level in three bands */
int func_L00_00236F38(HudElem *e) {
    int x, y;
    int was, on;
    int tex, sprite, size;
    int i;
    float level, fw, fh, a, b;
    float scale[4], pos[4], step0[4], step1[4], step2[4], p[4];

    x = e->unk50;
    y = e->unk54;
    e->w = D_L00_0015F840;
    e->h = D_L00_0015F844;
    func_L00_00236400(e, &x, &y);
    func_L00_00236468(e, &x, &y, e->unk6C, 0);
    {
        int x0 = (x + e->unk48) * 16 + D_L00_0015F840 * 208 / 64;
        int y0 = (y + e->unk4A) * 16 + D_L00_0015F844 * 160 / 256;
        func_00201640(x0, y0, x0 + D_L00_0015F840 * 608 / 64, y0 + D_L00_0015F844 * 3744 / 256,
                      0x80684C2F, 1);
    }

    was = hud_blink(e, 1);
    on = hud_blink(e, 0);
    if (!was && on == 1) {
        func_L00_00217570(0xB, 0);
    }

    size = 0;
    tex = func_00200198(*(int *)e, on);
    func_L00_0023C458(tex, x + e->unk48, y + e->unk4A, D_L00_0015F840, D_L00_0015F844, 0x80);

    fh = D_L00_0015F844;
    fw = D_L00_0015F840;
    b = D_L00_0015F84C;
    a = D_L00_0015F848;
    level = (float)(10000 - e->unk74) / 10000.0f;
    scale[2] = 1.0f;
    scale[3] = 1.0f;
    pos[2] = 0.0f;
    pos[3] = 0.0f;
    scale[0] = fw * a * 0.5f / 17.0f * 16.0f;
    scale[1] = -(fh * b * 0.5f) / 116.0f * 16.0f;
    pos[0] = (fw * a * 0.5f + (float)(x + e->unk48) + (1.0f - a) * 0.5f * fw) * 16.0f;
    pos[1] = (fh * b * 0.5f + (float)(y + e->unk4A) + (1.0f - b) * 0.5f * fh) * 16.0f;
    level = 116.0f - level * 2.0f * 116.0f;
    sprite = func_00200198(*(int *)e, 2);

    func_001F9BC0(step0);
    func_001F9BC0(step1);
    func_001F9BC0(step2);
    step0[1] = step0[0] = D_L00_0015F830_f * 16.0f;
    step1[1] = step1[0] = (D_L00_0015F834_f - D_L00_0015F830_f) * 16.0f;
    step2[1] = step2[0] = (D_L00_0015F838_f - D_L00_0015F834_f) * 16.0f;

    for (i = 0; i < 100; i++) {
        if (i == 0) {
            func_001F9BF0(pos, pos, step0);
            level += D_L00_0015F830_f;
            size = D_L00_0015F830_f * 32.0f;
        } else if (i == 10) {
            func_001F9BF0(pos, pos, step1);
            size = D_L00_0015F834_f * 32.0f;
            level += D_L00_0015F834_f - D_L00_0015F830_f;
        } else if (i == 30) {
            func_001F9BF0(pos, pos, step2);
            size = D_L00_0015F838_f * 32.0f;
            level += D_L00_0015F838_f - D_L00_0015F834_f;
        }
        if (level < D_L00_0017E760[i][1]) continue;
        func_L00_001FF2F8(p, D_L00_0017E760[i], scale);
        func_001F9BD8(p, p, pos);
        func_002008B8(sprite, p[0], p[1], size, size, 0x80);
    }
    return e->w;
}
