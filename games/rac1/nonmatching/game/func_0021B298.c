/*
 * The page menu's label draw (a widget's draw callback; its address is in the page tables): picks
 * what the label shows from its flags, cross-fades when that changes, and prints the text in the
 * label's window with a drop shadow, scrolling it when it is taller than the window. ReRAC's
 * label_draw (crates/rc-game/src/menus/pause.rs, US level 01 0x28dd30; ISC License, Copyright (c)
 * 2026 ReRAC contributors) gives the meaning; this is the PAL routine, read from its assembly.
 * The widget: +0x20 width, +0x24 height, +0x30 flags, +0x34 text id (or the text table), +0x38 the
 * table's stride, +0x3C scroll (12.4), +0x44 fade timer, +0x48 content shown, +0x4C its variant.
 * Returns 1 (nothing to draw) or 2.
 */
extern char D_001DF3D0_B298[] __asm__("D_001DF3D0");
extern char D_001DF770_B298[] __asm__("D_001DF770");
extern char D_001DFB10_B298[] __asm__("D_001DFB10");
extern char D_00160370_B298[] __asm__("D_00160370");
extern char D_00160378_B298[] __asm__("D_00160378");
extern char D_00160380_B298[] __asm__("D_00160380");
extern char D_00160388_B298[] __asm__("D_00160388");
extern int D_0015EE84_B298 __asm__("D_0015EE84");
extern int D_0015EE80_B298 __asm__("D_0015EE80");
extern int D_001A0414_B298 __asm__("D_001A0414");
extern char *D_001D5F74_B298 __asm__("D_001D5F74");
extern unsigned char D_0013D5C8_B298[] __asm__("D_0013D5C8");
extern unsigned char D_0013D490_B298[] __asm__("D_0013D490");
extern unsigned char D_0013D510_B298[] __asm__("D_0013D510");
extern unsigned char D_0013E620_B298[] __asm__("D_0013E620");
extern int D_0013CBE0_B298 __asm__("D_0013CBE0");
extern int D_001602B0_B298 __asm__("D_001602B0");
extern int D_001602B4_B298 __asm__("D_001602B4");
extern unsigned short D_001602B8_B298 __asm__("D_001602B8");
extern unsigned short D_001602BC_B298 __asm__("D_001602BC");
extern unsigned short D_00160358_B298 __asm__("D_00160358");
extern int D_00160368_B298 __asm__("D_00160368");
extern void func_00234C98_B298(int, long) __asm__("func_00234C98");
extern int func_001F98C0_B298(int) __asm__("func_001F98C0");
extern void func_001FF4F8_B298(int, int, void *) __asm__("func_001FF4F8");
extern char *func_001FE540_B298(int) __asm__("func_001FE540");
extern int func_00116248_B298(char *, const char *, ...) __asm__("func_00116248");
extern void func_001F4630_B298(int) __asm__("func_001F4630");
extern void func_001F4748_B298(void) __asm__("func_001F4748");
extern long func_001F4868_B298(int) __asm__("func_001F4868");
extern void func_001153FC_B298(void *, int, int) __asm__("func_001153FC");
extern int func_001FA8A8_B298(int, int, float) __asm__("func_001FA8A8");
extern int func_0021C6C0_B298(int, int, int) __asm__("func_0021C6C0");
extern void func_001F65A8_B298(void) __asm__("func_001F65A8");
extern void func_001F6598_B298(void) __asm__("func_001F6598");
extern void func_001F7070_B298(void *, void *, void *, void *, long, unsigned char *) __asm__("func_001F7070");

int func_0021B298(unsigned char *w) {
    short win[12];
    char buf[0x40];
    char *glyphs = D_001DF3D0_B298;
    char *text = D_00160370_B298;
    char *st;
    int font = 1;
    int flags, f, content, variant = 0, wflags, a21, a23, colour, t, shx, shy;
    long tex;

    flags = *(int *)(w + 0x30);
    if (flags & 8) {
        font = 3;
        glyphs = D_001DFB10_B298;
    }
    if (flags & 0x10) {
        font = 2;
        glyphs = D_001DF770_B298;
    }
    func_00234C98_B298(0x42, 0x44);
    func_00234C98_B298(0x47, 0x2004B);

    /* What the label shows. */
    flags = *(int *)(w + 0x30);
    if (flags & 0x20) {
        int c = D_0015EE84_B298 - 1;
        content = (unsigned int)c < 0x12 ? c : -1;
    } else if (flags & 0x40) {
        content = D_001A0414_B298 - 1;
    } else if (flags & 4) {
        t = func_001F98C0_B298(D_001602B4_B298);
        if (*(int *)(w + 0x44) < t) {
            *(int *)(w + 0x44) = func_001F98C0_B298(D_001602B4_B298);
        }
        content = 0;
        *(int *)(w + 0x48) = 0;
        *(int *)(w + 0x4C) = 0;
    } else if (flags & 0x80) {
        st = *(char **)(D_001D5F74_B298 + 0x40);
        content = *(int *)(st + 0x40);
        if (flags & 0x8000) {
            variant = D_0015EE80_B298 != 0;
        }
    } else if (flags & 0x100) {
        char *cell;
        st = *(char **)(D_001D5F74_B298 + 0x40);
        content = *(int *)(st + 0x3C);
        cell = *(char **)(st + 0x48) + content * 10;
        if ((*(short *)(cell + 4) != 0 ? D_0013D490_B298 : D_0013D5C8_B298)[*(short *)(cell + 6)] == 0) {
            content = -1;
        }
    } else if (flags & 0x1000) {
        st = *(char **)(D_001D5F74_B298 + 0x40);
        content = *(int *)(st + 0x40);
        *(int *)(w + 0x34) = 0xFFFF;
        func_001FF4F8_B298(*(short *)(*(char **)(st + 0x34) + content * 12), 1, w + 0x34);
    } else {
        char *cell;
        st = *(char **)(D_001D5F74_B298 + 0x40);
        cell = *(char **)(st + 0x48) + *(int *)(st + 0x3C) * 10;
        content = *(short *)(cell + 6);
        variant = D_0013E620_B298[content] != 0;
    }

    /* The cross-fade: the old content stays until the timer has run down. */
    if (*(int *)(w + 0x44) == -1) {
        *(int *)(w + 0x44) = func_001F98C0_B298(D_001602B4_B298);
        *(int *)(w + 0x48) = content;
        *(int *)(w + 0x4C) = variant;
    }
    if (content == *(int *)(w + 0x48)) {
        *(int *)(w + 0x44) += 3;
    } else {
        int k;
        t = func_001F98C0_B298(D_001602B4_B298);
        if (t < *(int *)(w + 0x44)) {
            *(int *)(w + 0x44) = func_001F98C0_B298(D_001602B4_B298);
        }
        t = *(int *)(w + 0x44);
        for (k = 0; k < 3; k++) {
            t = t < 1 ? 0 : t - 1;
        }
        *(int *)(w + 0x44) = t;
        if (t != 0) {
            content = *(int *)(w + 0x48);
            variant = *(int *)(w + 0x4C);
        } else {
            *(int *)(w + 0x48) = content;
            *(int *)(w + 0x4C) = variant;
            *(int *)(w + 0x30) &= ~0x400;
            *(int *)(w + 0x3C) = 0;
        }
    }

    /* The text. */
    flags = *(int *)(w + 0x30);
    if (flags & 4) {
        if (*(int *)(w + 0x34) == 0) {
            return 1;
        }
        text = func_001FE540_B298(*(int *)(w + 0x34));
    } else if (flags & 0x1000) {
        if (*(int *)(w + 0x34) == 0xFFFF) {
            return 1;
        }
        text = func_001FE540_B298(*(int *)(w + 0x34));
    } else if ((flags & 0x100) && content == -1) {
        text = D_00160378_B298;
    } else if (*(int *)(w + 0x34) != 0) {
        char *table = *(char **)(w + 0x34);
        unsigned int at = (unsigned int)(*(int *)(w + 0x38) * content) & ~3u;
        text = func_001FE540_B298(*(int *)(table + variant * 4 + at));
    }
    if ((*(int *)(w + 0x30) & 0x11E4) == 0 && D_0013D5C8_B298[content] == 0) {
        text = D_00160378_B298;
    }
    if (*(int *)(w + 0x30) & 0x200) {
        char *table = *(char **)(w + 0x34);
        int id = *(int *)(table + ((unsigned int)(*(int *)(w + 0x38) * content) & ~3u));
        if (id != 0x4ED2 && id != 0x4ED9 && id != 0x4EDD) {
            func_00116248_B298(buf, D_00160380_B298, func_001FE540_B298(0x4ECC), text);
            text = buf;
        }
    }
    f = *(int *)(w + 0x30);
    a23 = 4;
    a21 = 4;
    if ((f & 0x4004) == 0x4004 && *(int *)(w + 0x34) == 0x5243) {
        f |= 1;
        a21 = 0xC;
    }
    if ((*(int *)(w + 0x30) & 0x800) && D_0013D510_B298[content] == 0) {
        text = func_001FE540_B298(0x4F54);
        f |= 3;
    }
    if (text == 0) {
        text = D_00160388_B298;
    }
    wflags = 8;
    if (f & 1) {
        wflags = 9;
        a23 = *(int *)(w + 0x20) / 2;
    }
    if (f & 2) {
        wflags |= 2;
        a21 = *(int *)(w + 0x24) / 2;
    }

    /* The window (FontSetWindow's layout) and the three passes: shadow, colour, and the same again
       one wrap below while the text scrolls. */
    func_001F4630_B298(0);
    tex = func_001F4868_B298(font);
    func_001153FC_B298(win, 0, 0x18);
    win[0] = (short)D_00160358_B298;
    win[1] = (short)(*(unsigned short *)(w + 0x24) - D_00160358_B298);
    win[2] = 1;
    win[3] = (short)(*(unsigned short *)(w + 0x20) - 4);
    win[4] = (short)a23;
    win[5] = (short)(a21 - (*(int *)(w + 0x3C) >> 4));
    win[8] = (short)D_00160368_B298;
    win[9] = (short)wflags;
    win[11] = (short)-(*(unsigned short *)(w + 0x3C) & 0xF);
    if (*(int *)(w + 0x30) & 0x10000) {
        win[1] = (short)(*(unsigned short *)(w + 0x24) - 1);
    }
    colour = func_0021C6C0_B298(*(int *)(w + 0x44), func_001FA8A8_B298(D_001602B0_B298, (int)0x80FFA888, 0.5f), (int)0x80FFA888);
    win[9] |= 4;
    func_001F7070_B298(win, (void *)(long)colour, text, (void *)-1, tex, (unsigned char *)glyphs);
    win[9] ^= 4;
    flags = *(int *)(w + 0x30);
    if (!(flags & 0x2000) && !(win[7] + 4 < win[1] - win[0])) {
        if (!(flags & 0x400)) {
            *(int *)(w + 0x30) = flags | 0x400;
            *(int *)(w + 0x3C) = -(*(int *)(w + 0x24) << 3);
        }
    } else if (flags & 0x400) {
        *(int *)(w + 0x3C) = 0;
        *(int *)(w + 0x30) = flags ^ 0x400;
    }

    shx = D_001602BC_B298;
    shy = D_001602B8_B298;
    win[0] += shx;
    win[1] += shx;
    win[2] += shy;
    win[3] += shy;
    win[4] += shy;
    win[5] = (short)(a21 - (*(int *)(w + 0x3C) >> 4) + shx);
    func_001F65A8_B298();
    func_001F7070_B298(win, (void *)0x80000000L, text, (void *)-1, tex, (unsigned char *)glyphs);
    func_001F6598_B298();
    shx = D_001602BC_B298;
    shy = D_001602B8_B298;
    win[0] -= shx;
    win[1] -= shx;
    win[2] -= shy;
    win[3] -= shy;
    win[4] -= shy;
    win[5] -= shx;
    if (*(int *)(w + 0x30) & 0x20000) {
        func_001F65A8_B298();
    }
    func_001F7070_B298(win, (void *)(long)colour, text, (void *)-1, tex, (unsigned char *)glyphs);
    if (*(int *)(w + 0x30) & 0x20000) {
        func_001F6598_B298();
    }
    if (*(int *)(w + 0x30) & 0x400) {
        int below = (unsigned short)win[7] + (unsigned short)(D_00160368_B298 * 3);
        shx = D_001602BC_B298;
        shy = D_001602B8_B298;
        win[0] += shx;
        win[1] += shx;
        win[2] += shy;
        win[3] += shy;
        win[4] += shy;
        win[5] = (short)(win[5] + below + shx);
        func_001F65A8_B298();
        func_001F7070_B298(win, (void *)0x80000000L, text, (void *)-1, tex, (unsigned char *)glyphs);
        func_001F6598_B298();
        shx = D_001602BC_B298;
        shy = D_001602B8_B298;
        win[0] -= shx;
        win[1] -= shx;
        win[2] -= shy;
        win[3] -= shy;
        win[4] -= shy;
        win[5] -= shx;
        func_001F7070_B298(win, (void *)(long)colour, text, (void *)-1, tex, (unsigned char *)glyphs);
        if (*(int *)(w + 0x30) & 0x400) {
            int span = (win[7] + D_00160368_B298 * 3) << 4;
            int scroll = *(int *)(w + 0x3C) + ((D_0013CBE0_B298 & 1) ? 10 : 3);
            *(int *)(w + 0x3C) = span != 0 ? scroll % span : scroll;
        }
    }
    func_001F4748_B298();
    return 2;
}
