/* Carried from the PAL decompilation's src/overlays/shared/vendor_002D1168.c (matched C there) (games/rac1) through the version
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
} Vec4f_c2d5de8;
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c2d5de8 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2d5de8 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2d5de8;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2d5de8 {
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
struct Moby_c2d5de8 {
    Vec4f_c2d5de8 bsphere;
    Vec4f_c2d5de8 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2d5de8 *pclass;
    struct Moby_c2d5de8 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2d5de8 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c2d5de8 *moby);
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
    Vec4f_c2d5de8 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2d5de8 unkD0;
    Vec4f_c2d5de8 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2d5de8 Moby_c2d5de8;
typedef struct MobyClass_c2d5de8 MobyClass_c2d5de8;
extern float D_0015ED70 MACRO_ADDR;
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern char *D_L00_0015FFD8 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
typedef struct {
    u8 p0[0x60];
    float v60;
    float v64;
    float v68;
    float v6C;
} St_6e38_c2d5de8;
typedef struct {
    u8 p0[0x10];
    float v[3];
    u8 p1[0x20 - 0x1C];
    u8 flag;
    u8 p2[0x40 - 0x21];
    float f40;
    float f44;
    float f48;
    u8 p3[0x78 - 0x4C];
    St_6e38_c2d5de8 *st;
} Obj_6e38_c2d5de8;
typedef struct {
    s32 n;
    u8 pad[0x10 - 4];
    Vec4_c2d5de8 a[1];
    Vec4_c2d5de8 b[1];
} Path_6e38_c2d5de8;
extern int func_L00_0025DF68(void *, void *, void *, int *, float *, int, float, float, float);
extern float func_L00_00258110(float *vel, float cur, float target, float k, float d, float max);
void func_L00_002D5988(Obj_6e38_c2d5de8 *o, Path_6e38_c2d5de8 *path, s32 p);
typedef unsigned int Q128_c2d5de8 __attribute__((mode(TI), aligned(16)));
typedef union {
    Q128_c2d5de8 q;
    f32 f[4];
    struct {
        f32 x, y, z, w;
    } v;
} Vec_c2d5de8;
#define MPOS(m) ((Vec_c2d5de8 *)&(m)->pos)
typedef struct {
    s32 count;
    u8 pad4[0xC];
    Vec_c2d5de8 pts[1]; /* pts[i].v.w: length of segment i */
} Path_c2d5de8;
typedef struct {
    s32 arrived;    /* 0x00 */
    s32 planet;     /* 0x04 */
    s32 cutscene;   /* 0x08 */
    s8 msg0;        /* 0x0C */
    s8 msg1;        /* 0x0D */
    s8 msg2;        /* 0x0E */
    u8 pad0F[0x41];
    s32 attach;     /* 0x50 */
    s32 carrier;    /* 0x54 */
    s32 path;       /* 0x58 */
    f32 bob;        /* 0x5C */
    f32 speed;      /* 0x60 */
    f32 spin[3];    /* 0x64: x, y, z turn rates */
    s32 slots[8];   /* 0x70 */
    u8 pad90[0x14];
    f32 sway;       /* 0xA4 */
    s32 seg;        /* 0xA8 */
    f32 yaw;        /* 0xAC */
    s32 waiting;    /* 0xB0 */
    u8 padB4[0xC];
    Q128_c2d5de8 home;      /* 0xC0 */
} Infobot_c2d5de8;
typedef struct {
    Vec_c2d5de8 pos;
    u8 pad10[0x2218];
    s32 active;     /* 0x2228 */
} Player_c2d5de8;
typedef struct {
    u8 pad0[0x80];
    Vec_c2d5de8 target;     /* 0x80 */
} Camera_c2d5de8;
typedef struct {
    u8 pad0[0x30];
    Vec_c2d5de8 pos;        /* 0x30 */
    u8 pad40[0x30];
    Vec_c2d5de8 dir;        /* 0x70 */
} Cutscene_c2d5de8;
/* 0x80 */
extern char D_0013F350[];
#define CUTS ((Cutscene_c2d5de8 *)D_L00_001600EC)
typedef struct { Vec4f_c2d5de8 v0, v1, v2; float pos[3]; float w; u8 pad[0x40]; } T_c2d5de8;
extern T_c2d5de8 * D_L00_001600EC MACRO_ADDR;
extern void func_L00_002D6BF0(char *);
extern void func_L00_002D6CD0(char *);
extern s32 func_L00_00260400(struct Moby_c2d5de8 *, struct Moby_c2d5de8 *, void *, void *, void *, void *);
extern void func_L00_002607D0(s32);
extern void func_L00_002502A0(s32);
extern void func_L00_00284E50(void *, void *);
extern void func_L00_00298840(s32);
extern void func_L00_00299460(s32);
extern void func_L00_00262D38(s32);
extern void func_L00_00266858(struct Moby_c2d5de8 *, s32);
extern void func_0020B178(s32, s32);
extern void func_0020C828(void);
extern f32 func_001F9B48(void *, void *);
extern void func_001F9A68(void *, void *, f32);
extern void func_L00_001FF500(void *, void *, f32);
extern f32 func_001F9AF0(void *);
extern f32 func_L00_001FF8B0(f32, f32);
extern f32 func_001F9DC8(f32);
extern f32 func_001F9DE0(f32);
extern f32 func_001FA580(f32, f32);
extern f32 func_001FA5C8(f32, f32);
extern s32 func_001FA6D0(f32);
extern f32 func_001FA6C0(s32);
extern Path_c2d5de8 *D_L00_001B04B0[];
extern u8 D_0013DD40[];
extern Player_c2d5de8 D_0013F3D0;
extern s32 D_L00_0015F5C4 MACRO_ADDR;
static inline void measure_path_c2d5de8(Path_c2d5de8 *path) {
    s32 j;

    for (j = 0; j < path->count - 2; j++) {
        path->pts[j].v.w = func_001F9B48(&path->pts[j], &path->pts[j + 1]);
    }
}


void FUN_L00_002d5de8(struct Moby_c2d5de8 *moby)
{
    Vec_c2d5de8 offset;
    Vec_c2d5de8 step;
    Vec_c2d5de8 point;
    s32 segment;
    f32 fraction;
    Infobot_c2d5de8 *infobot;
    Path_c2d5de8 *path;
    Vec_c2d5de8 *pos;
    struct Moby_c2d5de8 *carrier;
    Cutscene_c2d5de8 *scene;
    s32 i;
    f32 range;
    f32 distance;
    f32 end;
    f32 accel;
    f32 speed;
    f32 heading;
    f32 tilt_y;
    f32 tilt_x;
    f32 max_speed;

    offset.q = 0;
    infobot = (Infobot_c2d5de8 *)moby->pvars;
    switch (moby->state) {
    case 0:
        moby->unk30 = 0xFF;
        infobot->yaw = moby->rot.z;
        infobot->home = MPOS(moby)->q;
        if (infobot->arrived != 0) {
            moby->state = 2;
            func_L00_002D6BF0((char *)moby);
        } else {
            if (infobot->waiting != 0) {
                moby->state = 2;
            } else {
                moby->state = 1;
            }
            moby->flags |= 0x41;
        }
        if (infobot->attach != -1) {
            moby->state = 5;
            func_L00_002D6BF0((char *)moby);
            break;
        }
        if (infobot->slots[0] == -1) {
            break;
        }
        for (i = 0; i < 8; i++) {
            if (infobot->slots[i] == -1) {
                continue;
            }
            measure_path_c2d5de8(D_L00_001B04B0[infobot->slots[i]]);
        }
        MPOS(moby)->q = D_L00_001B04B0[infobot->slots[0]]->pts[0].q;
        infobot->seg = 0;
        moby->state = 3;
        break;
    case 1:
        break;
    case 2:
        range = 1.0f;
        if (infobot->waiting != 0 && (moby->flags & 1)) {
            range = 3.0f;
        }
        if (infobot->planet == 0xC) {
            range = 3.0f;
        }
        moby->rot.z = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.z, infobot->yaw, 0.01f, 0.3f, 0.1f, &infobot->spin[2]);
        moby->rot.y = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.y, 0.0f, D_0015ED70 * 0.5235988f, D_0015ED70 * 1.0471976f, D_0015ED6C * 0.7853982f, &infobot->spin[1]);
        moby->rot.x = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.x, 0.0f, D_0015ED70 * 1.0471976f, D_0015ED70 * 2.0943952f, D_0015ED6C * 1.5707964f, &infobot->spin[0]);
        func_L00_002D6CD0((char *)moby);
        if (D_0013DD40[infobot->planet] != 0) {
            ((void (*)(struct Moby_c2d5de8 *))func_0020C828)(moby);
            return;
        }
        if (func_001F9B48(&D_0013F3D0, &moby->pos) < range && D_0013F3D0.active != 0) {
            moby->state = 8;
        }
        break;
    case 3:
        func_L00_002D6CD0((char *)moby);
        if (D_0013DD40[infobot->planet] != 0) {
            ((void (*)(struct Moby_c2d5de8 *))func_0020C828)(moby);
            return;
        }
        if (infobot->slots[infobot->seg] == -1) {
            moby->state = 2;
            break;
        }
        heading = func_L00_001FF8B0(((Camera_c2d5de8 *)D_0013F350)->target.v.x - moby->pos.x, ((Camera_c2d5de8 *)D_0013F350)->target.v.y - moby->pos.y);
        moby->rot.z = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.z, heading, 0.01f, 0.3f, 0.1f, &infobot->spin[2]);
        moby->rot.y = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.y, 0.0f, D_0015ED70 * 0.5235988f, D_0015ED70 * 1.0471976f, D_0015ED6C * 0.7853982f, &infobot->spin[1]);
        moby->rot.x = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.x, 0.0f, D_0015ED70 * 1.0471976f, D_0015ED70 * 2.0943952f, D_0015ED6C * 1.5707964f, &infobot->spin[0]);
        if (func_001F9B48(&((Camera_c2d5de8 *)D_0013F350)->target, &moby->pos) < 8.0f) {
            moby->state = 4;
            func_L00_00266858(moby, 1);
        }
        break;
    case 4:
        func_L00_002D5988(moby, D_L00_001B04B0[infobot->slots[infobot->seg]], 3);
        func_L00_002D6CD0((char *)moby);
        if (moby->state != 4) {
            infobot->seg++;
        }
        break;
    case 5:
        func_L00_002D6CD0((char *)moby);
        if (D_0013DD40[infobot->planet] != 0) {
            ((void (*)(struct Moby_c2d5de8 *))func_0020C828)(moby);
            return;
        }
        infobot->bob = func_001FA580(infobot->bob, D_0015ED6C * 3.1415927f);
        offset.v.x = func_001F9DC8(infobot->bob);
        offset.v.y = func_001F9DE0(infobot->bob);
        offset.v.z = 3.0f;
        moby->rot.z = func_001FA580(infobot->bob, 1.5707964f);
        if (infobot->carrier == -1) {
            break;
        }
        if (((struct Moby_c2d5de8 *)D_L00_0015FFD8)[infobot->carrier].oclass != 0x336) {
            break;
        }
        if (((struct Moby_c2d5de8 *)D_L00_0015FFD8)[infobot->carrier].unkBC != 6) {
            break;
        }
        if (infobot->path == -1) {
            break;
        }
        moby->state = 6;
        infobot->attach = -1;
        MPOS(moby)->q = D_L00_001B04B0[infobot->path]->pts[0].q;
        moby->rot.z = func_L00_001FF8B0(D_L00_001B04B0[infobot->path]->pts[1].v.x - D_L00_001B04B0[infobot->path]->pts[0].v.x,
                                D_L00_001B04B0[infobot->path]->pts[1].v.y - D_L00_001B04B0[infobot->path]->pts[0].v.y);
        measure_path_c2d5de8(D_L00_001B04B0[infobot->path]);
        break;
    case 6:
        path = D_L00_001B04B0[infobot->path];
        pos = (Vec_c2d5de8 *)&moby->pos;
        func_L00_002D6CD0((char *)moby);
        func_L00_0025DF68(path, pos, &point, &segment, &fraction, 0, 999.0f, 5.0f, 0.0f);
        end = func_001FA6C0(segment) * path->pts[0].v.w + fraction + 2.0f;
        distance = func_001F9B48(pos, &path->pts[path->count - 1]);
        accel = D_0015ED70 * 16.0f;
        speed = infobot->speed;
        if (distance <= speed * speed / (accel + accel)) {
            infobot->speed = speed - accel;
            if (speed - accel < 0.0f) {
                infobot->speed = 0.0f;
            }
        } else {
            max_speed = D_0015ED6C * 16.0f;
            infobot->speed = speed + accel;
            if (max_speed < speed + accel) {
                infobot->speed = max_speed;
            }
        }
        segment = func_001FA6D0(end / path->pts[0].v.w);
        fraction = (end - func_001FA6C0(segment) * path->pts[0].v.w) / path->pts[0].v.w;
        if (segment >= path->count - 1) {
            if (infobot->speed <= D_0015ED70 * 16.0f) {
                MPOS(moby)->q = ((Vec_c2d5de8 *)path)[path->count].q;
                moby->state = 7;
                break;
            }
            segment = path->count - 2;
            fraction = 1.0f;
        }
        func_001F9A28(&point, &path->pts[segment + 1], &path->pts[segment]);
        func_001F9A68(&point, &point, fraction);
        func_001F9A10(&point, &point, &path->pts[segment]);
        heading = func_L00_001FF8B0(path->pts[segment + 1].v.x - path->pts[segment].v.x,
                         path->pts[segment + 1].v.y - path->pts[segment].v.y);
        moby->rot.z = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.z, heading, 0.01f, 0.3f, 0.1f, &infobot->spin[2]);
        pos = &step;
        func_001F9A28(pos, &point, MPOS(moby));
        distance = func_001F9AF0(pos);
        if (infobot->speed < distance) {
            distance = infobot->speed;
        }
        func_L00_001FF500(pos, pos, distance);
        func_001F9A10(MPOS(moby), MPOS(moby), pos);
        heading = func_001FA5C8(func_L00_001FF8B0(step.v.x, step.v.y), moby->rot.z);
        tilt_y = infobot->speed * 0.34906584f * func_001F9DC8(heading) / (D_0015ED6C * 16.0f);
        tilt_x = infobot->speed * -0.34906584f * func_001F9DE0(heading) / (D_0015ED6C * 16.0f);
        moby->rot.y = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.y, tilt_y, D_0015ED70 * 0.5235988f, D_0015ED70 * 1.0471976f, D_0015ED6C * 0.7853982f, &infobot->spin[1]);
        moby->rot.x = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.x, tilt_x, D_0015ED70 * 1.0471976f, D_0015ED70 * 2.0943952f, D_0015ED6C * 1.5707964f, &infobot->spin[0]);
        break;
    case 7: {
        Path_c2d5de8 *p = D_L00_001B04B0[infobot->path];
        func_L00_002D6CD0((char *)moby);
        infobot->sway = infobot->sway + D_0015ED6C;
        if (1.0f < infobot->sway) {
            infobot->sway = 1.0f;
        }
        infobot->bob = func_001FA580(infobot->bob, D_0015ED6C * 3.1415927f);
        offset.v.x = func_001F9DC8(infobot->bob) * infobot->sway;
        offset.v.y = func_001F9DE0(infobot->bob) * infobot->sway;
        offset.v.z = 0.0f;
        heading = func_001FA580(infobot->bob, 1.5707964f);
        moby->rot.z = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.z, heading, D_0015ED6C * 25.132742f, D_0015ED6C * 25.132742f, D_0015ED6C * 12.566371f, &infobot->spin[2]);
        func_001F9A10(&step, &offset, &p->pts[p->count - 1]);
        MPOS(moby)->q = step.q;
        if (func_001F9B48(&D_0013F3D0, &p->pts[p->count - 1]) < 2.0f && D_0013F3D0.active != 0) {
            moby->state = 8;
        }
        moby->rot.y = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.y, 0.0f, D_0015ED70 * 0.5235988f, D_0015ED70 * 1.0471976f, D_0015ED6C * 0.7853982f, &infobot->spin[1]);
        moby->rot.x = ((f32 (*)(f32, f32, f32, f32, f32, f32 *))func_L00_00258110)(moby->rot.x, 0.0f, D_0015ED70 * 1.0471976f, D_0015ED70 * 2.0943952f, D_0015ED6C * 1.5707964f, &infobot->spin[0]);
        break;
    }
    case 8:
        moby->flags |= 0x41;
        func_L00_002607D0(infobot->planet);
        func_L00_002502A0(moby->unkB0);
        if (infobot->cutscene != -1) {
            func_L00_00284E50(&CUTS[infobot->cutscene].pos, &CUTS[infobot->cutscene].dir);
        }
        moby->state = 9;
        if (infobot->msg0 != -1) {
            func_L00_00298840(infobot->msg0);
        }
        break;
    case 9:
        if (D_L00_0015F5C4 == 2) {
            break;
        }
        moby->state = 10;
        if (infobot->msg1 != -1) {
            func_L00_00299460(infobot->msg1);
        }
        break;
    case 10:
        if (D_L00_0015F5C4 == 2) {
            break;
        }
        if (infobot->msg2 != -1) {
            func_L00_00298840(infobot->msg2);
        }
        moby->state = 11;
        break;
    case 11:
        if (D_L00_0015F5C4 == 2) {
            return;
        }
        func_0020B178(0, -1);
        func_L00_00262D38(infobot->planet);
        ((void (*)(struct Moby_c2d5de8 *))func_0020C828)(moby);
        return;
    }
    if (infobot->attach != -1 && infobot->carrier != -1) {
        carrier = &((struct Moby_c2d5de8 *)D_L00_0015FFD8)[infobot->carrier];
        if (carrier->oclass == 0x336) {
            step.q = 0;
            func_L00_00260400(moby, carrier, &CUTS[infobot->attach].pos, &step,
                             &CUTS[infobot->attach].pos, &step);
            func_001F9A10(&point, &offset, &CUTS[infobot->attach].pos);
            MPOS(moby)->q = point.q;
        }
    }
}
