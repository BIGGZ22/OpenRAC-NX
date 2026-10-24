#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * freeze.cpp in the original source; text 0x1FBC80-0x1FDF10.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;

extern void func_0012E528(int);
extern void func_00216EF0(int);
extern int func_001FE540(int);
extern void func_001FF560(int);
extern int func_001F98C0(int);
extern int D_0015F6E8 MACRO_ADDR;
extern int D_0013D440;
extern char D_00193400[];

/* Enters the freeze screen for reason arg0: pauses the music unless the
   game mode (D_0015F6E8) is already 3, saves arg1/arg2 and the old mode in
   the freeze record D_00193400, sets mode 4, then per reason fetches the
   message strings (func_001FE540) or starts the timers (func_001F98C0);
   an unknown reason gets a plain 0x78 countdown. Each case reads the
   record through its own `char *` local (retail keeps only the %hi and
   rebuilds the %lo per case); the case bodies are in retail's layout
   order, and the last store of a block is written last in the source
   because the scheduler issues it first. */
void mode_freeze_init(int arg0, int arg1, int arg2) {
    char *s = D_00193400;

    *(int *)(s + 0x30) = arg2;
    if (D_0015F6E8 != 3) {
        func_0012E528(0x1D);
        music_Pause(0);
    }
    *(int *)(s + 0x18) = arg1;
    *(int *)(s + 0x14) = D_0015F6E8;
    D_0015F6E8 = 4;
    *(int *)s = arg0;
    *(int *)(s + 0x2C) = 0;
    switch (arg0) {
    case 0: {
        char *p = D_00193400;
        *(int *)(p + 8) = msg_string(0x4F6E);
        *(int *)(p + 0xC) = msg_string(0x524D);
        *(int *)(p + 0x10) = msg_string(0x524E);
        *(int *)(p + 4) = 0;
        *(int *)(p + 0x1C) = 0;
        *(int *)(p + 0x20) = 0;
        *(int *)(p + 0x24) = 0;
        *(int *)(p + 0x28) = 0;
        break;
    }
    case 2: {
        int v = msg_string(0x524F);
        char *p = D_00193400;
        *(int *)(p + 8) = v;
        *(int *)(p + 0xC) = 0;
        *(int *)(p + 4) = 0;
        break;
    }
    case 1:
    case 4: {
        char *p = D_00193400;
        *(int *)(p + 8) = msg_string(0x522E);
        *(int *)(p + 0xC) = msg_string(0x4EE0);
        *(int *)(p + 0x10) = msg_string(0x524F);
        *(int *)(p + 4) = 0;
        break;
    }
    case 5: {
        char *p = D_00193400;
        push_help_history(0x4E2B);
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = 0;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        break;
    }
    case 7:
    case 8: {
        char *p = D_00193400;
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = -1;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        break;
    }
    case 3: {
        char *p = D_00193400;
        *(int *)(p + 4) = func_001F98C0(0x1E);
        *(int *)(p + 0x20) = -1;
        *(int *)(p + 0x24) = func_001F98C0(0x1E);
        D_0013D440 = 3;
        *(int *)(p + 0x2C) = func_001F98C0(0xA);
        break;
    }
    case 6: {
        int v = func_001F98C0(0x1E);
        char *p = D_00193400;
        *(int *)(p + 4) = v;
        *(int *)(p + 0x1C) = 0;
        break;
    }
    default:
        *(int *)(s + 4) = 0x78;
        *(int *)(s + 0xC) = 0;
        *(int *)(s + 0x10) = 0;
        *(int *)(s + 8) = 0;
        break;
    }
}
__asm__(".section .text\n\tnop\n");

struct TextRegionP {
    s16 top;
    s16 bottom;
    s16 left;
    s16 right;
    s16 anchor_x;
    s16 anchor_y;
    s16 measured_width;
    s16 rendered_height;
    s16 line_advance;
    u16 flags;
    s16 subpixel_x_sixteenths;
    s16 subpixel_y_sixteenths;
};
typedef struct {
    s32 state;
    s32 timer;
    char *title;
    char *line1;
    char *line2;
    u8 pad14[8];
    s32 choice;
    s32 unk20;
    s32 unk24;
    s32 unk28;
    s32 unk2C;
    s32 unk30;
} MessageBoxP;
typedef struct {
    u8 pad0[0x894];
    s32 unk894;
    u8 pad898[2];
    s16 unk89A;
    u8 pad89C[0xC];
    s32 unk8A8;
    u8 pad8AC[0x14];
    s32 unk8C0;
} HeroP;
typedef struct { s16 pad0; s16 pad2; s32 height; } ScreenP;
extern short D_0015F620;
extern short D_0015F5E8;
extern short D_0015F5EC;
extern short D_0015F5F0;
extern short D_0015F5F4;
extern short D_0015F5F8;
extern short D_0015F5FC;
extern short D_0015F600;
extern short D_0015F604;
extern short D_0015F608;
extern short D_0015F60C;
extern short D_0015F610;
extern short D_0015F614;
extern short D_0015F618;
extern short D_0015F61C;
extern short D_0015F624;
extern short D_0015F628;
extern short D_0015F62C;
extern short D_0015F630;
extern short D_0015F634;
extern short D_0015F638;
extern short D_0015F63C;
extern short D_0015F640;
extern s32 D_0015F6CC MACRO_ADDR;
extern short D_0015EFB0;
extern s32 D_0015EE80 MACRO_ADDR;
extern s32 D_0015EE84 MACRO_ADDR;
extern s32 D_0015F6C8 MACRO_ADDR;
extern s32 D_001D60C0;
extern s32 D_0013D440 MACRO_ADDR;
extern s32 D_0015F538 MACRO_ADDR;
extern char D_0015F648[];
extern char D_0015F650[];
extern char D_0015F660[];
extern char D_0015F668[];
extern char D_0015F670[];
extern char D_0015F680[];
extern s32 D_0015EF58[] MACRO_ADDR;
extern s32 D_0015EF68[] MACRO_ADDR;
extern struct TextRegionP D_001E7BF8;
extern struct TextRegionP D_001E7C10;
extern u8 D_001DF3D0[];
extern char D_0013F450[];
extern char D_0013E600[];
extern void func_001F55C0(s32, s32, s32, s32);
extern void func_001F62C8(s32, s32, s32, s32, s32);
extern void func_001F6410(s32, s32, s32, s32, s32);
extern s32 func_001F6EA8(s32, s32, s64, char *, s32);
extern void func_001F7560(struct TextRegionP *, long, char *, s32);
extern f32 func_001F9FA8(f32);
extern f32 func_001FA888(s32);
extern s32 func_001FA898(f32);
extern s32 func_001FA8A8(s32, s32, f32);
extern s32 func_00200198(s32, s32);
extern s64 func_00200248(s32);
extern void func_00200468(s32, s32, s32, s32, s32, s32);
extern void func_00200E38(f32, f32, f32, f32, f32, s32, s32, s64);
extern void func_00234C98(s32, s64);
extern void *func_00116B00(void *, char *, s32);
extern s32 func_00116248(char *, const char *, ...);
extern void func_001153FC(void *, s32, s32);
static __inline__ void draw_box_p(char *text, s32 left, s32 mid, s32 right, struct TextRegionP box) {
    s32 tint;
    s32 y;
    s32 h;
    f32 alpha;

    font_print_window_regular(&box, 0, text, -1);
    h = box.rendered_height + 0x28;
    y = (((ScreenP *)D_0013E600)->height - h) >> 1;
    box.bottom = y + h;
    box.anchor_y = y + 4;
    box.top = y;
    alpha = 1.0f - (f32)((MessageBoxP *)D_00193400)->timer / (f32)func_001F98C0(30);
    DrawUIFrame(box.top, box.bottom, 0x60, 0x1A0, (s32)(alpha * 80.0f));
    box.flags ^= 4;
    font_print_window_regular(&box, FastTweenColor(*(s32 *)&D_0015F5F0, *(s32 *)&D_0015F5F4, alpha), text, -1);
    tint = FastTweenColor(0x20FFFF, 0x8020FFFF, 1.0f - (f32)((MessageBoxP *)D_00193400)->unk24 / (f32)func_001F98C0(30));
    if (left != 0) {
        FontPrintCenter(0xCA, box.bottom - 0x14, tint, msg_string(left), -1);
    }
    if (right != 0) {
        FontPrintCenter(0x135, box.bottom - 0x14, tint, msg_string(right), -1);
    }
    if (mid != 0) {
        FontPrintCenter(0x100, box.bottom - 0x14, tint, msg_string(mid), -1);
    }
}

void func_001FBE80(void) {
    char buf[0x200];

    emit_rgba_draw_packet(0, 0, 0, 0x30);
    if (((MessageBoxP *)D_00193400)->unk2C != 0) {
        return;
    }
    SetupGifPaging(0);
    switch (((MessageBoxP *)D_00193400)->state) {
    case 5: {
        f32 alpha;

        alpha = 1.0f - (f32)((MessageBoxP *)D_00193400)->timer / (f32)func_001F98C0(30);
        DrawUIFrame(0x50, 0x154, 0x60, 0x1A0, (s32)(alpha * 80.0f));
        {
            struct TextRegionP box = D_001E7BF8;
            u8 *p;
            s32 color;
            s32 tint;
            s32 y;
            s32 top;
            s32 mid;
            f32 pos;
            f32 angle;
            f32 pivot;

            color = FastTweenColor(*(s32 *)&D_0015F5F0, *(s32 *)&D_0015F5F4, alpha);
            func_00116B00((void *)0x70000000, msg_string(0x4E2B), 0x400);
            p = (u8 *)0x70000000;
            while (*p >= 2) {
                p++;
            }
            if (*p == 1) {
                do {
                    *p++ = 0;
                } while (*p == 1);
            }
            font_print_window_regular(&box, color, (char *)0x70000000, -1);
            pivot = 272.0f;
            box.flags |= 4;
            y = box.anchor_y;
            y += box.rendered_height;
            font_print_window_regular(&box, color, (char *)p, -1);
            box.flags ^= 4;
            box.anchor_y = 0x136 - box.rendered_height;
            font_print_window_regular(&box, color, (char *)p, -1);
            tint = FastTweenColor(0x20FFFF, 0x8020FFFF, 1.0f - (f32)((MessageBoxP *)D_00193400)->unk24 / (f32)func_001F98C0(30));
            FontPrintCenter(0x100, 0x140, tint, msg_string(0x524F), -1);
            mid = (y + box.anchor_y) >> 1;
            VU1_addGSregister(0x47, 0x3004B);
            top = mid - 0x20;
            HudSprite(GetIconFrame(0x755D, 0), 0xE0, top, 0x40, 0x40, 0x80);
            pos = (f32)(mid << 4);
            angle = (f32)(D_0015F538 % 55) * -6.2831855f / 55.0f;
            func_00200E38(4096.0f, pos, pivot, pivot, angle, 0x40, 0x40,
                         GetFrameTex(GetIconFrame(0x755D, 1)));
        }
        break;
    }
    case 7: {
        char *text;
        u8 *p;

        if (((MessageBoxP *)D_00193400)->unk20 <= 0) {
            break;
        }
        text = func_00116B00((void *)0x70000000, msg_string(0x4FA6), 0x400);
        p = (u8 *)0x70000000;
        while (*p >= 2) {
            p++;
        }
        if (*p == 1) {
            do {
                *p++ = 0;
            } while (*p == 1);
        }
        text = (char *)p;
        draw_box_p(text, 0, ((MessageBoxP *)D_00193400)->timer != 0 ? 0 : 0x524F, 0, (struct TextRegionP){0, ((ScreenP *)D_0013E600)->height, 0x60, 0x1A0, 0x100, 0x68, 0, 0, 0x10, 5});
        break;
    }
    case 8: {
        char *text;

        if (((MessageBoxP *)D_00193400)->unk20 <= 0) {
            break;
        }
        text = msg_string(((MessageBoxP *)D_00193400)->unk30 != 0 ? 0x4FBD : 0x4FBC);
        draw_box_p(text, 0, ((MessageBoxP *)D_00193400)->timer != 0 ? 0 : 0x524F, 0, (struct TextRegionP){0, ((ScreenP *)D_0013E600)->height, 0x60, 0x1A0, 0x100, 0x68, 0, 0, 0x10, 5});
        break;
    }
    case 3: {
        char *text;
        s32 left;
        s32 right;
        s32 mid;
        s32 draw = 1;

        if (D_0013D440 != 0) {
            break;
        }
        if (((MessageBoxP *)D_00193400)->unk20 <= 0) {
            break;
        }
        text = D_0015F648;
        left = 0;
        mid = 0;
        right = 0;
        switch (*(s32 *)&D_0015EFB0) {
        case 6:
            left = 0x5254;
            right = 0x5250;
            text = msg_string(0x4FAF);
            break;
        case 12:
            if (D_001D60C0 != 0) {
                mid = 0x4FA8;
                text = msg_string(0x4FA7);
                break;
            }
        case 13:
            left = 0x5254;
            right = 0x5250;
            text = msg_string(0x4FAD);
            break;
        case 7:
        case 8:
            text = msg_string(0x4FB7);
            break;
        case 14:
        case 15:
            text = msg_string(0x4FB8);
            break;
        case 17:
            mid = 0x524F;
            text = msg_string(0x4FBA);
            break;
        case 18:
            mid = 0x524F;
            text = msg_string(0x4FBC);
            break;
        case 2:
            mid = 0x524F;
            text = msg_string(0x4FA6);
            if (((MessageBoxP *)D_00193400)->timer != 0) {
                mid = 0;
            }
            break;
        case 19:
            if (D_001D60C0 != 0) {
                mid = 0x4FA8;
                text = msg_string(0x4FA7);
            } else if (boot_level_active != 0) {
                left = 0x5253;
                right = 0x5250;
                func_00116248(buf, D_0015F650, msg_string(0x4FA9), 1, 1, msg_string(0x4FAA));
                text = buf;
            } else {
                mid = 0x4FA8;
                text = msg_string(0x4FA9);
            }
            break;
        case 21:
            mid = 0x524F;
            text = msg_string(0x4FBD);
            break;
        case 20:
            mid = 0x524F;
            text = msg_string(0x4FBB);
            break;
        case 24:
            left = 0x5253;
            right = 0x5250;
            func_00116248(buf, D_0015F650, msg_string(0x4FAE), 1, 1, msg_string(0x4FAA));
            text = buf;
            break;
        case 23:
            left = 0x5253;
            right = 0x5250;
            text = msg_string(0x4FB1);
            break;
        case 3:
        case 4:
            if (boot_level_active != 0 && D_001D60C0 == 0) {
                left = 0x5253;
                right = 0x5250;
                text = msg_string(0x4FAB);
            } else if (D_0015F6CC != 0) {
                left = 0x5254;
                right = 0x5250;
                text = msg_string(0x4FAB);
            } else if (D_001D60C0 == 0) {
                mid = 0x4FA8;
                text = msg_string(0x4FAC);
            } else {
                mid = 0x4FA8;
                text = msg_string(0x4FA7);
            }
            break;
        case 5:
            mid = 0x4FA8;
            text = msg_string(0x4FA7);
            break;
        case 9:
        case 10:
        case 11:
        case 16:
            draw = 0;
            break;
        }
        if (draw) {
            draw_box_p(text, left, mid, right, (struct TextRegionP){0, ((ScreenP *)D_0013E600)->height, 0x60, 0x1A0, 0x100, 0x68, 0, 0, 0x10, 5});
        }
        break;
    }
    case 6: {
        struct TextRegionP box;
        s32 text_color;
        s32 button_color;
        s32 id;
        f32 alpha;

        box = D_001E7C10;
        alpha = 1.0f - (f32)((MessageBoxP *)D_00193400)->timer / (f32)func_001F98C0(30);
        DrawUIFrame(0x64, 0x12C, 0x60, 0x1A0, (s32)(alpha * 80.0f));
        text_color = FastTweenColor(*(s32 *)&D_0015F5F0, *(s32 *)&D_0015F5F4, alpha);
        button_color = FastTweenColor(0x20FFFF, 0x8020FFFF, alpha);
        switch (((MessageBoxP *)D_00193400)->choice) {
        case 0:
        case 1:
            FontPrintCenter(0xCA, 0x118, button_color, msg_string(0x5253), -1);
            FontPrintCenter(0x135, 0x118, button_color, msg_string(0x5250), -1);
            id = 0x522F;
            break;
        case 2:
            FontPrintCenter(0xCA, 0x118, button_color, msg_string(0x5253), -1);
            FontPrintCenter(0x135, 0x118, button_color, msg_string(0x5250), -1);
            id = 0x5230;
            break;
        case 3:
            FontPrintCenter(0x100, 0x118, button_color, msg_string(0x524F), -1);
            id = 0x5231;
            break;
        default:
            id = 0;
            break;
        }
        if (id != 0) {
            ((void (*)(struct TextRegionP *, long, char *, s32, s32, u8 *))FontPrintWindow)(&box, text_color, msg_string(id), -1, GetEffectTex(1), normal_font_metrics);
        }
        break;
    }
    case 0: {
        char line[0x40];
        s32 pulse;
        s32 c1;
        s32 c2;
        s32 c3;
        s32 best;
        s32 record;
        s32 unit;
        s32 rem;
        s32 per;
        s32 min;
        s32 sec;
        s32 hund;
        s32 sel;
        f32 scale;
        f32 f;

        pulse = FastTweenColor(*(s32 *)&D_0015F624, *(s32 *)&D_0015F628,
                       FastSin((f32)(D_0015F538 % *(s32 *)&D_0015F620) / IntToFloat(*(s32 *)&D_0015F620) * 6.28318f - 3.14159f) * 0.5f + 0.5f);
        scale = (f32)((MessageBoxP *)D_00193400)->unk20 * 0.125f;
        if (scale > 1.0f) {
            scale = 1.0f;
        } else if (scale < 0.1f) {
            scale = 0.1f;
        }
        c1 = *(s32 *)&D_0015F62C;
        c2 = *(s32 *)&D_0015F634;
        c3 = *(s32 *)&D_0015F63C;
        if (((MessageBoxP *)D_00193400)->unk24 != 0) {
            f = (f32)((MessageBoxP *)D_00193400)->unk24 * 0.125f;
            if (f > 1.0f) {
                f = 1.0f;
            } else if (f < 0.0f) {
                f = 0.0f;
            }
            c1 = FastTweenColor(*(s32 *)&D_0015F62C, *(s32 *)&D_0015F630, f);
            c2 = FastTweenColor(*(s32 *)&D_0015F634, *(s32 *)&D_0015F638, f);
            c3 = FastTweenColor(*(s32 *)&D_0015F63C, *(s32 *)&D_0015F640, f);
        }
        if (((HeroP *)D_0013F450)->unk89A < 3) {
            draw_outlined_rect(*(s32 *)&D_0015F5FC - truncate_float_to_s32((f32)*(s32 *)&D_0015F604 * scale),
                            *(s32 *)&D_0015F5FC + truncate_float_to_s32((f32)*(s32 *)&D_0015F604 * scale),
                            *(s32 *)&D_0015F5F8 - truncate_float_to_s32((f32)*(s32 *)&D_0015F600 * scale),
                            *(s32 *)&D_0015F5F8 + truncate_float_to_s32((f32)*(s32 *)&D_0015F600 * scale), pulse);
            FontPrintCenter(0x100, *(s32 *)&D_0015F608, c1, msg_string(0x4F6E), -1);
            FontPrintCenter(0x100, *(s32 *)&D_0015F608 + 0x18, c1, msg_string(0x524E), -1);
            FontPrintCenter(0x100, *(s32 *)&D_0015F608 + 0x30, c1, msg_string(0x524D), -1);
        } else {
            draw_outlined_rect(*(s32 *)&D_0015F610 - truncate_float_to_s32((f32)*(s32 *)&D_0015F618 * scale),
                            *(s32 *)&D_0015F610 + truncate_float_to_s32((f32)*(s32 *)&D_0015F618 * scale),
                            *(s32 *)&D_0015F60C - truncate_float_to_s32((f32)*(s32 *)&D_0015F614 * scale),
                            *(s32 *)&D_0015F60C + truncate_float_to_s32((f32)*(s32 *)&D_0015F614 * scale), pulse);
            sel = ((HeroP *)D_0013F450)->unk8C0 - 1;
            if (!(sel < 5)) {
                sel = 4;
            }
            func_00116248(line, D_0015F660, msg_string(sel + 0x5229), msg_string(0x5245));
            if (((HeroP *)D_0013F450)->unk8C0 == 1) {
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C, c2, line, -1);
            } else {
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C, c3, line, -1);
            }
            best = D_0015EF58[(D_0015EE84 ^ 0x10) ? 0 : 1];
            if (best == ((HeroP *)D_0013F450)->unk894) {
                unit = D_0015EE80 ? 3000 : 3600;
                per = unit / 60;
                min = best / unit;
                rem = best - min * unit;
                sec = rem / per;
                hund = (rem - sec * per) * 100 / per;
                func_00116248(line, D_0015F668, msg_string(0x50A3));
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x26, c2, line, -1);
                func_00116248(line, D_0015F670, min, sec, hund);
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x3A, c2, line, -1);
            } else {
                func_00116248(line, D_0015F668, msg_string(0x50A2));
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x30, c3, line, -1);
            }
            record = D_0015EF68[(D_0015EE84 ^ 0x10) ? 0 : 1];
            if (record != 0 && record == ((HeroP *)D_0013F450)->unk8A8) {
                func_00116248(line, D_0015F668, msg_string(0x50A5));
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x56, c2, line, -1);
                func_00116248(line, D_0015F680, ((HeroP *)D_0013F450)->unk8A8);
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x6A, c2, line, -1);
            } else {
                func_00116248(line, D_0015F668, msg_string(0x50A4));
                FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x60, c3, line, -1);
            }
            FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0x90, c1, msg_string(0x524E), -1);
            FontPrintCenter(0x100, *(s32 *)&D_0015F61C + 0xA8, c1, msg_string(0x524D), -1);
        }
        break;
    }
    case 2:
        draw_outlined_rect(0x64, 0xA0, 0xB0, 0x150,
                        FastTweenColor(*(s32 *)&D_0015F624, *(s32 *)&D_0015F628,
                               FastSin((f32)(D_0015F538 % *(s32 *)&D_0015F620) / IntToFloat(*(s32 *)&D_0015F620) * 6.28318f - 3.14159f) * 0.5f + 0.5f));
        FontPrintCenter(0x100, 0x7A, 0x8000C0C0, ((MessageBoxP *)D_00193400)->title, -1);
        break;
    default: {
        s32 pulse;

        pulse = FastTweenColor(*(s32 *)&D_0015F624, *(s32 *)&D_0015F628,
                       FastSin((f32)(D_0015F538 % *(s32 *)&D_0015F620) / IntToFloat(*(s32 *)&D_0015F620) * 6.28318f - 3.14159f) * 0.5f + 0.5f);
        draw_outlined_rect(0x50, *(s32 *)&D_0015F5EC + 0x1A, 0xB0, 0x150, pulse);
        if (((MessageBoxP *)D_00193400)->title != 0) {
            FontPrintCenter(0x100, 0x5A, 0x8000C0C0, ((MessageBoxP *)D_00193400)->title, -1);
        }
        if (((MessageBoxP *)D_00193400)->line1 != 0) {
            FontPrintCenter(0x100, *(s32 *)&D_0015F5E8, 0x80FFA888, ((MessageBoxP *)D_00193400)->line1, -1);
        }
        if (((MessageBoxP *)D_00193400)->line2 != 0) {
            FontPrintCenter(0x100, *(s32 *)&D_0015F5EC, 0x80FFA888, ((MessageBoxP *)D_00193400)->line2, -1);
        }
        break;
    }
    }
    DoGifPaging();
}

INCLUDE_ASM("asm/nonmatchings/text", func_001FD3E8);
