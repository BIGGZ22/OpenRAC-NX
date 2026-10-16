/* Carried from the PAL decompilation's src/overlays/l04_eudora/vendor_0029FCF0.c (matched C there) (games/rac1) through the version
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
} AnimSeq_c29ecf8;
/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c29ecf8;
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c29ecf8 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c29ecf8 q;
    f32 f[4];
    s32 i[4];
} Vec4_c29ecf8;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c29ecf8 {
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
    struct AnimSeq_c29ecf8 *seqs[1];          /* animation sequences, indexed by Moby.seq */
};
struct Moby_c29ecf8 {
    Vec4f_c29ecf8 bsphere;
    Vec4f_c29ecf8 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c29ecf8 *pclass;
    struct Moby_c29ecf8 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c29ecf8 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c29ecf8 *moby);
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
    Vec4f_c29ecf8 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c29ecf8 unkD0;
    Vec4f_c29ecf8 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c29ecf8 Moby_c29ecf8;
typedef struct MobyClass_c29ecf8 MobyClass_c29ecf8;
typedef struct {
    s32 count;
    u8 pad4[0xC];
    Vec4_c29ecf8 point[1];
} PathPoints_eb20_9FCF0_c29ecf8;
s32 func_L04_0029EB20(struct Moby_c29ecf8 *m, PathPoints_eb20_9FCF0_c29ecf8 *path, s32 i, s32 dir, f32 dist);
struct PMoby_c29ecf8 {
    Vec4f_c29ecf8 bsphere;
    Vec4f_c29ecf8 pos;
    u8 state;
    u8 pad21[3];
    void *pclass;
    struct PMoby_c29ecf8 *next;
    f32 scale;
    u8 update_dist;
    u8 visible;
    s16 draw_dist;
    u8 pad34[0xC];
    Vec4f_c29ecf8 rot;
    u8 pad50[8];
    f32 anim_speed;
    u8 pad5C[0x1C];
    u8 *pvars;
    u8 pad7C[0x2A];
    s16 oclass;
};
extern void *func_001153FC(void *, int, unsigned int);
extern float func_001FA6C0(int);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F9B80(float *, float *);
extern void func_001F99F8(float *);
extern char *func_0020C4F8(int);
extern float func_002132A8(float, float);
extern f32 func_00213308(void);
extern void func_L00_00250DF8(void *);
extern void func_L04_001F3A80(f32, struct PMoby_c29ecf8 *, void *, void *);
extern struct Path5BD0 *D_L04_001B0630[];
extern f32 D_L04_00166130[8];
extern short D_L04_001613C0;
extern char *D_L04_0015FFE4 MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
extern float D_0015ED64 MACRO_ADDR;
typedef struct SwarmPartVars_c29ecf8 {
    u8 pad0[0x10];
    Vec4f_c29ecf8 velocity;
    u8 pad20[0x20];
    f32 unk40;
    s16 unk44;
    u8 pad46[2];
    f32 unk48;
    f32 unk4C;
    s32 unk50;
    Vec4f_c29ecf8 *leader_pos;
    s32 unk58;
    s16 unk5C;
    s16 unk5E;
    s16 oclass;
    s16 unk62;
    u8 pad64[0xC];
    f32 unk70;
    f32 unk74;
    u8 pad78[0x18];
    f32 unk90;
    f32 unk94;
} SwarmPartVars_c29ecf8;
typedef struct SwarmVars_c29ecf8 {
    Vec4f_c29ecf8 origin;
    struct PMoby_c29ecf8 *children[32];
    s32 count;
    s32 initialized;
    f32 angle;
    f32 radius;
    s32 path_index;
    s32 direction;
    s32 child_count;
} SwarmVars_c29ecf8;



void FUN_L04_0029ecf8(struct PMoby_c29ecf8 *moby) {
    SwarmVars_c29ecf8 *data = (SwarmVars_c29ecf8 *)moby->pvars;
    struct PMoby_c29ecf8 **children;
    struct PMoby_c29ecf8 *other;
    f32 distance;
    f32 angle;
    f32 angle2;
    f32 speed;
    f32 size;
    f32 effect[8];

    if (*(int *)&D_L04_001613C0 == 0) {
        f32 *effect_dest = D_L04_00166130;
        func_001153FC(effect, 0, 0x20);
        effect[1] = func_001FA6C0(data->child_count) * 2.073f;
        effect[3] = 2.0f;
        effect[4] = 1.0f;
        effect[5] = 1.0f;
        *(int *)&D_L04_001613C0 = 1;
        __builtin_memcpy(effect_dest, effect, 0x20);
    }
    if (data->initialized == 0) {
        qcopy(&data->origin, &moby->pos);
        data->angle = 0.0f;
    } else if (data->child_count == 0) {
        for (other = (struct PMoby_c29ecf8 *)D_L04_0015FFE4; other != 0; other = other->next) {
            if ((s8)other->state >= 0 && other->oclass == 0x55) {
                data->child_count++;
            }
        }
        D_L04_00166130[1] = func_001FA6C0(data->child_count) * 2.073f;
    }
    if (data->path_index == -1) {
        data->angle = func_001FA580(data->angle, D_0015ED60 * 0.0034906587f);
        moby->pos.x = data->origin.x + data->radius * func_001F9DC8(data->angle);
        moby->pos.y = data->origin.y + data->radius * func_001F9DE0(data->angle);
    } else if (data->initialized != 0) {
        s32 i;
        distance = 10000.0f;
        for (i = 0; i < data->count; i++) {
            f32 d = func_001F9B80((float *)&moby->pos, (float *)&data->children[i]->pos);
            if (d < distance) distance = d;
        }
        speed = 0.0f;
        if (distance < 3.0f) speed = 3.0f - distance;
        data->direction = func_L04_0029EB20((struct Moby_c29ecf8 *)moby, (PathPoints_eb20_9FCF0_c29ecf8 *)D_L04_001B0630[data->path_index], data->direction, 1, speed);
    }
    if (data->initialized == 0) {
        s32 i;
        struct PMoby_c29ecf8 *child;
        data->initialized = 1;
        data->child_count = 0;
        for (i = 0; i < data->count; i++) {
            data->children[i] = 0;
            child = (struct PMoby_c29ecf8 *)func_0020C4F8(0x55);
            if (child != 0) {
                SwarmPartVars_c29ecf8 *part = (SwarmPartVars_c29ecf8 *)child->pvars;
                data->children[i] = child;
                child->update_dist = 0x7f;
                child->draw_dist = 0x7f;
                child->scale *= func_002132A8(0.7f, 1.3f);
                child->rot.z = func_00213308();
                child->anim_speed = func_002132A8(0.6f, 1.2f);
                func_001F99F8((float *)&part->velocity);
                part->unk44 = 0;
                part->unk4C = 10.0f;
                qcopy(&child->pos, &moby->pos);
                angle = func_00213308();
                angle2 = func_00213308();
                size = func_002132A8(0.6f, 0.9f);
                child->pos.x += func_001F9DC8(angle) * func_001F9DE0(angle2) * size;
                child->pos.y += func_001F9DE0(angle) * func_001F9DE0(angle2) * size;
                child->pos.z += func_001F9DC8(angle2) * size;
                part->unk50 = 0;
                part->leader_pos = &moby->pos;
                part->unk58 = 0;
                part->unk70 = 1.694f;
                part->unk74 = 1.694f;
                part->unk48 = 1.056f;
                part->unk5C = -1;
                part->unk40 = 0.003f;
                part->unk5E = 2;
                part->oclass = 0x55;
                part->unk90 = 1.0f;
                part->unk62 = 0x1ee;
                part->unk94 = 5.0f;
                func_L00_00250DF8(child);
            }
        }
    } else {
        s32 i;
        struct PMoby_c29ecf8 *child;
        for (i = 0; i < data->count; i++) {
            void *part;
            children = data->children;
            child = children[i];
            part = child->pvars;
            D_L04_00166130[6] = ((data->radius * 7.753f) / 1800.0f) * D_0015ED60 * ((child->anim_speed + 1.0f) * 0.5f) * 0.05f * D_0015ED64 * ((child->anim_speed + 1.0f) * 0.5f);
            D_L04_00166130[7] = (((data->radius * 7.753f) / 1800.0f) * D_0015ED60) * ((child->anim_speed + 1.0f) * 0.5f);
            func_L04_001F30E0(children, data->count, child, part, D_L04_00166130);
            func_L04_001F3A80(0.0f, child, part, D_L04_00166130);
        }
    }
}
