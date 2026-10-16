/* The freeze screen's update (game mode 4: the dialogs over a frozen frame). D_00193400 is the
   dialog: +0 its kind (0-8), +4 and +0x2C countdowns, +0x14 the mode to return to, +0x18 a menu
   to show, +0x1C a step, +0x20 a frame counter, +0x24 a delay before input is taken. D_0013CBE4
   holds the buttons pressed this frame (0x40 cross, 0x10 triangle, 0x20 circle); D_0015F6E8 is the
   game's mode, which leaving the dialog sets; D_0015EFB4 the save menu's flags. Kind 3 is the
   memory card dialog, whose step comes from D_0015EFB0. Written from the instructions. */

typedef struct {
    int kind;
    int x4;
    int x8;
    int xC;
    int x10;
    int x14;
    int x18;
    int x1C;
    int x20;
    int x24;
    int x28;
    int x2C;
} Freeze_FD3E8;

extern Freeze_FD3E8 R_FD3E8 __asm__("D_00193400");
extern int pad_FD3E8[] __asm__("D_0013CBE4");
extern int mode_FD3E8 __asm__("D_0015F6E8");
extern int flags_FD3E8 __asm__("D_0015EFB4");
extern int level_FD3E8 __asm__("D_0015EE84");
extern int step_FD3E8 __asm__("D_0015EFB0");
extern int card_FD3E8 __asm__("D_0015F6C8");
extern int cardb_FD3E8 __asm__("D_0015F6CC");
extern int menu_open_FD3E8[] __asm__("D_001D60C0");
extern int menu_FD3E8[] __asm__("D_001D5F78");
extern int x728_FD3E8 __asm__("D_0015F728");
extern int ef38_FD3E8 __asm__("D_0015EF38");
extern int ef3c_FD3E8 __asm__("D_0015EF3C");
extern int d48c_FD3E8[] __asm__("D_0013D48C");
extern int f6e4_FD3E8 __asm__("D_0015F6E4");
extern int f6fc_FD3E8 __asm__("D_0015F6FC");
extern int f690_FD3E8 __asm__("D_0015F690");
extern short e15a_FD3E8[] __asm__("D_0013E15A");
extern unsigned char d44c_FD3E8 __asm__("D_0016044C");
extern char hero_FD3E8[] __asm__("D_0013F450");
extern char d390_FD3E8[] __asm__("D_0013D390");
extern char ca40_FD3E8[] __asm__("D_0013CA40");
extern char d141150_FD3E8[] __asm__("D_00141150");

extern void sound_update_FD3E8(void) __asm__("func_0022DD68");
extern int ticks_FD3E8(int) __asm__("func_001F98C0");
extern void f20BFC8_FD3E8(int, int) __asm__("func_0020BFC8");
extern void f1E9768_FD3E8(void) __asm__("func_001E9768");
extern void f1E97C0_FD3E8(void *, void *, int, int) __asm__("func_001E97C0");
extern void f1F4E08_FD3E8(int) __asm__("func_001F4E08");
extern void f1FFDA0_FD3E8(int, int) __asm__("func_001FFDA0");
extern void f1FFFA0_FD3E8(void) __asm__("func_001FFFA0");
extern void f216D30_FD3E8(int, int) __asm__("func_00216D30");
extern void f209DC0_FD3E8(void) __asm__("func_00209DC0");
extern void f22F4A0_FD3E8(int) __asm__("func_0022F4A0");
extern void f227DB0_FD3E8(int) __asm__("func_00227DB0");
extern void f12E558_FD3E8(int) __asm__("func_0012E558");
extern void f216F28_FD3E8(void) __asm__("func_00216F28");
extern int f12DDC0_FD3E8(void) __asm__("func_0012DDC0");

void func_001FD3E8(void) {
    Freeze_FD3E8 *r = &R_FD3E8;
    int t;
    int p;
    int st;
    char *m;

    sound_update_FD3E8();
    if (r->x4 != 0) {
        r->x4--;
    }
    if (r->x2C != 0) {
        r->x2C--;
    }
    if ((unsigned int)r->kind >= 9) {
        goto done;
    }
    if (r->kind == 0) goto kind0;
    if (r->kind == 1) goto kind1;
    if (r->kind == 2) goto kind2;
    if (r->kind == 3) goto kind3;
    if (r->kind == 4) goto kind4;
    if (r->kind == 5) goto kind5;
    if (r->kind == 6) goto kind6;
    if (r->kind == 7) goto kind7;
    goto kind8;

kind5:
    r->x20++;
    t = ticks_FD3E8(0x5A);
    if (t < r->x20 && r->x24 != 0) {
        r->x24--;
    }
    t = ticks_FD3E8(0x78);
    if (!(t < r->x20) || !(pad_FD3E8[0] & 0x40)) {
        goto done;
    }
    mode_FD3E8 = 0;
    if (level_FD3E8 == 1) {
        f20BFC8_FD3E8(0, -1);
    }
    goto done;

kind8:
    r->x20++;
    t = ticks_FD3E8(0x1E);
    if (t < r->x20 && r->x24 != 0) {
        r->x24--;
    }
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x40)) {
        goto done;
    }
    flags_FD3E8 &= ~0x40;
    d48c_FD3E8[0] = 0;
    goto leave_hud;

kind7:
    r->x20++;
    t = ticks_FD3E8(0x1E);
    if (t < r->x20 && r->x24 != 0) {
        r->x24--;
    }
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x40)) {
        goto done;
    }
    d48c_FD3E8[0] = 0;
    goto leave_hud;

kind4:
    p = pad_FD3E8[0];
    if (p & 0x10) {
        m = *(char **)(hero_FD3E8 + 0x2080);
        m[0x31] = 0;
        *(int *)(m + 0x94) = 0;
        *(unsigned short *)(m + 0x34) |= 1;
        f1E9768_FD3E8();
        f1E97C0_FD3E8(hero_FD3E8 + 0x1D00, hero_FD3E8 + 0x1D10, 0, 1);
        mode_FD3E8 = 0;
    } else if (p & 0x40) {
        mode_FD3E8 = 0;
    }
    goto done;

kind1:
    p = pad_FD3E8[0];
    x728_FD3E8 = 2;
    if (p & 0x10) {
        mode_FD3E8 = 0;
        hero_FD3E8[0x160F] |= 1;
    } else if (p & 0x40) {
        mode_FD3E8 = 0;
    }
    goto done;

kind2:
    p = pad_FD3E8[0];
    x728_FD3E8 = 2;
    if (p & 0x40) {
        mode_FD3E8 = 0;
    }
    goto done;

kind6:
    st = r->x1C;
    if (st == 1) {
        p = pad_FD3E8[0];
        if (p & 0x10) {
            mode_FD3E8 = r->x14;
        } else if (p & 0x40) {
            f1F4E08_FD3E8(4);
            d44c_FD3E8 = 0;
            r->x1C = 2;
            r->x20 = ticks_FD3E8(0x258);
        }
        goto done;
    }
    if (st == 0) {
        if (r->x4 == 0) {
            r->x1C = 1;
        }
        goto done;
    }
    if (st == 2) {
        if (r->x20 == 0 || --r->x20 == 0) {
            r->x1C = 3;
            d44c_FD3E8 = 1;
            goto done;
        }
        p = pad_FD3E8[0];
        if (p & 0x10) {
            f1F4E08_FD3E8(4);
            mode_FD3E8 = r->x14;
            d44c_FD3E8 = 1;
        } else if (p & 0x40) {
            mode_FD3E8 = r->x14;
            d44c_FD3E8 = 0;
        }
        goto done;
    }
    if (st == 3) {
        if (pad_FD3E8[0] & 0x40) {
            mode_FD3E8 = r->x14;
        }
        goto done;
    }
    mode_FD3E8 = r->x14;
    goto done;

kind0:
    st = r->x1C;
    if (st == 0) {
        if (r->x20 < 8) {
            r->x20++;
        } else if (r->x24 < 8) {
            r->x24++;
        } else {
            r->x1C = 1;
        }
        goto done;
    }
    if (st == 1) {
        p = pad_FD3E8[0];
        if (p & 0x40) {
            r->x1C = 2;
        } else if (p & 0x820) {
            r->x1C = 3;
        }
        goto done;
    }
    if (st >= 4 || st < 0) {
        goto done;
    }
    if (r->x24 != 0) {
        r->x24--;
        goto done;
    }
    if (r->x20 != 0) {
        r->x20--;
        goto done;
    }
    if (r->x1C == 2) {
        if (*(int *)(hero_FD3E8 + 0x880) != -1) {
            f1FFDA0_FD3E8(*(int *)(hero_FD3E8 + 0x880), 0);
            *(int *)(hero_FD3E8 + 0x880) = -1;
        }
        f1FFFA0_FD3E8();
        t = ticks_FD3E8(0x1068);
        if (t < *(int *)(hero_FD3E8 + 0x19C)) {
            if (level_FD3E8 == 5) {
                ef38_FD3E8 = ef38_FD3E8 + 1;
            } else if (level_FD3E8 == 0x10) {
                ef3c_FD3E8 = ef3c_FD3E8 + 1;
            }
        }
        mode_FD3E8 = 0;
        f216D30_FD3E8(0, 8);
        f1E97C0_FD3E8(d141150_FD3E8, d141150_FD3E8 + 0x10, 0, 1);
        goto done;
    }
    mode_FD3E8 = 0;
    if (*(short *)(hero_FD3E8 + 0x89A) >= 3) {
        *(short *)(hero_FD3E8 + 0x89A) = *(unsigned short *)(hero_FD3E8 + 0x89A) - 1;
        m = *(char **)(hero_FD3E8 + 0x890);
        m[0xBC] = 3;
    }
    goto done;

kind3:
    if (*(int *)(d390_FD3E8 + 0xB0) == 1) {
        goto done;
    }
    r->x20++;
    t = ticks_FD3E8(0x1E);
    if (t < r->x20 && r->x24 != 0) {
        r->x24--;
    }
    t = step_FD3E8 - 1;
    if ((unsigned int)t >= 24) {
        goto done;
    }
    if (t == 0 || t == 15) goto s_menu;
    if (t == 1) goto s_cross_clear1;
    if (t == 2 || t == 4) goto s_triangle;
    if (t == 3) goto s_ca40;
    if (t == 5) goto s_circle8;
    if (t == 8) goto s_card_wait;
    if (t == 11) goto s_menu_triangle;
    if (t == 12) goto s_circle10;
    if (t == 16 || t == 17 || t == 20) goto s_cross_return;
    if (t == 18) goto s_card_cross;
    if (t == 19) goto s_cross_exit;
    if (t == 22 || t == 23) goto s_cross_or_triangle;
    goto done;

s_menu:
    menu_FD3E8[0] = r->x18;
    mode_FD3E8 = r->x14;
    goto done;

s_cross_clear1:
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x40)) {
        goto done;
    }
    flags_FD3E8 &= ~1;
    mode_FD3E8 = r->x14;
    goto done;

s_triangle:
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x10)) {
        goto done;
    }
    flags_FD3E8 = flags_FD3E8 & ~2 & ~4;
    mode_FD3E8 = r->x14;
    goto done;

s_menu_triangle:
    if (menu_open_FD3E8[0] == 0) {
        goto done;
    }
    goto s_triangle;

s_card_cross:
    if (card_FD3E8 == 0) {
        goto s_triangle;
    }
    if (menu_open_FD3E8[0] != 0) {
        goto s_triangle;
    }
    if (r->x24 != 0) {
        goto done;
    }
    p = pad_FD3E8[0];
    if (p & 0x40) {
        f209DC0_FD3E8();
        flags_FD3E8 = (flags_FD3E8 & ~2 & ~4) | 0x20;
        goto refresh;
    }
    if (!(p & 0x10)) {
        goto done;
    }
    flags_FD3E8 = flags_FD3E8 & ~2 & ~4;
    mode_FD3E8 = r->x14;
    goto done;

s_cross_or_triangle:
    if (r->x24 != 0) {
        goto done;
    }
    p = pad_FD3E8[0];
    if (p & 0x40) {
        f209DC0_FD3E8();
        flags_FD3E8 = (flags_FD3E8 & ~2 & ~4) | 0x20;
        goto refresh;
    }
    if (!(p & 0x10)) {
        goto done;
    }
    flags_FD3E8 |= 0x20;
    mode_FD3E8 = r->x14;
    goto done;

s_ca40:
    if (card_FD3E8 == 0) {
        goto s_cardb;
    }
    if (menu_open_FD3E8[0] != 0) {
        goto s_triangle_set20;
    }
    if (r->x24 != 0) {
        goto done;
    }
    if (*(int *)(ca40_FD3E8 + 0x1A4) & 0x40) {
        f209DC0_FD3E8();
        flags_FD3E8 = flags_FD3E8 & ~2 & ~4;
        f22F4A0_FD3E8(0);
        e15a_FD3E8[0] = 1;
    }
    if (r->x24 != 0) {
        goto done;
    }
    if (!(*(int *)(ca40_FD3E8 + 0x1A4) & 0x10)) {
        goto done;
    }
    flags_FD3E8 = (flags_FD3E8 | 0x20) & ~2 & ~4;
    mode_FD3E8 = r->x14;
    goto done;

s_cardb:
    if (cardb_FD3E8 == 0) {
        goto s_triangle_set20;
    }
    if (r->x24 != 0) {
        goto done;
    }
    p = pad_FD3E8[0];
    if (p & 0x10) {
        mode_FD3E8 = r->x14;
        flags_FD3E8 = (flags_FD3E8 | 0x20) & ~2 & ~4;
    }
    if (!(p & 0x20)) {
        goto done;
    }
    goto leave_hud;

s_triangle_set20:
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x10)) {
        goto done;
    }
    flags_FD3E8 = (flags_FD3E8 | 0x20) & ~2 & ~4;
    mode_FD3E8 = r->x14;
    goto done;

s_circle8:
    if (r->x24 == 0 && (pad_FD3E8[0] & 0x20)) {
        flags_FD3E8 |= 8;
        goto done;
    }
    goto s_triangle_unless_card;

s_circle10:
    if (r->x24 == 0 && (pad_FD3E8[0] & 0x20)) {
        flags_FD3E8 |= 0x10;
        goto done;
    }
    goto s_triangle_unless_card;

s_triangle_unless_card:
    if (r->x4 != 0 || !(pad_FD3E8[0] & 0x10)) {
        goto done;
    }
    flags_FD3E8 = (flags_FD3E8 | 0x20) & ~2 & ~4;
    if (card_FD3E8 != 0) {
        goto done;
    }
    mode_FD3E8 = r->x14;
    goto done;

s_cross_exit:
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x40)) {
        goto done;
    }
    flags_FD3E8 = flags_FD3E8 & ~0x40 & ~0x400 & ~2 & ~4;
    if (card_FD3E8 != 0) {
        mode_FD3E8 = r->x14;
        goto done;
    }
    d48c_FD3E8[0] = 0;
    f6e4_FD3E8 = -1;
    f6fc_FD3E8 = 1;
    f690_FD3E8 = 1;
    goto done;

s_cross_return:
    if (r->x24 != 0 || !(pad_FD3E8[0] & 0x40)) {
        goto done;
    }
    mode_FD3E8 = r->x14;
    flags_FD3E8 = flags_FD3E8 & ~0x40 & ~0x400 & ~2 & ~4;
    goto done;

s_card_wait:
    if (card_FD3E8 == 0 || (flags_FD3E8 & 6) != 0) {
        goto done;
    }
    mode_FD3E8 = r->x14;
    goto done;

leave_hud:
    f227DB0_FD3E8(-1);
refresh:
    f22F4A0_FD3E8(0);
    e15a_FD3E8[0] = 1;

done:
    if ((unsigned int)(mode_FD3E8 - 3) >= 2) {
        f12E558_FD3E8(0x1D);
        f216F28_FD3E8();
        f12DDC0_FD3E8();
    }
}
