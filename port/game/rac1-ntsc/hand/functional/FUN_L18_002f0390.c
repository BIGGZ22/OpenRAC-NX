/* Carried from the PAL decompilation's src/overlays/l18_veldin2/vendor_002A8400.c (matched C there) (games/rac1) through the version
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
/* An animation sequence header: only the frame count is read. */
typedef struct {
    char _pad00[0x10];
    unsigned char nframes;
} AnimSeq_c2f0390;
/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c2f0390;
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c2f0390 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2f0390 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2f0390;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2f0390 {
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
    struct AnimSeq_c2f0390 *seqs[1];          /* animation sequences, indexed by Moby.seq */
};
struct Moby_c2f0390 {
    Vec4f_c2f0390 bsphere;
    Vec4f_c2f0390 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2f0390 *pclass;
    struct Moby_c2f0390 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2f0390 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c2f0390 *moby);
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
    Vec4f_c2f0390 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2f0390 unkD0;
    Vec4f_c2f0390 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2f0390 Moby_c2f0390;
typedef struct MobyClass_c2f0390 MobyClass_c2f0390;
extern int func_001FA6E0(int, int, float);
extern int func_001F44B8(int);
extern void func_L00_001FD228(void *, void *, int);
extern int *D_L18_001AC240[];
extern int D_L18_0015F5CC MACRO_ADDR;
extern f32 func_001FA6C0(s32);
extern void func_001F9A10(void *, void *, void *);
extern short D_L18_00162238;
struct GeometryQuad_1780_c2f0390 {
    Vec4_c2f0390 positions[4];
    u32 colors[4];
    f32 texture_coordinates[4][2];
    union {
        u64 value;
        volatile u64 ordered;
    } reserved;
    u64 texture;
    u64 texture_state;
    u64 primitive;
};
extern short D_L18_0016223C;
extern short D_L18_00162240;
extern short D_L18_00162244;
extern short D_L18_00162248;
extern short D_L18_0016224C;
extern short D_L18_00162250;
extern short D_L18_00162254;
extern void func_001153FC(void *, int, int);
extern float D_L18_001DA2F0[4][2];
extern char *D_L18_0015FFD8 MACRO_ADDR;


void FUN_L18_002f0390(struct Moby_c2f0390 *moby) {
    struct GeometryQuad_1780_c2f0390 quads[16];
    float extra[8];
    short *p;
    struct Moby_c2f0390 *m;
    char *data;
    float offset;
    float t;
    int i;
    int j;
    int k;
    int n;
    int base;

    if (*(int *)&D_L18_00162238 != 0) {
        return;
    }
    for (i = 0; i < 16; i++) {
        quads[i].texture = ((long (*)(int))func_001F44B8)(19);
        quads[i].reserved.value = 0;
        quads[i].texture_state = 0xFF9000000260ULL;
        quads[i].primitive = (u64)*(int *)&D_L18_00162240 | ((u64)*(int *)&D_L18_00162244 << 2) | ((u64)*(int *)&D_L18_00162248 << 4) |
                             ((u64)*(int *)&D_L18_0016224C << 6) | 0x8000000000ULL;
        offset = func_001FA6C0((D_L18_0015F5CC + 3) & 3) * 0.0625f;
        for (j = 0; j < 4; j++) {
            quads[i].texture_coordinates[j][0] = D_L18_001DA2F0[j][0];
            quads[i].texture_coordinates[j][1] = D_L18_001DA2F0[j][1];
            t = func_001FA6C0(i + 1 - (j >> 1)) * 0.25f + offset;
            if (t > 1.0f) {
                t = 1.0f;
            } else if (t < 0.0f) {
                t = 0.0f;
            }
            quads[i].colors[j] = func_001FA6E0(*(int *)&D_L18_00162250, *(int *)&D_L18_00162254, t);
        }
    }
    func_001153FC(extra, 0, 0x20);
    extra[2] = 0.15f;
    extra[6] = -0.15f;
    p = (short *)D_L18_001AC240[moby->group];
    do {
    again:
        m = (struct Moby_c2f0390 *)(((*p & 0x7FFF) << 8) + (int)D_L18_0015FFD8);
        if (m->oclass != 0x54B) {
            goto again;
        }
        data = (char *)m->pvars;
        for (n = 0; n < *(short *)(data + 0x164); n++) {
            base = (*(short *)(data + 0x166) - n + 15) & 15;
            if (*(int *)&D_L18_0016223C == 0) {
                for (k = 0; k < 4; k++) {
                    func_001F9A10(&quads[n].positions[k], data + 0x10 + (((base + (k >> 1)) & 15) << 4),
                                   extra + ((k & 1) << 2));
                }
                func_L00_001FD228(&quads[n], 0, 0);
            }
        }
    } while (*p++ >= 0);
}
