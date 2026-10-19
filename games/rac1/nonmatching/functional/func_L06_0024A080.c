/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). HUD gauge drawn from a struct: a row of icon pieces (3, 6 or 7 wide by mode) with a coloured fill bar over it whose length follows p[0x74]/p[8], plus an end cap pair; byte 0x70 gates it and scales the fade-in. The real entry is 0x20 bytes into the retail symbol (the first 8 words are unreachable stack-adjust filler and callers pass func+0x20), so only the body is written here. Statically checked only. */
extern int f_tex_4A080(int, int) __asm__("func_00200198");
extern void f_icon_4A080(int, int, int, int, int, int) __asm__("func_00200650");
extern void f_sprite_4A080(int, int, int, int, int, int) __asm__("func_00200468");
extern void f_rect_4A080(int, int, int, int, long, int) __asm__("func_00201640");
extern float f_i2f_4A080(int) __asm__("func_001FA888");
extern int f_f2i_4A080(float) __asm__("func_001FA898");
extern int f_tween_4A080(int, int, float) __asm__("func_001FA8A8");
extern unsigned char D_0013E15A_4A080[] __asm__("D_0013E15A");
extern int D_0015EE80_4A080 __asm__("D_0015EE80") MACRO_ADDR;
extern int D_0015EE84_4A080 __asm__("D_0015EE84") MACRO_ADDR;
extern int D_L06_0015F9BC_4A080 __asm__("D_L06_0015F9BC") MACRO_ADDR;
extern float D_L06_0015F9C4_4A080 __asm__("D_L06_0015F9C4") MACRO_ADDR;
extern float D_L06_0015F9C8_4A080 __asm__("D_L06_0015F9C8") MACRO_ADDR;
int func_L06_0024A080_body(unsigned char *p) __asm__("func_L06_0024A080");

int func_L06_0024A080_body(unsigned char *p) {
    int base = *(int *)(D_0013E15A_4A080 + 0x4AA);
    int y, n, left, j, ar, ab, fill, ytop, ybot, ycap;
    int x, x0, xcap;
    int mode = D_0015EE84_4A080;
    int color;
    float f, g, h, cnt, step, part;
    int pal = D_0015EE80_4A080;

    y = base - 0x2A;
    if (pal == 0)
        y = base - 0x32;
    x = D_L06_0015F9BC_4A080;
    n = 3;
    if (mode == 7 || mode == 0xD)
        n = 6;
    if (mode == 0x12)
        n = 7;
    if (p[0x70] != 0) {
        if (mode == 0xD) {
            y = 0x12;
            if (pal != 0)
                y = 0xA;
        }
        f = f_i2f_4A080(p[0x70]) * 0.125f;
        if (1.0f < f)
            f = 1.0f;
        else if (f < 0.0f)
            f = 0.0f;
        ytop = y + 8;
        left = n - 1;
        g = f * 128.0f;
        ybot = y + 0x1A;
        ycap = y - 8;
        ar = f_f2i_4A080(g);
        ab = f_f2i_4A080(g);
        x -= n << 4;
        x0 = x;
        if (n > 0) {
            j = n;
            do {
                f_icon_4A080(f_tex_4A080(0x7562, 3), x, y + 6, 0x20, 0x12, ar);
                j--;
                x += 0x20;
            } while (j != 0);
        }
        color = (int)0x800000FF;
        fill = 0;
        if (*(int *)(p + 0x74) != 0) {
            f = f_i2f_4A080(*(int *)(p + 8));
            h = (float)*(int *)(p + 0x74) / f;
            cnt = f_i2f_4A080(n);
            step = 1.0f / cnt;
            part = (float)left * step;
            if (part < h) {
                h = (h - part) * D_L06_0015F9C4_4A080;
                h = h + part;
            } else if (h < step) {
                h = (h - step) * D_L06_0015F9C8_4A080;
                h = h + step;
            }
            fill = f_f2i_4A080((float)(n << 5) * h);
            if (0.5f < h)
                color = f_tween_4A080((int)0x8000FFFF, (int)0x8000FF00, h + h - 1.0f);
            else
                color = f_tween_4A080((int)0x800000FF, (int)0x8000FFFF, h + h);
        }
        x = x0;
        xcap = x0 - 0x28;
        f_rect_4A080(x0, ytop, x0 + fill, ybot, color, 0);
        if (left > 0) {
            j = left;
            do {
                f_sprite_4A080(f_tex_4A080(0x7562, 1), x, y, 0x20, 0x20, ab);
                j--;
                x += 0x20;
            } while (j != 0);
        }
        f_sprite_4A080(f_tex_4A080(0x7562, 2), x, y, 0x20, 0x20, ab);
        f_sprite_4A080(f_tex_4A080(0x7562, 4), xcap, ycap, 0x30, 0x30, ar);
        f_sprite_4A080(f_tex_4A080(0x7562, 0), xcap, ycap, 0x30, 0x30, ar);
    }
    return *(int *)(p + 0x58);
}
