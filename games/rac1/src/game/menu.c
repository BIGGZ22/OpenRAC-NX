#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * menu.cpp in the original source; text 0x2071A8-0x209A60.
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

int Menu_IsPointInFixedQuad(int x1, int y1) {
    return menu_pointIsClockwise(x1, y1, 0xD3, 0xDB, 0x129, 0xF9);
}

extern unsigned char D_0013D49C NOT_SDA;

int func_002071D0(void) {
    return D_0013D49C != 0;
}

extern unsigned char D_0013D49D NOT_SDA;

int func_002071E0(void) {
    return D_0013D49D != 0;
}

extern unsigned char D_0013D4A5 NOT_SDA;

int func_002071F0(void) {
    return D_0013D4A5 != 0;
}

typedef struct {
    char _pad0[0x12E4];
    unsigned char unk12E4;
    char _pad12E5[0x208C - 0x12E5];
    int unk208C;
} Menu13F450;
extern Menu13F450 D_0013F450;

/* Menu hit test: arg3 is the third float ($f14). b (state 16) is
   computed before a, as retail evaluates it; the second if's own
   `arg1 >= 0xC8` is retail's second test of $a1. */
int func_00207200(void *arg0, int arg1, float unused1, float unused2, float arg3) {
    Menu13F450 *s = &D_0013F450;
    int b = s->unk208C == 16;
    int a = s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;

    if (arg1 < 0xC8 && arg3 >= 39.5f && arg3 <= 42.5f && !a) {
        return 1;
    }
    if (arg1 >= 0xC8 && arg3 >= 87.0f && !b) {
        return 1;
    }
    return 0;
}

extern int D_001414DC_early __asm__("D_001414DC") NOT_SDA;
extern unsigned char D_0013D49E NOT_SDA;

/* `if (x) return 1;` in both arms with one shared `return 0;` keeps the
   flag test a beqz with the li in its slot; a result variable or a
   return 0 per arm becomes sltu. */
int func_002072C0(int arg0, float unused1, float unused2, float arg1) {
    int is16 = D_001414DC_early == 0x10;

    if (arg0 < 0x100) {
        if (D_0013D49E != 0) {
            return 1;
        }
    } else if (arg1 >= 58.0f && arg1 <= 86.0f && !is16) {
        return 1;
    }
    return 0;
}

/* The struct's address in a local keeps one base register for both
   field reads. */
int func_00207340(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0x100) {
        Menu13F450 *s = &D_0013F450;
        return s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;
    }
    return (arg1 >= 95.0f) ? 1 : 0;
}

extern short D_0015FE24;   /* declared small so -G2 puts it in SDA */

int func_002073A8(void) {
    return *(int *)&D_0015FE24 == 0;
}

extern unsigned char D_0013D4AC NOT_SDA;

int func_002073B8(void) {
    return D_0013D4AC != 0;
}

extern unsigned char D_0013D4AD NOT_SDA;

int func_002073C8(void) {
    return D_0013D4AD != 0;
}

extern unsigned char D_0013D4AE NOT_SDA;

int func_002073D8(void) {
    return D_0013D4AE != 0;
}

extern unsigned char D_0013D4AF NOT_SDA;

int func_002073E8(void) {
    return D_0013D4AF != 0;
}

extern unsigned char D_0013D4B5 NOT_SDA;

int func_002073F8(void) {
    return D_0013D4B5 != 0;
}

typedef u32 u128 __attribute__((mode(TI), aligned(16)));
struct RegionPlayerState {
    u8 pad0[0x12E4];
    u8 region_mode;
    u8 pad12E5[0x208C - 0x12E5];
    s32 active_state;
};
extern struct RegionPlayerState D_0013F450_07408 __asm__("D_0013F450");
extern s32 D_001A04B0[];
union RegionVector {
    u128 region_center;
    f32 f[4];
};
extern s32 func_00209048(s32, s32, s32, s32, s32, s32);
extern f32 func_001F9D48_07408(union RegionVector *, union RegionVector *) __asm__("func_001F9D48");
s32 func_00207408(s32 projected_x, s32 projected_y, f32 x, f32 y, f32 z);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_00206bd8.c, passes_projected_region_callback_0. */
s32 func_00207408(s32 projected_x, s32 projected_y, f32 x, f32 y, f32 z) {
    s32 special_state;

    if (menu_pointIsClockwise(projected_x, projected_y, 0x189, 0x16F, 0x87, 0xED)) {
        if (menu_pointIsClockwise(projected_x, projected_y, 0x75, 0x146, 0x18B, 0x1CC) &&
            menu_pointIsClockwise(projected_x, projected_y, 0x141, 0x130, 0xB0, 0x130)) {
            if (43.9f <= z) {
                return 1;
            }
            return 0;
        }
    } else if (menu_pointIsClockwise(projected_x, projected_y, 0x191, 0xCD, 0xD2, 0x13B)) {
        union RegionVector position;
        union RegionVector region_center;

        position.region_center = 0;
        region_center.region_center = 0;
        position.f[0] = x;
        position.f[1] = y;
        region_center.f[0] = 337.5f;
        region_center.f[1] = 250.0f;
        if (func_001F9D48_07408(&position, &region_center) <= 7.0f) {
            return 1;
        }
        if (menu_pointIsClockwise(projected_x, projected_y, 0x142, 0x12A, 0x173, 0xFC) && D_001A04B0[0]) {
            return 1;
        }
        return 0;
    } else {
        special_state = 0;
        if ((u32)(D_0013F450_07408.active_state - 0x11) < 2 || D_0013F450_07408.region_mode == 1) {
            special_state = 1;
        }
        if (special_state && menu_pointIsClockwise(projected_x, projected_y, 0x12B, 0xB8, 0x13A, 0xF2) &&
            menu_pointIsClockwise(projected_x, projected_y, 0x130, 0xED, 0x165, 0xCB) &&
            menu_pointIsClockwise(projected_x, projected_y, 0x161, 0xDC, 0x13B, 0xB0) &&
            menu_pointIsClockwise(projected_x, projected_y, 0x156, 0xA8, 0x119, 0xD1)) {
            return 1;
        }
    }
    return 0;
}

extern int D_001A04A8 NOT_SDA;

/* Menu hit test (func_00207200's family: `a` is the Menu13F450 state test
   shared with func_00207340/func_00207930). Two independent ways to hit:
   the first box with arg1 >= 0x135 and D_001A04A8 set and arg3 >= 47.7,
   which can succeed even when `a` is true; or, only when `a` is true,
   all three of a second group of boxes hit. */
int func_00207648(int arg0, int arg1, float unused1, float unused2,
                   float arg3) {
    Menu13F450 *s = &D_0013F450;
    int a = s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;

    if (menu_pointIsClockwise(arg0, arg1, 0x93, 0x168, 0x182, 0x168) != 0
        && arg1 >= 0x135 && D_001A04A8 != 0 && arg3 >= 47.7f) {
        return 1;
    }
    if (!a) {
        return 0;
    }
    if (menu_pointIsClockwise(arg0, arg1, 0xC5, 0x9A, 0x13C, 0xE1) != 0
        && menu_pointIsClockwise(arg0, arg1, 0xD6, 0xC3, 0x157, 0xC5) != 0
        && menu_pointIsClockwise(arg0, arg1, 0x107, 0xDA, 0x171, 0xA0) != 0) {
        return 1;
    }
    return 0;
}

/* Menu hit test. Left of x 0xE9 (arg1 < 0xE9) the point must be inside
   all four func_00209048 boxes. Otherwise it needs a hit in each of
   three pairs of boxes, where both boxes of a pair are always tested.
   Each pair's results are locals of their own block: the first one then
   lives in one basic block across the second call, so local-alloc gives
   it $s0 before the arguments are allocated, as retail has it. The two
   compound conditions (`&&` chain, `!a && !b`) keep the return values
   as branches instead of an sltu. */
int func_00207780(int arg0, int arg1) {
    if (arg1 < 0xE9) {
        if (menu_pointIsClockwise(arg0, arg1, 0x132, 0xA0, 0x15F, 0xD8)
            && menu_pointIsClockwise(arg0, arg1, 0x14D, 0xD8, 0x181, 0x9A)
            && menu_pointIsClockwise(arg0, arg1, 0x182, 0xB4, 0x137, 0x93)
            && menu_pointIsClockwise(arg0, arg1, 0x157, 0x8C, 0x130, 0xAA)) {
            return 1;
        }
        return 0;
    }
    {
        int a = menu_pointIsClockwise(arg0, arg1, 0x8F, 0x115, 0x148, 0x14B);
        int b = menu_pointIsClockwise(arg0, arg1, 0xE7, 0x108, 0x127, 0x164);
        if (a == 0 && b == 0) {
            return 0;
        }
    }
    {
        int a = menu_pointIsClockwise(arg0, arg1, 0xED, 0x15F, 0x154, 0x10E);
        int b = menu_pointIsClockwise(arg0, arg1, 0xA2, 0x12B, 0x16F, 0x147);
        if (a == 0 && b == 0) {
            return 0;
        }
    }
    {
        int a = menu_pointIsClockwise(arg0, arg1, 0x132, 0x163, 0x141, 0xCC);
        int b = menu_pointIsClockwise(arg0, arg1, 0xC2, 0x108, 0x1A0, 0x12E);
        if (!a && !b) {
            return 0;
        }
        return 1;
    }
}

/* Retail carries 4 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n");

extern int D_001A04BC NOT_SDA;

/* Menu hit test in two layouts (func_00207200's family). The first arm
   needs its own `return 0`: jump.c then sets v0 = 1 before the test
   there too, which gives retail's registers and lets the arm be
   cross-jumped into the second one. */
int func_00207930(int arg0, int arg1, float unused1, float unused2, float arg3) {
    if (arg1 < 0xBB) {
        Menu13F450 *s = &D_0013F450;
        int a = s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;

        if (!a && D_001A04BC != 0) {
            return 1;
        }
        return 0;
    }
    if (arg3 >= 51.5f && arg3 <= 54.0f
        && menu_pointIsClockwise(arg0, arg1, 0x10A, 0xE5, 0x124, 0xF9) != 0) {
        return 1;
    }
    return 0;
}

extern int D_001A04B4 NOT_SDA;

/* Same hit test as func_002071A8, run against two boxes: nonzero when
   D_001A04B4 is set and either box is hit. Retail sets the result before
   each test (0, 1, 0, then 1 on the fall-through), so the C does the same.
   The last store goes through a short: jump.c folds `r = 0; if (b) r = 1;`
   into an sltu only when the store sets a whole register, and a subreg
   store keeps retail's beqz. */
int func_002079F0(int x1, int y1) {
    int a = menu_pointIsClockwise(x1, y1, 0x99, 0xED, 0x160, 0x117);
    int b = menu_pointIsClockwise(x1, y1, 0x10E, 0xF7, 0x13D, 0x119);
    int r = 0;

    if (D_001A04B4 != 0) {
        r = 1;
        if (a == 0) {
            r = 0;
            if (b != 0) {
                *(short *)&r = 1;
            }
        }
    }
    return r;
}

extern int D_001A04C0 NOT_SDA;
extern int D_001A04B8 NOT_SDA;

/* Hit test in two layouts: arg3 (the third float, $f14) must lie in
   [73.5, 80] with D_001A04C0 set, or for arg1 >= 0x9D in [51.5, 54] with
   D_001A04B8 set. Written as `<`/`>` rejections so they compile to retail's
   c.lt/bc1t; the second arm's `return 0` cross-jumps into the first's. */
int func_00207A80(void *arg0, int arg1, float unused1, float unused2, float arg3) {
    if (arg1 < 0x9D) {
        if (arg3 < 73.5f || arg3 > 80.0f || D_001A04C0 == 0) {
            return 0;
        }
        return 1;
    } else {
        if (arg3 < 51.5f || arg3 > 54.0f || D_001A04B8 == 0) {
            return 0;
        }
        return 1;
    }
}

extern int D_001A04AC NOT_SDA;
extern int func_00209048(int, int, int, int, int, int);

/* Menu hit test (func_00207200's family): for arg1 >= 0x105, needs
   D_001A04AC set and arg3 >= 47.7; otherwise returns `a` directly when
   arg1 < 0xC1, else `a` unless func_00209048's box test hits (which
   clears it to 0). */
int func_00207B30(int arg0, int arg1, float unused1, float unused2, float arg3) {
    Menu13F450 *s = &D_0013F450;
    int a = s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;
    int result;

    if (arg1 >= 0x105) {
        if (D_001A04AC == 0) {
            return 0;
        }
        return (arg3 >= 47.7f) ? 1 : 0;
    }
    if (arg1 < 0xC1) {
        return a;
    }
    result = a;
    if (menu_pointIsClockwise(arg0, arg1, 0xD9, 0xB8, 0x156, 0xD2) != 0) {
        result = 0;
    }
    return result;
}

/* Menu hit test (func_00207200's family): hits unless the menu state
   blocks it, arg3 ($f14) is below 71.5, or the first box is hit, and
   then only if the second box is not. */
int func_00207BE8(int arg0, int arg1, float unused1, float unused2, float arg3) {
    Menu13F450 *s = &D_0013F450;
    int a = s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;

    if (a) {
        return 0;
    }
    if (arg3 < 71.5f) {
        return 0;
    }
    if (menu_pointIsClockwise(arg0, arg1, 0x131, 0xE2, 0xC6, 0x93) != 0) {
        return 0;
    }
    return menu_pointIsClockwise(arg0, arg1, 0x190, 0x89, 0xD1, 0xFB) == 0;
}

extern unsigned char D_0013D4C5 NOT_SDA;
extern int D_001414DC NOT_SDA;

int func_00207CB0(int arg0, int arg1) {
    if (arg1 >= 0x101) {
        return D_001414DC == 0xF;
    }
    return D_0013D4C5 != 0;
}

/* The two unused float parameters put the threshold's argument in
   $f14 (floats count consecutively from $f12). The nops after the mtc1
   and the compare are ps2eeas's (tools/ps2eeas_nops.py). */
int func_00207CE0(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0xE0) {
        return (arg1 >= 47.75f) ? 1 : 0;
    }
    return (arg1 < 29.0f) ? 1 : 0;
}

/* `(a && b) ? 1 : 0` gives retail's bc1f then bc1tl; the other arm is
   a plain `? 1 : 0`. */
int func_00207D38(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0xE0) {
        return (arg1 >= 44.0f && arg1 <= 45.0f) ? 1 : 0;
    }
    return (arg1 >= 37.0f) ? 1 : 0;
}

/* func_00207D38's twin. */
int func_00207DB0(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0xE0) {
        return (arg1 >= 42.0f && arg1 <= 43.0f) ? 1 : 0;
    }
    return (arg1 >= 39.0f) ? 1 : 0;
}

/* Returning the compare as `? 1 : 0` gives bc1t with the `li 1` in its
   slot; folded into `&&` it becomes bc1tl. The two nops, after the mtc1
   and after the compare, are ps2eeas's (tools/ps2eeas_nops.py). */
int func_00207E28(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 >= 0xE0) {
        return 0;
    }
    return (arg1 <= 38.0f) ? 1 : 0;
}

extern unsigned char D_0013D4C0 NOT_SDA;
extern unsigned char D_0013D4C1 NOT_SDA;
extern unsigned char D_0013D4C2 NOT_SDA;
extern unsigned char D_0013D4D3 NOT_SDA;
extern unsigned char D_0013D4D4 NOT_SDA;
extern unsigned char D_0013D4D5 NOT_SDA;

int func_00207E60(void) {
    return D_0013D4C0 != 0;
}

int func_00207E70(void) {
    return D_0013D4C1 != 0;
}

int func_00207E80(void) {
    return D_0013D4C2 != 0;
}

int func_00207E90(void) {
    return D_0013D4D3 != 0;
}

int func_00207EA0(void) {
    return D_0013D4D4 != 0;
}

int func_00207EB0(void) {
    return D_0013D4D5 != 0;
}

extern unsigned char D_0013D4E0;

extern unsigned char D_0013D4E0_far __asm__("D_0013D4E0") NOT_SDA;
/* The recorded C was right; the two ps2eeas nops were the only
   residual. */
int func_00207EC0(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 >= 0xBE) return D_0013D4E0_far != 0;
    return (arg1 >= 58.5f) ? 1 : 0;
}

extern unsigned char D_0013D4DC NOT_SDA;
extern unsigned char D_0013D4DD NOT_SDA;
extern unsigned char D_0013D4DE NOT_SDA;
extern unsigned char D_0013D4DF NOT_SDA;
extern unsigned char D_0013D4E1 NOT_SDA;

int func_00207F00(void) {
    return D_0013D4DC != 0;
}

int func_00207F10(void) {
    return D_0013D4DD != 0;
}

int func_00207F20(void) {
    return D_0013D4DE != 0;
}

int func_00207F30(void) {
    return D_0013D4DF != 0;
}

int func_00207F40(void) {
    return D_0013D4E1 != 0;
}

extern unsigned char D_0013D4E6 NOT_SDA;

int func_00207F50(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0xDD) {
        return arg1 >= 232.0f && arg1 <= 235.0f && D_0013D4E6 != 0;
    }
    return (arg1 <= 180.0f) ? 1 : 0;
}

extern unsigned char D_0013D4E7 NOT_SDA;

int func_00207FD0(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0xDD) {
        return (arg1 >= 242.0f) ? 1 : 0;
    }
    return arg1 <= 180.0f && D_0013D4E7 != 0;
}

extern unsigned char D_0013D4E8 NOT_SDA;

int func_00208030(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0xDD) {
        return (arg1 >= 238.0f && arg1 <= 241.0f) ? 1 : 0;
    }
    return arg1 <= 180.0f && D_0013D4E8 != 0;
}

extern unsigned char D_0013D4EA NOT_SDA;

/* The Menu13F450 state test of func_00207340, computed up front (retail
   evaluates it before the arg0 branch), gates the arg0 < 0x15F arm; the
   other arm is func_00208160's second test with D_0013D4EA. */
int func_002080B0(int arg0, float unused1, float unused2, float arg1) {
    Menu13F450 *s = &D_0013F450;
    int a = s->unk208C == 17 || s->unk208C == 18 || s->unk12E4 == 1;

    if (arg0 < 0x15F) {
        return (arg1 >= 200.0f) ? a : 0;
    }
    return arg1 >= 233.0f && arg1 <= 235.0f && D_0013D4EA != 0;
}

extern unsigned char D_0013D4EB NOT_SDA;

int func_00208160(int arg0, float unused1, float unused2, float arg1) {
    if (arg0 < 0x15F) {
        return (arg1 >= 228.0f && arg1 <= 230.0f) ? 1 : 0;
    }
    return arg1 >= 233.0f && arg1 <= 235.0f && D_0013D4EB != 0;
}

extern unsigned char D_0013D4E9 NOT_SDA;

int func_002081F8(void) {
    return D_0013D4E9 != 0;
}

/* arg0 is unused; the two unused float parameters put arg3 in $f14
   (floats count consecutively from $f12, one register each; see
   func_00207CE0). With `r` defaulting to 1, reorg turns the reset into
   retail's bc1fl with `r = 0` in its delay slot. The nop after the mtc1
   is ps2eeas's (tools/ps2eeas_nops.py). */
int func_00208208(void *arg0, int arg1, float unused1, float unused2, float arg3) {
    int r = 1;

    if (arg1 < 0x141 && !(63.5f <= arg3)) {
        r = 0;
    }
    return r;
}

int func_00208238(void) {
    return 1;
}

int func_00208240(void) {
    return 1;
}

typedef float FVec4[4] __attribute__((aligned(16)));
extern float func_001F9D48(float *, float *);

/* True when (x, y) is within 35 of the point (161.5, 249), or always
   while D_001A04A8 is set. Both vectors are 16-byte aligned like the
   SDK's sceVu0FVECTOR; a partial initializer then clears the whole
   quadword with one por/sq (a plain float[4] gets a memset call). */
int func_00208248(int x, int y) {
    if (D_001A04A8 == 0) {
        FVec4 p = { (float)x, (float)y };
        FVec4 c = { 0 };

        c[0] = 161.5f;
        c[1] = 249.0f;
        return func_001F9D48(p, c) <= 35.0f ? 1 : 0;
    }
    return 1;
}

extern unsigned char D_0013D502 NOT_SDA;

int func_002082E8(void) {
    return D_0013D502 != 0;
}

extern unsigned char D_0013D503 NOT_SDA;

int func_002082F8(void) {
    return D_0013D503 != 0;
}

extern unsigned char D_0013D504 NOT_SDA;
extern unsigned char D_0013D505 NOT_SDA;
extern unsigned char D_0013D50F NOT_SDA;

int func_00208308(void) {
    return D_0013D504 != 0;
}

int func_00208318(void) {
    return D_0013D505 != 0;
}

int func_00208328(void) {
    return D_0013D50F != 0;
}

extern int D_001A01F0[];
extern int D_0013D668[];
extern void func_00209040(void);
extern int func_001FAA28(void *dst, int size, int a, int b);
extern void func_00208860(void *dst);

/* The same object, twice. gp is 0x00166D00, so the `lw $4,-0x7E7C($28)`
   in retail's delay slot and the `lui/%lo(D_0015EE84)` a few
   instructions later are the SAME address; retail reaches it both ways
   inside one function. Declared small it lands in SDA at -G2, declared
   int it stays out. */
extern int D_0015EE84 MACRO_ADDR;

/* Read a sector into arg0 from the open file (D_001A01F0 slot state) or
   clear it when none is open, and track the largest read per slot in
   D_0013D668. Adapted from Lombyte (MIT) for PAL. */
void Menu_ReadSector(int arg0) {
    int n;

    func_00209040();
    if (D_001A01F0[0xA] == 0) {
        FastMemSet((void *)arg0, 0, 0x800);
        return;
    }
    n = func_001FAA28((void *)arg0, 0x800, D_001A01F0[5], D_001A01F0[3]);
    if (n == -1) {
        CondenseBitmapTo1bpp((void *)arg0);
    }
    if (D_0013D668[D_0015EE84] < n) {
        D_0013D668[D_0015EE84] = n;
    }
}

extern int D_001A0218[] NOT_SDA;
extern void func_00208458(void *, unsigned char *, int);
extern void func_00208688(void *, unsigned char *);

void UnpackOcclusionMap(void *arg0, unsigned char *arg1, int arg2) {
    /* func_00209040 is empty in retail and takes no arguments; $4/$5/$6
       still hold our own incoming arguments across it, which is why retail
       saves them into $16-$18 rather than reloading. */
    func_00209040();
    if (!D_001A0218[0]) {
        return;
    }
    if (*arg1 & 1) {
        DecodeOcclusionMap(arg0, arg1, arg2);
    } else {
        ExpandBitmapTo4bpp(arg0, arg1);
    }
}

/* Decodes the run-length compressed 1-bit occlusion map into 0x8000 bytes at dst: runs are expanded one byte per bit in the scratchpad, packed eight to a byte and copied out 0x400 bytes at a time. */
void DecodeOcclusionMap(void *dst, unsigned char *ctrl, int spans) {
    unsigned char *dst_end;
    unsigned char *end;
    unsigned char *span;
    unsigned char *cursor;
    unsigned char *next_dst;
    unsigned char *next;
    unsigned char *wrapped;
    unsigned char *r;
    unsigned char *w;
    unsigned char *from;
    unsigned char *to;
    int count;
    int n;
    int bit;

    dst_end = (unsigned char *)dst + 0x8000;
    end = (unsigned char *)0x70002000;
    span = (unsigned char *)spans;
    count = *ctrl >> 1;
    ctrl++;
    FastMemSet((void *)0x70000000, 0, 0x2400);
    bit = 1;
    cursor = (unsigned char *)0x70000000;
    for (;;) {
        next_dst = (unsigned char *)dst + 0x400;
        do {
            cursor += *span++;
            n = *span++;
            if (n != 0) {
                do {
                    n--;
                    next = cursor + 1;
                    if (count == 0) {
                        do {
                            count = *ctrl++;
                            bit = !bit;
                        } while (count == 0);
                    }
                    *cursor = bit;
                    count--;
                    cursor = next;
                } while (n != 0);
            }
        } while (cursor < end);
        r = (unsigned char *)0x70000000;
        w = (unsigned char *)0x70000000;
        do {
            *w = *r++;
            *w |= *r++ << 1;
            *w |= *r++ << 2;
            *w |= *r++ << 3;
            *w |= *r++ << 4;
            *w |= *r++ << 5;
            *w |= *r++ << 6;
            *w |= *r++ << 7;
            w++;
        } while (r < end);
        FastMemCopy(dst, (void *)0x70000000, 0x400);
        dst = next_dst;
        if (next_dst == dst_end) {
            return;
        }
        FastMemSet((void *)0x70000000, 0, 0x2000);
        wrapped = cursor - 0x2000;
        from = (unsigned char *)0x70002000;
        to = (unsigned char *)0x70000000;
        if (cursor > end) {
            do {
                *to++ = *from++;
            } while (from < cursor);
        }
        FastMemSet((void *)0x70002000, 0, 0x400);
        cursor = wrapped;
    }
}

/* Expands 128 rows of 16 source bytes into 4-bit-per-pixel masks: a
   256-entry table maps each byte to a word with nibble k set to 0xF when
   bit k is set, each row's 16 bytes are looked up into a 64-byte row
   buffer, and the buffer is copied to dst four times (0x100 bytes of dst
   per source row). The first bit is a plain store over the zeroed entry
   (retail's store-in-delay-slot), and the row buffer is filled through
   a block-local pointer, which gives retail's separate buffer copies. */
void ExpandBitmapTo4bpp(void *dst, unsigned char *src) {
    unsigned int table[256];
    unsigned int rowbuf[16];
    int i, j, row;

    for (i = 0; i < 256; i++) {
        table[i] = 0;
        if (i & 1) {
            table[i] = 0xF;
        }
        if (i & 0x2) {
            table[i] |= 0xF0;
        }
        if (i & 0x4) {
            table[i] |= 0xF00;
        }
        if (i & 0x8) {
            table[i] |= 0xF000;
        }
        if (i & 0x10) {
            table[i] |= 0xF0000;
        }
        if (i & 0x20) {
            table[i] |= 0xF00000;
        }
        if (i & 0x40) {
            table[i] |= 0xF000000;
        }
        if (i & 0x80) {
            table[i] |= 0xF0000000;
        }
    }
    for (row = 0; row < 128; row++) {
        unsigned int *p = rowbuf;
        for (j = 0; j < 16; j++) {
            *p++ = table[*src++];
        }
        FastMemCopy(dst, rowbuf, 0x40);
        dst = (char *)dst + 0x40;
        FastMemCopy(dst, rowbuf, 0x40);
        dst = (char *)dst + 0x40;
        FastMemCopy(dst, rowbuf, 0x40);
        dst = (char *)dst + 0x40;
        FastMemCopy(dst, rowbuf, 0x40);
        dst = (char *)dst + 0x40;
    }
}

LINKER_REMNANT("asm/remnants/text", func_00208858);

/* Bit counts of the 16 nibbles. */
typedef struct {
    int n[16];
} NibbleBits;

extern NibbleBits D_001E83B0;
extern unsigned char *D_001A01FC;


/* Condenses D_001A01FC's 0x200 rows of 64 bytes (two 4-bit samples
   each) into a 1-bit image at arg0: every four rows the per-column
   bit counts (D_001E83B0 is the nibble popcount table, copied onto the
   stack) are summed, and a column with 8 or more set bits becomes a
   set bit, packed eight columns to a byte. The first byte is then
   flagged with 0x02 and bit 0 cleared. */
void CondenseBitmapTo1bpp(void *arg0) {
    unsigned char *dst = arg0;
    int acc[128];
    NibbleBits bits;
    unsigned char *src;
    unsigned char *out;
    int i, j, k, m;

    bits = D_001E83B0;
    src = D_001A01FC;
    out = dst;
    for (i = 0; i < 0x200; i++) {
        if ((i & 3) == 0) {
            FastMemSet(acc, 0, 0x200);
        }
        for (j = 0; j < 128; j += 2) {
            unsigned int b = *src++;
            acc[j] += bits.n[b & 0xF];
            acc[j + 1] += bits.n[b >> 4];
        }
        if (i % 4 == 3) {
            for (k = 0; k < 128; k++) {
                if (acc[k] < 8) {
                    acc[k] = 0;
                } else {
                    acc[k] = 1 << (k % 8);
                }
            }
            for (m = 0; m < 128; m += 8) {
                *out++ = acc[m] | acc[m + 1] | acc[m + 2] | acc[m + 3]
                       | acc[m + 4] | acc[m + 5] | acc[m + 6] | acc[m + 7];
            }
        }
    }
    dst[0] = (dst[0] & 0xFE) | 2;
}

extern void *func_001FE540_id(int) __asm__("func_001FE540");
extern char D_0015FE60[]; /* "%d" */
extern char D_0015FE68[]; /* "error" */
extern char D_001E02B0[];
extern int func_00116248_3(void *, char *, int) __asm__("func_00116248");
extern int func_00116248_2(void *, char *) __asm__("func_00116248");

/* Copies the text of menu entry `bank` (text id at +0xA of its 0x28-byte
   record in the table at D_001A01F0[8]) into dst, expanding the first
   '%' escape: "%b" becomes the entry's D_001E02B0 value (the record's
   +0xC item, first int of its 0x18-byte row) printed with "%d", any
   other letter becomes "error". The strings are unsigned char (one lbu
   serves both the test and the copy), and each block that reads
   D_001A01F0 has its own char * local, which gives retail's kept %hi
   and rebuilt %lo. */
void FormatMenuItemText(int bank, unsigned char *dst) {
    unsigned char buf[16];
    unsigned char *src;
    unsigned char *p;
    char *t;

    t = (char *)D_001A01F0;
    src = func_001FE540_id(*(short *)(*(char **)(t + 0x20) + bank * 0x28 + 0xA));
    p = buf;
    if (src == 0) {
        return;
    }
    while (*src != 0 && *src != '%') {
        *dst++ = *src++;
    }
    if (*src == 0) {
        *dst = *src;
        return;
    }
    src++;
    if (*src == 'b') {
        char *t2 = (char *)D_001A01F0;
        func_00116248_3(buf, D_0015FE60,
                        *(int *)(D_001E02B0 + *(short *)(*(char **)(t2 + 0x20) + bank * 0x28 + 0xC) * 0x18));
    } else {
        func_00116248_2(buf, D_0015FE68);
    }
    src++;
    while (*p != 0) {
        *dst++ = *p++;
    }
    while (*src != 0) {
        *dst++ = *src++;
    }
    *dst = 0;
}

/* Retail carries 12 bytes of inter-function padding after this endlabel. */
__asm__(".section .text\n\tnop\n\tnop\n\tnop\n");

typedef struct {
    float a, b, c, d;
} OrientEntry;
extern OrientEntry D_0019EA70[];

/* arg2 (a table index, offset by 100 when the +100 variant is wanted)
   selects a row of D_0019EA70 (19 rows, 0..0x12); -1 means "use
   D_0015EE84"; anything out of [0,0x13) collapses to row 0. Row 6 with
   the +100 variant blends against fixed constants instead of the
   generic per-row formula. */
void WorldToMapCoords(float *out0, float *out1, int arg2, float arg3, float arg4) {
    int idx;
    int flag = 0;

    if (arg2 >= 100) {
        arg2 -= 100;
        flag = 1;
    }

    idx = arg2;
    if (idx == -1) {
        idx = D_0015EE84;
    }
    if (idx < 0) {
        idx = 0;
    }
    if (idx >= 0x13) {
        idx = 0;
    }

    if (idx == 6 && flag) {
        *out0 = (D_0019EA70[6].d * arg4 + 1053.0f) * (1.0f / 512.0f);
        *out1 = (740.0f - D_0019EA70[6].b * arg3) * (1.0f / 512.0f);
        return;
    }

    *out0 = (D_0019EA70[idx].a + D_0019EA70[idx].b * arg3) * (1.0f / 512.0f);
    *out1 = (D_0019EA70[idx].c + D_0019EA70[idx].d * arg4) * (1.0f / 512.0f);
}

LINKER_REMNANT("asm/remnants/text", func_00208D30);

typedef struct {
    float x;
    float y;
    int icon;           /* icon id, or -1 for a plain square */
    unsigned int sub;   /* the icon's frame, or the square's colour */
} MapMarker;

extern int D_0015FE70 MACRO_ADDR;
extern float D_001A02A4[];
extern char D_0019A4E8_raw[] __asm__("D_0019A4E8");
extern void func_00201640(int, int, int, int, long, int);
extern int func_00200198(int, int);
extern void func_002008B8(int, int, int, int, int, int);

void DrawMapMarkers(int left, int top, int right, int bottom) {
    float nx;
    float ny;
    int n = D_001A01F0[0x2C];
    MapMarker *m = (MapMarker *)D_001A01F0[7];

    while (n-- != 0) {
        int sx;
        int sy;

        WorldToMapCoords(&nx, &ny, D_0015EE84, m->x, m->y);
        sx = left + (int)((float)(right - left) * nx);
        sy = top + (int)((float)(bottom - top) * ny);
        if (sx < -0x199 || sy < -0x199 || sx >= 0x219A || sy >= 0x1B9A) {
            m++;
            continue;
        }
        if (m->icon == -1) {
            func_00201640(sx - D_0015FE70, sy - D_0015FE70, sx + D_0015FE70,
                          sy + D_0015FE70, m->sub, 1);
        } else {
            int tex = GetIconFrame(m->icon, m->sub);
            int *st = D_001A01F0;
            float *sc = (float *)(st + 0x2D);   /* the zoom scale table, at 0xB4 of the same block (the assembly's D_001A02A4) */
            char *arena = D_0019A4E8_raw;
            unsigned char *e = (unsigned char *)(*(char **)(arena + 0x24)
                + *(short *)(*(char **)(arena + 0x20) + tex * 4 + 2) * 8);
            float scale = (2.0f * sc[st[0x89]] + 5.0f) / 13.0f;
            float w = scale * (float)(1 << (e[6] + 4));

            DrawHudSpriteSubpixel(tex, sx - (int)(scale * (float)(1 << (e[6] + 3))),
                          sy - (int)(scale * (float)(1 << (e[7] + 3))),
                          (int)w, (int)(scale * (float)(1 << (e[7] + 4))), 0x80);
        }
        m++;
    }
}

extern int D_001E06B8[];
extern void *D_00199578[];
extern char D_0013D6B8[];

/* Copies three floats (+0x10, +0x14, +0x48) of each of level
   D_0015EE84's objects in D_00199578 into the 16-byte slots at
   D_0013D6B8; the index range is D_001E06B8[i]..[i + 1]. Binding the
   two bases to locals in retail's order orders their %hi halves. */
void Map_CopyObjectPositions(void) {
    int i = D_0015EE84;
    if ((unsigned int)i < 0x13) {
        int start = D_001E06B8[i];
        int end = D_001E06B8[i + 1];
        if (start < end) {
            void **sbase = D_00199578;
            char *dbase = D_0013D6B8;
            float *dst = (float *)(dbase + start * 16);
            void **src = &sbase[start];
            start = end - start;
            do {
                char *p = (char *)*src;
                if (p != 0) {
                    dst[0] = *(float *)(p + 0x10);
                    dst[1] = *(float *)(p + 0x14);
                    dst[2] = *(float *)(p + 0x48);
                }
                dst += 4;
                start--;
                src++;
            } while (start != 0);
        }
    }
}

void func_00209040(void) {
}

/*
 * Close but not exact (16/36 bytes): 2D cross-product orientation test.
 * arg0 -= arg2; arg1 -= arg3; arg4 -= arg2; arg5 -= arg3;
 * return (arg4*arg1 - arg5*arg0) < 0;
 * Same operations/order/register reuse (in-place subtraction) as
 * retail, confirmed via objdump, but the final sign test compiles to
 * `srl $2,$2,0x1f` here where retail uses `slti $2,$2,0` -- two
 * different instructions computing the identical 0/1 result for
 * "value < 0". Not a scheduling/register question like the usual
 * near-misses, a pure instruction-selection choice for the same
 * boolean-from-sign-bit pattern; not investigated further.
 */
/*
 * 4/36 (one instruction): 2D cross-product orientation test — is (x2,y2)
 * left of the (x0,y0)->(x1,y1) edge. Every register and all 7 arithmetic
 * instructions match retail exactly; the sole diff is the final sign
 * test, where this compiler emits `srl $2,$2,31` and retail has
 * `slti $2,$2,0`. Identical result, different instruction selection, and
 * it is not steerable from source: `< 0`, `<= -1`, `< 1-1`, a named
 * local, and `?1:0` all canonicalize to the same `srl`; only widening to
 * `long` changes it (to `dsrl32`, which is worse).
 *
 * The subtractions must be written in this order (x2/y2 before x1/y1) —
 * the last-source-statement-emits-first rotation applies to runs of
 * independent *arithmetic* statements, not just stores.
 */
/* Sign of the 2D cross product (x2-x0,y2-y0) x (x1-x0,y1-y0): true when
   the second vector is clockwise from the first.

   The `< 0` has to be written as an explicit if/return pair. Returning
   the comparison directly makes GCC emit the cheap sign-bit extract
   `srl $v0,$v0,31`; retail has `slti $v0,$v0,0`, which is what you get
   when the comparison feeds a branch rather than being the return value
   itself. That one instruction was this function's entire residual. */
int menu_pointIsClockwise(int x1, int y1, int x0, int y0, int x2, int y2) {
    int cross;

    x2 -= x0;
    y2 -= y0;
    x1 -= x0;
    y1 -= y0;
    cross = x2 * y1 - y2 * x1;
    if (cross < 0) {
        return 1;
    }
    return 0;
}

extern char D_0013D390[];
extern int D_0015EFB0 MACRO_ADDR;
extern int D_0015EFB4 MACRO_ADDR;
extern int D_0015FF4C MACRO_ADDR;
extern int D_00161380 MACRO_ADDR;
extern void (*D_001A0538[])(void);

/* Calls the handler for the state in D_0015EFB0 from the table at
   D_001A0538. Bits 0x80 and 0x100 of D_0015EFB4 request states 0x15 and
   0x14; D_00161380 counts calls since the state last changed. */
void Menu_RunStateHandler(void) {
    char *b = D_0013D390;
    int old;

    if (*(int *)(b + 0xEC) != 0 || *(int *)(b + 0x1C) != 0) {
        D_0015FF4C = 1;
    }
    old = D_0015EFB0;
    if (D_0015EFB4 & 0x80) {
        D_0015EFB0 = 0x15;
        D_0015EFB4 = (D_0015EFB4 & ~0x80) | 0x40;
    }
    if (D_0015EFB4 & 0x100) {
        D_0015EFB0 = 0x14;
        D_0015EFB4 = (D_0015EFB4 & ~0x100) | 0x40;
    }
    D_001A0538[D_0015EFB0]();
    D_00161380++;
    if (D_0015EFB0 != old) {
        D_00161380 = 0;
    }
}

extern char D_0013D390[];
/* menu.cpp's state word and flags. Stored through the assembler's lui
   macro, and $gp-relative where the access sits in a delay slot (see
   tools/check_macro_slots.py). */
extern int D_0013D3AC;

/* Word view of the D_0013D390 record: indexing an int array here, rather
   than casting byte offsets off a char pointer, is what lets GCC keep the
   record address's lui/addiu pair together before the D_0015EFB0 store. */
extern int D_0013D390_i[] __asm__("D_0013D390");

/* Sets menu state 3, clears the record's word at +0xFC and copies +0xC4
   into +0x1C. */
void func_00209160(void) {
    D_0015EFB0 = 3;
    D_0013D390_i[0x3F] = 0;
    D_0013D390_i[0x07] = D_0013D390_i[0x31];
}

/* Clears the 4 and 2 flag bits of D_0015EFB4, then picks the next
   state into D_0015EFB0 from the 0x80/0x100/0x200 bits and the
   D_0013D390 record. The cross-jumped `b` back into the 0x80 arm is
   one GNU as pads as a short loop and ps2eeas did not;
   tools/ps2eeas_nops.py writes it as a .word. */
void func_00209188(void) {
    int flags = D_0015EFB4;
    char *b;
    int nf;
    D_0015EFB4 = flags & ~4;
    b = D_0013D390;
    nf = D_0015EFB4 & ~2;
    D_0015EFB4 = nf;
    if (*(int *)(b + 0xFC) == 0) {
        D_0015EFB0 = 3;
        return;
    }
    if (flags & 0x80) {
        D_0015EFB0 = 0x15;
        D_0015EFB4 = (nf ^ 0x80) | 0x40;
        return;
    }
    if (flags & 0x100) {
        D_0015EFB0 = 0x14;
        D_0015EFB4 = (nf ^ 0x100) | 0x40;
        return;
    }
    if (*(int *)(b + 0x1C) != 0) {
        *(int *)(b + 0xFC) = 0;
        D_0015EFB4 = nf | 1;
        D_0015EFB0 = 2;
        return;
    }
    if (flags & 0x200) {
        D_0015EFB4 = nf ^ 0x200;
        D_0015EFB0 = 0x16;
    }
}

void func_00209238(void) {
    char *b = D_0013D390;
    char *base = b + 0xB0;
    int idx = *(int *)(b + 0xCC);
    int *slot = (int *)(base + idx * 0xC0);
    if (*slot == 2) {
        *slot = 0;
    }
    if ((D_0015EFB4 ^ 1) & 1) {
        D_0015EFB0 = 3;
    }
}

void func_00209290(void) {
    char *b = D_0013D390;
    char *base = b + 0xB0;
    int idx = *(int *)(b + 0xCC);
    int *slot;
    *(int *)(b + 0xE4) = -1;
    *(int *)(b + 0xE8) = -1;
    *(int *)(b + 0x20) = -1;
    *(int *)(b + 0x3C) = -1;
    *(int *)(b + 0x58) = -1;
    *(int *)(b + 0x74) = -1;
    *(int *)(b + 0x90) = -1;
    slot = (int *)(base + idx * 0xC0);
    if (*slot == 2) {
        *slot = 0;
    }
    D_0015EFB0 = 4;
}

void func_002092E8(void) {
    char *b = D_0013D390;
    int v;
    D_0015EFB4 &= ~0x20;
    if (*(int *)(b + 8) != 2) {
        return;
    }
    v = *(int *)(b + 0x1C);
    if (v == 0) {
        D_0015EFB0 = 9;
    } else if (v == -1) {
        *(int *)(b + 0x1C) = 0;
        D_0015EFB0 = 9;
    } else if (v == -2) {
        D_0015EFB0 = 5;
    }
}

void func_00209358(void) {
    if (D_0013D3AC != -2) {
        D_0015EFB0 = 3;
        return;
    }
    if (D_0015EFB4 & 2) {
        D_0015EFB0 = 6;
    }
}

extern int D_0015F6C8 MACRO_ADDR;

void func_00209398(void) {
    int flags;
    if (D_0013D3AC != -2) {
        D_0015EFB0 = 3;
        return;
    }
    flags = D_0015EFB4;
    if (flags & 0x20) {
        D_0015EFB4 = flags ^ 0x20;
        if (D_0015F6C8 != 0) {
            D_0015EFB0 = 0x17;
            return;
        }
        D_0015EFB0 = 5;
        return;
    }
    if (flags & 8) {
        D_0015EFB4 = flags ^ 8;
        D_0015EFB0 = 7;
    }
}

void menu_post_preparePageOpen(void) {
    char *s = D_0013D390;
    *(int *)(s + 0x1C) = 0;
    if (*(int *)(s + 0xE4) < 0) {
        *(int *)(s + 0xE8) = 0;
        *(int *)(s + 0xE4) = 3;
    }
    D_0015EFB0 = 8;
}

void func_00209448(void) {
    char *s = D_0013D390;
    if (*(int *)(s + 0xDC) == 2 && *(int *)(s + 0xE4) < 0) {
        if (*(int *)(s + 0xEC) != 0) {
            D_0015EFB0 = 0x11;
            D_0015EFB4 |= 0x40;
            return;
        }
        D_0015EFB0 = 0xE;
    }
}

void func_002094A8(void) {
    if (D_0013D3AC != 0) {
        D_0015EFB0 = 3;
        return;
    }
    if (D_0015EFB4 & 6) {
        D_0015EFB0 = 0xA;
    }
}

void func_002094E0(void) {
    char *s = D_0013D390;
    if (*(int *)(s + 0xDC) == 2 && *(int *)(s + 0xE4) < 0) {
        *(int *)(s + 0xE4) = 7;
        *(int *)(s + 0xE8) = 0;
        D_0015EFB0 = 0xB;
    }
}

extern char D_0013D390[];

/* Dispatch-table handler for menu.cpp's D_0013D390 state record. First,
   guarded by unkDC>=3 || unkE4>=0, clears a per-index slot in the
   0xB0-byte array (D_0013D390+0xB0 + idx*0xC0) from 1 to 2 -- named as
   its own `base` local, computed before `idx`, matching the sibling
   func_00209238's identical slot-lookup shape (that ordering is what
   makes the allocator put the base pointer in $a0 and the literal 1 in
   $a1, as retail does; keeping `b` mutated in place puts them the other
   way round). Then it re-reads the record fresh and sets D_0015EFB0
   (the next-state word) from unk1C/unkEC/unk14/unkC+unkAC. */
void func_00209520(void) {
    char *b = D_0013D390;
    int v;
    if (*(int *)(b + 0xDC) >= 3 || *(int *)(b + 0xE4) >= 0) {
        char *base = b + 0xB0;
        int idx = *(int *)(b + 0xCC);
        int *slot = (int *)(base + idx * 0xC0);
        if (*slot == 1) {
            *slot = 2;
        }
    }
    b = D_0013D390;
    if (*(int *)(b + 0x1C) < -1 || *(int *)(b + 0xEC) != 0) {
        D_0015EFB0 = 3;
        return;
    }
    v = *(int *)(b + 0x14);
    if (v == -2) {
        if (*(int *)(b + 0xC) + *(int *)(b + 0xAC) < 0x15E) {
            D_0015EFB0 = 0x13;
        } else {
            D_0015EFB0 = 0xC;
        }
        return;
    }
    if (v >= -1) {
        D_0015EFB0 = 0x10;
    }
}

void func_002095E8(void) {
    if (D_0013D3AC != 0) {
        D_0015EFB0 = 3;
        return;
    }
    if (D_0015EFB4 & 2) {
        D_0015EFB0 = 0xD;
    }
}

void func_00209620(void) {
    int flags;
    if (D_0013D3AC != 0) {
        D_0015EFB0 = 3;
        return;
    }
    flags = D_0015EFB4;
    if (flags & 0x20) {
        D_0015EFB4 = flags ^ 0x20;
        if (D_0015F6C8 != 0) {
            D_0015EFB0 = 0x18;
            return;
        }
        D_0015EFB0 = 0xC;
        return;
    }
    if (flags & 0x10) {
        D_0015EFB4 = flags ^ 0x10;
        D_0015EFB0 = 0xE;
    }
}

void func_00209698(void) {
    char *s = D_0013D390;
    if (*(int *)(s + 0xDC) == 2 && *(int *)(s + 0xE4) < 0) {
        *(int *)(s + 0xE4) = 9;
        *(int *)(s + 0xE8) = 0;
        D_0015EFB0 = 0xF;
    }
}

void func_002096D8(void) {
    char *s = D_0013D390;
    if (*(int *)(s + 0xDC) == 2 && *(int *)(s + 0xE4) < 0) {
        if (*(int *)(s + 0xEC) != 0) {
            D_0015EFB0 = 0x12;
            D_0015EFB4 |= 0x40;
            return;
        }
        *(int *)(s + 0xE4) = 7;
        *(int *)(s + 0xC8) = 0;
        *(int *)(s + 0x14) = 0;
        *(int *)(s + 0xE8) = 0;
        D_0015EFB0 = 0x10;
    }
}

/* func_00209188's sibling: clears the 4 and 2 bits, then the same
   0x80/0x100 arms and the D_0013D390 record's tests. Its cross-jumped
   `b` is written as a .word too (tools/ps2eeas_nops.py). */
void func_00209750(void) {
    int flags;
    char *b;
    if (D_0015EFB4 & 4) {
        D_0015EFB4 &= ~4;
    }
    if (D_0015EFB4 & 2) {
        D_0015EFB4 &= ~2;
    }
    flags = D_0015EFB4;
    if (flags & 0x80) {
        D_0015EFB0 = 0x15;
        D_0015EFB4 = (flags ^ 0x80) | 0x40;
        return;
    }
    if (flags & 0x100) {
        D_0015EFB0 = 0x14;
        D_0015EFB4 = (flags ^ 0x100) | 0x40;
        return;
    }
    b = D_0013D390;
    if (*(int *)(b + 0x1C) != 0) {
        D_0015EFB0 = 3;
        return;
    }
    if (*(int *)(b + 0xFC) != 0) {
        D_0015EFB0 = 1;
    }
}


void func_00209808(void) {
    char *b = D_0013D390;
    char *base = b + 0xB0;
    int idx = *(int *)(b + 0xCC);
    int *slot = (int *)(base + idx * 0xC0);
    if (*slot == 2) {
        *slot = 0;
    }
    if (!(D_0015EFB4 & 0x40)) {
        D_0015EFB0 = 3;
    }
}

void func_00209858(void) {
    char *b = D_0013D390;
    char *base = b + 0xB0;
    int idx = *(int *)(b + 0xCC);
    int *slot = (int *)(base + idx * 0xC0);
    if (*slot == 2) {
        *slot = 0;
    }
    if (!(D_0015EFB4 & 0x40)) {
        D_0015EFB0 = 3;
    }
}

void func_002098A8(void) {
    if (D_0013D3AC != 0) {
        D_0015EFB0 = 3;
    }
}

void func_002098C8(void) {
    char *b = D_0013D390;
    char *base = b + 0xB0;
    int idx = *(int *)(b + 0xCC);
    int *slot = (int *)(base + idx * 0xC0);
    if (*slot == 2) {
        *slot = 0;
    }
    if (!(D_0015EFB4 & 0x40)) {
        D_0015EFB0 = 3;
    }
}

void func_00209918(void) {
    char *b = D_0013D390;
    char *base = b + 0xB0;
    int idx = *(int *)(b + 0xCC);
    int *slot = (int *)(base + idx * 0xC0);
    if (*slot == 2) {
        *slot = 0;
    }
    if (!(D_0015EFB4 & 0x40)) {
        D_0015EFB0 = 3;
    }
}

void func_00209968(void) {
    char *b = D_0013D390;
    if (*(int *)(b + 0xDC) < 3 && *(int *)(b + 0xE4) < 0) {
        if (*(int *)(b + 0xEC) != 0) {
            *(int *)(b + 0xFC) = 0;
            D_0015EFB0 = 0x15;
            D_0015EFB4 |= 0x440;
            return;
        }
        D_0015EFB0 = 1;
    }
}

void func_002099D0(void) {
    int flags;
    if (D_0013D3AC != -2) {
        D_0015EFB0 = 3;
        return;
    }
    flags = D_0015EFB4;
    if (flags & 0x20) {
        D_0015EFB4 = flags ^ 0x20;
        D_0015EFB0 = 5;
    }
}

void func_00209A18(void) {
    int flags;
    if (D_0013D3AC != 0) {
        D_0015EFB0 = 3;
        return;
    }
    flags = D_0015EFB4;
    if (flags & 0x20) {
        D_0015EFB4 = flags ^ 0x20;
        D_0015EFB0 = 0xC;
    }
}
