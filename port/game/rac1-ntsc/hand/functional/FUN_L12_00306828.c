/* Carried from the PAL decompilation's src/overlays/l12_hoven/vendor_002EDAA0.c (matched C there) (games/rac1) through the version
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
} Vec4f_c306828;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c306828 {
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
struct Moby_c306828 {
    Vec4f_c306828 bsphere;
    Vec4f_c306828 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c306828 *pclass;
    struct Moby_c306828 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c306828 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c306828 *moby);
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
    Vec4f_c306828 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c306828 unkD0;
    Vec4f_c306828 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c306828 Moby_c306828;
typedef struct MobyClass_c306828 MobyClass_c306828;
extern float func_001F9DE0(float);
extern Moby_c306828 *D_L12_00160058_303CA8 __asm__("D_L12_0015FFD8") MACRO_ADDR;
extern int func_001F9688(int);
extern int D_L12_0015F5CC MACRO_ADDR;
extern int func_001FA6D0(float);
extern float func_001FA6C0(int);



void FUN_L12_00306828(char *moby) {
    int i;
    int alpha = 0x80000000;
    char *data = *(char **)(moby + 0x78);
    for (i = 0; i < 18; i++) {
        char *other;
        int period;
        float sine;
        float color;
        int red;
        int green;
        int blue;
        int packed_color;
        float phase;
        float divisor;
        if (i == 17) {
            other = moby;
        } else {
            other = (char *)D_L12_00160058_303CA8 + ((unsigned int)*(int *)(data + i * 0x30 + 0x20) << 8);
        }
        if (*(short *)(other + 0xA6) == 0x4FE) continue;
        {
            period = func_001F9688(0xAA);
            divisor = func_001FA6C0(period);
            phase = (float)(D_L12_0015F5CC % period) / divisor;
            sine = func_001F9DE0((phase + phase) * 3.1415927f + -3.1415927f);
            color = sine * 70.0f;
            red = func_001FA6D0(color);
            green = func_001FA6D0(color);
            blue = func_001FA6D0(sine * 10.0f);
            packed_color = ((blue + 0x46) << 16) | alpha |
                ((green + 0xB4 > 0xFF ? 0xFF : green + 0xB4) << 8) |
                (red + 0xB4 > 0xFF ? 0xFF : red + 0xB4);
            *(unsigned short *)(other + 0x34) |= 0x10;
            *(int *)(other + 0x90) = packed_color;
        }
    }
}
