#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * vendor.cpp in the original source; text 0x239628-0x23B670.
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
extern int func_001FE4D0(void);
extern char D_00199A68[];
extern short D_0015F780;
extern int D_001941CC NOT_SDA;
extern int D_0019A4E8 NOT_SDA;
extern int func_001FF668(int);
typedef struct {
    char b[0x13];
} Cfg13;
extern Cfg13 D_0019A540 NOT_SDA;
extern Cfg13 D_001E7DD8 NOT_SDA;
extern int func_00116810(void);
extern void func_001166FC(Cfg13 *, void *);
extern short D_0015F9D0;
extern void func_00201960(int, int, int, int, int);
extern void func_002023E0(int);
extern void func_002027C0(int);
extern void func_00204FC0(void *);
extern int D_0018CC20 NOT_SDA;
extern int D_001941C8 NOT_SDA;
extern int D_0016100C;
extern int D_001A0468[];
extern void func_00205830(int a, int b);
extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");
extern int D_001A01F0[];
extern int *D_001602E0;
extern unsigned char D_0013D49C NOT_SDA;
extern unsigned char D_0013D49D NOT_SDA;
extern unsigned char D_0013D4A5 NOT_SDA;
extern short D_0015FE24;
extern unsigned char D_0013D4AC NOT_SDA;
extern unsigned char D_0013D4AD NOT_SDA;
extern unsigned char D_0013D4AE NOT_SDA;
extern unsigned char D_0013D4AF NOT_SDA;
extern unsigned char D_0013D4B5 NOT_SDA;
extern int D_001A04B4 NOT_SDA;
extern unsigned char D_0013D4C5 NOT_SDA;
extern int D_001414DC NOT_SDA;
extern unsigned char D_0013D4C0 NOT_SDA;
extern unsigned char D_0013D4C1 NOT_SDA;
extern unsigned char D_0013D4C2 NOT_SDA;
extern unsigned char D_0013D4D3 NOT_SDA;
extern unsigned char D_0013D4D4 NOT_SDA;
extern unsigned char D_0013D4D5 NOT_SDA;
extern unsigned char D_0013D4E0;
extern unsigned char D_0013D4DC NOT_SDA;
extern unsigned char D_0013D4DD NOT_SDA;
extern unsigned char D_0013D4DE NOT_SDA;
extern unsigned char D_0013D4DF NOT_SDA;
extern unsigned char D_0013D4E1 NOT_SDA;
extern unsigned char D_0013D4E9 NOT_SDA;
extern unsigned char D_0013D502 NOT_SDA;
extern unsigned char D_0013D503 NOT_SDA;
extern unsigned char D_0013D504 NOT_SDA;
extern unsigned char D_0013D505 NOT_SDA;
extern unsigned char D_0013D50F NOT_SDA;
extern int D_0013D668[];
extern void func_00209040(void);
extern int func_001FAA28(void *dst, int size, int a, int b);
extern void func_00208860(void *dst);
extern short D_0015EE84;
extern int D_0015EE84_far __asm__("D_0015EE84") NOT_SDA;
extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);
extern char D_0013D390[];
extern short D_0015EFB0;
extern int D_0015EFB4;
extern int D_001A05C0[];
extern int D_001A08C0[];
extern int func_0020BAD8(int *p);
extern int func_0020BBC8(void *dst, int i, int *table);
extern int func_001236F0(void);
extern int func_001E9730();
extern char D_001E8690[];
extern int D_0013D844 NOT_SDA;
extern unsigned char D_0013D4A8 NOT_SDA;
extern int D_0013D9B4 NOT_SDA;
extern unsigned char D_0013D490[];
extern unsigned char D_0013D5CA NOT_SDA;
extern int D_0013D6B8 NOT_SDA;
extern int D_0013DAE4 NOT_SDA;
extern unsigned char D_0013D4E5 NOT_SDA;
extern int D_0013DB24 NOT_SDA;
extern unsigned char D_0013D4F1 NOT_SDA;
extern int D_0013DC34 NOT_SDA;
extern unsigned char D_0013D605 NOT_SDA;
extern int D_0013D5C8 NOT_SDA;
extern unsigned char D_0013D4B0 NOT_SDA;
extern unsigned char D_0013DE55 NOT_SDA;
extern unsigned char D_0013D5DD NOT_SDA;
extern unsigned char D_0013D5E7 NOT_SDA;
extern int D_001B2F40[];
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern int D_001414D0 NOT_SDA;
extern float D_001CAE00[] NOT_SDA;
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_00118D80(int);
extern char D_00165600[];
extern int D_0015F718;
extern short D_0015F71C;
extern char D_001B3200[];
extern int func_001160D8(void);
extern float func_00214158(void);
extern float func_002140F8(float, float);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9DC0(void *, void *, float);
extern void func_001FA460(void *);
extern void func_002150B0(void *, void *);
extern void func_001FA480(void *, void *);
extern float func_0020D830(void);
extern float func_00215A98(int, float);
extern unsigned char D_0014BFC0[];
extern unsigned char D_0013E620[];
extern unsigned char D_0013D510[];
extern void func_0012F068(void *);
extern void func_002177F0(int);
extern short D_001517D0[];
extern void func_0012EDE0(void *);
extern void func_0012EFE8(void);
extern char D_001E8980[];
extern int func_0012EE98(int, int, int, void *);
extern int func_00217628_v(void) __asm__("func_00217628");
extern void func_00122598(int);
extern void func_00217130(void);
extern void func_0012EC40(void);
extern void func_0012DDC0(void);
extern void func_0012EC30(void);
extern int func_0012F030(void);
extern void func_002167C0(short, short, short);
extern void func_002169B8(short, short, short);
extern short D_001517F0 NOT_SDA;
extern char D_0013CA40[];
extern int D_001CDAE0 NOT_SDA;
extern void func_00124650(void);
extern void func_00124B88(int);
extern int func_00124BC8(void *, void *);
extern void func_00217F68(void *);
extern int D_0015EF90;
extern char D_001D4B90[];
extern char D_001D4BC0[];
extern char D_001D5F70[] NOT_SDA;
extern char D_001D603B[];
extern int D_001A0414;
extern int D_001CFBF4;
extern int D_001CFAD8;
extern void func_0020C7A0(void *);
extern int func_0020CA50(void *, void *, void *, int);
extern int D_00141FA0[];
extern char D_001D0A50[];
extern char D_001D0A88[];
extern int D_001A0418 NOT_SDA;
extern float func_001FA748(float, float);
extern char *D_001D5F74 NOT_SDA;
extern void func_0020E180(int, int);
extern char D_00187040[];
extern void func_00220128(void *);
extern void *func_00226720_a(int) __asm__("func_00226720");
extern int func_002267C0(int);
extern void func_00234C98(int, int);
extern void func_00205E70(void);
extern void func_001F4630(int);
extern void func_001F4748(void);
extern void func_001F68E8_c(int, int, long, void *, int)
    __asm__("func_001F68E8");
extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern short D_001602B0;
extern void func_00201640(int, int, int, int, long, long);
extern int func_00200198(int, int);
extern void func_00200468(int, int, int, int, int, int);
extern void func_001F5800(int, int, int, int, int, int, int, int, long,
                          long);
extern short D_00151880[];
extern long D_001A0448;
extern int func_00226EA8(int);
extern int func_00226F68(int);
extern int D_0013CC04 NOT_SDA;
extern char D_001D2678[];
extern char *D_001D5F78 NOT_SDA;
extern unsigned char D_001B3E40[] NOT_SDA;
extern void *func_0020D348(void);
extern void func_0020E340(void *, int, int, int, int);
typedef struct {
    int key;
    int flags;
} PadBind;
extern PadBind D_001D6448_t[] __asm__("D_001D6448");
extern int func_00227018(int handle);
extern int D_001D6448[];
extern char D_001D5D58[] NOT_SDA;
extern char *D_001B3580[] NOT_SDA;
extern int D_001D6860[];
extern int D_001D74C0[];
extern int D_001D6760[];
extern char D_00187180_a[] __asm__("D_00187180");
extern char D_00194220[];
extern int D_0013E6BC;
extern void func_002141A8(void *, float, float);
extern void func_001F9BD8_a(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9C30_a(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BF0_a(void *, void *, void *) __asm__("func_001F9BF0");
extern int func_001EFE10_a(void *, void *, int, int, int) __asm__("func_001EFE10");
extern char D_00187180[];
extern void func_001F9BF0(void *, void *, void *);
extern void func_001EFE10(void *, void *, int, int, int);
extern float func_001F9D10(int, void *);
extern void func_0022DA10(void *, float, float, float);
extern void func_001F9EE8(void *, void *, void *);
extern float func_001F9CE8(void *);
extern float func_001FA058_a(float, float) __asm__("func_001FA058");
extern void func_001FA898(float);
extern void func_00120F30(int);
extern int func_0012E060(void *, int);
extern void func_0012EE70(int);
extern void func_0012EF48(int);
extern void func_0012E2E8(void);
extern void func_0011AE20(int arg0);
typedef struct {
    int unk_00;
    int unk_04;
    int unk_08;
    int unk_0C;
} Rec10;
extern Rec10 D_001DD568[];
typedef struct {
    short a;
    short b;
} TexRemap;
typedef struct {
    char *items;
    int count;
} TexChunk;
extern TexChunk D_001E1200[];
extern TexRemap D_001E0F00[];
extern short D_00160FE0;
extern char D_001E8D10[];
extern void func_001F2568(void);
extern void func_00236A98(void);
extern char D_001E3300[];
extern char D_001E4500[];
extern char D_001E2D00[];
extern char D_001E4100[];
extern void func_00238688(void *);
extern char D_001E3500[];
extern char D_001E4700[];
extern char D_001E66C0[];
extern char D_001E8DA0[];
extern void func_001FFE88(void *arg0);
extern void func_0020DB98(char *arg0, int arg1, void *arg2, char *arg3);

/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern s32 D_001E6318[64];
extern s32 D_001E6418[64];
extern s32 D_0015F538 MACRO_ADDR;
extern void func_001F5800(s32, s32, s32, s32, s32, s32, s32, s32, s64, s64);
extern s32 func_001F98C0(s32);
extern f32 func_001FA888(s32);
#define TRUNC(x) ((s32 (*)(f32))func_001FA898)(x)
extern u64 func_00200248(s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_00238310.c, render_capture_scrolling_text. */
void Vendor_DrawScrollText(char *text, s32 start_x, s32 start_y, f32 scale) {
    s64 texture;
    s32 cursor_x;
    s32 cursor_y;
    s32 glyph_index;
    s32 blink_next_glyph;
    s32 skip_draw;
    s32 long_blink_period;
    s32 short_blink_period;
    s32 blink_period;
    s32 blink_remainder;
    char *cursor_text;
    s32 packed_coordinates;
    s32 texture_v;
    s32 texture_u;
    s32 glyph_width;
    s32 glyph_height;
    u8 character;

    blink_next_glyph = 0;
    cursor_text = text;
    texture = GetFrameTex(GetIconFrame(0xE935, 0));
    cursor_x = start_x;
    cursor_y = start_y;

    for (;;) {
        skip_draw = 0;
        if (*(u8 *)cursor_text == 0) {
            return;
        }
        character = (u8)*cursor_text++;
        glyph_index = (s32)character - 0x20;

        if (glyph_index == 0x42) {
            blink_next_glyph = 1;
        }
        /* Retail retains this branch even though an unsigned byte minus 0x20 cannot reach 0xEA. */
        if (glyph_index == 0xEA) {
            cursor_x = start_x;
            cursor_y += 9;
        }

        if (glyph_index < 0x3B) {
            {
                s32 *glyph_entry;
                glyph_entry = D_001E6318 + glyph_index;
                if (*glyph_entry != -1 || glyph_index == 0) {
                    if (blink_next_glyph != 0) {
                        skip_draw = 0;
                        blink_next_glyph = 0;
                        long_blink_period = func_001F98C0(0x1E);
                        short_blink_period = func_001F98C0(0xA);
                        blink_period = long_blink_period + short_blink_period;
                        blink_remainder = D_0015F538 % blink_period;
                        long_blink_period = func_001F98C0(0xA);
                        if (blink_remainder < long_blink_period) {
                            skip_draw = 1;
                        }
                    }

                    if (glyph_index != 0 && skip_draw == 0) {
                        packed_coordinates = *glyph_entry;
                        texture_v = (u32)packed_coordinates >> 0x14;
                        texture_u = (packed_coordinates & 0xFFFF) >> 4;

                        if (cursor_x + 9 > 0) {
                            if (cursor_x < 0x100) {
                                glyph_width =
                                    TRUNC(IntToFloat(9) * scale);
                                glyph_height =
                                    TRUNC(IntToFloat(9) * scale);
                                DrawTexturedQuad(cursor_x, cursor_y, glyph_width, glyph_height,
                                                   texture_u, texture_v, 9, 9, (s64)0x80404040,
                                                   texture);
                            }
                        }
                    }

                    cursor_x += TRUNC(
                        IntToFloat(D_001E6418[glyph_index]) * scale);
                }
            }
        }
    }
}

extern int func_00116810_s(char *) __asm__("func_00116810");
extern int func_002140B0(int);
extern int D_001E62B8[];
extern void func_002391A8(void *);
extern void func_00239628(char *, int, int, float);
extern void func_001FBAB8(int, int, int, int, int, int, int);

/* Two base locals, one before the join and one after, make gcse share
   only the %hi and rebuild the %lo base after the join; the first must
   be assigned after the first call. func_00116810 is strlen. */
void update_scrolling_status_message(void) {
    char *v;
    char *w;
    Framebuf_DrawAlphaRect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    v = D_001E66C0;
    if (func_00116810_s(*(char **)(v + 0x2C)) - *(int *)(v + 0x44) / 20 > 0) {
        *(int *)(v + 0x44) += 2;
    } else {
        set_scrolling_status_message(func_001FE540_id(D_001E62B8[random_integer_below(0x18)]));
    }
    w = D_001E66C0;
    Vendor_DrawScrollText(*(char **)(w + 0x2C), -*(int *)(w + 0x44), 8, 2.0f);
    Framebuf_DrawAlphaRect(0, 0, 4, 0x40, 0x200, 0x80, 0);
    Framebuf_DrawAlphaRect(0xE2, 0, 0xE6, 0x40, 0x200, 0x80, 0);
}

extern int D_001611A8 MACRO_ADDR;
extern int D_001E66E4;

void VendorDrawSalesmanVideo(void) {
    Framebuf_DrawAlphaRect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    if (D_001611A8 == 1) {
        DrawMobyList(D_001E66E4, 1);
    }
}

extern char D_00161178[];
extern char D_00161180[];
/* Unprototyped: the two call sites pass different argument counts. */
extern void func_00116248();

void FormatThousands(void *arg0, int arg1) {
    if (arg1 >= 1000) {
        func_00116248(arg0, D_00161178, arg1 / 1000, arg1 % 1000);
    } else {
        func_00116248(arg0, D_00161180, arg1);
    }
}

struct VendorMenuState {
    u8 pad_0[0x4];
    s32 pulseTick;
    u8 pad_8[0x44];
    s32 scrollOffset;
    u8 pad_50[0x8];
    s32 selectedIndex;
    u8 pad_5C[0x1B4];
    s32 slotCount;
};

struct VendorItemResource {
    u8 pad_0[0x38];
    u16 iconId;
};

extern u8 D_001864D0[];
extern struct VendorMenuState D_001E66C0_ui __asm__("D_001E66C0");
extern s32 func_001F9B70();
extern void func_001FBAB8_u(int, int, int, int, int, int, unsigned int)
    __asm__("func_001FBAB8");

/* Vendor item carousel and icon pulse. The solid black draw calls use the
   unsigned color signature, matching retail's argument setup order. */
void VendorDrawIconStrip(void) {
    s32 selectedOffset;
    s32 slotOffset;
    s32 selectedMinusThree;
    s32 x;
    s32 i;
    s32 itemIndex;
    s32 itemX;
    s32 displayIcon;
    s32 itemIcon;
    u16 displayIconId;
    u16 itemIconId;
    s32 *slotKind;
    s32 pulseColor;

    Framebuf_DrawAlphaRect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    if (D_001E66C0_ui.slotCount < 8) {
        itemX = 0xC;
        itemIndex = 0;
        selectedOffset = D_001E66C0_ui.selectedIndex * 0x38;
        pulseColor = ((func_001F9B70(((D_001E66C0_ui.pulseTick * 4) & 0x3F) - 0x20) + 0x40) * 0x10202) | 0x80000000;
        Framebuf_DrawAlphaRect(selectedOffset + 8, 2, selectedOffset + 0x40, 0x3A, 0x200, 0x80, pulseColor);
        func_001FBAB8_u(selectedOffset + 0xA, 4, selectedOffset + 0x3E, 0x38, 0x200, 0x80, 0x80000000);
        if (D_001E66C0_ui.slotCount > 0) {
            do {
                slotKind = (s32 *) (void *) ((u8 *)&D_001E66C0_ui + 0xD4 + itemIndex * 0x14);
                itemIconId = ((struct VendorItemResource *) (void *) ((u8 *)D_001864D0 + slotKind[-1] * 0x4C))->iconId;
                if (slotKind[0] == 1) {
                    itemIcon = GetIconFrame(itemIconId, 2);
                } else {
                    itemIcon = GetIconFrame(itemIconId, 0);
                }
                HudSprite(itemIcon, itemX, 6, 0x30, 0x30, 0x80);
                itemX += 0x38;
                itemIndex += 1;
            } while (itemIndex < D_001E66C0_ui.slotCount);
        }
    } else {
        if (D_001E66C0_ui.scrollOffset < 0) {
            D_001E66C0_ui.scrollOffset = (s32) (D_001E66C0_ui.scrollOffset + 4);
        } else if (D_001E66C0_ui.scrollOffset > 0) {
            D_001E66C0_ui.scrollOffset = (s32) (D_001E66C0_ui.scrollOffset - 4);
        } else {
            Framebuf_DrawAlphaRect(0xB0, 2, 0xE8, 0x3A, 0x200, 0x80, ((func_001F9B70(((D_001E66C0_ui.pulseTick * 4) & 0x3F) - 0x20) + 0x40) * 0x10202) - (s32) 0x80000000);
            func_001FBAB8_u(0xB2, 4, 0xE6, 0x38, 0x200, 0x80, 0x80000000);
        }
        x = D_001E66C0_ui.scrollOffset - 0x64;
        i = -2;
        do {
            selectedMinusThree = D_001E66C0_ui.selectedIndex - 3;
            slotOffset = ((s32) ((D_001E66C0_ui.slotCount * 2) + i + selectedMinusThree) % (s32) D_001E66C0_ui.slotCount) * 0x14;
            displayIconId = ((struct VendorItemResource *) (void *) ((u8 *)D_001864D0 + *(s32 *)(void *)(slotOffset + ((u8 *)&D_001E66C0_ui + 0xD0)) * 0x4C))->iconId;
            if (*(s32 *)(void *)(slotOffset + ((u8 *)&D_001E66C0_ui + 0xD4)) == 1) {
                displayIcon = GetIconFrame(displayIconId, 2);
            } else {
                displayIcon = GetIconFrame(displayIconId, 0);
            }
            i += 1;
            HudSprite(displayIcon, x, 6, 0x30, 0x30, 0x80808080);
            x += 0x38;
        } while (i < 9);
    }
}

struct VM239 {
    char pad0[0x20];
    int f20;
    char pad24[0x1C];
    int f40;
    char pad44[0x14];
    int idx;
    char pad5C[0x74];
    struct { int id; int kind; int pad[3]; } slots[1];
};
struct T239 { int text; char pad[0x48]; };
struct P239 { int a; int b; unsigned short c; unsigned short d; int pad[3]; };

extern void func_001F6968_c(int, int, long, void *, int) __asm__("func_001F6968");
extern void func_002399A0(void *arg0, int arg1);
extern void func_001F6D88_c(int, int, long, void *, int) __asm__("func_001F6D88");
extern struct VM239 D_001E66C0_m __asm__("D_001E66C0");
extern struct T239 D_001864D0_t[] __asm__("D_001864D0");
extern struct P239 D_001E02B0_p[] __asm__("D_001E02B0");
extern unsigned char D_0013D5C8_b[] __asm__("D_0013D5C8") NOT_SDA;

/* Vendor item panel: draws the selected item's name and description, and for the ammo/alt slots a pulsing price shadow. Adapted from Lombyte (MIT) for PAL: src/ui/menus/fun_002389e0.c, fun_002389e0. */
void VendorDrawItemPanel(void) {
    char buf[0x100];
    int w;

    Framebuf_DrawAlphaRect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    DrawMobyList(D_001E66C0_m.f20 + 0x100, 1);
    DrawMobyList(D_001E66C0_m.f20, 1);
    if (D_001E66C0_m.slots[D_001E66C0_m.idx].kind == 1) {
        func_001F6968_c(6, 8, 0x80F0F0F0L, func_001FE540_id(D_001864D0_t[D_001E66C0_m.slots[D_001E66C0_m.idx].id].text), -1);
        func_001F6968_c(0x18, 0x18, 0x80F0F0F0L, func_001FE540_id(0x4F5D), -1);
        FormatThousands(buf, D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].c);
        func_001F6D88_c(0x76, 0x65, 0x80F0F0F0L, buf, -1);
        if (D_001E66C0_m.f40 != 0) {
            w = MeasureTextWidthRegular((unsigned char *)buf, -1);
            func_00201640(0x75 - w, 0x6D, 0x7B, 0x70, 0x20959544L, 0);
            func_00201640(0x76 - w, 0x6D, 0x7A, 0x70, 0x30959544L, 0);
            func_00201640(0x77 - w, 0x6D, 0x79, 0x70, 0x40959544L, 0);
            func_00201640(0x78 - w, 0x6D, 0x78, 0x70, 0x50959544L, 0);
            func_00201640(0x79 - w, 0x6D, 0x77, 0x70, 0x60959544L, 0);
            func_00201640(0x7A - w, 0x6D, 0x76, 0x70, 0x70959544L, 0);
            func_00201640(0x7B - w, 0x6D, 0x75, 0x70, 0x80959544L, 0);
            FormatThousands(buf, D_001E66C0_m.f40 ? D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].d
                                                 : D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].c);
            func_001F6D88_c(0x76, 0x55, 0x80F0F0F0L, buf, -1);
        }
    } else {
        func_001F6968_c(6, 8, 0x80F0F0F0L, func_001FE540_id(D_001864D0_t[D_001E66C0_m.slots[D_001E66C0_m.idx].id].text), -1);
        FormatThousands(buf, D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].a);
        func_001F6D88_c(0x76, 0x65, D_0013D5C8_b[0x23] ? 0x80808080L : 0x80F0F0F0L, buf, -1);
        if (D_0013D5C8_b[0x23] != 0) {
            w = MeasureTextWidthRegular((unsigned char *)buf, -1);
            func_00201640(0x75 - w, 0x6D, 0x7B, 0x70, 0x20959544L, 0);
            func_00201640(0x76 - w, 0x6D, 0x7A, 0x70, 0x30959544L, 0);
            func_00201640(0x77 - w, 0x6D, 0x79, 0x70, 0x40959544L, 0);
            func_00201640(0x78 - w, 0x6D, 0x78, 0x70, 0x50959544L, 0);
            func_00201640(0x79 - w, 0x6D, 0x77, 0x70, 0x60959544L, 0);
            func_00201640(0x7A - w, 0x6D, 0x76, 0x70, 0x70959544L, 0);
            func_00201640(0x7B - w, 0x6D, 0x75, 0x70, 0x80959544L, 0);
            FormatThousands(buf, D_0013D5C8_b[0x23] ? D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].b
                                                 : D_001E02B0_p[D_001E66C0_m.slots[D_001E66C0_m.idx].id].a);
            func_001F6D88_c(0x76, 0x55, 0x80F0F0F0L, buf, -1);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/text", func_0023A220);

extern int D_0013D530[];
extern int D_0015EE98 MACRO_ADDR;
extern char D_001E02B0[];
extern int func_001F6F40_c(int, int, long, void *, int) __asm__("func_001F6F40");

typedef struct {
    int type; /* +0x0 */
    int kind; /* +0x4: 1 == ammo */
    int pad[3];
} VendorSlot;

typedef struct {
    char pad0[0x40];
    int alt;             /* +0x40 */
    char pad1[0x14];
    int idx;             /* +0x58 */
    int sel;             /* +0x5C */
    char pad2[0x70];
    VendorSlot slots[1]; /* +0xD0 */
} Vendor;
extern Vendor D_001E66C0_v __asm__("D_001E66C0");

typedef struct {
    char pad0[8];
    unsigned short price;    /* +0x8 */
    unsigned short priceAlt; /* +0xA */
    char pad1[2];
    unsigned short max;      /* +0xE */
    char pad2[8];
} WeaponInfo; /* 0x18 */

/* Vendor ammo prompt: draws the panel, then shows text 0x4EE0 when
   nothing is selected, or when the selected slot is an ammo slot
   (kind 1) that is not full (D_0013D530[type] < max) and the player's
   D_0015EE98 covers its price (priceAlt when +0x40 is set).
   The kind test is two separate ifs: the first one's failure jumps to
   the second, so CSE sees a label there and keeps the re-test retail
   has. The D_001E02B0 records are reached as char arithmetic cast to a
   struct, which gives retail's index-first addu with the field offset
   left as the load displacement. */
void VendorDrawAmmoPrompt(void) {
    Vendor *v;
    int idx;
    int price;

    Framebuf_DrawAlphaRect(0, 0, 0x200, 0x80, 0x200, 0x80, 0);
    v = &D_001E66C0_v;
    if (v->sel != 0) {
        idx = v->idx;
        if (v->slots[idx].kind == 1 &&
            D_0013D530[v->slots[idx].type] >=
                ((WeaponInfo *)(D_001E02B0 + v->slots[idx].type * 0x18))->max) {
            return;
        }
        if (v->slots[idx].kind == 1) {
            if (v->alt) {
                price = ((WeaponInfo *)(D_001E02B0 + v->slots[idx].type * 0x18))->priceAlt;
            } else {
                price = ((WeaponInfo *)(D_001E02B0 + v->slots[idx].type * 0x18))->price;
            }
            if (gBolts >= price) {
                func_001F6F40_c(0x28, 0x14, 0x80F0F0F0L, func_001FE540_id(0x4EE0), -1);
            }
        }
    } else {
        func_001F6F40_c(0x28, 0x14, 0x80F0F0F0L, func_001FE540_id(0x4EE0), -1);
    }
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_0023A5D8);

extern s32 D_001E6920[];
extern s32 D_001E6940[];
extern void func_00234C98_3A5E0(s32, u64) __asm__("func_00234C98");
extern s32 func_002140B0(s32);
extern s32 func_001F9B70(s32);
extern s64 func_001F4868_3A5E0(s32) __asm__("func_001F4868");
extern f32 func_001FA888(s32);
extern void func_001F5988(f32, f32, f32, f32, s32, s32, s32, s32, u64, s64);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/render_vendor_capture_texture_overlays_pass.c, render_vendor_capture_texture_overlays_pass. */
void func_0023A5E0(s32 pass_index, f32 capture_width,
                                                 f32 capture_height) {
    s32 flash_timer;
    s32 scroll_opacity;
    s32 flash_opacity;
    f32 random_u;
    f32 random_v;
    f32 zero_offset;
    f32 scroll_offset;
    f32 overlay_height;
    s32 overlay_width;

    func_00234C98_3A5E0(0x47, 0x32003);
    if (D_001E6920[pass_index] != 0 || pass_index == 0) {
        if (D_001E6920[pass_index] != 0) {
            D_001E6920[pass_index] += 2;
        }
        flash_timer = D_001E6920[pass_index];
        if (pass_index == 0) {
            /* The floor changes this frame's opacity, not the stored timer. */
            if (flash_timer < 0x18) {
                flash_timer = 0x18;
            }
        }
        random_u = random_integer_below(200);
        random_v = random_integer_below(200);
        zero_offset = 0.0f;
        flash_opacity = 0x80 - func_001F9B70(flash_timer - 0x80);
        func_00234C98_3A5E0(8, 0);
        flash_opacity *= 2;
        if (flash_opacity > 0x80)
            flash_opacity = 0x80;
        func_00234C98_3A5E0(0x42, ((u64)flash_opacity << 32) | 0x68);
        func_001F5988(0.0f, 0.0f, capture_width, capture_height,
                                             random_u + zero_offset, random_v + zero_offset,
                                             capture_width + random_u, capture_height + random_v,
                                             0x808080, func_001F4868_3A5E0(0x1A));
        if (D_001E6920[pass_index] >= 0x100) {
            D_001E6920[pass_index] = 0;
        }
    }
    if (D_001E6920[pass_index] == 0 && random_integer_below(700) == 0) {
        D_001E6920[pass_index] = 2;
    }
    func_00234C98_3A5E0(8, 0);
    func_00234C98_3A5E0(0x42, 0x8000000044ULL);
    if (pass_index > 0) {
        if (D_001E6940[pass_index] != 0) {
            D_001E6940[pass_index] += 2;
            overlay_width = capture_width;
            scroll_opacity =
                0x100 - func_001F9B70(D_001E6940[pass_index] - 0x100);
            if (scroll_opacity > 0x50) {
                scroll_opacity = 0x50;
            }
            scroll_offset =
                -(IntToFloat(0x200 - D_001E6940[pass_index]) * 0.03125f);
            overlay_height = capture_height + 16.0f;
            func_001F5988(0.0f, scroll_offset, capture_width, overlay_height,
                                                 0, 0, overlay_width, (s32)(overlay_height * 1.5f),
                                                 (scroll_opacity << 24) | 0x505050,
                                                 func_001F4868_3A5E0(0x1C));
            if (D_001E6940[pass_index] >= 0x200) {
                D_001E6940[pass_index] = 0;
            }
        } else if (random_integer_below(360) == 0) {
            D_001E6940[pass_index] = 2;
        }
    }
    if (pass_index == 6) {
        /* The known capture coordinator supplies only passes zero through five. */
        func_001F5988(0.0f, 0.0f, capture_width, capture_height, 0, 0, 0x40,
                                             0x40, 0x80808080, func_001F4868_3A5E0(0x19));
    }
}

extern void func_001FB608(int, int, int);
extern void func_001F3760(int, int, float, float, float, float, float);
extern void func_00234C98_l(int, long) __asm__("func_00234C98");

/* Sets up a (1 << a) x (1 << b) area: func_001FB608 gets the sizes and a
   base below 0x3FF000 by 4 << min(a + b, 16) bytes, rounded down to 8 KB
   (the GS page), func_001F3760 the sizes and the float setup (f, 0,
   524288, 255, 0); then GS registers 0x47 and 0x42 are written through
   func_00234C98, whose value argument is 64-bit (hence the long alias,
   which builds 0x8000000044 as one dli). */
void Framebuf_AllocRenderTarget(int a, int b, float f) {
    int t = a + b;

    if (t > 16) {
        t = 16;
    }
    Framebuf_SetRenderTarget(a, b, ((0x3FF000 - (4 << t)) >> 13) << 13);
    SetScreenSize(1 << a, 1 << b, f, 0.0f, 524288.0f, 255.0f, 0.0f);
    func_00234C98_l(0x47, 0x30000);
    func_00234C98_l(0x42, 0x8000000044L);
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");


void setupViewContext(void) {
    PutDrawBufferLarge();
    InitViewContext();
    UpdateViewContext();
}

typedef int u128_3AA38 __attribute__((mode(TI)));
extern float D_001E6518[][4];
extern int D_00161398 MACRO_ADDR;
extern short D_001613A0;
extern short D_0016139C;
extern short D_00161394;
extern int D_0015EE80 MACRO_ADDR;
extern long D_0015EFD0 MACRO_ADDR;
extern float func_001F9CB8(void *);
extern void func_00238F98(void *, void *, float *, float *, float *, float *);
extern void func_001FB908(int, int);
extern void func_00234E80(void);
extern int func_001FA898_3AA38(float) __asm__("func_001FA898");
extern void func_00239838_3AA38(char *) __asm__("func_00239838");
extern void func_00239CF8_3AA38(char *) __asm__("func_00239CF8");
extern void func_00239948_3AA38(char *) __asm__("func_00239948");
extern void func_0023A220(char *, int, int);
extern void func_0023A478_3AA38(char *) __asm__("func_0023A478");
extern void func_00239A00_3AA38(char *) __asm__("func_00239A00");

/* Renders the six vendor monitor screens: projects each screen's joint box, renders its view off-screen and draws it back. */
void VendorDrawScreens(char *m) {
    float len;
    float f;
    float g;
    int i;
    long tex;

    for (i = 0; i < 6; i++) {
        u128_3AA38 d1[1];
        u128_3AA38 d2[1];
        u128_3AA38 base[1];
        u128_3AA38 out[3];
        int ids[3] = { i * 4, i * 4 + 1, i * 4 + 2 };
        u128_3AA38 tmp[1];
        u128_3AA38 c[4];
        u128_3AA38 pad[5];
        u128_3AA38 a[1];
        u128_3AA38 b[1];
        float sx, sy, sw, sh;
        float x, y, w, h;
        MobyTransformScaledVertexBatch(m, 3, ids, (char *)out);
        qcopy(base, out);
        FastVecSub(d1, &out[1], out);
        FastVecSub(d2, &out[2], out);
        len = FastVecLength(d1);
        func_001F9DC0(tmp, d1, D_001E6518[i][0]);
        FastVecAdd(base, base, tmp);
        func_001F9DC0(d1, d1, len - 2.0f * D_001E6518[i][2]);
        len = FastVecLength(d2);
        func_001F9DC0(tmp, d2, D_001E6518[i][1]);
        FastVecAdd(base, base, tmp);
        func_001F9DC0(d2, d2, len - 2.0f * D_001E6518[i][3]);
        FastVecAdd(tmp, base, d1);
        FastVecAdd(tmp, tmp, d2);
        func_00238F98(base, tmp, &sx, &sy, &sw, &sh);
        if (D_00161398 != 0 || *(int *)&D_001613A0 != 0) {
            f = 1.0f;
            if (*(int *)&D_001613A0 != 0) {
                f = IntToFloat(8 - *(int *)&D_0016139C) * 0.125f;
            }
            if (D_00161398 != 0) {
                f = IntToFloat(*(int *)&D_00161394) * 0.125f;
            }
            len = FastVecLength(d1);
            g = 1.0f - f;
            func_001F9DC0(tmp, d1, len * g * 0.5f);
            func_001F9DC0(d1, d1, len * f);
            FastVecAdd(base, base, tmp);
            len = FastVecLength(d2);
            func_001F9DC0(tmp, d2, len * g * 0.5f);
            func_001F9DC0(d2, d2, len * f);
            FastVecAdd(base, base, tmp);
        }
        qcopy(&c[0], base);
        FastVecAdd(&c[1], base, d1);
        FastVecAdd(&c[2], base, d2);
        FastVecAdd(&c[3], &c[2], d1);
        a[0] = c[0];
        b[0] = c[3];
        func_00238F98(a, b, &x, &y, &w, &h);
        Framebuf_AllocRenderTarget(9, 7, 1.0f);
        Framebuf_ClearScreen(0x200, 0x200);
        func_00234C98_l(0x42, 0x8000000064L);
        switch (i) {
        case 0:
            func_00239838_3AA38(m);
            break;
        case 1:
            func_00239CF8_3AA38(m);
            break;
        case 2:
            func_00239948_3AA38(m);
            break;
        case 3:
            func_0023A220(m, (int)x, (int)y);
            break;
        case 4:
            func_0023A478_3AA38(m);
            break;
        case 5:
            func_00239A00_3AA38(m);
            break;
        }
        func_0023A5E0(i, x, y);
        setupViewContext();
        Framebuf_ClearScreen(0x200, 0x200);
        func_00234C98_l(0x42, 0x8000000064L);
        func_00234C98_l(8, 5);
        VU1_texFlush();
        tex = D_0015EFD0;
        if (D_0015EE80 != 0) {
            func_001F5988(w, h, x, y, 0, 0, (int)(sx - 1.0f),
                          func_001FA898_3AA38(sy * 0.92857146f - 1.0f),
                          0x80808080, tex);
        } else {
            func_001F5988(w, h, x, y, 0, 0, (int)(sx - 1.0f), (int)(sy - 1.0f),
                          0x80808080, tex);
        }
        func_00234C98_l(8, 0);
    }
}

LINKER_REMNANT("asm/remnants/text", func_0023B008);

typedef struct {
    int x;
    int y;
    int z;
    unsigned short w;
    unsigned short h;
} ZoneBox;
extern ZoneBox *D_00161294 MACRO_ADDR;
extern char *D_00161290 MACRO_ADDR;
extern int D_00161298 MACRO_ADDR;
extern float D_001E69E0[];
extern int func_001FA898_i(float) __asm__("func_001FA898");

/* Index of the zone whose box (in 1/1024 units, 0x800 deep) holds the
   point and whose 4x4 cell mask at +0x1E has the point's cell set, or
   -1. Cells are sized and offset by the grid in D_001E69E0. */
int FindZoneAtPoint(float x, float y, float z) {
    int ix = func_001FA898_i(x * 1024.0f);
    int iy = func_001FA898_i(y * 1024.0f);
    int iz = func_001FA898_i(z * 1024.0f);
    ZoneBox *b = D_00161294;
    int i;

    for (i = 0; i < D_00161298; i++, b++) {
        char *zn;
        int cx;
        int cy;
        int bit;

        if (ix < b->x || iy < b->y || iz < b->z) {
            continue;
        }
        if (ix >= b->x + b->w || iy >= b->y + b->h || iz >= b->z + 0x800) {
            continue;
        }
        zn = D_00161290 + i * 0x1190;
        cx = func_001FA898_i((x - (*(float *)(zn + 0) + D_001E69E0[2])) / D_001E69E0[4]);
        cy = func_001FA898_i((y - (*(float *)(zn + 4) + D_001E69E0[3])) / D_001E69E0[5]);
        bit = 1 << (((cx >> 2) & 3) | (cy & 0xC));
        if (*(unsigned short *)(zn + 0x1E) & bit) {
            return i;
        }
    }
    return -1;
}

/* 12 bytes of post-endlabel nop padding in retail -- see func_001F6668. */
__asm__(".section .text\n\tnop\n\tnop\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_0023B1E8);

typedef struct {
    f32 samples[16][16];
    f32 pad_400[0x20];
    f32 bottom_edge[16];
    f32 right_edge[16];
    f32 pad_500[0x23];
    f32 corner;
    f32 pad_590[0xC];
} SurfaceHeightLayer;
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    u8 pad_C[0x44];
    SurfaceHeightLayer layers[3];
} SurfaceHeightTile;
typedef struct {
    u8 pad_0[8];
    f32 origin_x;
    f32 origin_y;
    f32 cell_width;
    f32 cell_height;
} SurfaceHeightGrid;
extern SurfaceHeightTile * D_00161290_3B210 __asm__("D_00161290") MACRO_ADDR;
extern SurfaceHeightGrid D_001E69E0_3B210 __asm__("D_001E69E0");
extern short D_001611E0;
extern s32 func_0023B018(f32, f32, f32);
extern s32 func_001FA898_3B210(f32) __asm__("func_001FA898");
extern void func_001F9CA0(f32 *, f32 *, f32 *);
extern void func_001F9DC0_3B210(f32 *, f32 *, f32) __asm__("func_001F9DC0");

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_00239f58.c, sample_surface_height_map. */
s32 SampleSurfaceHeight(f32 *height, f32 *normal, f32 x, f32 y, f32 z) {
    f32 x_tangent[4] __attribute__((aligned(16)));
    f32 y_tangent[4] __attribute__((aligned(16)));
    SurfaceHeightTile *tile;
    s32 tile_index;
    s32 column;
    s32 row;
    f32 fraction_x;
    f32 fraction_y;
    f32 h00;
    f32 h10;
    f32 h01;
    f32 h11;
    f32 first_row_height;
    f32 weight_x;
    f32 weight_y;

    tile_index = FindZoneAtPoint(x, y, z);
    if (tile_index < 0) {
        return 0;
    }
    tile = &D_00161290_3B210[tile_index];
    fraction_x = tile->x;
    fraction_y = tile->y;
    fraction_x += D_001E69E0_3B210.origin_x;
    fraction_y += D_001E69E0_3B210.origin_y;
    fraction_x = x - fraction_x;
    fraction_y = y - fraction_y;
    column = func_001FA898_3B210(fraction_x / D_001E69E0_3B210.cell_width);
    row = func_001FA898_3B210(*&fraction_y / D_001E69E0_3B210.cell_height);
    fraction_x -= IntToFloat(column) * D_001E69E0_3B210.cell_width;
    weight_x = fraction_x / D_001E69E0_3B210.cell_width;
    fraction_y -= IntToFloat(row) * D_001E69E0_3B210.cell_height;
    /* Divide the signed remainder after rounding the row, before choosing the sample bank. */
    weight_y = fraction_y / D_001E69E0_3B210.cell_height;
    h00 = tile->layers[(*(s32 *)&D_001611E0)].samples[row][column];
    if (column == 15) {
        h10 = tile->layers[(*(s32 *)&D_001611E0)].right_edge[row];
    } else {
        h10 = tile->layers[(*(s32 *)&D_001611E0)].samples[row][column + 1];
    }
    if (row == 15) {
        h01 = tile->layers[(*(s32 *)&D_001611E0)].bottom_edge[column];
    } else {
        h01 = tile->layers[(*(s32 *)&D_001611E0)].samples[row + 1][column];
    }
    if (column == 15) {
        if (row == 15) {
            h11 = tile->layers[(*(s32 *)&D_001611E0)].corner;
        } else {
            h11 = tile->layers[(*(s32 *)&D_001611E0)].right_edge[row + 1];
        }
    } else if (row == 15) {
        h11 = tile->layers[(*(s32 *)&D_001611E0)].bottom_edge[column + 1];
    } else {
        h11 = tile->layers[(*(s32 *)&D_001611E0)].samples[row + 1][column + 1];
    }
    if (height != 0) {
        first_row_height = h00 + (h10 - h00) * weight_x;
        *height = first_row_height +
                  ((h01 + (h11 - h01) * weight_x) - first_row_height) * weight_y + tile->z;
    }
    if (normal != 0) {
        x_tangent[0] = D_001E69E0_3B210.cell_width;
        x_tangent[1] = 0.0f;
        x_tangent[2] = h10 - h00;
        x_tangent[3] = 1.0f;
        y_tangent[0] = 0.0f;
        y_tangent[1] = D_001E69E0_3B210.cell_height;
        y_tangent[2] = h01 - h00;
        y_tangent[3] = 1.0f;
        FastVecCross(normal, x_tangent, y_tangent);
        func_001F9DC0_3B210(normal, normal, 1.0f);
    }
    return 1;
}
__asm__(".section .text\n\tnop\n\tnop\n\tnop\n");

LINKER_REMNANT("asm/remnants/text", func_0023B510);

/* If flag 0x2 at +0x70 is set, sets +0x54 to 1.0 (0.0 once +0x58 is
   positive) and clears +0x58; then calls func_0023B5D0. The 0x68 bytes of
   linker remnant in front of it were split off. */
extern void func_0023B5D0(char *arg0);

void func_0023B578(char *arg0) {
    if (*(unsigned char *)(arg0 + 0x70) & 0x2) {
        float f0 = *(float *)(arg0 + 0x58);
        float f1 = 0.0f;
        float f2 = 1.0f;
        if (f1 < f0) {
            f2 = f1;
        }
        *(float *)(arg0 + 0x54) = f2;
        *(float *)(arg0 + 0x58) = f1;
    }
    QuadMoby_UpdateEdgeLengths(arg0);
}

typedef struct {
    char b[0x10];
} Cfg16;

extern Cfg16 D_00160FD0 NOT_SDA;
extern float func_001F9CB8(void *);
extern void func_001F9BF0_b(void *, void *, void *) __asm__("func_001F9BF0");

/* Run the four-vertex transform over the block hanging off arg0+0x78
   with a fresh copy of the D_00160FD0 parameters (a plain struct
   assignment: at alignment 1 this compiler expands the 16 bytes as
   unaligned ldl/ldr + sdl/sdr pairs, which is retail's shape -- same
   idiom as func_001FFE88), then record the two resulting lengths at
   +0x40 and +0x44. */
void QuadMoby_UpdateEdgeLengths(char *arg0) {
    float a[4];
    float b[4];
    Cfg16 cfg;
    char *p = *(char **)(arg0 + 0x78);

    cfg = D_00160FD0;
    MobyTransformScaledVertexBatch(arg0, 4, &cfg, p);
    func_001F9BF0_b(a, p + 0x10, p);
    func_001F9BF0_b(b, p + 0x20, p);
    *(float *)(p + 0x40) = FastVecLength(a);
    *(float *)(p + 0x44) = FastVecLength(b);
}
