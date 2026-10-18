#ifndef MOBY_H
#define MOBY_H

/*
 * The moby: the game's 0x100-byte object record, and its class header.
 * One definition for every source file, in place of the private copies
 * candidates carried to land without clashing with their files.
 *
 * Adapted from Lombyte (MIT), the US decompilation:
 * include/rnc/gameplay/entities/moby.h at 88c78925. Its layout and
 * field names come from the matched code it cites; the PAL code is the
 * same structure. See THIRD_PARTY_NOTICES.md.
 */

#include "common.h"
#include "vector.h"


struct Manip;
struct GifEntry;
struct AnimSeq;

/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass {
    u8 pad_0[0xC];
    u8 seq_count;                     /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision;                    /* collision data; copied into Moby.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table;             /* word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs;            /* patched by patch_moby_gifs */
    f32 scale;                        /* default draw scale: copied into Moby.scale, divides it */
    s32 sound_defs;
    void **callbacks;                 /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags;                        /* initial Moby.flags */
    s16 type;                         /* class type; 5 is tested by targeting code */
    struct AnimSeq *seqs[1];          /* animation sequences, indexed by Moby.seq */
};

struct Moby {
    Vec4f bsphere;
    Vec4f pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot;                    /* class slot: pclass = D_L00_00197300[class_slot] (FUN_L00_002cf218) */
    u8 alpha;                         /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass *pclass;
    struct Moby *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist;                   /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible;                       /* drawn last frame; set to 1 by spawners */
    s16 draw_dist;                    /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion;                    /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
    u8 frame;                         /* animation frame */
    u8 prev_frame;                    /* frame index in prev_seq */
    u8 seq;                           /* animation sequence id */
    u8 prev_seq;
    f32 anim_time;
    f32 anim_speed;
    u8 pad5C[8];
    struct Manip *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8 anim_flags;
    u8 unk71;                         /* set to 0xFF when a moby changes class */
    u8 unk72;
    u8 unk73;
    void (*update)(struct Moby *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot;                    /* D_0013E550 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range;                  /* shadow range in units of 1024; set to 0x17 by FUN_L09_002c5990 near D_L09_00166F40 */
    s32 unk80;                        /* colour from the level's moby records, c2 << 16 | c1 << 8 | c0 (func_L00_002422D8) */
    u8 pad84[2];
    s16 unk86;                        /* compared with 5 (func_L02_002D9F08) */
    u8 pad88[8];
    s32 glow_color;                   /* glow colour (class header + 0x40) */
    u32 collision;                    /* collision data, from the class header (MobyClass.collision) */
    s32 collision_disable;            /* per-primitive collision disable bits; 1 while a carrier holds the moby (FUN_L00_002c7a58) */
    u8 pad9C[8];
    u8 hit_slot;                      /* hit-message slot (0xFF: none) */
    u8 padA5;
    s16 oclass;
    u8 padA8[8];
    u8 mission;                       /* 0xB0: index into the level's D_0014C050 row (0xFF: not spawned) */
    u8 unkB1;                         /* set from the level's moby records (func_L00_002422D8) */
    u16 save_id;                      /* index into the level collected[]/killed[] tables and save bits D_0014C190[level][id >> 5] */
    s16 unkB4;
    s16 unkB6;                        /* set from the level's moby records (func_L00_002422D8) */
    void *parent;                     /* 0xB8: spawner (bolt source record); its byte 0xB1 is a per-level id (FUN_L00_002a6b70) */
    u8 command;                       /* external command byte */
    u8 padBD[3];
    Vec4f rot_row0;                   /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f rot_row1;
    Vec4f rot_row2;
    u8 padF0[0x10];
};

typedef struct Moby Moby;
typedef struct MobyClass MobyClass;

#endif /* MOBY_H */
