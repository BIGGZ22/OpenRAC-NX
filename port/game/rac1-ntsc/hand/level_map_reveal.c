/* Carried from port/game/rac1-pal/hand/level_map_reveal.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only map reveal, reviewed against PAL 00248EF8..00249604 and
 * nonmatching/shared/FUN_L00_002484e0.c in the sibling decompilation.
 * Explicit pointer fields retain the EE layout through hostgen. No match claim.
 */
typedef int (*MapRevealPredicate)(int, int, float, float, float);
typedef int (*MapRevealHeightPredicate)(int, float, float, float);
typedef struct MapRevealZone {
    float low, high;
    unsigned int flags;
    int switch_index;
} MapRevealZone;
typedef struct MapRevealMoby {
    unsigned char pad00[0x18];
    float height;
} MapRevealMoby;
typedef struct MapRevealHero {
    unsigned char pad00[0x308];
    short active;
    unsigned char pad30A[0xFDA], water, pad12E5[0xD9B];
    MapRevealMoby* moby;
    unsigned char pad2084[8];
    int mode;
} MapRevealHero;
typedef struct MapRevealState {
    unsigned char pad00[0xC];
    unsigned char* fog;
    unsigned char pad10[0x28];
    unsigned char* tile;
    unsigned char pad3C[0x168], brush[128];
} MapRevealState;
extern int mr_ready __asm__("D_L00_00184218");
/* Retail loads a word here, even though another unit names it as a short. */
extern int mr_alternate __asm__("D_L00_0015FDF8");
extern int mr_level __asm__("D_0015ED84");
extern unsigned char mr_hero[] __asm__("D_0013E533");
extern MapRevealZone mr_zones[][16] __asm__("D_L00_00182BA0");
extern int mr_switches[] __asm__("D_L00_001844A8");
extern MapRevealPredicate mr_predicates[][8] __asm__("D_L00_00183F90");
extern MapRevealState mr_map __asm__("D_L00_001841F0");
extern void mr_project(float, float, float*, float*, int) __asm__("func_L00_0024A7E8");
extern int mr_load_tile(int) __asm__("func_L00_00249D80");
extern int mr_height_a(int, float, float, float) __asm__("func_L00_00249780");
extern int mr_height_b(int, float, float, float) __asm__("func_L00_002497F8");
extern int mr_height_c(int, float, float, float) __asm__("func_L00_00249A78");
extern int mr_height_d(int, float, float, float) __asm__("func_L00_00249BA8");

void FUN_L00_002484e0(float* position) {
    float ox, oy;
    int gx, gy, level, water, mode, zone, predicate, allowed[16];
    int x0, x1, y0, y1, x, y;
    MapRevealHero* hero = (MapRevealHero*)(mr_hero + 0xE1D);
    if (!mr_ready) return;
    mr_project(position[0], position[1], &ox, &oy,
               mr_level + (mr_alternate ? 100 : 0));
    ox *= 512.0f;
    oy *= 512.0f;
    /* Only these truncation results can produce a coordinate in [0,512).
     * Reject before casting, also avoiding undefined casts of NaN/infinity. */
    if (!(ox > -2.0f && ox < 511.0f && oy > -2.0f && oy < 511.0f)) return;
    gx = (int)ox + 1;
    gy = (int)oy + 1;
    level = mr_level;
    if (level < 0 || level >= 19) level = 0;
    mode = hero->mode;
    water = (unsigned int)mode - 0x11u < 2 || hero->water == 1;
    allowed[0] = 1;
    for (zone = 1; zone < 16; ++zone) {
        MapRevealZone* z = &mr_zones[level][zone];
        unsigned int flags;
        allowed[zone] = 0;
        if (z->low != z->high &&
            (hero->moby->height < z->low || z->high < hero->moby->height)) continue;
        flags = z->flags;
        if ((flags & 1) && !water) continue;
        if ((flags & 2) && water) continue;
        if ((flags & 4) && mode != 0x10) continue;
        if ((flags & 8) && mode == 0x10) continue;
        if ((flags & 0x10) && mode != 0xF) continue;
        if ((flags & 0x20) && mode == 0xF) continue;
        if ((flags & 0x40) && !hero->active) continue;
        if ((flags & 0x80) && !mr_switches[z->switch_index]) continue;
        for (predicate = 0; predicate < 8; ++predicate) {
            if (flags & (0x100u << predicate)) {
                /* The callback table uses the current level, not the clamped
                 * descriptor level. Reload it for each callback as retail does. */
                MapRevealPredicate fn = mr_predicates[mr_level][predicate];
                if (fn) {
                    int result;
                    /* EE integers and floats use separate register banks.
                     * Windows arguments use their positional slots instead.
                     * These four existing C definitions omit the unused gy;
                     * all other implemented entries take gx,gy or no floats.
                     * Keep each indirect call compatible with its definition. */
                    if (fn == (MapRevealPredicate)mr_height_a ||
                        fn == (MapRevealPredicate)mr_height_b ||
                        fn == (MapRevealPredicate)mr_height_c ||
                        fn == (MapRevealPredicate)mr_height_d)
                        result = ((MapRevealHeightPredicate)fn)(gx, position[0], position[1], position[2]);
                    else
                        result = fn(gx, gy, position[0], position[1], position[2]);
                    if (!result) break;
                }
            }
        }
        if (predicate == 8) allowed[zone] = 1;
    }
    x0 = gx - 16; if (x0 < 0) x0 = 0;
    y0 = gy - 16; if (y0 < 0) y0 = 0;
    x1 = gx + 16; if (x1 >= 512) x1 = 511;
    y1 = gy + 16; if (y1 >= 512) y1 = 511;
    /* Retail clamps to 511 and uses exclusive upper bounds. The last row
     * and column therefore remain untouched, including at the map edges. */
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            unsigned char* fog = mr_map.fog + y * 64 + x / 8;
            int bit = 1 << (x & 7), bx = x - gx + 16, by = y - gy + 16;
            int cell, packed, id;
            if (!(*fog & bit)) continue;
            /* Clipping guarantees both brush coordinates are in [0,32). */
            if (mr_map.brush[by * 4 + bx / 8] & (1 << (bx & 7))) continue;
            cell = (x >> 5) + ((y >> 5) << 4);
            mr_load_tile(cell);
            packed = mr_map.tile[(y & 31) * 16 + ((x & 31) >> 1)];
            id = (x & 1) ? packed >> 4 : packed & 15;
            if (allowed[id]) *fog &= (unsigned char)~bit;
        }
    }
}
