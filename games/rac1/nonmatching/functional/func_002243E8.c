/* func_002243E8 -- src/game/pause.c (functional C for the port, not a match)
 * Load-slot menu handler, the Load Game page's sibling of func_00224040 (the save page).
 * While a load is armed (D_001D5F70+0x128) and the card state machine D_0013D390 is idle, it
 * disarms it, then either fails (card error +0xEC, +0x1C or D_0015FF4C: sets flag 0x100 in
 * D_0015EFB4 and opens the 3 dialog) or finishes the load: the mixer's six group volumes from the
 * sound and music volumes, func_0012E380, then func_0022F4A0 for the level the save names.
 * Otherwise the usual pad handling (0xD00 closes, 0x10 goes back), and while the card is a PS2 card
 * (+0x8 == 2) up/down (0x1000/0x4000) pick one of the five slots and 0x40 on a used slot arms
 * the load (card state 0xD, prompt 0x4FB6). Decoded from the PAL assembly; the NTSC decomp's
 * loading_data_menu (FUN_002232d8) was the structural reference.
 * equiv: written for the port; follows every branch and store of retail's assembly.
 */
extern s32 D_0015EEF0_2243E8 __asm__("D_0015EEF0") MACRO_ADDR; /* sound volume */
extern s32 D_0015EEEC_2243E8 __asm__("D_0015EEEC") MACRO_ADDR; /* music volume */
extern s32 D_0015EEE8_2243E8 __asm__("D_0015EEE8") MACRO_ADDR;
extern s32 D_0015EE84_2243E8 __asm__("D_0015EE84") MACRO_ADDR; /* the level */
extern s32 D_0015EFB4_2243E8 __asm__("D_0015EFB4") MACRO_ADDR;
extern s32 D_0015EFB0_2243E8 __asm__("D_0015EFB0") MACRO_ADDR;
extern s32 D_0015EF34_2243E8 __asm__("D_0015EF34") MACRO_ADDR; /* the selected slot */
extern s32 D_0015FF4C_2243E8 __asm__("D_0015FF4C") MACRO_ADDR;
extern s16 D_0013E15A_2243E8 __asm__("D_0013E15A");
extern char D_0013E650_2243E8[] __asm__("D_0013E650"); /* the mixer */
extern char D_001D5F70_2243E8[] __asm__("D_001D5F70"); /* the menu system */
extern char D_0013D390_2243E8[] __asm__("D_0013D390"); /* the memory card state */
extern char D_0013CA40_2243E8[] __asm__("D_0013CA40"); /* the pad */
extern void func_001FBC80_2243E8(s32, void *, s32) __asm__("func_001FBC80");
extern s32 func_0022ED80_2243E8(s32, s32, s32) __asm__("func_0022ED80");
extern void func_0012E380_2243E8(s32) __asm__("func_0012E380");
extern void func_0022F4A0_2243E8(s32) __asm__("func_0022F4A0");

s32 func_002243E8(char *arg0) {
    char *g = D_001D5F70_2243E8;
    char *d = D_0013D390_2243E8;
    char *pad = D_0013CA40_2243E8;
    s32 old = *(s32 *)(arg0 + 0x40);
    s32 p;
    s32 x;

    if (*(s32 *)(g + 0x128) != 0) {
        if (*(s32 *)(d + 0xDC) >= 3 || *(s32 *)(d + 0xE4) >= 0 || *(s32 *)(g + 0x154) < 11) {
            return 0;
        }
        *(s32 *)(g + 0x128) = 0;
        if (*(s32 *)(d + 0xEC) != 0 || *(s32 *)(d + 0x1C) != 0 || D_0015FF4C_2243E8 != 0) {
            D_0015EFB4_2243E8 |= 0x100;
            *(s32 *)(d + 0xFC) = 0;
            func_001FBC80_2243E8(3, *(void **)(g + 4), 0);
            return 0;
        }
        {
            char *mix = D_0013E650_2243E8;
            s32 v = D_0015EEF0_2243E8;
            s32 v8 = (v * 8) / 10;
            s32 v7 = (v * 7) / 10;

            *(s32 *)(d + 0xFC) = 1;
            *(s32 *)(mix + 0x4C) = D_0015EEEC_2243E8;
            *(s32 *)(mix + 0x48) = v8;
            *(s32 *)(mix + 0x50) = v8;
            *(s32 *)(mix + 0x54) = v7;
            *(s32 *)(mix + 0x58) = v7;
            *(s32 *)(mix + 0x5C) = v;
        }
        func_0012E380_2243E8(D_0015EEE8_2243E8 == 0);
        func_0022F4A0_2243E8(D_0015EE84_2243E8);
        D_0013E15A_2243E8 = 0;
        return 0;
    }

    if ((*(s32 *)(pad + 0x1C4) & 0xD00) && *(s32 *)(g + 0x124) == 0) {
        return 1;
    }
    if (*(s32 *)(pad + 0x1C4) & 0x10) {
        s32 back = *(s32 *)(*(char **)(g + 4) + 0x38);
        if (back != 0) {
            *(s32 *)(g + 8) = back;
            return 0;
        }
        if (*(s32 *)(g + 0x124) == 0) {
            return -1;
        }
    }
    if (D_0015EFB0_2243E8 != 0x10 && D_0015EFB0_2243E8 != 1) {
        *(s32 *)(g + 8) = *(s32 *)(*(char **)(g + 4) + 0x38);
        return 0;
    }
    if (*(s32 *)(d + 0xDC) >= 3 || *(s32 *)(d + 0xE4) >= 0 || *(s32 *)(g + 0x154) < 11
        || *(s32 *)(d + 8) != 2) {
        return 0;
    }

    p = (*(s32 *)(arg0 + 0x30) & 1) ? *(s32 *)(pad + 0x1B4) : *(s32 *)(pad + 0x1C4);
    x = D_0015EF34_2243E8;
    *(s32 *)(arg0 + 0x40) = x;
    if ((p & 0x1000) && x != 0) {
        *(s32 *)(arg0 + 0x40) = x - 1;
    }
    x = *(s32 *)(arg0 + 0x40);
    if ((p & 0x4000) && x < 4) {
        *(s32 *)(arg0 + 0x40) = x + 1;
        x = *(s32 *)(arg0 + 0x40);
    }
    D_0015EF34_2243E8 = x;
    if ((p & 0x40) && *(s32 *)(d + 8) == 2 && *(s32 *)(d + 0x20 + x * 0x1C) >= 0) {
        func_0022ED80_2243E8(0, 0x11, *(s32 *)(arg0 + 0x14));
        *(s32 *)(d + 0xC8) = 0;
        *(s32 *)(d + 0x14) = *(s32 *)(arg0 + 0x40);
        if (*(s32 *)(d + 0xE4) < 0) {
            *(s32 *)(d + 0xE8) = 0;
            *(s32 *)(d + 0xE4) = 0xD;
        }
        *(s32 *)(g + 0x128) = 1;
        *(s32 *)(g + 0x12C) = 0x4FB6;
        D_0015FF4C_2243E8 = 0;
    }
    if (*(s32 *)(arg0 + 0x40) != old) {
        func_0022ED80_2243E8(1, 0x11, *(s32 *)(arg0 + 0x14));
    }
    return 0;
}
