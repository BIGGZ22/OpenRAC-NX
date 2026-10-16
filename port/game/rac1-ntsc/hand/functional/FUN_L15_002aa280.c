/* Carried from the PAL decompilation's src/overlays/l15_quartu/vendor_0029C1D0.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c2aa280;
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c2aa280 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2aa280 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2aa280;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2aa280 {
    u8 pad_0[0xC];
    u8 seq_count;                     /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 unk10;                        /* copied into Moby.unk94 when a moby is (re)classed */
    u8 pad_14[0x8];
    void *unk1C;                      /* word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs;            /* patched by patch_moby_gifs */
    f32 scale;                        /* default draw scale: copied into Moby.scale, divides it */
    s32 unk28;
    void **callbacks;                 /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags;                        /* initial Moby.flags */
    s16 unk46;                        /* class category; 5 is tested by targeting code */
    struct AnimSeq *seqs[1];          /* animation sequences, indexed by Moby.seq */
};
struct Moby_c2aa280 {
    Vec4f_c2aa280 bsphere;
    Vec4f_c2aa280 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2aa280 *pclass;
    struct Moby_c2aa280 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2aa280 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
    u8 frame;                         /* animation frame */
    u8 prev_frame;                    /* frame index in prev_seq */
    u8 seq;                           /* animation sequence id */
    u8 prev_seq;
    f32 unk54;
    f32 unk58;
    u8 pad5C[8];
    struct Manip *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8 unk70;
    u8 unk71;                         /* set to 0xFF when a moby changes class */
    u8 unk72;
    u8 unk73;
    void (*update)(struct Moby_c2aa280 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 pad7D;
    u8 unk7E;
    u8 unk7F;                         /* set to 0x17 by FUN_L09_002c5990 near D_L09_00166EC0 */
    s32 unk80;                        /* colour from the level's moby records, c2 << 16 | c1 << 8 | c0 (func_L00_00241940) */
    u8 pad84[2];
    s16 unk86;                        /* compared with 5 (func_L02_002D8AD0) */
    u8 pad88[8];
    s32 unk90;
    u32 unk94;                        /* set from the class header's word 0x10 */
    s32 unk98;                        /* set to 1 while a carrier holds the moby (FUN_L00_002c7a58) */
    u8 pad9C[8];
    u8 unkA4;
    u8 padA5;
    s16 oclass;
    u8 padA8[8];
    u8 unkB0;                         /* 0xB0: index into the level's D_0014BF50 row (0xFF: not spawned) */
    u8 unkB1;                         /* set from the level's moby records (func_L00_00241940) */
    u16 save_id;                      /* index into the level collected[]/killed[] tables and save bits D_0014C090[level][id >> 5] */
    s16 unkB4;
    s16 unkB6;                        /* set from the level's moby records (func_L00_00241940) */
    void *unkB8;                      /* 0xB8: bolt source record; its byte 0xB1 is a per-level id (FUN_L00_002a6b70) */
    u8 unkBC;
    u8 padBD[3];
    Vec4f_c2aa280 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2aa280 unkD0;
    Vec4f_c2aa280 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2aa280 Moby_c2aa280;
typedef struct MobyClass_c2aa280 MobyClass_c2aa280;
extern f32 D_0015ED6C MACRO_ADDR;
extern char D_L15_00178700[];
extern char D_L15_001615B8[];
extern u32 D_L15_001615C8[3];
extern unsigned char D_0013E533[];
extern unsigned char D_0013DD4D[];
extern unsigned char D_0014161B[] NOT_SDA;
extern int D_0015ED84 MACRO_ADDR;
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A68(void *, void *, float);
extern float func_001F9AF0(void *);
extern float func_002132A8(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_L00_001FF290(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern int func_00213260(int);
extern int func_L00_00257B90(int, int);
extern void *func_L01_002F8530(void *, void *, int, float, int, float, float, float, int);
extern void func_001F99F8(void *);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_002371E0(float, int, int);
extern int func_001F9688(int);
extern void *func_L00_0026BED0(void *, void *, int, int, float, int, int);
extern void func_L00_00257470(void *, int, int);
extern void func_0020C828(void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, unsigned char);



void FUN_L15_002aa280(struct Moby_c2aa280 *moby) {
    char *hit = 0;
    Vec4_c2aa280 velocity, hit_direction, push, offset, center;
    u32 palette[4];
    u32 colors[4];
    Vec4_c2aa280 delta;
    int spark, puff, pick, n;
    u32 *color;
    float size;
    void *debris;

    if (D_0013E533[0x2EC1] != 2 && moby->state == 1)
        return;

    for (n = 0; n < 64; n++) {
        char *entry = D_L15_00178700 + n * 0x40;
        if (*(struct Moby_c2aa280 **)(entry + 0x34) != moby)
            continue;
        if ((*(u32 *)(entry + 0x24) & 0x80000) == 0)
            continue;
        hit = entry;
        break;
    }
    *((u8 *)moby + 0xA4) = 0xff;
    if (hit != 0) {
        __builtin_memcpy(palette, D_L15_001615B8, 16);
        func_L00_001FF500(&hit_direction, hit + 0x10, D_0015ED6C * 10.0f);
        func_001F9A68(&push, &hit_direction, 2.0f);
        if (func_001F9AF0(hit) != 0.0f) {
            center.q = *(VecQuad_c2aa280 *)hit;
        } else {
            center.q = *(VecQuad_c2aa280 *)&moby->pos;
            center.f[2] += 6.0f;
        }

        for (spark = 0; spark < 25; spark++) {
            float radius, angle;
            int life;
            angle = func_002132A8(moby->rot.z, 1.5707963267948966f);
            radius = func_001F9DC8(angle);
            radius *= func_002132A8(-3.0f, 3.0f);
            offset.f[0] = radius;
            angle = func_002132A8(moby->rot.z, 1.5707963267948966f);
            radius = func_001F9DE0(angle);
            radius *= func_002132A8(-3.0f, 3.0f);
            offset.f[1] = radius;
            offset.f[2] = 0.0f;
            offset.f[2] = func_002132A8(1.0f, 7.0f);
            func_L00_001FF290(colors, &offset, &moby->pos);
            func_001F9A28(colors, &center, &offset);
            velocity.q = *(VecQuad_c2aa280 *)colors;
            func_L00_001FF500(&velocity, &velocity, D_0015ED6C * 5.0f);
            func_L00_001FF290(colors, &velocity, &push);
            velocity.f[2] += D_0015ED6C * 3.0f;
            size = func_002132A8(0.75f, 1.25f);
            func_001F9A68(&velocity, &velocity, size);
            pick = func_00213260(4);
            color = &palette[pick];
            radius = func_002132A8(0.1f, 0.3f);
            life = func_L00_00257B90(100, 160);
            size = func_002132A8(3.0f, 7.0f);
            func_L01_002F8530(&offset, &velocity, *color, radius, life, size, 0.15f, 2.0f, 1);
        }
        func_001F99F8(&velocity);
        func_L00_0025E450(moby, &velocity, &center, 0.0f, 0.0f, 20, 3, 4, 10.0f, 5.0f, 100000.0f, 3.0f, 0,
                         15.0f, 1, 1, -1, 0);

        for (puff = 0; puff < 20; puff++) {
            float radius, angle;
            int color1, color2, life;
            __builtin_memcpy(colors, D_L15_001615C8, 12);
            angle = func_002132A8(moby->rot.z, 1.5707963267948966f);
            radius = func_001F9DC8(angle);
            radius *= func_002132A8(-3.0f, 3.0f);
            offset.f[0] = radius;
            angle = func_002132A8(moby->rot.z, 1.5707963267948966f);
            radius = func_001F9DE0(angle);
            radius *= func_002132A8(-3.0f, 3.0f);
            offset.f[1] = radius;
            offset.f[2] = 0.0f;
            func_001F9A10(&offset, &offset, &moby->pos);
            offset.f[2] += func_002132A8(1.0f, 5.5f);
            func_001F9A28(&delta, &center, &offset);
            velocity.q = delta.q;
            func_L00_001FF500(&velocity, &velocity, D_0015ED6C * 5.0f);
            func_L00_001FF290(&delta, &velocity, &hit_direction);
            size = func_002132A8(0.75f, 1.25f);
            func_001F9A68(&velocity, &velocity, size);
            size = func_002132A8(0.25f, 1.0f);
            color1 = func_L00_002371E0(size, 0x7f000000, 0x7f182030);
            size = func_002132A8(0.5f, 1.0f);
            color2 = func_L00_002371E0(size, 0, 0x5f5f5f);
            size = func_002132A8(1.0f, 3.0f) * 210000.0f;
            life = func_001F9688(180);
            pick = func_00213260(3);
            debris = func_L00_0026BED0(&offset, &velocity, color1, color2, size, life, colors[pick]);
            if (debris != 0)
                *(float *)((char *)debris + 0x2c) = moby->pos.z - 0.5f;
        }
        func_L00_00257470(moby, 0, -1);
        func_0020C828(moby);
    } else {
        *((u8 *)moby + 0xA4) = 0xff;
        if (moby->state != 0)
            return;
        moby->unk32 = 0x80;
        moby->unk30 = 0xff;
        moby->state = 1;
        if (D_0013DD4D[3] != 0) {
            u16 save_id = moby->save_id;
            if ((*(s32 *)(D_0014161B + 0xAB75 + (((s16)save_id >> 5) * 4) + D_0015ED84 * 256) >> (save_id & 31)) & 1)
                func_0020C828(moby);
        }
    }
}
