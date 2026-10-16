/* Carried from the PAL decompilation's src/overlays/l18_veldin2/vendor_002F9D48.c (matched C there) (games/rac1) through the version
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
extern void func_001F9A28(void *, void *, void *);
extern short D_L18_00162504;
extern short D_L18_00162510;
extern char D_L18_001677C0[];
extern unsigned long func_001F44B8(int);
extern void func_00233980(int, long);
extern void func_001F9CF8(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9AD8(void *, void *, void *);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_001FD228(void *, void *, int);
typedef u32 u128_c2f9eb8 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_FB318_c2f9eb8;
typedef union {
    u128_c2f9eb8 q;
    f32 f[4];
    s32 i[4];
} Vec4_FB318_c2f9eb8;
struct MobyClass_FB318_c2f9eb8 {
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
struct Moby_FB318_c2f9eb8 {
    Vec4f_FB318_c2f9eb8 bsphere;
    Vec4f_FB318_c2f9eb8 pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 group; /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 class_slot; /* class slot: pclass = D_L00_00196F80[class_slot] (crate_update) */
    u8 alpha; /* 0x80 by default; 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_FB318_c2f9eb8 *pclass;
    struct Moby_FB318_c2f9eb8 *next;
    f32 scale; /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 update_dist; /* update distance; 0xFF (always) or 0x7F for beams, set by spawners */
    u8 visible; /* drawn last frame; set to 1 by spawners */
    s16 draw_dist; /* draw distance; 0xFF (0x7F for beams), set by spawners */
    u16 flags;
    u16 occlusion; /* occlusion bits; 0x7F80 set by spawners */
    u64 spawn_frame; /* frame count at which it may respawn */
    Vec4f_FB318_c2f9eb8 rot; /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_FB318_c2f9eb8 *moby);
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
    Vec4f_FB318_c2f9eb8 rot_row0; /* 0xC0: rows of the matrix built from rot (FUN_001fa030) */
    Vec4f_FB318_c2f9eb8 rot_row1;
    Vec4f_FB318_c2f9eb8 rot_row2;
    u8 padF0[0x10];
};
extern float D_L18_0016252C MACRO_ADDR;
typedef unsigned int u128_9c20_FB318_c2f9eb8 __attribute__((mode(TI)));
typedef struct {
    float m[4][4];
    int col[4];
    float uv[8];
    unsigned long a, b, c, d;
} Pk_9c20_FB318_c2f9eb8;
extern int D_L18_00162560;
extern short D_L18_00162514;
extern short D_L18_00162518;
extern short D_L18_0016251C;
extern float D_L18_001DFBD0[4][4];



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l18/gameplay/vendor/002f88e8.c, FUN_L18_002f9eb8. */
void FUN_L18_002f9eb8(struct Moby_FB318_c2f9eb8 *m) {
    Pk_9c20_FB318_c2f9eb8 pk;
    float mat[4][4];
    float up[4];
    Vec4_FB318_c2f9eb8 vertices[4];
    int i = 0;
    int color;

    pk.b = func_001F44B8(0xB);
    pk.c = 0x0000FF9000000260ULL;
    pk.d = 0x8000000048ULL;
    pk.a = 0;
    func_00233980(0x4A, 0);
    func_00233980(0x47, 0x51001);
    func_001F9CF8(mat[3], &D_L18_00162560, &m->rot_row0);
    func_001F9A10(mat[3], mat[3], &m->pos);
    *(u128_9c20_FB318_c2f9eb8 *)up = 0;
    up[2] = -1.0f;
    up[3] = 1.0f;
    func_001F9CF8(mat[0], up, &m->rot_row0);
    func_001F9A28(mat[1], mat[3], D_L18_001677C0);
    func_001F9AD8(mat[2], mat[1], mat[0]);
    func_L00_001FF500(mat[2], mat[2], 1.0f);
    func_001F9AD8(mat[1], mat[0], mat[2]);

    color = (*(int *)&D_L18_00162504) | 0xFF000000;
    pk.uv[0] = 1.0f;
    pk.uv[1] = 1.0f;
    pk.uv[2] = 0.0f;
    pk.uv[3] = 1.0f;
    pk.uv[4] = 1.0f;
    pk.uv[5] = 0.0f;
    pk.uv[6] = 0.0f;
    pk.uv[7] = 0.0f;

    pk.col[3] = color;
    pk.col[2] = color;
    pk.col[1] = color;
    pk.col[0] = color;
    i = 0;
    do {
        qcopy_nc(&vertices[i], D_L18_001DFBD0[i]);
        {
            vertices[i].f[2] *= (*(float *)&D_L18_00162514);
            vertices[i].f[0] *= D_L18_0016252C;
        }
        i++;
    } while (i < 4);
    for (i = 0; i < 4; i++) {
        func_001F9D20(pk.m[i], &vertices[i], mat);
    }
    func_L00_001FD228(&pk, 0, 0);

    pk.col[3] = (*(int *)&D_L18_00162510) | 0x7F000000;
    pk.col[2] = pk.col[3];
    pk.col[1] = pk.col[3];
    pk.col[0] = pk.col[3];
    i = 0;
    do {
        qcopy_nc(&vertices[i], D_L18_001DFBD0[i]);
        {
            vertices[i].f[2] *= (*(float *)&D_L18_00162518);
            vertices[i].f[0] *= D_L18_0016252C;
        }
        i++;
    } while (i < 4);
    for (i = 0; i < 4; i++) {
        func_001F9D20(pk.m[i], &vertices[i], mat);
    }
    func_L00_001FD228(&pk, 0, 0);

    pk.col[3] = (*(int *)&D_L18_00162510) | 0x7F000000;
    pk.col[2] = pk.col[3];
    pk.col[1] = pk.col[3];
    pk.col[0] = pk.col[3];
    i = 0;
    do {
        qcopy_nc(&vertices[i], D_L18_001DFBD0[i]);
        {
            vertices[i].f[2] *= (*(float *)&D_L18_0016251C);
            vertices[i].f[0] *= D_L18_0016252C;
        }
        i++;
    } while (i < 4);
    for (i = 0; i < 4; i++) {
        func_001F9D20(pk.m[i], &vertices[i], mat);
    }
    func_L00_001FD228(&pk, 0, 0);
}
