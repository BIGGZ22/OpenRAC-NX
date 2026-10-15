/* functional: medium. Pause/menu page cursor update: m is a menu page (+0x3C cursor, +0x40 rows, +0x44 columns, +0x4C/+0x50/+0x54/+0x58 up/down/left/right neighbour pages, +0x48 item table of 10-byte items); D_0013CA40+0x1C4 holds the pad bits (0x1000 up, 0x4000 down, 0x8000 left, 0x2000 right, 0x40 select). Moves the cursor (wrapping or hopping to a neighbour page, skipping pages the flags at +0x30 mark hidden), plays sound 0x11 when it changed, and on select toggles the entry in the slot table at D_001D5F70+0x30. Returns 0, 1 (cancel/back with no pending state) or -1. Not run against real calls. */
extern u8 D_001D5F70_C840[] __asm__("D_001D5F70");
extern u8 D_0013CA40_C840[] __asm__("D_0013CA40");
extern u8 D_0013D5C8_C840[] __asm__("D_0013D5C8");
extern u8 D_001864D0_C840[] __asm__("D_001864D0");
extern u8 D_0013F450_C840[] __asm__("D_0013F450");
extern void func_0022ED80_C840(s32, s32, s32) __asm__("func_0022ED80");
extern s32 func_001E97E8_C840(s32) __asm__("func_001E97E8");

s32 func_0021C840(u8 *m) {
    u8 *S = D_001D5F70_C840;
    u8 *P = D_0013CA40_C840;
    u32 btn;
    s32 A, cnt, B, row, col, col6;
    u8 *p;
    u8 *item;
    s32 k, t, i, skip, v, x;
    u8 *slot;
    s32 hide;

    if (*(s32 *)(*(u8 **)(S + 4) + 0x40) != (s32)(u32)(u64)m)
        return 0;

    btn = *(u32 *)(P + 0x1C4);
    if (btn & 0xD00) {
        if (*(s32 *)(S + 0x124) == 0)
            return 1;
    }
    btn = *(u32 *)(P + 0x1C4);
    if (btn & 0x10) {
        v = *(s32 *)(*(u8 **)(S + 4) + 0x38);
        if (v == 0) {
            if (*(s32 *)(S + 0x124) == 0)
                return -1;
        } else {
            *(s32 *)(S + 8) = v;
            return 0;
        }
    }

    cnt = *(s32 *)(m + 0x44);
    A = *(s32 *)(m + 0x3C);
    B = *(s32 *)(m + 0x40);
    row = A / cnt;
    col = A % cnt;
    col6 = col;
    btn = *(u32 *)(P + 0x1C4);

    if (btn & 0x1000) {
        if (row == 0) {
            p = *(u8 **)(m + 0x4C);
            if (p != 0) {
                do {
                    skip = 0;
                    if (*(s32 *)(S + 0x134) != 0)
                        skip = (*(s32 *)(p + 0x30) & 8) ? 1 : 0;
                    if (*(s32 *)(S + 0x138) != 0 && (*(s32 *)(p + 0x30) & 4))
                        skip = 1;
                    if (skip)
                        p = *(u8 **)(p + 0x4C);
                } while (skip);
                k = *(s32 *)(p + 0x44);
                t = *(s32 *)(p + 0x40) - 1;
                *(u8 **)(*(u8 **)(S + 4) + 0x80) = p;
                if (k == 5) {
                    if (*(s32 *)(m + 0x44) == 3)
                        col6 = col + 1;
                    else
                        col6 = col;
                }
                if (k == 3) {
                    if (*(s32 *)(m + 0x44) == 5) {
                        col6 = col6 - 1;
                        if (col6 < 3) {
                            if (!(-1 < col6))
                                col6 = 0;
                            t = *(s32 *)(p + 0x40) - 1;
                        } else {
                            col6 = 2;
                        }
                    }
                }
                x = k - 1;
                if (col6 < x)
                    x = col6;
                *(s32 *)(p + 0x3C) = t * k + x;
            } else if (!(*(s32 *)(m + 0x30) & 0x8000)) {
                *(s32 *)(m + 0x3C) = cnt * (B - 1) + A;
            }
        } else {
            *(s32 *)(m + 0x3C) = A - cnt;
        }
    }

    if (btn & 0x4000) {
        if (row + 1 < B) {
            *(s32 *)(m + 0x3C) = *(s32 *)(m + 0x3C) + cnt;
        } else {
            p = *(u8 **)(m + 0x50);
            if (p != 0) {
                do {
                    skip = 0;
                    if (*(s32 *)(S + 0x134) != 0)
                        skip = (*(s32 *)(p + 0x30) & 8) ? 1 : 0;
                    if (*(s32 *)(S + 0x138) != 0 && (*(s32 *)(p + 0x30) & 4))
                        skip = 1;
                    if (skip)
                        p = *(u8 **)(p + 0x50);
                } while (skip);
                k = *(s32 *)(p + 0x44);
                *(u8 **)(*(u8 **)(S + 4) + 0x80) = p;
                if (k == 5) {
                    if (*(s32 *)(m + 0x44) == 3)
                        col6 = col6 + 1;
                }
                if (k == 3) {
                    if (*(s32 *)(m + 0x44) == 5) {
                        col6 = col6 - 1;
                        if (col6 < 3) {
                            if (!(-1 < col6))
                                col6 = 0;
                        } else {
                            col6 = 2;
                        }
                    }
                }
                x = k - 1;
                if (col6 < x)
                    x = col6;
                *(s32 *)(p + 0x3C) = x;
            } else if (!(*(s32 *)(m + 0x30) & 0x8000)) {
                *(s32 *)(m + 0x3C) = *(s32 *)(m + 0x3C) - *(s32 *)(m + 0x44) * (*(s32 *)(m + 0x40) - 1);
            }
        }
    }

    if (btn & 0x8000) {
        if (col6 == 0) {
            p = *(u8 **)(m + 0x54);
            if (p != 0)
                *(u8 **)(*(u8 **)(S + 4) + 0x80) = p;
            else if (!(*(s32 *)(m + 0x30) & 0x8000))
                *(s32 *)(m + 0x3C) = *(s32 *)(m + 0x3C) - 1 + *(s32 *)(m + 0x44);
        } else {
            *(s32 *)(m + 0x3C) = *(s32 *)(m + 0x3C) - 1;
        }
    }

    if (btn & 0x2000) {
        if (col6 + 1 < cnt) {
            *(s32 *)(m + 0x3C) = *(s32 *)(m + 0x3C) + 1;
        } else {
            p = *(u8 **)(m + 0x58);
            if (p != 0)
                *(u8 **)(*(u8 **)(S + 4) + 0x80) = p;
            else if (!(*(s32 *)(m + 0x30) & 0x8000))
                *(s32 *)(m + 0x3C) = *(s32 *)(m + 0x3C) + 1 - *(s32 *)(m + 0x44);
        }
    }

    if (*(s32 *)(m + 0x3C) != A || *(u8 **)(*(u8 **)(S + 4) + 0x80) != 0)
        func_0022ED80_C840(1, 0x11, *(s32 *)(m + 0x14));

    if (*(u32 *)(P + 0x1C4) & 0x40) {
        if (*(s32 *)(m + 0x30) & 1)
            return 0;
        if (*(s32 *)(S + 0x148) != 0)
            return 0;
        i = *(s32 *)(m + 0x3C);
        item = *(u8 **)(m + 0x48) + i * 10;
        k = *(s16 *)(item + 6);
        if (k == 0 || D_0013D5C8_C840[k] == 0) {
            func_0022ED80_C840(2, 0x11, *(s32 *)(m + 0x14));
            return 0;
        }
        t = *(s32 *)(D_001864D0_C840 + k * 0x4C + 8);
        func_0022ED80_C840(0, 0x11, *(s32 *)(m + 0x14));
        *(s32 *)(S + 0x148) = 10;
        slot = S + 0x30 + t * 4;
        if (*(s32 *)slot == k) {
            if (t != 0 && t != 3) {
                *(s32 *)slot = 0;
                return 0;
            }
        }
        if (k != 0x18) {
            *(s32 *)(S + 0x30 + t * 4) = k;
            return 0;
        }
        if (func_001E97E8_C840(0x18) == 0)
            return 0;
        if (*(s32 *)S != 3)
            return 0;
        v = func_001E97E8_C840(0x18);
        D_0013F450_C840[0x1FF5] = 1;
        v = D_0013F450_C840[0x1FF7] + v;
        if (!(v < 7))
            v = 6;
        D_0013F450_C840[0x1FF7] = (u8)v;
        return 0;
    }
    return 0;
}
