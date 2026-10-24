#include "common.h"
#include "hero.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * missionfunc.cpp in the original source; text 0x20C7A0-0x20D348.
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

extern int func_0020C940(short type, int arg);
extern char *D_001A2EF0[];
extern unsigned char D_0013DE60[];
extern char D_001A2F90[];

/* Per-frame objective update over the 0x28-byte node list of the current
   pad slot (D_001A2EF0[D_001A01F0[0x89]], kept at D_001A2F90+0x10):
   status +0x24 = 0 when a gated node (+0x10 & 4) has its D_0013DE60 byte
   clear or its first condition (func_0020C940) fails, else 2 or 1 by the
   second condition; then each +0x1C callback's result goes to +0x26; then
   the count of status-1 nodes without +0x10 & 2 goes to D_001A2F90+0xC.
   Returns whether that count is 0. The head is stored and then read back
   (CSE forwards it, leaving retail's copy into $s0 after the null test),
   and each block reads D_001A2F90 through its own `char *` local. */
int Mission_UpdateObjectives(void) {
    char *node;

    {
        char *p = D_001A2F90;
        *(char **)(p + 0x10) = D_001A2EF0[D_001A01F0[0x89]];
        if (*(char **)(p + 0x10) == 0) {
            return 0;
        }
        node = *(char **)(p + 0x10);
    }
    while (*(short *)node != 0) {
        if ((*(unsigned short *)(node + 0x10) & 4) && D_0013DE60[D_001A01F0[0x89]] == 0) {
            *(short *)(node + 0x24) = 0;
        } else if (!Mission_CheckCondition(*(short *)(node + 2), *(int *)(node + 4))) {
            *(short *)(node + 0x24) = 0;
        } else if (!Mission_CheckCondition(*(short *)(node + 8), *(int *)(node + 0xC))) {
            *(short *)(node + 0x24) = 1;
        } else {
            *(short *)(node + 0x24) = 2;
        }
        node += 0x28;
    }
    {
        char *p = D_001A2F90;
        for (node = *(char **)(p + 0x10); *(short *)node != 0; node += 0x28) {
            if (*(int (**)(int))(node + 0x1C) != 0) {
                *(short *)(node + 0x26) = (*(int (**)(int))(node + 0x1C))(*(int *)(node + 0x20));
            }
        }
    }
    {
        char *p = D_001A2F90;
        *(int *)(p + 0xC) = 0;
        for (node = *(char **)(p + 0x10); *(short *)node != 0; node += 0x28) {
            if (*(short *)(node + 0x24) == 1 && !(*(unsigned short *)(node + 0x10) & 2)) {
                *(int *)(p + 0xC) += 1;
            }
        }
    }
    {
        char *p = D_001A2F90;
        return *(int *)(p + 0xC) == 0;
    }
}

extern unsigned char D_0013DE48[];
extern unsigned char D_0013D5C8_b[] __asm__("D_0013D5C8");
extern unsigned char D_0013D5F0[];
typedef struct { int a, b, c, d; } Rec16_C940;
extern Rec16_C940 D_0013D6B8_r[] __asm__("D_0013D6B8");
extern unsigned char D_0013D490[];
extern unsigned char D_0014BFC0[][4];

/* A 10-case switch; the explicit `case 9: break;` keeps retail's table. */
int Mission_CheckCondition(short type, int arg) {
    switch (type) {
    case 0:
        return 1;
    case 1:
        return D_0013DE48[arg] != 0;
    case 2:
        return D_0013D5C8_b[arg] != 0;
    case 3:
        return D_0013D5F0[arg] != 0;
    case 4:
        if (arg < 0x79) return D_0013D6B8_r[arg].d != 0;
        break;
    case 5:
        if (arg < 0x79) return D_0013D6B8_r[arg].d >= 2;
        break;
    case 6:
        return gSpecialItems[arg] != 0;
    case 7:
        return ((int (*)(void))arg)() != 0;
    case 8:
        return D_0014BFC0[arg >> 16][arg & 0xFFFF] != 0;
    case 9:
        break;
    }
    return 0;
}

typedef struct {
    short id;             /* 0x00 */
    char _pad02[0xE];
    unsigned short flags; /* 0x10 */
    short base;           /* 0x12 */
    short ids[8];         /* 0x14 */
    short status;         /* 0x24 */
    short sel;            /* 0x26 */
} MissionNode;
extern MissionNode *D_001A2FA0;

/* Lists the current objectives (the 0x28-byte node list at D_001A2FA0,
   ended by id 0): each node not hidden (flags & 2), with a status, and
   not an optional (flags & 1) one already done (status 2) goes to
   out[count] as its id (with arg3: 0x5243 when done, else ids[sel]),
   sets bit count of *mask when done and puts base + sel in *nums++.
   Bit 31 of *mask says every visible node is done. Returns the count.
   out is indexed by count: loop strength reduction then gives retail's
   pointer and its copy for the second store. */
int Mission_CollectObjectiveIds(int *out, int *mask, int *nums, int arg3) {
    MissionNode *p = D_001A2FA0;
    int count = 0;
    int all = 1;

    *out = 0;
    if (mask != 0) {
        *mask = 0;
    }
    if (nums != 0) {
        *nums = -1;
    }
    if (p == 0) {
        return 0;
    }
    while (p->id != 0) {
        unsigned short f = p->flags;
        short t = p->status;

        if (t != 2 && !(f & 2)) {
            all = 0;
        }
        if (!(f & 2) && t != 0 && !((f & 1) && t == 2)) {
            out[count] = p->id;
            if (arg3 != 0) {
                out[count] = (p->status == 2) ? 0x5243 : p->ids[p->sel];
            }
            if (mask != 0 && p->status == 2) {
                *mask |= 1 << count;
            }
            if (nums != 0) {
                *nums++ = p->base + p->sel;
            }
            count++;
        }
        p++;
    }
    if (mask != 0 && all != 0) {
        *mask |= 0x80000000;
    }
    return count;
}
__asm__(".section .text\n\tnop\n");

extern int D_0013D844 NOT_SDA;
extern unsigned char D_0013D4A8 NOT_SDA;

/* Predicate: true when D_0013D844 and the flag byte D_0013D4A8 are both set. */
int MissionCond_Obj18AndSpecial18(void) {
    if (D_0013D844 != 0 && D_0013D4A8 != 0) return 1;
    return 0;
}

extern int D_0013D9B4 NOT_SDA;

/* Predicate: true when D_0013D9B4 and special items 0x20 and 0x21 are all set. */
int MissionCond_Obj2FAndSpecials(void) {
    if (D_0013D9B4 != 0 && gSpecialItems[0x20] != 0 && gSpecialItems[0x21] != 0) return 1;
    return 0;
}

extern unsigned char D_0013D5CA NOT_SDA;
extern int D_0013D6B8 NOT_SDA;

/* Predicate: true when both mission words at D_0013D6B8 are non-zero. */
int MissionCond_Obj3FAnd40(void) {
    char *base = (char *)&D_0013D6B8;
    if (*(int *)(base + 0x40C) != 0 && *(int *)(base + 0x3FC) != 0) return 1;
    return 0;
}

extern int D_0013DAE4 NOT_SDA;
extern unsigned char D_0013D4E5 NOT_SDA;

/* Predicate: true when D_0013DAE4 and the flag byte D_0013D4E5 are both set. */
int MissionCond_Obj42AndSpecial55(void) {
    if (D_0013DAE4 != 0 && D_0013D4E5 != 0) return 1;
    return 0;
}

extern int D_0013DB24 NOT_SDA;
extern unsigned char D_0013D4F1 NOT_SDA;

/* Predicate: true when D_0013DB24 and the flag byte D_0013D4F1 are both set. */
int MissionCond_Obj46AndSpecial61(void) {
    if (D_0013DB24 != 0 && D_0013D4F1 != 0) return 1;
    return 0;
}

extern int D_0013DC34 NOT_SDA;
extern unsigned char D_0013D605 NOT_SDA;

/* Predicate: true when D_0013DC34 and the flag byte D_0013D605 are both set. */
int MissionCond_Obj57AndItem3D(void) {
    if (D_0013DC34 != 0 && D_0013D605 != 0) return 1;
    return 0;
}

extern int D_0013D5C8 NOT_SDA;

/* Predicate: true when two entries of the D_0013D5C8 table are both set. */
int MissionCond_Items1FAnd21(void) {
    unsigned char *base = (unsigned char *)&D_0013D5C8;
    if (base[0x21] != 0 && base[0x1F] != 0) return 1;
    return 0;
}

/* Returns whether entry arg0 of the D_0013D5C8 table is set. */
int HasItem(int arg0) {
    unsigned char *base = (unsigned char *)&D_0013D5C8;
    return base[arg0] != 0;
}

/* Two flat `&&` returns over D_0013D6B8_r[20/24/22].d; retail's reuse of
   the %hi register comes out by itself. */
int MissionCond_Obj14Obj18Stage(void) {
    if (D_0013D6B8_r[20].d != 0 && D_0013D6B8_r[24].d == 0) {
        return 1;
    }
    if (D_0013D6B8_r[24].d != 0 && gHaveHeliPack != 0 && D_0013D6B8_r[22].d == 0) {
        return 2;
    }
    return 0;
}

extern unsigned char D_0013D4B0 NOT_SDA;

/* Returns 0 when D_0013D9B4 is clear, else 2 or 1 depending on D_0013D4B0. */
int MissionCond_Obj2FVariant(void) {
    if (D_0013D9B4 != 0) {
        return D_0013D4B0 ? 2 : 1;
    }
    return 0;
}

extern unsigned char D_0013DE55 NOT_SDA;

/* Predicate: true when both D_0013D4F1 and D_0013DE55 are set. */
int MissionCond_Special61AndFlag0D(void) {
    if (D_0013D4F1 != 0 && D_0013DE55 != 0) return 1;
    return 0;
}

extern unsigned char D_0013D5DD NOT_SDA;

/* Returns 2 when the Morph-o-Ray is owned, else whether D_0013DC34 is set. */
int Mission_GetMorphORayStatus(void) {
    if (gHaveMorphORay != 0) return 2;
    return D_0013DC34 != 0;
}

extern unsigned char D_0013D5E7 NOT_SDA;

/* Returns whether the Hologuise is owned. */
int HasHologuise(void) {
    return gHaveHologuise != 0;
}

/* Returns 2 or the state of another entry of the D_0013D5C8 table, depending on the first. */
int Mission_GetHologuiseOrMapStatus(void) {
    unsigned char *base = (unsigned char *)&D_0013D5C8;
    if (base[0x1F] != 0) return 2;
    return base[0x21] != 0;
}

typedef struct {
    u8 pad0[0x24];
    s16 active;
    u8 pad26[2];
} MapIconLink_0CDE0;

typedef struct {
    f32 x;
    f32 y;
    f32 z;
    s32 flag;
} MapPoint_0CDE0;

typedef struct {
    u8 pad0[0x10];
    f32 x;
    f32 y;
    u8 pad18[0x30];
    f32 z;
} MapWorldObject_0CDE0;

struct TextRegion_0CDE0 {
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
/* One entry of a level's map icon list; the list ends at the first entry with flags bit 4. */
struct MapIcon_0CDE0 {
    s16 id;             /* 0x00: moby index (current level) or D_0013D6B8 index; -1..-9 are fixed positions */
    s16 link;           /* 0x02: -1 always active, else the link's active word decides */
    u16 flags;          /* 0x04: 0x4 ends the list, 0x10 label shown */
    u16 texture_id;     /* 0x06 */
    s16 frame_index;    /* 0x08 */
    s16 label_text_id;  /* 0x0A: 0 = no label */
    s16 label_item;     /* 0x0C */
    u16 label_width;    /* 0x0E */
    u16 label_height;   /* 0x10 */
    s16 label_offset_x; /* 0x12 */
    s16 label_offset_y; /* 0x14 */
    u8 pad16[0x2];
    f32 x;              /* 0x18: map position 0..1 */
    f32 y;              /* 0x1C */
    f32 angle;          /* 0x20 */
    s32 active;         /* 0x24 */
};
/* Map screen state at D_001A01F0, as far as this function reads it. */
struct MapState_0CDE0 {
    u8 pad0[0x20];
    struct MapIcon_0CDE0 *icons; /* 0x020: icon list of the shown level */
    u8 pad24[0xE0];
    s32 pan_x[20];               /* 0x104 */
    s32 pan_y[20];               /* 0x154 */
};
extern struct MapState_0CDE0 D_001A01F0_0CDE0 __asm__("D_001A01F0");
extern struct Hero D_0013F450_0CDE0 __asm__("D_0013F450");
extern s32 D_0015EE84_0CDE0 __asm__("D_0015EE84") MACRO_ADDR;
extern u8 D_0013DE60_0CDE0[] __asm__("D_0013DE60");
extern struct MapIcon_0CDE0 *D_001A2F40_0CDE0[] __asm__("D_001A2F40");
extern s32 D_001A02F4_0CDE0[] __asm__("D_001A02F4");
extern s32 D_0015FE20_0CDE0 __asm__("D_0015FE20") MACRO_ADDR;
extern MapWorldObject_0CDE0 *D_00199578_0CDE0[] __asm__("D_00199578");
extern MapPoint_0CDE0 D_0013D6B8_0CDE0[] __asm__("D_0013D6B8");
extern s32 D_0013D6C4_0CDE0[] __asm__("D_0013D6C4");
extern struct {
    u8 pad0[0x10];
    MapIconLink_0CDE0 *links;
} D_001A2F90_0CDE0 __asm__("D_001A2F90");

extern void func_00208C38_0CDE0(f32 *outx, f32 *outy, s32 view, f32 x,
                                f32 y) __asm__("func_00208C38");
extern void func_00208AB0_0CDE0(s32 idx, char *dst) __asm__("func_00208AB0");
extern void func_001F75D0_0CDE0(struct TextRegion_0CDE0 *, long, char *,
                                    int) __asm__("func_001F75D0");

/* Map screen icon refresh: picks the level's icon list, centres the map on the hero when
   it is the current level, places the fixed icons (-1..-9) and projects the others to map
   coordinates, then sets each icon's active flag and sizes its label box.
   Adapted from Lombyte (GPL-3.0) for PAL: src/ui/map/update_map_icons.c, update_map_icons. */
void Map_RefreshIcons(s32 level, s32 flag) {
    char label_text[128];
    f32 map_x;
    f32 map_y;
    struct MapIcon_0CDE0 *icon;
    s32 icon_index;

    if (level < 19 && D_0013DE60_0CDE0[level] != 0) {
        D_001A01F0_0CDE0.icons = D_001A2F40_0CDE0[level];
    } else {
        D_001A01F0_0CDE0.icons = 0;
    }

    if (D_0013F450_0CDE0.motion.pos.f[2] != 0.0f && level == D_0015EE84_0CDE0 && flag) {
        func_00208C38_0CDE0(&map_x, &map_y, D_0015FE20_0CDE0 ? level + 100 : level, D_0013F450_0CDE0.motion.pos.f[0],
                            D_0013F450_0CDE0.motion.pos.f[1]);
        D_001A01F0_0CDE0.pan_x[level] = (s32)(map_x * 4096.0f) << 16;
        D_001A01F0_0CDE0.pan_y[level] = (s32)(map_y * 4096.0f) << 16;
    } else {
        /* D_001A02F4_0CDE0 is &D_001A01F0_0CDE0.posx: a null-guarded reset that can
           never run, but retail still emits it with p folded to 0. */
        s32 *p = D_001A02F4_0CDE0;
        if (p == 0) {
            p[level] = 0x8000000;
            p[level + 20] = 0x8000000;
        }
    }

    if (D_001A01F0_0CDE0.icons == 0) {
        return;
    }

    if (!(D_001A01F0_0CDE0.icons->flags & 4)) {
        icon_index = 0;
        do {
            icon = &D_001A01F0_0CDE0.icons[icon_index];
            if (icon->id == -1) {
                icon->x = 0.234375f;
                icon->y = 0.30078125f;
            } else if (icon->id == -2) {
                icon->x = 0.5390625f;
                icon->y = 0.365234375f;
            } else if (icon->id == -3) {
                icon->x = 0.58203125f;
                icon->y = 0.6875f;
            } else if (icon->id == -4) {
                icon->x = 0.720703125f;
                icon->y = 0.728515625f;
            } else if (icon->id == -5) {
                icon->x = 0.384765625f;
                icon->y = 0.396484375f;
            } else if (icon->id == -7) {
                icon->x = 0.8671875f;
                icon->y = 0.23046875f;
            } else if (icon->id == -8) {
                icon->x = 0.48046875f;
                icon->y = 0.5703125f;
            } else if (icon->id == -9) {
                icon->x = 0.625f;
                icon->y = 0.72265625f;
            } else {
                if (level == D_0015EE84_0CDE0) {
                    if (D_00199578_0CDE0[icon->id] != 0) {
                        func_00208C38_0CDE0(&icon->x, &icon->y, level, D_00199578_0CDE0[icon->id]->x,
                                            D_00199578_0CDE0[icon->id]->y);
                        D_001A01F0_0CDE0.icons[icon_index].angle = D_00199578_0CDE0[icon->id]->z;
                    }
                } else {
                    func_00208C38_0CDE0(&icon->x, &icon->y, level, D_0013D6B8_0CDE0[icon->id].x,
                                        D_0013D6B8_0CDE0[icon->id].y);
                    D_001A01F0_0CDE0.icons[icon_index].angle = D_0013D6B8_0CDE0[icon->id].z;
                }
            }
            icon_index++;
        } while (!(D_001A01F0_0CDE0.icons[icon_index].flags & 4));
    }

    for (icon_index = 0; !(D_001A01F0_0CDE0.icons[icon_index].flags & 4); icon_index++) {
        icon = &D_001A01F0_0CDE0.icons[icon_index];
        icon->flags &= ~0x10;
        icon->active = 0;
        if (icon->link == -1) {
            icon->active = 1;
        } else {
            icon->active = D_001A2F90_0CDE0.links[icon->link].active == 1;
        }
        if ((icon->flags & 0x1000) && (D_0013D6C4_0CDE0[icon->id * 4] ^ 1) & 1) {
            icon->active = 0;
        }
        if (D_001A01F0_0CDE0.icons[icon_index].label_text_id != 0) {
            s32 label_height_changed = 0;
            s16 previous_label_height;

            D_001A01F0_0CDE0.icons[icon_index].flags |= 0x10;
            func_00208AB0_0CDE0(icon_index, label_text);
            {
                struct TextRegion_0CDE0 text_window = {
                    0, icon->label_height, 0, icon->label_width, 4, 4, 0, 0, 0xF, 4};
                func_001F75D0_0CDE0(&text_window, 0x80FFA888L, label_text, -1);
                icon->label_height = text_window.rendered_height + 8;
                previous_label_height = text_window.rendered_height;
                do {
                    text_window.right -= 4;
                    func_001F75D0_0CDE0(&text_window, 0x80FFA888L, label_text, -1);
                    if (text_window.rendered_height != previous_label_height) {
                        label_height_changed = 1;
                    }
                } while (!label_height_changed);
                icon->label_width = text_window.right + 4;
            }
        }
    }
}
__asm__(".section .text\n\tnop\n\tnop\n");
