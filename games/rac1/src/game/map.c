#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * map.cpp in the original source; text 0x205520-0x2071A8.
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

extern int func_001F9968(int);
extern int D_0015EF74 MACRO_ADDR;
extern int D_0015F558 MACRO_ADDR;
typedef struct {
    int addr;            /* +0 */
    short unk4;          /* +4 */
    short cbp;           /* +6 */
    int unk8;            /* +8 */
    unsigned char tw;    /* +C */
    unsigned char th;    /* +D */
    short tbp;           /* +E */
} TexSlot;
extern TexSlot D_0018D140[];
typedef struct {
    char *clut;          /* 0x00 */
    char *pix;           /* 0x04 */
    char pad08[0xC];
    int clutSize;        /* 0x14 */
    char pad18[0x34];
    int tw;              /* 0x4C */
    int th;              /* 0x50 */
    char pad54[0xC];
} TexDesc;

/* Loads a 256-colour texture file p (width at +8, height at +0xC, CLUT
   at +0x20, pixels after it): the CLUT and the pixels take the next VRAM
   space at D_0015EF74 (cbp, then tbp 0x400 later, then the allocator
   moves on by 1 << (tw + th)), and the texture is registered and its GS
   TEX0 value returned as in func_00205660. The allocator update comes
   before the TEX0 value and the buffer width is shifted back into w:
   both decide which of tw and the constant 1 local-alloc serves first. */
long LoadMapTexture(char *p) {
    TexDesc t;
    int w;
    int cbp;
    int tbp;
    int addr;
    long reg;

    t.clut = p + 0x20;
    t.clutSize = 0x400;
    t.tw = log2dim(*(int *)(p + 8));
    t.th = log2dim(*(int *)(p + 0xC));
    t.pix = p + 0x20 + t.clutSize;
    addr = D_0015EF74;
    cbp = addr >> 8;
    addr += 0x400;
    tbp = addr >> 8;
    D_0015EF74 = addr + (1 << (t.tw + t.th));
    w = t.tw - 6;
    if (w < 0) {
        w = 0;
    }
    w = 1 << w;
    reg = (long)tbp | ((long)w << 14) | ((long)0x13 << 20)
        | ((long)t.tw << 26) | ((long)t.th << 30) | ((long)1 << 34)
        | ((long)cbp << 37) | ((long)4 << 61);
    if (D_0015F558 < 0x40) {
        D_0018D140[D_0015F558].addr = (int)t.clut;
        D_0018D140[D_0015F558].cbp = cbp;
        D_0018D140[D_0015F558].unk4 = 0;
        D_0018D140[D_0015F558].unk8 = (int)t.pix;
        D_0018D140[D_0015F558].tw = t.tw;
        D_0018D140[D_0015F558].th = t.th;
        D_0018D140[D_0015F558].tbp = tbp;
        D_0015F558++;
    }
    return reg;
}

/* Registers an 8-bit texture: returns its GS TEX0 value (tbp = tex >> 8,
   tbw = 1 << max(tw - 6, 0), PSMT8, tw, th, tcc 1, cbp = clut >> 8,
   cld 4) and, while there is room (64 entries), records it in the next
   D_0018D140 slot. The buffer width is shifted back into w itself: the
   shift then writes w's register and the constant 1 keeps its own. */
long RegisterMapTexture(int tw, int th, int a2, int a3, int clut, int tex) {
    int w = tw - 6;
    long reg;
    int tbp = tex >> 8;
    int cbp = clut >> 8;

    if (w < 0) {
        w = 0;
    }
    w = 1 << w;
    reg = (long)tbp | ((long)w << 14) | ((long)0x13 << 20)
        | ((long)tw << 26) | ((long)th << 30) | ((long)1 << 34)
        | ((long)cbp << 37) | ((long)4 << 61);
    if (D_0015F558 < 0x40) {

        D_0018D140[D_0015F558].addr = a2;
        D_0018D140[D_0015F558].cbp = cbp;
        D_0018D140[D_0015F558].unk4 = 0;
        D_0018D140[D_0015F558].unk8 = a3;
        D_0018D140[D_0015F558].tw = tw;
        D_0018D140[D_0015F558].th = th;
        D_0018D140[D_0015F558].tbp = tbp;
        D_0015F558++;
    }
    return reg;
}
__asm__(".section .text\n\tnop\n");

extern int D_001A0468[];

int Map_FindFreeSlot(int arg0) {
    int *a = D_001A0468;
    int *b = D_001A0468 + 5;
    int i = 0;
    do {
        int idx = 4 - i;
        if (arg0 == 0) idx = i;
        if (a[idx] != 0 && b[idx] == -1) {
            return idx;
        }
        i++;
    } while (i < 5);
    return -1;
}

extern void func_00205830(int a, int b);

/* D_001A01F0 typed as the object it is, so that the two 5/6-entry int
   arrays at +0x278 and +0x28C are members rather than constants added
   to an index. See the note on func_00205790 -- this is what makes it
   match. Aliased rather than renamed because the rest of this file
   still reaches the same symbol as a flat int array. */
extern PadSlots D_001A01F0_slots __asm__("D_001A01F0");

/*
 * Allocate a slot. func_00205728(1) gets first refusal; if it returns
 * nonzero that IS the answer. Otherwise scan slots 1..4 for one that is
 * neither flagged 0x1000 nor empty, and hand it to func_00205830, which
 * compacts entry `i` down onto entry 0. Falling out of the loop at i==5
 * still calls func_00205830(0, 5) -- retail shares that call site with
 * the break, so the source must too.
 *
 * Byte-exact, registers included, and it took three things:
 *
 * 1. ONE variable for the callee's result and the loop counter. Retail
 *    keeps both in $16 and pays for a `daddu $16,$2,$0` that separate
 *    locals would not need. Two locals is 4 bytes SHORT.
 *
 * 2. The peeled first test is not a peeled iteration in the source: it
 *    is gcc's while -> do-while rotation, whose entry guard `1 < 5`
 *    folds away, leaving the i==1 addresses as the constants 0x290 and
 *    0x27C. A plain `while` starting at i = 1 produces it for free.
 *
 * 3. TYPE THE BASE AS A STRUCT. Three spellings, against retail's 160:
 *      D_001A01F0[0xA3 + i]              172  adds the constant to the
 *                                             index and THEN shifts,
 *                                             once per array
 *      int *p = D_001A01F0 + i; p[0xA3]  164  right inside the loop,
 *                                             but the constant-folded
 *                                             i==1 peel then needs its
 *                                             own `addu $3,$3,4`
 *                                             instead of folding into
 *                                             the lw displacement
 *      struct member .flags[i]           160  EXACT
 *    Only the struct gives base-first `sll`/`addu` with the array's
 *    byte offset left in the load, in BOTH the loop and the folded
 *    peel. This widens the existing "type the table, don't rewrite the
 *    addition" lever from a stride to a base: where retail shows
 *    `sll idx,2` / `addu base` / `lw CONST(reg)`, that CONST is a
 *    member offset, so declare the member.
 *
 *    It does not contradict the base-pointer rule on func_00205830
 *    below, where three arrays come off one live base in straight-line
 *    code and naming the base wins. The rule covering both is: describe
 *    the memory, not the arithmetic.
 *
 * Watch the 164-byte middle spelling: its extra word was not an extra
 * instruction at all. The body was 40 words either way, but the odd
 * word count ahead of the loop label made gcc's `.p2align 3` emit a
 * real nop -- and internal alignment padding sits INSIDE the .ent/.end
 * pair, so it counts toward the symbol size and toward the bytes. A
 * size miss of exactly 4 with an otherwise correct instruction stream
 * means a misaligned block, not a missing instruction.
 */
int AllocMapSlot(void) {
    int i;

    i = Map_FindFreeSlot(1);
    if (i != 0) {
        return i;
    }
    i = 1;
    while (i < 5) {
        if ((D_001A01F0_slots.flags[i] & 0x1000) == 0 &&
            D_001A01F0_slots.use[i] != 0) {
            break;
        }
        i++;
    }
    move_map_entry_slot(0, i);
    return i;
}

extern int D_001A01F0[];

extern int D_001A01F0[];

/* Same three parallel arrays func_002054E0 walks: 0x278/4 = 0x9E,
   0x28C/4 = 0xA3 and 0x2A4/4 = 0xA9 into D_001A01F0. Moves entry `b`
   onto entry `a` and frees `b`.

   Where the pointers are derived is load-bearing. Naming the unoffset
   `base` keeps the symbol's address live and derives each array with
   its own addiu (`&D_001A01F0[0x9E]` folds the offset into %lo and
   loses an instruction). `flags` is derived only after the call, as in
   retail: deriving it up front with the others swaps a*4 and b*4
   between $s1 and $s2. */
void move_map_entry_slot(int a, int b) {
    int *base = D_001A01F0;
    int *dst = base + 0x9E;
    int *size = base + 0xA9;
    int *flags;

    FastMemCopy((void *)dst[a], (void *)dst[b], size[b] << 4);

    flags = base + 0xA3;
    flags[a] = flags[b];
    size[a] = size[b];
    flags[b] = -1;
}

extern int D_001A01F0[];

int findMapSlot(int arg0) {
    int i;
    for (i = 0; i < 5; i++) {
        if (D_001A01F0[0x9E + i] != 0 && D_001A01F0[0xA3 + i] == arg0) {
            return i;
        }
    }
    return -1;
}

extern unsigned char D_0013D5E9 NOT_SDA;
/* The MACRO_ADDR view declared further down, needed here already. */
extern int *D_001602E0_m __asm__("D_001602E0") MACRO_ADDR;

/* Picks the map to load next: the current map id (D_001A01F0+0x224,
   +0x100 when D_0013D5E9 is set) unless func_002058D0 finds it already
   in a slot, otherwise the nearest entry of the 20-id list D_001602E0
   around the current one (+1, -1, +2, -2, +3, -3) that is not loaded;
   -1 if none. The two bounds tests are nested so that fold does not
   merge them into one unsigned compare, and the step update is the
   ternary that gives retail's select (slti/movn). */
int PickNextMap(void) {
    char *m = (char *)D_001A01F0;
    int off = gHaveMapOMatic ? 0x100 : 0;
    int key;
    int i;
    int step;

    key = *(int *)(m + 0x224) + off;
    if (findMapSlot(key) == -1) {
        return key;
    }
    i = 0;
    if (*(int *)(m + 0x224) < 20) {
        while (D_001602E0_m[i] != *(int *)(m + 0x224)) {
            i++;
        }
    }
    step = 1;
    do {
        int j = i + step;
        if (j >= 0) {
            if (j < 20 && D_001602E0_m[j] != 0) {
                key = D_001602E0_m[j] + off;
                if (findMapSlot(key) == -1) {
                    return key;
                }
            }
        }
        step = (step > 0) ? -step : 1 - step;
    } while (step != 4);
    return -1;
}

extern int *D_001602E0;


/* The global is read at both uses in the loop, giving retail's two
   pointers, and through a MACRO_ADDR alias. */
int find_id_in_terminated_table(int arg0) {
    int i;

    for (i = 0; D_001602E0_m[i] != 0 && i < 20; i++) {
        if (D_001602E0_m[i] == arg0) {
            return i;
        }
    }
    return -1;
}

extern int func_001F9B70(int); /* abs */

/* Picks the map slot to reuse (D_001A01F0's five slots: in use at
   [0x9E + i], map id at [0xA3 + i], active slot [0xA8]). With no current
   map (+0x224 == 0) a used, inactive slot holding no map (-1) is looked
   for from the top first. Then the slots are scanned from the bottom:
   such a slot is taken, or else the one whose map is furthest from the
   current map in the D_001602E0 list (func_00205A50) is freed; 1 if the
   current map is not in the list.
   The slots are indexed off the global itself, as in func_002058D0, so
   that loop.c keeps one pointer per loop; the final store goes through
   the struct view, which adds the base first. */
int PickMapSlotToEvict(void) {
    int best = 0;
    int bestIdx = -1;
    int cur;

    if (D_001A01F0[0x89] == 0) {
        int i;
        for (i = 4; i >= 0; i--) {
            if (D_001A01F0[0x9E + i] != 0 && i != D_001A01F0[0xA8]
                && D_001A01F0[0xA3 + i] == -1) {
                return i;
            }
        }
    }
    {
        int i;
        cur = find_id_in_terminated_table(D_001A01F0[0x89]);
        if (cur == -1) {
            return 1;
        }
        for (i = 0; i < 5; i++) {
            if (D_001A01F0[0x9E + i] != 0 && i != D_001A01F0[0xA8]) {
                int d;
                if (D_001A01F0[0xA3 + i] == -1) {
                    return i;
                }
                d = func_001F9B70(find_id_in_terminated_table(D_001A01F0[0xA3 + i] & 0xFF) - cur);
                if (best < d) {
                    best = d;
                    bestIdx = i;
                }
            }
        }
    }
    if (bestIdx == -1) {
        bestIdx = 0;
    }
    ((PadSlots *)D_001A01F0)->flags[bestIdx] = -1;
    return bestIdx;
}

void compose_bitmap_from_mask(unsigned char *dst, unsigned char *a, unsigned char *b,
                   unsigned char *mask) {
    int i;
    int j;
    int bit;
    unsigned char *next;

    for (i = 0; i <= 0x7FFF; i++) {
        bit = 1;
        next = mask + 1;
        for (j = 7; j >= 0; j--) {
            if (*mask & bit) {
                *dst = *a;
            } else {
                *dst = *b;
            }
            bit <<= 1;
            a++;
            b++;
            dst++;
        }
        mask = next;
    }
}

extern void func_00209040(void);
extern char D_0013CA40[];

/* Map screen pan/zoom update: returns 1 while the pad word (D_0013CA40
   +0x1A4) has 0x500 set, 0 when the map object D_001A01F0 has nothing open
   (+0x24) or no slot (+0x228 < 0). Otherwise scales the slot's zoom (+0xB4
   floats) by 1 - speed*0.02 and clamps it to [0.65, 4], pans x/y (+0x104 /
   +0x154, 16.15 fixed point) by the pad velocities times 3000000/zoom, and
   clamps them to [lo, 0x10000000 - lo] with lo = (A/zoom) << 15, A = 0 for
   x and 1280 for y. The slot is re-read at every use, the three array
   bases are locals set where retail first builds them, and the x bound is
   computed before the y bound (the scheduler emits them in reverse). */
int UpdateMapZoomAndPan(void) {
    char *pad;
    char *m;
    float *zoom;
    int *xs;
    int *ys;

    func_00209040();
    pad = D_0013CA40;
    if (*(int *)(pad + 0x1A4) & 0x500) {
        return 1;
    }
    m = (char *)D_001A01F0;
    if (*(int *)(m + 0x24) == 0) {
        return 0;
    }
    if (*(int *)(m + 0x228) < 0) {
        return 0;
    }
    zoom = (float *)(m + 0xB4);
    zoom[*(int *)(m + 0x228)] *= 1.0f - *(float *)(pad + 0x104) * 0.02f;
    if (zoom[*(int *)(m + 0x228)] > 4.0f) {
        zoom[*(int *)(m + 0x228)] = 4.0f;
    }
    if (zoom[*(int *)(m + 0x228)] < 0.65f) {
        zoom[*(int *)(m + 0x228)] = 0.65f;
    }
    xs = (int *)(m + 0x104);
    ys = (int *)(m + 0x154);
    {
        float scale = 3000000.0f / zoom[*(int *)(m + 0x228)];
        xs[*(int *)(m + 0x228)] += (int)(*(float *)(pad + 0x108) * scale);
        ys[*(int *)(m + 0x228)] += (int)(*(float *)(pad + 0x10C) * scale);
    }
    {
        float z = zoom[*(int *)(m + 0x228)];
        int xlo = (int)(0.0f / z) << 15;
        int ylo = (int)(1280.0f / z) << 15;
        int xhi = 0x10000000 - xlo;
        int yhi = 0x10000000 - ylo;

        if (xs[*(int *)(m + 0x228)] < xlo) {
            xs[*(int *)(m + 0x228)] = xlo;
        }
        if (xs[*(int *)(m + 0x228)] > xhi) {
            xs[*(int *)(m + 0x228)] = xhi;
        }
        if (ys[*(int *)(m + 0x228)] < ylo) {
            ys[*(int *)(m + 0x228)] = ylo;
        }
        if (ys[*(int *)(m + 0x228)] > yhi) {
            ys[*(int *)(m + 0x228)] = yhi;
        }
    }
    return 0;
}

/* Adapted from Lombyte (MIT): src/assembly/textbin/ui/map/draw_map_overlay.c. */
struct DmaTag_205E70 { u32 tag, addr, vif0, vif1; };
struct TagPtr_205E70 { struct DmaTag_205E70 *p; };
typedef struct { s32 flags; u8 pad4[12]; } MapIdFlags_205E70;

struct ScreenOffsets_205E70 {
    u8 pad0[0x10];
    s32 x;
    s32 y;
};

typedef struct {
    s16 id; /* 0x00 */
    u16 pad02;
    u16 flags;       /* 0x04 */
    u16 texture_id;  /* 0x06 */
    s16 frame_index; /* 0x08 */
    u16 pad0A[2];
    u16 label_width;    /* 0x0E */
    s16 label_height;   /* 0x10 */
    s16 label_offset_x; /* 0x12 */
    s16 label_offset_y; /* 0x14 */
    u16 pad16;
    f32 x;      /* 0x18 */
    f32 y;      /* 0x1C */
    f32 angle;  /* 0x20 */
    s32 active; /* 0x24 */
} MapIcon_205E70;

typedef struct {
    s32 cell;
    s32 pad[3];
} MapHighlightedCell_205E70;

typedef struct {
    u8 pad0[0x8];
    s32 marks_enabled; /* 0x08 */
    u8 padC[0xC];
    s32 z;          /* 0x18 */
    s32 grid;       /* 0x1C */
    MapIcon_205E70 *icons; /* 0x20 */
    s32 enabled;    /* 0x24 */
    u8 pad28[0x4];
    s32 show_markers;            /* 0x2C */
    MapHighlightedCell_205E70 marks[8]; /* 0x30 */
    u8 padB0[0x4];
    f32 zoom[20];     /* 0xB4 */
    s32 offset_x[20]; /* 0x104 */
    s32 offset_y[20]; /* 0x154 */
    u8 pad1A4[0x84];
    s32 selected_map; /* 0x228 */
    u8 pad22C[0x14];
    s32 texture_address; /* 0x240 */
} MapOverlayState_205E70;

typedef struct {
    u8 pad0[0x80];
    f32 x; /* 0x80 */
    f32 y; /* 0x84 */
    u8 pad88[0x10];
    f32 angle; /* 0x98 */
    u8 pad9C[0x1FF0];
    s32 mode; /* 0x208C */
} MapPlayerState_205E70;

typedef struct {
    s16 pad0;
    s16 image_index;
} MapTextureReference_205E70;

typedef struct {
    u8 pad0[6];
    u8 width_exponent;
    u8 height_exponent;
} MapTextureInfo_205E70;

typedef struct {
    u8 pad0[0x20];
    MapTextureReference_205E70 *references; /* 0x20 */
    MapTextureInfo_205E70 *textures;        /* 0x24 */
} MapTextureTables_205E70;

typedef struct {
    s16 s[12];
} FontWindow_205E70;

typedef struct {
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
} MapIconBounds_205E70;

extern struct TagPtr_205E70 map_data_00161000 __asm__("D_00161000") MACRO_ADDR;
extern struct ScreenOffsets_205E70 map_data_0013E600 __asm__("D_0013E600");
extern MapOverlayState_205E70 map_data_001A01F0 __asm__("D_001A01F0");
extern MapPlayerState_205E70 map_data_0013F450 __asm__("D_0013F450");
extern MapTextureTables_205E70 map_data_0019A4E8 __asm__("D_0019A4E8");
extern MapIdFlags_205E70 map_data_0013D6C4[] __asm__("D_0013D6C4");
extern u16 map_data_001519D2[] __asm__("D_001519D2");
extern s32 map_data_0015EE84 __asm__("D_0015EE84") MACRO_ADDR;
extern s32 map_current_level_205E70 SDATA(D_0015EE84);
extern u8 map_data_0015EEB4[4] __asm__("D_0015EEB4") MACRO_ADDR;
extern s32 map_data_0015FE20 SDATA(D_0015FE20);
extern f32 map_data_0015FE40 SDATA(D_0015FE40);
extern f32 map_data_0015FE44 SDATA(D_0015FE44);
extern f32 map_data_0015FE48 SDATA(D_0015FE48);
extern f32 map_data_0015FE4C SDATA(D_0015FE4C);
extern f32 map_data_0015FE50 SDATA(D_0015FE50);
extern f32 map_data_0015FE54 SDATA(D_0015FE54);
extern u8 map_data_001E8398[] __asm__("D_001E8398");

extern s32 print_debug_text_centered(s32, s32, s32, u8 *) __asm__("func_001F0FF8");
extern void setup_gif_paging(s32) __asm__("func_001F4630");
extern void do_gif_paging(void) __asm__("func_001F4748");
extern void draw_ui_frame(s32, s32, s32, s32, s32) __asm__("func_001F62C8");
extern void font_print_window_small(void *, u64, void *, s32) __asm__("func_001F75D0");
extern f32 fast_add_rotations(f32, f32) __asm__("func_001FA748");
extern f32 fast_subtract_rotations(f32, f32) __asm__("func_001FA790");
extern s32 get_icon_frame(s32, s32) __asm__("func_00200198");
extern u64 get_frame_texture(s32) __asm__("func_00200248");
extern void draw_hud_sprite_subpixel(s32, s32, s32, s32, s32, s32) __asm__("func_002008B8");
extern void draw_rotated_sprite(f32, f32, f32, f32, f32, s32, s32, u64) __asm__("func_00200E38");
extern void append_screen_rect_packet(s32, s32, s32, s32, u64, s32) __asm__("func_00201640");
extern void format_menu_item_text(s32, void *) __asm__("func_00208AB0");
extern void world_to_map_coords(f32 *, f32 *, s32, f32, f32) __asm__("func_00208C38");
/* EABI places floating and integer arguments in independent register banks. */
extern void world_to_map_coords_xy_205E70(f32, f32, f32 *, f32 *, s32) __asm__("func_00208C38");
extern void draw_map_markers(s32, s32, s32, s32) __asm__("func_00208D38");
extern void vu1_add_g_sregister(s32, s64) __asm__("func_00234C98");
extern void *map_memset_205E70(void *, s32, u32) __asm__("func_001153FC");

/* UNK_NoMapAvailable */
void DrawMapView(void) {
    union {
        u8 b[0x80];
        f32 f[2];
    } label_buffer;
    FontWindow_205E70 text_window;
    s32 rx0, ry0, rx1, ry1;
    struct DmaTag_205E70 *tag;
    u64 *packet_words;
    u64 background_tex0;
    u64 map_tex0;
    s32 palette_block;
    s32 mirror_sign;
    s32 tile_size;
    s32 left_tile_count, top_tile_count, right_tile_count, bottom_tile_count;
    s32 x0, y0, x1, y1;
    s32 u, v;
    s32 dx, dy;
    s32 i, j;
    s32 cell, cell_y, cell_x;
    MapHighlightedCell_205E70 *highlighted_cell;
    MapIconBounds_205E70 *r;
    MapIconBounds_205E70 *icon_bounds;
    MapIcon_205E70 *icon;
    MapTextureInfo_205E70 *texture_info;
    f32 zoom;
    f32 icon_scale;
    f32 icon_size_multiplier;
    f32 s;
    f32 icon_center_x, icon_center_y;
    s32 pan_x, map_extent, pan_y, oy1;
    s32 ax, ay, bx, by;
    /* The frame temporary becomes the vertical label offset after drawing. */
    s32 id, sprite_word, texture_width;
    f32 angle, sprite_width, sprite_height, center_x, center_y;
    s16 label_width;
    s32 label_height, label_offset_x;
    s32 label_x, label_y;
    s32 image_index;
    s32 flip;
    s32 texture_id;
    s32 tile_limit;

    if (map_data_001A01F0.enabled == 0) {
        u8 *font = map_data_001E8398;

        setup_gif_paging(0);
        print_debug_text_centered(0x100, (s16)map_data_001519D2[0] >> 1, 0x80909090, font);
        do_gif_paging();
        return;
    }
    if (map_data_001A01F0.selected_map < 0) {
        return;
    }

    rx0 = 0x1000;
    setup_gif_paging(0);
    ry0 = 0x800;
    mirror_sign = map_data_0015EEB4[0] ? -1 : 1;
    zoom = map_data_001A01F0.zoom[map_data_001A01F0.selected_map];
    pan_y = zoom * (f32)(map_data_001A01F0.offset_y[map_data_001A01F0.selected_map] >> 15);
    map_extent = zoom * 8192.0f;
    pan_x = zoom * (f32)(map_data_001A01F0.offset_x[map_data_001A01F0.selected_map] >> 15);
    tile_size = zoom * 512.0f;
    tile_limit = tile_size + 0x2000;
    rx0 -= pan_x * mirror_sign;
    ry0 -= pan_y;
    rx1 = rx0 + map_extent * mirror_sign;
    ry1 = ry0 + map_extent;
    left_tile_count = (rx0 + tile_size - 1) / tile_size;
    top_tile_count = (ry0 + tile_size - 1) / tile_size;
    right_tile_count = (tile_limit - rx1 - 1) / tile_size;
    bottom_tile_count = (tile_limit - ry1 - 1) / tile_size;
    x0 = rx0 - tile_size * left_tile_count;
    y0 = ry0 - tile_size * top_tile_count;
    x1 = rx1 + tile_size * right_tile_count;
    y1 = ry1 + tile_size * bottom_tile_count;
    u = ((left_tile_count + right_tile_count) << 9) + 0x2000;
    v = ((top_tile_count + bottom_tile_count) << 9) + 0x2000;

    vu1_add_g_sregister(8, 0);
    vu1_add_g_sregister(0x47, 0);
    map_data_00161000.p->tag = 0x10000005;
    map_data_00161000.p->addr = 0;
    map_data_00161000.p->vif0 = 0;
    map_data_00161000.p->vif1 = 0x50000005;
    map_data_00161000.p++;
    background_tex0 =
        get_frame_texture(get_icon_frame(0xE999, map_data_001A01F0.selected_map));
    packet_words = (u64 *)map_data_00161000.p;
    packet_words[0] = 0x7400000000008001;
    packet_words[1] = 0x5353106;
    packet_words[2] = background_tex0;
    packet_words[3] = 0x156;
    packet_words[4] = 0x80808080;
    packet_words[5] = 0;
    packet_words[6] = ((x0 + map_data_0013E600.x) - 8) | ((u64)((y0 + map_data_0013E600.y) - 8) << 16) |
                      ((u64)map_data_001A01F0.z << 32);
    packet_words[7] = u | ((u64)v << 16);
    packet_words[8] = ((x1 + map_data_0013E600.x) - 8) | ((u64)((y1 + map_data_0013E600.y) - 8) << 16) |
                      ((u64)map_data_001A01F0.z << 32);
    packet_words[9] = 0;
    map_data_00161000.p = (struct DmaTag_205E70 *)((u8 *)map_data_00161000.p + 0x50);
    vu1_add_g_sregister(8, 5);
    vu1_add_g_sregister(0x47, 0x60B);

    palette_block = (background_tex0 >> 37) & 0x3FFF;
    map_data_00161000.p->tag = 0x10000005;
    map_data_00161000.p->addr = 0;
    map_data_00161000.p->vif0 = 0;
    map_data_00161000.p->vif1 = 0x50000005;
    map_tex0 = (u64)((map_data_001A01F0.texture_address >> 8) | (8 << 14) | (0x13 << 20) | (9 << 26)) |
               ((u64)9 << 30) | ((u64)1 << 34) | ((u64)palette_block << 37) |
               ((long)0x8000000000000000ULL);
    tag = map_data_00161000.p;
    map_data_00161000.p = tag + 1;
    packet_words = (u64 *)(tag + 1);
    packet_words[0] = 0x7400000000008001;
    packet_words[1] = 0x5353106;
    packet_words[2] = map_tex0;
    packet_words[3] = 0x156;
    packet_words[4] = 0x80808080;
    packet_words[5] = 0;
    packet_words[6] = ((rx0 + map_data_0013E600.x) - 8) | ((u64)((ry0 + map_data_0013E600.y) - 8) << 16) |
                      ((u64)map_data_001A01F0.z << 32);
    packet_words[7] = 0x20002000;
    packet_words[8] = ((rx1 + map_data_0013E600.x) - 8) | ((u64)((ry1 + map_data_0013E600.y) - 8) << 16) |
                      ((u64)map_data_001A01F0.z << 32);
    packet_words[9] = 0;
    map_data_00161000.p = (struct DmaTag_205E70 *)((u8 *)map_data_00161000.p + 0x50);
    vu1_add_g_sregister(0x47, 0x360B);

    if (map_data_001A01F0.show_markers != 0 && map_data_001A01F0.marks_enabled != 0) {
        dx = rx1 - rx0;
        dy = ry1 - ry0;
        for (i = 0; i < 8; i++) {
            cell = map_data_001A01F0.marks[i].cell;
            if (cell >= 0) {
                cell_x = cell % 16;
                cell_y = cell / 16;
                append_screen_rect_packet(rx0 + cell_x * dx / 16, ry0 + cell_y * dy / 16,
                                          rx0 + (cell_x + 1) * dx / 16,
                                          ry0 + (cell_y + 1) * dy / 16, 0x20000000, 1);
            }
        }
    }

    if (map_data_001A01F0.icons != 0) {
        icon_bounds = ((MapIconBounds_205E70 *)0x70000000);
        icon_scale = (map_data_001A01F0.zoom[map_data_001A01F0.selected_map] * 2.0f + 5.0f) / 13.0f;
        if (!(map_data_001A01F0.icons[0].flags & 4)) {
            MapIconBounds_205E70 *r;
            s32 i;
            MapIcon_205E70 *icon;
            f32 icon_size_multiplier;
            s32 image_index;
            f32 icon_center_x, icon_center_y;
            MapTextureInfo_205E70 *texture_info;
            f32 s;
            s32 texture_id;

            i = 0;
            do {
                if (map_data_001A01F0.icons[i].active != 0 &&
                    (texture_id = map_data_001A01F0.icons[i].texture_id) != 0 &&
                    !(map_data_001A01F0.icons[i].flags & 1)) {
                    icon_size_multiplier = 1.0f;
                    if (map_data_001A01F0.icons[i].flags & 0x80) {
                        icon_size_multiplier = 1.5f;
                    }
                    image_index = get_icon_frame(texture_id,
                                                                   map_data_001A01F0.icons[i].frame_index);
                    icon_center_x = (f32)rx0 + map_data_001A01F0.icons[i].x * (f32)(rx1 - rx0);
                    icon_center_y = (f32)ry0 + map_data_001A01F0.icons[i].y * (f32)(ry1 - ry0);
                    texture_info =
                        &map_data_0019A4E8.textures[map_data_0019A4E8.references[image_index].image_index];
                    if (map_data_001A01F0.icons[i].flags & 0x200) {
                        s = map_data_001A01F0.zoom[map_data_001A01F0.selected_map];
                    } else {
                        s = icon_scale;
                    }
                    icon_bounds[i].x0 =
                        icon_center_x -
                        icon_size_multiplier * s * (f32)(1 << (texture_info->width_exponent + 3));
                    icon_bounds[i].x1 =
                        (f32)icon_bounds[i].x0 +
                        icon_size_multiplier * s * (f32)(1 << (texture_info->width_exponent + 4));
                    icon_bounds[i].y0 =
                        icon_center_y -
                        icon_size_multiplier * s * (f32)(1 << (texture_info->height_exponent + 3));
                    icon_bounds[i].y1 =
                        (f32)icon_bounds[i].y0 +
                        icon_size_multiplier * s * (f32)(1 << (texture_info->height_exponent + 4));
                }
                i++;
            } while (!(map_data_001A01F0.icons[i].flags & 4));
        }

        {
            s32 i, j;
            s32 pan_x, map_extent, pan_y, oy1;
            s32 ax, ay, bx, by;
            MapIcon_205E70 *icon;
            s32 inner_offset;

            /* The flag-4 entry terminates the list. Separate overlapping icon
               bounds before either icon is drawn, using half of the smallest
               positive edge distance for each pair. */
            i = 0;
            if (!(map_data_001A01F0.icons[1].flags & 4))
                do {
                    if (map_data_001A01F0.icons[i].active == 0 || map_data_001A01F0.icons[i].texture_id == 0 ||
                        (map_data_001A01F0.icons[i].flags & 3)) {
                        goto next_outer_icon;
                    }
                    j = i + 1;
                    for (; !(map_data_001A01F0.icons[j].flags & 4); j++) {
                        inner_offset = j * sizeof(MapIcon_205E70);
                        pan_x = icon_bounds[j].x1 - icon_bounds[i].x0;
                        if (pan_x <= 0)
                            continue;
                        map_extent = icon_bounds[i].x1 - icon_bounds[j].x0;
                        if (map_extent <= 0)
                            continue;
                        pan_y = icon_bounds[j].y1 - icon_bounds[i].y0;
                        if (pan_y <= 0)
                            continue;
                        oy1 = icon_bounds[i].y1 - icon_bounds[j].y0;
                        if (oy1 <= 0)
                            continue;
                        icon = (MapIcon_205E70 *)(inner_offset + (s32)map_data_001A01F0.icons);
                        if (icon->active == 0 || icon->texture_id == 0 || (icon->flags & 3)) {
                            continue;
                        }
                        ax = 0;
                        ay = 0;
                        bx = 0;
                        by = 0;
                        if (pan_x <= map_extent && pan_x <= pan_y && pan_x <= oy1) {
                            ax = pan_x >> 1;
                            bx = ax - pan_x;
                        } else if (map_extent <= pan_y && map_extent <= oy1) {
                            bx = map_extent >> 1;
                            ax = bx - map_extent;
                        } else if (pan_y <= oy1) {
                            ay = pan_y >> 1;
                            by = ay - pan_y;
                        } else {
                            by = oy1 >> 1;
                            ay = by - oy1;
                        }
                        icon_bounds[i].x0 += ax;
                        icon_bounds[i].x1 += ax;
                        icon_bounds[i].y0 += ay;
                        icon_bounds[i].y1 += ay;
                        icon_bounds[j].x0 += bx;
                        icon_bounds[j].x1 += bx;
                        icon_bounds[j].y0 += by;
                        icon_bounds[j].y1 += by;
                    }
                next_outer_icon:
                    i++;
                } while (!(map_data_001A01F0.icons[i + 1].flags & 4));
        }

        {
            s32 i;

            i = 0;
            if (!(map_data_001A01F0.icons[0].flags & 4)) {
                do {
                    if (map_data_001A01F0.icons[i].active != 0 && !(map_data_001A01F0.icons[i].flags & 1)) {
                        id = map_data_001A01F0.icons[i].texture_id;
                        sprite_word = map_data_001A01F0.icons[i].frame_index;
                        if (id != 0) {
                            if (map_data_001A01F0.icons[i].flags & 0x40) {
                                append_screen_rect_packet(
                                    icon_bounds[i].x0 - 0x20, icon_bounds[i].y0 - 0x20,
                                    icon_bounds[i].x1 + 0x20, icon_bounds[i].y1 + 0x20, 0x80000000, 1);
                            }
                            if (map_data_001A01F0.icons[i].flags & 0x200) {
                                s = map_data_001A01F0.zoom[map_data_001A01F0.selected_map];
                            } else {
                                s = icon_scale;
                            }
                            if (map_data_001A01F0.icons[i].flags & 0x100) {
                                angle = map_data_001A01F0.icons[i].angle;
                                texture_width = 0x20;
                                sprite_height = s * 256.0f;
                                sprite_width = sprite_height;
                                if (map_data_001A01F0.icons[i].flags & 0x400) {
                                    angle = fast_add_rotations(angle, 1.5707964f);
                                    texture_width = 0x40;
                                    sprite_width = s * map_data_0015FE48;
                                    sprite_height = s * map_data_0015FE4C;
                                    if (map_data_0013D6C4[map_data_001A01F0.icons[i].id].flags & 2) {
                                        sprite_word++;
                                    }
                                }
                                if (map_data_001A01F0.icons[i].flags & 0x800) {
                                    angle = fast_add_rotations(angle, 1.5707964f);
                                    texture_width = 0x40;
                                    sprite_width = s * map_data_0015FE40;
                                    sprite_height = s * map_data_0015FE44;
                                    if (map_data_0013D6C4[map_data_001A01F0.icons[i].id].flags & 2) {
                                        sprite_word++;
                                    }
                                }
                                if (map_data_001A01F0.icons[i].flags & 0x1000) {
                                    angle = fast_add_rotations(angle, 1.5707964f);
                                    sprite_width = s * map_data_0015FE50;
                                    sprite_height = s * map_data_0015FE54;
                                }
                                center_x = (f32)(icon_bounds[i].x0 + icon_bounds[i].x1) * 0.5f;
                                center_y = (f32)(icon_bounds[i].y0 + icon_bounds[i].y1) * 0.5f;
                                draw_rotated_sprite(center_x, center_y, sprite_width, sprite_height,
                                                    angle, texture_width, 0x20,
                                                    get_frame_texture(get_icon_frame(
                                                        id, sprite_word)));
                            } else {
                                draw_hud_sprite_subpixel(
                                    get_icon_frame(id, sprite_word),
                                    icon_bounds[i].x0, icon_bounds[i].y0,
                                    icon_bounds[i].x1 - icon_bounds[i].x0,
                                    icon_bounds[i].y1 - icon_bounds[i].y0, 0x80);
                            }
                            if (map_data_001A01F0.icons[i].flags & 0x10) {
                                label_width = map_data_001A01F0.icons[i].label_width;
                                label_offset_x = map_data_001A01F0.icons[i].label_offset_x;
                                label_height = map_data_001A01F0.icons[i].label_height;
                                sprite_word = map_data_001A01F0.icons[i].label_offset_y;
                                if (label_offset_x == 0) {
                                    label_x = ((icon_bounds[i].x0 + icon_bounds[i].x1) >> 5) -
                                              label_width / 2;
                                } else {
                                    if (label_offset_x > 0) {
                                        label_x = (icon_bounds[i].x1 >> 4) - label_width / 2 + label_offset_x;
                                    } else {
                                        label_x = (icon_bounds[i].x0 >> 4) - label_width / 2 + label_offset_x;
                                    }
                                }
                                if (sprite_word == 0) {
                                    label_y = ((icon_bounds[i].y0 + icon_bounds[i].y1) >> 5) -
                                              label_height / 2;
                                } else {
                                    if (sprite_word > 0) {
                                        label_y = (icon_bounds[i].y1 >> 4) - label_height / 2 + sprite_word;
                                    } else {
                                        label_y = (icon_bounds[i].y0 >> 4) - label_height / 2 + sprite_word;
                                    }
                                }
                                draw_ui_frame(label_y, label_y + label_height, label_x,
                                              label_x + label_width, 0x40);
                                format_menu_item_text(i, &label_buffer);
                                map_memset_205E70(&text_window, 0, sizeof(text_window));
                                text_window.s[1] = label_y + label_height;
                                text_window.s[3] = label_x + label_width;
                                text_window.s[4] = label_x + label_width / 2;
                                text_window.s[5] = label_y + 4;
                                text_window.s[8] = 0xF;
                                text_window.s[9] = 1;
                                text_window.s[0] = label_y;
                                text_window.s[2] = label_x;
                                font_print_window_small(&text_window, 0x80FFA888, &label_buffer, -1);
                            }
                        }
                    }
                    i++;
                } while (!(map_data_001A01F0.icons[i].flags & 4));
            }
        }
    }

    {
        MapOverlayState_205E70 *m = &map_data_001A01F0;

        if (map_data_0015EE84 == m->selected_map) {
            s32 image_index;
            f32 s;
            f32 angle;
            s32 flip;

            image_index = get_icon_frame(0xE99A, 5);
            s = ((m->zoom[m->selected_map] * 4.0f + 10.0f) * 0.75f) / 13.0f;
            angle = map_data_0013F450.angle;
            flip = map_data_0013F450.mode == 0xF;
            if (flip) {
                angle = fast_add_rotations(angle, 1.5707964f);
            }
            if (map_data_0015FE20 != 0) {
                world_to_map_coords(&label_buffer.f[0], &label_buffer.f[1], map_data_0015EE84 + 100,
                                    map_data_0013F450.x, map_data_0013F450.y);
                angle = fast_add_rotations(angle, 1.5707964f);
            } else {
                world_to_map_coords_xy_205E70(map_data_0013F450.x, map_data_0013F450.y,
                                             &label_buffer.f[0], &label_buffer.f[1], map_current_level_205E70);
            }
            label_buffer.f[0] = (f32)rx0 + label_buffer.f[0] * (f32)(rx1 - rx0);
            label_buffer.f[1] = (f32)ry0 + label_buffer.f[1] * (f32)(ry1 - ry0);
            if (map_data_0015EEB4[0] != 0) {
                angle = fast_subtract_rotations(-fast_add_rotations(angle, 1.5707964f), 1.5707964f);
            }
            {
                f32 sz = s * 256.0f;

                draw_rotated_sprite(label_buffer.f[0], label_buffer.f[1], sz, sz, angle, 0x40, 0x40,
                                    get_frame_texture(image_index));
            }
        }
    }
    do_gif_paging();
    if (map_data_001A01F0.grid != 0) {
        setup_gif_paging(0);
        draw_map_markers(rx0, ry0, rx1, ry1);
        do_gif_paging();
    }
}

extern void func_00207090(unsigned char *dst, int row, unsigned char *src,
                          short *offs);

/* Builds the map's 1-bit coverage mask: each of the 256 rows is decoded
   into the scratchpad (func_00207090), then every 4-bit pixel that is
   not 0 sets its bit, 32 pixels to a word, one word every 16 words of
   the output, 16 rows side by side (dst + row % 16 + row / 16 * 512).
   The remainder is computed before the quotient (retail's surviving
   copy of the quotient), and the scratchpad is a pointer local (retail
   adds the base first). */
void DecodeMapMask(int *dst, unsigned char *src, short *offs) {
    int row;
    unsigned char *scr = (unsigned char *)0x70000000;

    for (row = 0; row < 0x100; row++) {
        int *p;
        int j;

        DecodeMapRleRow(scr, row, src, offs);
        {
            int r = row % 16;
            int q = row / 16;
            p = dst + (r + q * 512);
        }
        for (j = 0; j < 32; j++) {
            int bits = 0;
            int k;

            for (k = 0; k < 32; k++) {
                unsigned char b = scr[j * 16 + (k >> 1)];
                int v;

                if (k & 1) {
                    v = b >> 4;
                } else {
                    v = b & 0xF;
                }
                if (v != 0) {
                    bits |= 1 << k;
                }
                if ((k & 31) == 31) {
                    *p++ = bits;
                    bits = 0;
                }
            }
            p += 15;
        }
    }
}

/* Unpacks run-length coded 4-bit pixels (the map image) into dst: row's
   codes run from src + offs[row - 1] (src + 0x200 for row 0) to src +
   offs[row], three nibbles each, a pixel value and a count byte (0 means
   0x100). Pixels are packed two to a byte, low nibble first, and `half`
   carries an odd run's last pixel into the next run. The count starts
   at 0x100 in its own statement so that it stays a plain set in the loop
   (a cmove constant would be hoisted); the two bytes are read low byte
   first and `half` is cleared before the count drops, retail's order. */
void DecodeMapRleRow(unsigned char *dst, int row, unsigned char *src, short *offs) {
    int half = 0;
    unsigned char *p;
    int n;
    int i;

    if (row != 0) {
        p = src + offs[row - 1];
    } else {
        p = src + 0x200;
    }
    n = (src + offs[row] - p) * 2 / 3;
    for (i = 0; i < n; i++) {
        int nib = i * 3;
        unsigned char *q = p + (nib >> 1);
        unsigned char v = q[0];
        unsigned char c = q[1];
        unsigned char pair;
        int count;

        if (nib & 1) {
            v >>= 4;
        } else {
            c = (c << 4) | (v >> 4);
            v &= 0xF;
        }
        pair = v | (v << 4);
        count = 0x100;
        if (c != 0) {
            count = c;
        }
        if (half) {
            *dst |= v << 4;
            dst++;
            half = 0;
            count--;
        }
        if (count != 0) {
            do {
                *dst = pair;
                count -= 2;
                dst++;
            } while (count > 0);
            if (count != 0) {
                dst--;
                half = 1;
                *dst = v;
            }
        }
    }
}
