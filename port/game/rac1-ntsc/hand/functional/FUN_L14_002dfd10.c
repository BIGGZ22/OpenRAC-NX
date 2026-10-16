/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B2A28.c (matched C there) (games/rac1) through the version
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
extern int func_L00_0025D808(void *, void *, void *, void *, int, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *dst, void *a, void *b);
typedef u32 u128_E1110_c2dfd10 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_E1110_c2dfd10;
typedef union {
    u128_E1110_c2dfd10 q;
    f32 f[4];
    s32 i[4];
} Vec4_E1110_c2dfd10;
extern f32 D_0015ED60 MACRO_ADDR;
extern f32 D_0015ED64 MACRO_ADDR;
struct MobyClass_E1110_c2dfd10 {
    u8 pad_0[0xC];
    u8 seq_count; /* number of entries in seqs[] (menu previews clamp the sequence to it) */
    u8 pad_D[3];
    u32 collision; /* collision data; copied into Moby.collision when a moby is (re)classed */
    u8 pad_14[0x8];
    void *retarget_table; /* retarget table: word table, indexed id * 4 + 4 (0024eec0) */
    struct GifEntry *gifs; /* patched by patch_moby_gifs */
    f32 scale; /* default draw scale: copied into Moby.scale, divides it */
    s32 sound_defs; /* sound definitions, 0x20 bytes each */
    void **callbacks; /* function-pointer table, called with the moby */
    u8 pad_30[0x14];
    u16 flags; /* initial Moby.flags */
    s16 type; /* class type; 5 (a creature) is tested by targeting code */
    struct AnimSeq *seqs[1]; /* animation sequences, indexed by Moby.seq */
};
struct Moby_E1110_c2dfd10 {
    Vec4f_E1110_c2dfd10 bsphere;
    Vec4f_E1110_c2dfd10 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00196F80[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_E1110_c2dfd10 *pclass;
    struct Moby_E1110_c2dfd10 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_E1110_c2dfd10 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
    u8 frame; /* animation frame */
    u8 prev_frame; /* frame index in prev_seq */
    u8 seq; /* animation sequence id */
    u8 prev_seq;
    f32 anim_time;
    f32 anim_speed;
    u8 pad5C[8];
    struct Manip *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8 anim_flags;
    u8 unk71; /* set to 0xFF when a moby changes class */
    u8 unk72;
    u8 unk73;
    void (*update)(struct Moby_E1110_c2dfd10 *moby);
    u8 *pvars;
    u8 unk7C;
    u8 sound_slot; /* 0x7D: D_0013E450 slot of its looping sound, 0xFF: none (FUN_L00_00232670) */
    u8 anim_trigger_count;
    u8 shadow_range; /* shadow range in units of 1024; 0x17 from FUN_L09_002c5990 near D_L09_00166EC0 */
    u8 pad80[6];
    s16 unk86;
    u8 pad88[8];
    s32 glow_color; /* glow colour (class header + 0x40) */
    u32 collision; /* collision data, from the class header (MobyClass.collision) */
    s32 collision_disable; /* per-primitive collision disable bits; 1 while a carrier holds the moby (FUN_L00_002c7a58) */
    u8 pad9C[8];
    u8 hit_slot; /* hit-message slot (0xFF: none) */
    u8 padA5;
    s16 oclass;
    u8 padA8[8];
    u8 mission; /* 0xB0: index into the level's D_0014BF50 row (0xFF: not spawned) */
    u8 padB1;
    u16 save_id; /* index into the level collected[]/killed[] tables and save bits D_0014C090[level][id >> 5] */
    s16 unkB4;
    u8 padB6[2];
    void *parent; /* 0xB8: spawner (bolt source record); its byte 0xB1 is a per-level id (FUN_L00_002a6b70) */
    u8 command; /* external command byte */
    u8 padBD[3];
    Vec4f_E1110_c2dfd10 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_E1110_c2dfd10 rot_row1;
    Vec4f_E1110_c2dfd10 rot_row2;
    u8 padF0[0x10];
};
extern void func_L14_002E17B8_E1110(struct Moby_E1110_c2dfd10 *) __asm__("func_L14_002E03B8");
extern s32 D_001413D0;
extern s32 func_001F9908_E1110(s32 *) __asm__("func_001F9740");
extern f32 func_001F99C0(f32);
extern void func_L00_001FF500(void *, void *, f32);
extern f32 func_001FA580(f32, f32);
extern s32 func_L00_001F0D60(f32, void *, s32, void *);
extern void func_L00_0025F4A8_E1110(struct Moby_E1110_c2dfd10 *, void *, void *, f32, f32, s32, s32, s32, f32, f32, f32, f32, s32, f32, s32, s32, s32, s32) __asm__("func_L00_0025E450");
extern void func_0020D678_E1110(struct Moby_E1110_c2dfd10 *) __asm__("func_0020C828");
extern int func_00213260(int);
extern int func_L00_0028DC90(int, int, struct Moby_E1110_c2dfd10 *, int);
typedef struct {
    u8 pad0[0x10];
    void *points;
    u8 pad14[0xC];
} RacePath_dfd10_c2dfd10;
extern RacePath_dfd10_c2dfd10 * D_L14_0015F70C MACRO_ADDR;
typedef struct {
    struct Moby_E1110_c2dfd10 *owner;
    s32 timer;
    f32 path_speed;
    s32 path;
    f32 path_t;
    f32 path_u;
    f32 offset_x;
    f32 offset_y;
    f32 spin_x;
    f32 spin_y;
    f32 spin_z;
    u8 pad2C[4];
    f32 velocity[3];
} Data_dfd10_c2dfd10;
extern short D_L14_00161B40;
extern short D_L14_00161B44;
extern short D_L14_00161B48;
typedef struct Moby_E1110_c2dfd10 Moby_E1110_c2dfd10;



/* A piece of debris: follows its race path until its timer runs out, then flies, spins and falls until it hits something or leaves the map.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/entities/002b17d8.c, FUN_L14_002dfd10. */
void FUN_L14_002dfd10(struct Moby_E1110_c2dfd10 *moby) {
    Data_dfd10_c2dfd10 *data = (Data_dfd10_c2dfd10 *)moby->pvars;
    Vec4_E1110_c2dfd10 probe;
    f32 vec[4];
    f32 *velocity;
    void *points;
    f32 speed;

    if (moby->state == 0) {
        points = D_L14_0015F70C[data->path].points;
        qcopy(&probe, &moby->pos);
        func_L00_0025D808(points, &moby->pos, &data->path_t, &data->path_u, 0, data->path_speed);
        func_001F9A28(vec, &moby->pos, &probe);
        data->offset_x = vec[0];
        data->offset_y = vec[1];
        func_L14_002E17B8_E1110(moby);
        if (func_001F9908_E1110(&data->timer) == 0)
            return;
        func_L00_001FF500(vec, vec, 0.5f);
        qcopy(&probe, &moby->pos);
        probe.f[2] += 0.25f;
        func_L00_0028DC90(1, 0, moby, data->owner->oclass);
        func_L00_0025F4A8_E1110(moby, vec, &probe, 0.0f, 0.0f, 5, 2, 4, 1.0f, 0.5f, 9.0f, 1.0f, -1, 15.0f, 0, 3, -1, 0);
        func_L00_001F0D60(1.0f, &moby->pos, 0, (void *)D_001413D0);
        func_0020D678_E1110(moby);
        return;
    }
    velocity = data->velocity;
    func_001F9A10(&moby->pos, &moby->pos, velocity);
    speed = func_001F99C0(data->velocity[0]);
    if ((*(f32 *)&D_L14_00161B48) * D_0015ED60 < speed)
        data->velocity[0] *= ((*(f32 *)&D_L14_00161B44) - 1.0f) * D_0015ED60 + 1.0f;
    speed = func_001F99C0(data->velocity[1]);
    if ((*(f32 *)&D_L14_00161B48) * D_0015ED60 < speed)
        data->velocity[1] *= ((*(f32 *)&D_L14_00161B44) - 1.0f) * D_0015ED60 + 1.0f;
    if (data->velocity[2] > 0.0f)
        data->velocity[2] *= ((*(f32 *)&D_L14_00161B44) - 1.0f) * D_0015ED60 + 1.0f;
    data->velocity[2] -= (*(f32 *)&D_L14_00161B40) * D_0015ED64;
    moby->rot.x = func_001FA580(moby->rot.x, data->spin_x);
    moby->rot.y = func_001FA580(moby->rot.y, data->spin_y);
    moby->rot.z = func_001FA580(moby->rot.z, data->spin_z);
    if (moby->pos.x < 2.0f || 1021.0f < moby->pos.x ||
        moby->pos.y < 2.0f || 1021.0f < moby->pos.y ||
        moby->pos.z < 2.0f || 1021.0f < moby->pos.z) {
        func_0020D678_E1110(moby);
        return;
    }
    func_L00_001FF500(vec, &moby->rot_row2, 0.52f);
    func_001F9A10(&probe, &moby->pos, vec);
    if (func_001F9908_E1110(&data->timer) != 0 || func_L00_001F0D60(0.5f, &probe, 0, data->owner) != 0) {
        func_L00_0028DC90(0, 0, moby, data->owner->oclass);
        func_L00_0025F4A8_E1110(moby, velocity, &moby->pos, 0.0f, 0.0f, 5, 2, 4, 1.0f, 0.5f, 9.0f, 1.0f, -1, 15.0f, 0, 3, -1, 0);
        func_L00_001F0D60(1.0f, &moby->pos, 0, (void *)D_001413D0);
        func_0020D678_E1110(moby);
        return;
    }
    if (func_00213260(2) != 0)
        func_L14_002E17B8_E1110(moby);
}
