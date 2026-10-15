/* Map reveal from the hero position POS (every frame, when the level has a map, block
   D_L00_001842F0 + 0x28): the hero's map pixel (cx, cy) = (int)(u * 512) + 1 from
   func_L00_0024B200 (level + 100 for the alternative map of D_L00_0015FEB8); within the map,
   which of the level's 16 zones are open now (zone table D_L00_00182CA0 + level * 0x100 +
   zone * 0x10: z range, rule flags, flag-0x80 argument into D_L00_001845A8; flags 0x100 << k
   call the level's predicate k from D_L00_00184090 + level * 0x20); then every fogged pixel
   of the reveal mask (block +0xC, 64 bytes a row) in the square cx - 16 .. cx + 16 (clamped to
   0 .. 0x1FF) that is inside the brush (block +0x1A4, 4 bytes a row, a set bit skips) and in an
   open zone (zone nibbles of the tile cache, func_L00_0024A798 then block +0x38) is cleared.
   Adapted from ReRAC (crates/rc-game/src/map.rs MapState::reveal, MapState::zones_open; ISC
   License, Copyright (c) 2026 ReRAC contributors). */
typedef int (*MapRevealPred)(int, int, float, float, float);
extern char D_L00_001842F0_rv[] __asm__("D_L00_001842F0");
extern char D_L00_00182CA0_rv[] __asm__("D_L00_00182CA0");
extern int D_L00_001845A8_rv[] __asm__("D_L00_001845A8");
extern MapRevealPred D_L00_00184090_rv[] __asm__("D_L00_00184090");
extern int D_L00_0015FEB8_rv[] __asm__("D_L00_0015FEB8");
extern int D_0015EE84_rv[] __asm__("D_0015EE84");
extern char D_0013E633_rv[] __asm__("D_0013E633");
extern void func_L00_0024B200_rv(float *, float *, int, float, float) __asm__("func_L00_0024B200");
extern int func_L00_0024A798_rv(int) __asm__("func_L00_0024A798");

void func_L00_00248EF8(float *pos) {
    char *blk = D_L00_001842F0_rv;
    char *h = D_0013E633_rv + 0xE1D;
    int open[16];
    float u;
    float v;
    int cx;
    int cy;
    int lvl;
    int riding;
    int group;
    int isF;
    int is10;
    int z;
    unsigned int flags;
    int arg;
    float zmin;
    float zmax;
    float hz;
    char *zone;
    MapRevealPred fn;
    int x0;
    int x1;
    int y0;
    int y1;
    int x;
    int y;
    int bx;
    int by;
    unsigned char *mp;
    int bit;
    int nib;

    if (*(int *)(blk + 0x28) == 0) {
        return;
    }
    if (D_L00_0015FEB8_rv[0] != 0) {
        func_L00_0024B200_rv(&u, &v, D_0015EE84_rv[0] + 0x64, pos[0], pos[1]);
    } else {
        func_L00_0024B200_rv(&u, &v, D_0015EE84_rv[0], pos[0], pos[1]);
    }
    cx = (int)(u * 512.0f) + 1;
    cy = (int)(v * 512.0f) + 1;
    if (cx < 0 || cy < 0 || cx >= 0x200 || cy >= 0x200) {
        return;
    }

    lvl = D_0015EE84_rv[0];
    if (lvl < 0) {
        lvl = 0;
    }
    if (lvl >= 0x13) {
        lvl = 0;
    }
    riding = 0;
    if ((unsigned int)(*(int *)(h + 0x208C) - 0x11) < 2 || *(unsigned char *)(h + 0x12E4) == 1) {
        riding = 1;
    }
    group = *(int *)(h + 0x208C);
    isF = group == 0xF;
    is10 = group == 0x10;

    for (z = 0; z < 16; z++) {
        open[z] = 1;
        if (z <= 0) {
            continue;
        }
        zone = D_L00_00182CA0_rv + lvl * 0x100 + z * 0x10;
        zmax = *(float *)(zone + 4);
        zmin = *(float *)(zone + 0);
        if (zmin != zmax) {
            hz = *(float *)(*(char **)(h + 0x2080) + 0x18);
            if (hz < zmin || zmax < hz) {
                open[z] = 0;
                continue;
            }
        }
        flags = *(unsigned int *)(zone + 8);
        arg = *(int *)(zone + 0xC);
        if (flags == 0) {
            continue;
        }
        if (((flags & 1) && riding == 0) || ((flags & 2) && riding != 0) ||
            ((flags & 4) && is10 == 0) || ((flags & 8) && is10 != 0) ||
            ((flags & 0x10) && isF == 0) || ((flags & 0x20) && isF != 0) ||
            ((flags & 0x40) && *(short *)(h + 0x308) == 0) ||
            ((flags & 0x80) && D_L00_001845A8_rv[arg] == 0)) {
            open[z] = 0;
            continue;
        }
        if (flags & 0x100) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 0];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x200) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 1];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x400) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 2];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x800) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 3];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x1000) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 4];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x2000) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 5];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x4000) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 6];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
        if (flags & 0x8000) {
            fn = D_L00_00184090_rv[D_0015EE84_rv[0] * 8 + 7];
            if (fn != 0 && fn(cx, cy, pos[0], pos[1], pos[2]) == 0) {
                open[z] = 0;
                continue;
            }
        }
    }

    y0 = cy - 0x10;
    y1 = cy + 0x10;
    x0 = cx - 0x10;
    x1 = cx + 0x10;
    if (y0 < 0) {
        y0 = 0;
    }
    if (y1 >= 0x200) {
        y1 = 0x1FF;
    }
    if (x0 < 0) {
        x0 = 0;
    }
    if (x1 >= 0x200) {
        x1 = 0x1FF;
    }
    for (y = y0; y < y1; y++) {
        by = y - cy + 0x10;
        for (x = x0; x < x1; x++) {
            mp = (unsigned char *)(*(char **)(blk + 0xC) + y * 64 + x / 8);
            if (*mp == 0) {
                continue;
            }
            bit = 1 << (x & 7);
            if ((*mp & bit) == 0) {
                continue;
            }
            bx = x - cx + 0x10;
            if ((*(unsigned char *)(blk + 0x1A4 + by * 4 + bx / 8) >> (bx % 8)) & 1) {
                continue;
            }
            func_L00_0024A798_rv((y >> 5) * 16 + (x >> 5));
            nib = *(unsigned char *)(*(char **)(blk + 0x38) + ((x & 0x1F) >> 1) + (y & 0x1F) * 16);
            if (x & 1) {
                nib = nib >> 4;
            } else {
                nib = nib & 0xF;
            }
            if (open[nib] != 0) {
                *mp = *mp & ~bit;
            }
        }
    }
}
