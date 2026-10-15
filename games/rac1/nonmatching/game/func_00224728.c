/*
 * The save page's update (PAL; the US twin is the NTSC decomp's saving_data_menu2, which
 * this follows, checked against the PAL assembly): with the page focused, a confirmed
 * overwrite or a new file starts the card write (step 1); while the write is pending it waits
 * for the card (and for 11 ticks of the page), then either shows the error dialog or stores the
 * slot's summary (level, bolts, the two counters, the 8-byte play time) and starts the game.
 * Otherwise the pad moves the slot cursor (D_0015EF34), picks a slot (an occupied one asks
 * to overwrite), or continues without saving (a new game straight away).
 *
 * Offsets: menu system D_001D5F70 (+0x04 current page, +0x08 next page, +0xD0 previous page,
 * +0xD4 overwrite kind, +0x124 close locked, +0x128 card op pending, +0x12C its text,
 * +0x154 ticks on the page); a page's +0x38 back page, +0x40 focus, +0x84 confirmed; the
 * widget's +0x14 moby, +0x30 flags (0x2000 load, 1 raw pad), +0x40 slot, +0x48 save data,
 * +0x4C step; card state D_0013D390 (+0x08 card type, +0x14 save index, +0x1C error, +0xDC
 * state, +0xE4 pending state, +0xEC, +0xFC, 0x1C-byte slot summaries from +0x20); pad
 * D_0013CA40 (+0x1A4 pressed, +0x1B4 raw pressed, +0x1C4 pressed unmasked).
 */
extern char D_001D5F70_24728[] __asm__("D_001D5F70");
extern char D_0013D390_24728[] __asm__("D_0013D390");
extern char D_0013CA40_24728[] __asm__("D_0013CA40");
extern char D_001D51B8_24728[] __asm__("D_001D51B8");
extern char D_001D5318_24728[] __asm__("D_001D5318");
extern int D_001D29C0_24728 __asm__("D_001D29C0");
extern int D_0015EFB4_24728 __asm__("D_0015EFB4");
extern int D_0015EFB0_24728 __asm__("D_0015EFB0");
extern int D_0015FF4C_24728 __asm__("D_0015FF4C");
extern int D_0015F6CC_24728 __asm__("D_0015F6CC");
extern int D_0015EF34_24728 __asm__("D_0015EF34");
extern int D_0015EE98_24728 __asm__("D_0015EE98");
extern int D_0015EE84_24728 __asm__("D_0015EE84");
extern int D_0015EF24_24728 __asm__("D_0015EF24");
extern int D_0015EF20_24728 __asm__("D_0015EF20");
extern char D_0015EF98_24728[] __asm__("D_0015EF98");
extern short D_0013E15A_24728 __asm__("D_0013E15A");
extern void func_00227DB0_24728(int) __asm__("func_00227DB0");
extern void func_00227D20_24728(int, int) __asm__("func_00227D20");
extern void func_001FBC80_24728(int, int, int) __asm__("func_001FBC80");
extern void func_0022F4A0_24728(int) __asm__("func_0022F4A0");
extern void func_00209DC0_24728(void) __asm__("func_00209DC0");
extern int func_0022ED80_24728(int, int, int) __asm__("func_0022ED80");

#define MS(o) (*(int *)(D_001D5F70_24728 + (o)))
#define MC(o) (*(int *)(D_0013D390_24728 + (o)))
#define PAD(o) (*(int *)(D_0013CA40_24728 + (o)))
#define W(o) (*(int *)(w + (o)))

int func_00224728(char *w) {
    int old;
    int prev;
    int pad;
    int cursor;
    char *rec;
    int i;

    if (*(int *)(MS(0x04) + 0x40) != (int)w) {
        return 0;
    }
    old = W(0x40);
    if (W(0x4C) == 0) {
        prev = MS(0xD0);
        if (prev == (int)D_001D51B8_24728 && *(int *)(prev + 0x84) != 0) {
            W(0x4C) = 1;
        }
        if (W(0x4C) == 0 && prev == (int)D_001D5318_24728 && *(int *)(prev + 0x84) != 0) {
            W(0x4C) = 1;
        }
    }
    if (W(0x4C) == 1) {
        if (W(0x30) & 0x2000) {
            func_00227DB0_24728(W(0x40));
        } else {
            func_00227D20_24728(W(0x48), W(0x40));
        }
        MS(0x128) = 1;
        MS(0x12C) = 0x4FB5;
        D_0015FF4C_24728 = 0;
    }
    W(0x4C) = 2;

    if (MS(0x128) != 0) {
        if (MC(0xDC) >= 3 || MC(0xE4) >= 0 || MS(0x154) < 0xB) {
            return 0;
        }
        MS(0x128) = 0;
        if (MC(0xEC) != 0 || MC(0x1C) != 0 || D_0015FF4C_24728 != 0) {
            /* The write failed: the error dialog (8 when the card was pulled, else 3). */
            if (D_0015F6CC_24728 != 0) {
                D_0015EFB4_24728 |= 0x80;
                func_001FBC80_24728(8, MS(0x04), MC(0xFC));
                MC(0xFC) = 0;
            } else {
                MC(0xFC) = 0;
                D_0015EFB4_24728 |= 0x80;
                func_001FBC80_24728(3, MS(0x04), 0);
            }
            return 0;
        }
        MC(0xFC) = 1;
        rec = D_0013D390_24728 + MC(0x14) * 0x1C;
        *(int *)(rec + 0x24) = D_0015EE98_24728;
        *(int *)(rec + 0x20) = D_0015EE84_24728;
        *(int *)(rec + 0x2C) = D_0015EF24_24728;
        for (i = 0; i < 8; i++) {
            rec[0x30 + i] = D_0015EF98_24728[i];
        }
        *(int *)(rec + 0x28) = D_0015EF20_24728;
        func_0022F4A0_24728(0);
        D_0013E15A_24728 = 1;
        return 0;
    }

    if ((PAD(0x1C4) & 0xD00) && MS(0x124) == 0) {
        return 1;
    }
    if (PAD(0x1C4) & 0x10) {
        int back = *(int *)(MS(0x04) + 0x38);
        if (back != 0) {
            MS(0x08) = back;
            return 0;
        }
        if (MS(0x124) == 0) {
            return -1;
        }
    }
    if (D_0015EFB0_24728 != 0x10 && D_0015EFB0_24728 != 1) {
        MS(0x08) = *(int *)(MS(0x04) + 0x38);
        return 0;
    }
    if (MC(0xDC) >= 3 || MC(0xE4) >= 0 || MS(0x154) < 0xB || MC(0x08) != 2) {
        return 0;
    }
    pad = (W(0x30) & 1) ? PAD(0x1B4) : PAD(0x1A4);
    W(0x40) = D_0015EF34_24728;
    if ((pad & 0x1000) && D_0015EF34_24728 != 0) {
        W(0x40) = D_0015EF34_24728 - 1;
    }
    if ((pad & 0x4000) && W(0x40) < 4) {
        W(0x40) = W(0x40) + 1;
    }
    cursor = W(0x40);
    D_0015EF34_24728 = cursor;
    if ((pad & 0x40) && MC(0x08) == 2) {
        if (*(int *)(D_0013D390_24728 + cursor * 0x1C + 0x20) != -1) {
            /* An occupied slot: ask to overwrite (or to load). */
            MS(0x08) = (W(0x30) & 0x2000) ? (int)D_001D5318_24728 : (int)D_001D51B8_24728;
            MS(0xD4) = (W(0x30) & 0x2000) ? 2 : 1;
            D_001D29C0_24728 = W(0x40);
        } else {
            W(0x4C) = 1;
        }
    } else if (pad & 0x20) {
        if (W(0x30) & 0x2000) {
            if (MC(0xFC) != 0) {
                func_001FBC80_24728(7, 0, 0);
                goto moved;
            }
            MC(0xFC) = 0;
            func_00227DB0_24728(-1);
        } else {
            /* Continue without saving: a new game. */
            MC(0xFC) = 0;
            D_0015EFB4_24728 = D_0015EFB4_24728 & ~2 & ~4;
            func_00209DC0_24728();
        }
        func_0022F4A0_24728(0);
        D_0013E15A_24728 = 1;
        return 0;
    }
moved:
    if (W(0x40) != old) {
        func_0022ED80_24728(1, 0x11, W(0x14));
    }
    return 0;
}
