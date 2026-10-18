/* functional: high. Checked against the US assembly: window offsets, flags, wrap and rebalance passes, colour table D_0018CAF8, both draw calls and their arguments, the clip resets. */
/* Carried from the PAL decompilation's nonmatching/game/func_001F7070.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
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
extern int D_0013E500[];
/* gp-relative: declared as a 2-byte type purely so -G2 places it in the
   small-data area (placement is decided by DECLARED size), then accessed
   as the 4-byte word it really is. gp base 0x166C00 - 0x7764 = 0x15F49C. */
extern short D_0015F49C;
extern int func_001F6200(unsigned char *arg0, int arg1, void *arg2);
struct Glyph_c1f7090 {
    u8 u;
    u8 v;
    s8 top;
    s8 adv;
};
extern s32 D_0015F4A0 MACRO_ADDR;
extern s32 D_0018CAF8[];
void func_001F6638(u64 color, u8 *s, s32 n, s64 tex, struct Glyph_c1f7090 *g, f32 x, f32 y, f32 scale);
/*
 * FontPrintWindow: word-wraps a string into the window rectangle (balancing
 * the lines when they fit), then draws each line, or only measures it (flag 4).
 * Window shorts: [0] y_min [1] y_max [2] x_min [3] x_max [4] x_anchor
 * [5] y_start [6] out max width [7] out height [8] line height [9] flags
 * [10] x sub-pixel [11] y sub-pixel. Flags: 1 centre on x_anchor, 2 centre
 * vertically, 4 measure only, 8 float positions.
 */
extern void func_00233A40(int, int, int, int);
/* Same symbols as the file declares, with the 64-bit argument the registers carry. */
extern void func_001F6668_L(void *, void *, void *, void *, void *, long, unsigned char *)
    __asm__("func_001F62B0");

void func_001F7070_impl(void *win_p, void *a1_p, void *text_p, void *len_p, long a4, unsigned char *glyphs)
    __asm__("FUN_001f7090");



void func_001F7070_impl(void *win_p, void *a1_p, void *text_p, void *len_p, long a4, unsigned char *glyphs)
{
    short *win = (short *)win_p;
    unsigned char *text = (unsigned char *)text_p;
    int a1 = (int)(long)a1_p;
    int len = (int)(long)len_p;
    short start[32];
    short endi[32];
    short colr_arr[32];
    int W, Wsave, line, i, brk, width, colr, lines, n0, last, rewrapped;
    int k, y, lh, flags, height, wc, c, cnt, s, e, half, cc, xw, yh;
    int adv, x;
    float f0, f1, f13;

    func_00233A40(win[2], win[3] - 1, win[0], win[1] - 1);
    D_0015F4A0 = 1;
    flags = (unsigned short)win[9];

    if (flags & 1) {
        int a = win[3] - win[4];
        int b = win[4] - win[2];
        W = (a < b ? a : b) * 2;
    } else {
        W = win[3] - win[4];
    }
    Wsave = W;
    colr = 0;
    n0 = 0;
    last = 0;
    rewrapped = 0;

pass:
    line = 0;
    i = 0;
    if (len == 0 || text[0] == 0)
        goto after_lines;
    for (;;) {
        start[line] = (short)i;
        colr_arr[line] = (short)colr;
        brk = i;
        width = 0;
        if (W > 0) {
            wc = *(int *)&D_0015F49C;
            for (;;) {
                c = text[i];
                if (c == 0x20 || c < 0x10)
                    brk = i;
                if (wc != 0 && (unsigned int)(c - 8) < 8)
                    colr = c - 8;
                if (c < 2)
                    break;
                i++;
                adv = (signed char)glyphs[c * 4 + 3];
                if (adv != 0)
                    width += adv;
                if (width >= W)
                    break;
            }
        }
        endi[line] = (short)brk;
        if ((short)brk == start[line])
            endi[line] = (short)i;
        i = endi[line];
        cc = text[i];
        if (cc == 0x20 || cc < 0x10)
            endi[line] = (short)(i - 1);
        line = line + 1;
        if (text[i] == 0) {
            last = width;
            goto after_lines;
        }
        i = i + 1;
        if (i == len)
            goto after_lines;
        if (text[i] == 0)
            goto after_lines;
    }

after_lines:
    lines = line;
    if (n0 == 0)
        n0 = lines;
    if (rewrapped || lines < 2)
        goto draw;
    if (n0 < lines) {
        W = Wsave;
        rewrapped = 1;
        goto pass;
    }
    if (last < W / 3) {
        W -= 16;
        goto pass;
    }
    W -= 16;
    goto draw;

draw:
    lh = win[8];
    height = lines * lh;
    win[6] = 0;
    y = win[5];
    if (flags & 2)
        y -= height >> 1;
    win[7] = (short)height;
    if (lines <= 0)
        goto final;

    for (k = 0; k < lines; k++) {
        if (y + lh < win[0] || win[1] < y)
            goto next;
        s = start[k];
        e = endi[k];
        cnt = e - s + 1;
        width = func_001F6200(text + s, cnt, (void *)glyphs);
        if (win[6] < width)
            win[6] = (short)width;
        if (flags & 4)
            goto next;
        D_0018CAF8[0] = a1;
        if (flags & 8) {
            half = width >> 1;
            f0 = (float)win[10] * 0.0625f;
            if (flags & 1)
                f1 = (float)(win[4] - half);
            else
                f1 = (float)win[4];
            f13 = (float)win[11] * 0.0625f;
            f13 = (float)y + f13;
            func_001F6638((u64)(long)D_0018CAF8[colr_arr[k]], text + s, cnt, a4,
                          (struct Glyph_c1f7090 *)glyphs, f1 + f0, f13, 1.0f);
        } else {
            if (flags & 1) {
                half = width >> 1;
                x = win[4] - half;
            } else {
                x = win[4];
            }
            func_001F6668_L((void *)(long)x, (void *)(long)y,
                          (void *)(long)D_0018CAF8[colr_arr[k]], text + s,
                          (void *)(long)cnt, a4, glyphs);
        }
    next:
        y += lh;
    }

final:
    xw = D_0013E500[0];
    yh = D_0013E500[1];
    D_0015F4A0 = 0;
    func_00233A40(0, xw - 1, 0, yh - 1);
}
