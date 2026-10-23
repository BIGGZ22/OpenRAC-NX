/* SPDX-License-Identifier: GPL-3.0-or-later */
/* func_001F6410, DrawOutlinedRect (the frame of the freeze and message boxes: nine
 * DrawRectOverlay_FiiiiUl calls around a rectangle), as the port runs it: the decompilation's
 * matched C (src/game/draw.c) with its colour a 64-bit value, as retail's register keeps it. Not a
 * PS2 match; the matched C stays as it matches (hostgen.json "overrides": this C replaces it in the
 * port only).
 *
 * The matched C takes the colour as an int and passes it to DrawRectOverlay_FiiiiUl's u64 RGBAQ
 * word. Retail never narrows the register: its callers hand it FastTweenColor's zero-extended
 * colour (0x0000000080xxxxxx, the message box's pulse in func_001FBE80), the first call's colour is
 * the register `and` lui's sign-extended 0xFF000000, `ori` 4 (both on all 64 bits), and the other
 * eight pass the register on. As an int the port sign-extended it (0xFFFFFFFF80000004: the GS's Q
 * word all ones). It goes at the end of draw.c, which defines DrawRectOverlay_FiiiiUl above it. It
 * is defined by its address name: hostgen matches an override to its row by the name it defines, and
 * a readable name can change with the decompilation's names.h (rac1-decomp's is draw_outlined_rect
 * since it took the NTSC decomp's names). */
void func_001F6410(int a0, int a1, int a2, int a3, unsigned long a4) {
    unsigned long color;
    int t0, t1, t2;
    int t_a0m5, t4, t5;
    int t_a0p3, t6, t_a2m3;
    int t_a1p3, t3, t_a3p1;

    color = (a4 & (unsigned long)(long)(int)0xFF000000) | 4;
    DrawRectOverlay_FiiiiUl(a0, a1, a2, a3, color);

    t0 = a0 + 1;
    t1 = a2 + 3;
    t2 = a3 + 5;
    DrawRectOverlay_FiiiiUl(a0 - 1, t0, t1, t2, a4);

    t_a0m5 = a0 - 5;
    t4 = a2 - 1;
    t5 = a3 - 3;
    DrawRectOverlay_FiiiiUl(a0 - 3, t_a0m5, t4, t5, a4);

    t3 = a1 - 3;
    DrawRectOverlay_FiiiiUl(t_a0m5, t3, t4, a2 + 1, a4);

    t_a0p3 = a0 + 3;
    t6 = a1 + 1;
    t_a2m3 = a2 - 3;
    DrawRectOverlay_FiiiiUl(t_a0p3, t6, t_a2m3, a2 - 5, a4);

    DrawRectOverlay_FiiiiUl(a1 - 1, t6, t_a2m3, t5, a4);

    t_a1p3 = a1 + 3;
    t_a3p1 = a3 + 1;
    DrawRectOverlay_FiiiiUl(t_a1p3, a1 + 5, t1, t_a3p1, a4);

    DrawRectOverlay_FiiiiUl(t_a0p3, t_a1p3, a3 - 1, t_a3p1, a4);

    DrawRectOverlay_FiiiiUl(t0, t3, a3 + 3, t2, a4);
}
