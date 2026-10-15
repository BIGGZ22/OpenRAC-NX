/*
 * Pause menu draw, the alternative to DrawMobys: for each menu moby, takes its rectangle from
 * the moby's +0x78 block and the page's screen; then draws every screen rectangle in two passes
 * (pass 0 and pass 1), restoring the moby class dists after pass 0; then clears the moby draw
 * flags and runs the GIF paging.
 */
extern void func_00234C98_21A610(s32, long) __asm__("func_00234C98");
extern void func_0020E0C8_21A610(void) __asm__("func_0020E0C8");
extern void func_0020E040_21A610(void) __asm__("func_0020E040");
extern void func_0020E068_21A610(void) __asm__("func_0020E068");
extern void func_0020E098_21A610(void) __asm__("func_0020E098");
extern void func_0020E200_21A610(void) __asm__("func_0020E200");
extern void func_0020E180_21A610(void *, s32) __asm__("func_0020E180");
extern void func_00219C08_21A610(void) __asm__("func_00219C08");
extern void func_001F2608_21A610(void) __asm__("func_001F2608");
extern void func_001F4630_21A610(s32) __asm__("func_001F4630");
extern void func_001F4748_21A610(void) __asm__("func_001F4748");
extern void func_002250B8_21A610(void *) __asm__("func_002250B8");
extern void func_00238D90_21A610(void *, void *, s32 *, s32 *, s32 *, s32 *)
    __asm__("func_00238D90");
extern void func_00201640_21A610(s32, s32, s32, s32, long, long) __asm__("func_00201640");
extern void func_001F7A50_21A610(s32, s32, s32, float) __asm__("func_001F7A50");
extern void func_001F7B40_21A610(void) __asm__("func_001F7B40");
extern void func_002017C8_21A610(s32, s32, s32, s32, long, s32, s32) __asm__("func_002017C8");
extern void func_001F5800_21A610(s32, s32, s32, s32, s32, s32, s32, s32, long, long)
    __asm__("func_001F5800");
extern s32 D_00160018_21A610 __asm__("D_00160018") MACRO_ADDR;
extern long D_0015EFD0_21A610 __asm__("D_0015EFD0") MACRO_ADDR;
extern s32 D_001602B0_21A610 __asm__("D_001602B0") MACRO_ADDR;
extern s32 D_001CE640_21A610[] __asm__("D_001CE640");
extern void *D_001D6120_21A610[] __asm__("D_001D6120");

typedef struct {
    s32 q[4];
} Quad_21A610;

void func_0021A610(void) {
    char *g = D_001D5F70;
    char *cur;
    char **base;
    char *o;
    char *p;
    char *ent;
    char *ent7;
    s32 i, j, pass;
    s32 flags, r, k1, k2, s1, s2, x, y, w, h, ww, hh, dx, dy;
    s32 wv, hv, xv, yv;

    func_00234C98_21A610(0x47, 0x5360B);
    func_0020E0C8_21A610();
    func_0020E040_21A610();
    func_0020E068_21A610();
    func_0020E180_21A610((void *)D_00160018_21A610, 4);
    func_00219C08_21A610();
    func_001F2608_21A610();

    for (i = 0; i < 14; i++) {
        if (D_001CE640_21A610[i] == 0) {
            continue;
        }
        if (D_001D6120_21A610[i] == 0) {
            continue;
        }
        if (i == 6 && *(s32 *)(g + 0xD8) == 0) {
            continue;
        }
        func_0020E180_21A610(D_001D6120_21A610[i], 1);
    }

    cur = *(char **)(g + 4);
    base = (cur != 0) ? (char **)(cur + 0x44) : 0;

    for (j = 0; j < 14; j++) {
        o = (char *)D_001D6120_21A610[j];
        if (o == 0) {
            continue;
        }
        if (D_001CE640_21A610[j] == 0) {
            continue;
        }
        if (j == 6 && *(s32 *)(g + 0xD8) == 0) {
            continue;
        }
        ent = *(char **)(o + 0x78);
        {
            Quad_21A610 qa = *(Quad_21A610 *)ent;
            Quad_21A610 qd = *(Quad_21A610 *)(ent + 0x30);

            func_00238D90_21A610(&qa, &qd, &wv, &hv, &xv, &yv);
        }
        xv = xv + 1;
        yv = yv + 1;
        if (base != 0 && base[j] != 0) {
            p = base[j];
            *(s32 *)(p + 0x20) = wv;
            *(s32 *)(p + 0x24) = hv;
            *(s32 *)(p + 0x18) = xv;
            *(s32 *)(p + 0x1C) = yv;
        }
        *(s32 *)(ent + 0x50) = xv;
        *(s32 *)(ent + 0x54) = yv;
        *(s32 *)(ent + 0x58) = wv;
        *(s32 *)(ent + 0x5C) = hv;
        func_00201640_21A610(xv + 1, yv + 1, xv + wv - 1, yv + hv - 1, D_001602B0_21A610, 0);
    }

    pass = 0;
    do {
        for (i = 0; i < 14; i++) {
            o = (char *)D_001D6120_21A610[i];
            if (o == 0) {
                continue;
            }
            if (base == 0) {
                continue;
            }
            p = base[i];
            if (p == 0) {
                continue;
            }
            flags = *(s32 *)(p + 0x10);
            if (flags & 4) {
                continue;
            }
            if (D_001CE640_21A610[i] == 0) {
                continue;
            }
            if (*(void **)(p + 4) == 0) {
                continue;
            }
            if (i == 6 && *(s32 *)(g + 0xD8) == 0) {
                continue;
            }
            if (pass == 0 && (flags & 2) == 0) {
                continue;
            }
            if (pass == 1 && (flags & 2) != 0) {
                continue;
            }
            if (flags & 1) {
                ((void (*)(char *, s32))*(void **)(p + 4))(p, flags);
                continue;
            }

            ent7 = *(char **)(o + 0x78);
            x = *(s32 *)(ent7 + 0x50);
            y = *(s32 *)(ent7 + 0x54);
            w = *(s32 *)(ent7 + 0x58);
            h = *(s32 *)(ent7 + 0x5C);

            k1 = 7;
            if (0x80 < w) {
                k1 = 8;
                while ((1 << k1) < w) {
                    k1++;
                }
            }
            k2 = 7;
            if (0x80 < h) {
                k2 = 8;
                while ((1 << k2) < h) {
                    k2++;
                }
            }
            while (k1 + k2 >= 18) {
                k2--;
            }
            func_001F7A50_21A610(k1, k2, pass != 0, 1.0f);

            s1 = 1 << k1;
            s2 = 1 << k2;
            func_002017C8_21A610(0, 0, s1, s2, D_001602B0_21A610, 0, 0);

            p = base[i];
            r = ((s32 (*)(void *))*(void **)(p + 4))(p);
            func_001F7B40_21A610();
            if (r & 1) {
                continue;
            }

            ww = s1;
            hh = s2;
            dx = 0;
            dy = 0;
            if (r & 2) {
                ww = w;
                hh = h;
            } else if (r & 8) {
                dx = (s1 - w) / 2;
                dy = (s2 - h) / 2;
                ww = s1 - dx;
                hh = s2 - dy;
                if (s1 < ww) {
                    ww = s1;
                }
                if (s2 < hh) {
                    hh = s2;
                }
                if (dx < 0) {
                    dx = 0;
                }
                if (dy < 0) {
                    dy = 0;
                }
            } else if (r & 4) {
                if (w < h) {
                    dx = s1 / 2 - (w * s1) / (h * 2);
                    ww = s1 - dx;
                } else {
                    dy = s2 / 2 - (h * s2) / (w * 2);
                    hh = s2 - dy;
                }
            } else if ((r & 16) == 0) {
                continue;
            }

            func_00234C98_21A610(0x42, ((long)0x8000 << 24) | 0x64);
            func_00234C98_21A610(0x47, 0x43);
            func_001F5800_21A610(x, y, w, h, dx, dy, ww - dx, hh - dy, 0x80808080L,
                                 D_0015EFD0_21A610);
        }
        if (pass == 0) {
            func_0020E098_21A610();
            func_0020E200_21A610();
        }
        pass = pass + 1;
    } while (pass < 2);

    func_001F4630_21A610(0);
    for (i = 0; i < 14; i++) {
        if (D_001CE640_21A610[i] == 0) {
            continue;
        }
        if (i == 6 && *(s32 *)(g + 0xD8) == 0) {
            continue;
        }
        func_002250B8_21A610(D_001D6120_21A610[i]);
    }
    func_001F4748_21A610();
}
