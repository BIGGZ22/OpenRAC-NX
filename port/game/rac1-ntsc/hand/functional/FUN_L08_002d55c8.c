/* Carried from the PAL decompilation's src/overlays/l08_batalia/vendor_002B9438.c (matched C there) (games/rac1) through the version
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
extern int func_001F9688(int);
extern float func_L00_001FF8B0(float, float);
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_D68D0_c2d55c8;
extern f32 D_0015ED6C MACRO_ADDR;
extern f32 D_0015ED70 MACRO_ADDR;
struct MobyClass_D68D0_c2d55c8 {
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
struct Moby_D68D0_c2d55c8 {
    Vec4f_D68D0_c2d55c8 bsphere;
    Vec4f_D68D0_c2d55c8 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00196F80[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_D68D0_c2d55c8 *pclass;
    struct Moby_D68D0_c2d55c8 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_D68D0_c2d55c8 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_D68D0_c2d55c8 *moby);
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
    Vec4f_D68D0_c2d55c8 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_D68D0_c2d55c8 rot_row1;
    Vec4f_D68D0_c2d55c8 rot_row2;
    u8 padF0[0x10];
};
typedef struct Moby_D68D0_c2d55c8 Moby_D68D0_c2d55c8;
extern int D_L08_001B0CB0[];
extern char D_0013E550[];
extern short D_L08_001618A0;
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern void func_00214D28_D68D0(float *, float, float) __asm__("func_00213ED8");
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern float func_00213508(void *, int, float);
extern float func_001F9B80(void *, void *);
extern float func_001F9AF0(void *);
extern int func_L00_0028D8C0(void *, int);
extern int func_0022DA68(int, int, void *);
extern void func_L00_0028DF38(int, int);
extern void func_L00_0028D918(int);
typedef struct {
    u8 pad0[0x80];
    int volume;
} SoundSlot_2d55c8_c2d55c8;
typedef struct {
    u8 pad0[0x40];
    float velocity[4];
    u8 pad50[0x108];
    float turn_speed;
    int sound;
    void *linked[2];
    int point;
    float target_speed;
    u8 pad170[0x10];
    float speed;
} PathMoverVars_2d55c8_c2d55c8;



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l08/gameplay/entities/002b8228.c, FUN_L08_002d55c8. */
void FUN_L08_002d55c8(struct Moby_D68D0_c2d55c8 *moby, int path_id) {
    PathMoverVars_2d55c8_c2d55c8 *vars = (PathMoverVars_2d55c8_c2d55c8 *)moby->pvars;
    int *path = (int *)D_L08_001B0CB0[path_id];
    float point[4];
    float step[3];
    float scale;
    float angle;
    float speed;
    float *heading;

    qcopy(point, path + 4 + vars->point * 4);
    heading = &moby->rot.z;
    angle = func_L00_001FF8B0(point[0] - moby->pos.x, point[1] - moby->pos.y);
    func_L00_0025BE00(heading, &vars->turn_speed, angle,
        D_0015ED70 * 4.712389f, D_0015ED70 * 4.712389f, D_0015ED6C * 3.1415927f);
    func_00214D28_D68D0(&vars->speed, vars->target_speed, D_0015ED70 + D_0015ED70);
    step[0] = func_001F9DC8(moby->rot.z) * vars->speed;
    step[1] = func_001F9DE0(moby->rot.z) * vars->speed;
    step[2] = 0.0f;
    func_001F9A10(&moby->pos, &moby->pos, step);
    moby->pos.z = func_00213508(&moby->pos, 0, 0.5f);
    if (func_001F9B80(&moby->pos, point) < 0.5f) {
        vars->point = (vars->point + 1) % *path;
    }
    scale = vars->speed / ((*(float *)&D_L08_001618A0) * D_0015ED6C);
    if (scale > 2.0f) {
        scale = 2.0f;
    } else if (scale < 0.0f) {
        scale = 0.0f;
    }
    if (vars->linked[0] != 0) {
        char *linked;

        *(float *)((char *)vars->linked[0] + 0x58) = scale;
        linked = vars->linked[0];
        qcopy(linked + 0x40, (char *)moby + 0x40);
        qcopy(linked + 0x10, &moby->pos);
    }
    if (vars->linked[1] != 0) {
        char *linked;

        *(float *)((char *)vars->linked[1] + 0x58) = scale;
        linked = vars->linked[1];
        qcopy(linked + 0x40, (char *)moby + 0x40);
        qcopy(linked + 0x10, &moby->pos);
    }
    speed = func_001F9AF0(vars->velocity);
    if (D_0015ED6C * 0.1f < speed) {
        if (func_L00_0028D8C0(moby, vars->sound) == 0) {
            vars->sound = func_0022DA68(1, 4, moby);
            func_L00_0028DF38(vars->sound, 1);
        } else if (((SoundSlot_2d55c8_c2d55c8 *)(D_0013E550 + (vars->sound) * 0x70))->volume < 0x400) {
            func_L00_0028DF38(vars->sound, ((SoundSlot_2d55c8_c2d55c8 *)(D_0013E550 + (vars->sound) * 0x70))->volume + 0x400 / func_001F9688(60));
        }
    } else if (func_L00_0028D8C0(moby, vars->sound) != 0) {
        if (((SoundSlot_2d55c8_c2d55c8 *)(D_0013E550 + (vars->sound) * 0x70))->volume < 0x100) {
            func_L00_0028D918(vars->sound);
            vars->sound = -1;
        } else {
            func_L00_0028DF38(vars->sound, ((SoundSlot_2d55c8_c2d55c8 *)(D_0013E550 + (vars->sound) * 0x70))->volume - 0x400 / func_001F9688(60));
        }
    }
}
