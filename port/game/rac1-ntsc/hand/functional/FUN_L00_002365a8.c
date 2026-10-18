/* Carried from the PAL decompilation's nonmatching/shared/func_L00_00236F38.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * functional: high. Read against the US assembly on 2026-10-18: the rect call now goes to the US
 * draw_hud_rect (00200C80) and the blink's frame scaling to 001F96F8 (both were PAL addresses);
 * offsets, constants, bands and the particle loop agree. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
typedef struct {
    char pad0[0x8];
    int unk08;
    int *unk0C;
    char pad10[0x38];
    short unk48;
    short unk4A;
    char pad4C[0x4];
    int unk50;
    int unk54;
    int w;
    int h;
    int flags;
    char pad64[0x8];
    int unk6C;
    unsigned char cnt[4];
    int unk74;
    int unk78;
    int unk7C;
    void *unk80;
} HudElem_c2365a8;
int func_L00_00235A70(HudElem_c2365a8 *rec, int *x, int *y);
void func_L00_00235AD8(HudElem_c2365a8 *e, int *x, int *y, int t, int d);
extern int D_L00_0015F3F8 MACRO_ADDR;
extern float D_L00_0017E660[][4];
extern short D_L00_0015F750;
extern short D_L00_0015F754;
extern short D_L00_0015F758;
/* NON_MATCHING FUN_L00_002365a8 -- src/overlays/shared/hud_00235960.c
 * Best so far: BYTES 99/1384 (92.8% of the bytes match), checked 2026-10-16.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Gauge draw: frame rect, a blink below a third (sound 0xB on its rising edge), then 100 particles in bands.
 *   The two blink tests are one static inline helper (retail reloads 0.75 and 60.0 and the frame inside each).
 *   SDATA floats for 0x15F750..38 (the file's -G2 made the neighbours declare them short).
 *   Scale before position and step[1] before step[0] each brought the score down (209 -> 99).
 *   Still differing: 160*H/208*W swap a2/a3 in the rect, and four float regs in the setup (1.0, 0.5, b, 1-a).
 *   Tried: term and statement orders, locals or none, constant stores moved, half sizes kept in scale[].
 */
/* US draw_hud_rect (FUN_00200c80); the carry had kept PAL's 00200E08, another US function. */
extern void draw_hud_rect_c2365a8(int, int, int, int, unsigned long, int) __asm__("func_00200C80");
/* US scale_game_frames (FUN_001f96f8); the carry had kept PAL's 001F9688. */
extern int scale_frames_c2365a8(int) __asm__("func_001F96F8");
extern float func_001F96B0(float);
extern int func_L00_00216DE8(int, int);
extern int func_001FF960(int, int);
extern void func_001F99F8(void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF348(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_00200080(int, int, int, int, int, int);
extern void func_L00_0023BAC0(int, int, int, int, int, int);
extern int D_L00_0015F760 SDATA(D_L00_0015F760);
extern int D_L00_0015F764 SDATA(D_L00_0015F764);
extern float D_L00_0015F768 SDATA(D_L00_0015F768);
extern float D_L00_0015F76C SDATA(D_L00_0015F76C);
extern float D_L00_0015F830_f SDATA(D_L00_0015F750);
extern float D_L00_0015F834_f SDATA(D_L00_0015F754);
extern float D_L00_0015F838_f SDATA(D_L00_0015F758);
/* whether a value below a third blinks on, ago frames back */
static inline int hud_blink_c2365a8(HudElem_c2365a8 *e, int ago) {
    if (e->unk74 * 3 <= 10000) {
        return (D_L00_0015F3F8 - ago) % (int)(scale_frames_c2365a8(1) * 60.0f) < (int)(func_001F96B0(0.75f) * 60.0f);
    }
    return 0;
}



/* draws a gauge: its frame, an icon that blinks while the value (+0x74, out of 10000) is below a
   third, and the particles of func_L00_00236268 up to the value's level in three bands */
int FUN_L00_002365a8(HudElem_c2365a8 *e) {
    int x, y;
    int was, on;
    int tex, sprite, size;
    int i;
    float level, fw, fh, a, b;
    float scale[4], pos[4], step0[4], step1[4], step2[4], p[4];

    x = e->unk50;
    y = e->unk54;
    e->w = D_L00_0015F760;
    e->h = D_L00_0015F764;
    func_L00_00235A70(e, &x, &y);
    func_L00_00235AD8(e, &x, &y, e->unk6C, 0);
    {
        int x0 = (x + e->unk48) * 16 + D_L00_0015F760 * 208 / 64;
        int y0 = (y + e->unk4A) * 16 + D_L00_0015F764 * 160 / 256;
        draw_hud_rect_c2365a8(x0, y0, x0 + D_L00_0015F760 * 608 / 64, y0 + D_L00_0015F764 * 3744 / 256,
                      0x80684C2F, 1);
    }

    was = hud_blink_c2365a8(e, 1);
    on = hud_blink_c2365a8(e, 0);
    if (!was && on == 1) {
        func_L00_00216DE8(0xB, 0);
    }

    size = 0;
    tex = func_001FF960(*(int *)e, on);
    func_L00_0023BAC0(tex, x + e->unk48, y + e->unk4A, D_L00_0015F760, D_L00_0015F764, 0x80);

    fh = D_L00_0015F764;
    fw = D_L00_0015F760;
    b = D_L00_0015F76C;
    a = D_L00_0015F768;
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
    sprite = func_001FF960(*(int *)e, 2);

    func_001F99F8(step0);
    func_001F99F8(step1);
    func_001F99F8(step2);
    step0[1] = step0[0] = D_L00_0015F830_f * 16.0f;
    step1[1] = step1[0] = (D_L00_0015F834_f - D_L00_0015F830_f) * 16.0f;
    step2[1] = step2[0] = (D_L00_0015F838_f - D_L00_0015F834_f) * 16.0f;

    for (i = 0; i < 100; i++) {
        if (i == 0) {
            func_001F9A28(pos, pos, step0);
            level += D_L00_0015F830_f;
            size = D_L00_0015F830_f * 32.0f;
        } else if (i == 10) {
            func_001F9A28(pos, pos, step1);
            size = D_L00_0015F834_f * 32.0f;
            level += D_L00_0015F834_f - D_L00_0015F830_f;
        } else if (i == 30) {
            func_001F9A28(pos, pos, step2);
            size = D_L00_0015F838_f * 32.0f;
            level += D_L00_0015F838_f - D_L00_0015F834_f;
        }
        if (level < D_L00_0017E660[i][1]) continue;
        func_L00_001FF348(p, D_L00_0017E660[i], scale);
        func_001F9A10(p, p, pos);
        func_00200080(sprite, p[0], p[1], size, size, 0x80);
    }
    return e->w;
}
