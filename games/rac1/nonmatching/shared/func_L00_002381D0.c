/* NON_MATCHING func_L00_002381D0 -- src/overlays/shared/hud_00235960.c
 * Best so far: SIZE ours 672 / retail 680, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Cannot land as written (#define in a candidate): rewrite that in plain C first.
 * What the last attempts found:
 *   func_L00_002381D0: draws a HUD gauge (shared hud): stores the element's w/h from two gp globals, anchors it wi
 *   Best p4/p6/p7: 668 bytes vs retail 680, structure and all multiplies match (separate temps h1/h2/h3 = HH*0xEA0
 */
extern short D_L00_0015F840;
extern short D_L00_0015F844;
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_L00_00236468(HudElem *, int *, int *, int, int);
extern void func_L00_0023C458(int, int, int, int, int, int);
#define HW (*(int *)&D_L00_0015F840)
#define HH (*(int *)&D_L00_0015F844)

/* Draws a HUD gauge frame: a background bar and tick marks from the element's position and size. */
int func_L00_002381D0(HudElem *e) {
    int pos[2];
    int x0, x1, y, i, hs;

    pos[0] = e->unk50;
    pos[1] = e->unk54;
    e->w = HW;
    e->h = HH;
    func_L00_00236400(e, &pos[0], &pos[1]);
    func_L00_00236468(e, &pos[0], &pos[1], e->unk6C, 0);
    hs = HH * 0xEA0;
    x0 = ((pos[0] + e->unk48) << 4) + (HW * 0xD0) / 64;
    y = ((pos[1] + e->unk4A) << 4) + (HH * 0xA0) / 256;
    x1 = x0 + (HW * 0x260) / 64;
    func_00201640(x0, y, x1, y + hs / 256, 0x80000000, 1);
    y = ((pos[1] + e->unk4A) << 4) + (HH * 0xF40) / 256;
    func_00201640(x0, y, x1, y - (e->unk74 * hs) / (e->unk08 << 8), 0x80829E00, 1);
    for (i = 1; i < e->unk74; i++) {
        hs = HH * 0xEA0;
        y = ((pos[1] + e->unk4A) << 4) + (HH * 0xF40) / 256 - (i * hs) / (e->unk08 << 8) - 0x10;
        func_00201640(x0, y, x1, y + 0x20, 0x80000000, 1);
    }
    func_L00_0023C458(func_00200198(*(int *)e, 0), pos[0] + e->unk48, pos[1] + e->unk4A, HW, HH, 0x80);
    return e->w;
}
