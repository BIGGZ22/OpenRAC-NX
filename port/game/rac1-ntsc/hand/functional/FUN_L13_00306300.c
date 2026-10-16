/* Carried from the PAL decompilation's src/overlays/l13_gemlik/vendor_002EBD00.c (matched C there) (games/rac1) through the version
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
} AnimSeq_c306300;
/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c306300;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c306300 {
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
    struct AnimSeq_c306300 *seqs[1];          /* animation sequences, indexed by Moby.seq */
};
struct Moby_c306300 {
    Vec4f_c306300 bsphere;
    Vec4f_c306300 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c306300 *pclass;
    struct Moby_c306300 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c306300 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c306300 *moby);
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
    Vec4f_c306300 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c306300 unkD0;
    Vec4f_c306300 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c306300 Moby_c306300;
typedef struct MobyClass_c306300 MobyClass_c306300;
extern int func_001F9688(int);
extern float func_001F9B48(void *, void *);
extern int func_L00_00257B90(int, int);
extern int func_0022DA68(int, int, int);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern float func_001F9B80(void *, void *);
extern int func_001F9740(void *);
extern float func_002132A8(float, float);
extern void func_0020C828(void *);
extern float D_0015ED6C MACRO_ADDR;
extern char D_L13_00174140[];
extern void func_L00_001FF660(void *, void *, void *);
extern char D_0013E533[] NOT_SDA;
typedef int OvlQuad_p_c306300 __attribute__((mode(TI)));
typedef struct { float x, y, z, w; } __attribute__((aligned(16))) L13Vec_p_c306300;
extern void func_L00_001FF550(void *, void *, float);
extern void func_L00_00259888(void *, struct Moby_c306300 *, int, float, void *);
extern void func_L00_0026DD20(void *, void *, int, int, float);
extern void func_L07_0029A2A0(void *, void *);
typedef struct { char pad[0x18]; struct Moby_c306300 *moby; int pad1; L13Vec_p_c306300 pos; L13Vec_p_c306300 pad2; L13Vec_p_c306300 matrix; } L13Hit_p_c306300;
extern float func_001F9AF0(void *);
extern void func_L00_00259A88(void *, void *);
struct L13Ent00178480_c306300 {
    char pad0[0x24];
    int i24;
    unsigned char b28;
    unsigned char b29;
    unsigned short s2A;
    float f2C;
    int pad30;
    char *p34;
    char pad38[8];
};
extern struct L13Ent00178480_c306300 D_L13_00178400[];



// Gemlik moby (class 1238) update: moves along its velocity, hands its hit to a tracked target, sparks on impact
void FUN_L13_00306300(struct Moby_c306300 *moby) {
    char *data = (char *)moby->pvars;
    struct {
        L13Vec_p_c306300 old_pos;
        char hit[48];
        L13Vec_p_c306300 velocity;
        L13Vec_p_c306300 direction;
        L13Vec_p_c306300 random_direction;
    } work;
    int i;
    int flags;
    int stationary;
    unsigned char *base;
    char *ent;
    struct L13Ent00178480_c306300 *e;

    qcopy(&work.old_pos, &moby->pos);
    func_001F9A10(&moby->pos, &moby->pos, data);
    qcopy(&work.velocity, data);
    work.velocity.z = 0.0f;
    func_L00_001FF550(&work.velocity, &work.velocity, 1.0f);
    work.velocity.z = 1.0f;
    flags = 0x10001;
    base = (unsigned char *)D_0013E533 + 0xE1D;
    if (base[0x1FF6] != 0 && *(float *)(data + 0xC) != 31337.6015625f &&
        func_001F9B48(&moby->pos, base + 0x80) < 2.0f) {
        func_L00_001FF500(data, data, 0.5f);
        func_001F9A28(&moby->pos, &moby->pos, data);
        qzero(data);
        *(int *)(data + 0x2C) = 5;
        *(float *)(data + 0xC) = 31337.6015625f;
        for (i = 0; i < 6; i++) {
            unsigned char *b = (unsigned char *)D_0013E533 + 0xE1D;
            ent = ((char **)(b + 0x2020))[i];
            if (ent != 0) {
                int f2 = flags | 1;
                func_L00_00259888(work.hit, moby, f2, 1.0f, &work.velocity);
                work.hit[0x18] = 1;
                *(unsigned short *)(work.hit + 0x1A) = *(unsigned short *)((char *)moby + 0xA6);
                work.hit[0x19] = 1;
                if ((unsigned char)ent[0xA4] == 0xFF || (e = &D_L13_00178400[(unsigned char)ent[0xA4]])->p34 != ent) {
                    func_L00_00259A88(ent, work.hit);
                } else {
                    e->f2C += 1.0f;
                    D_L13_00178400[(unsigned char)ent[0xA4]].b28 = 1;
                    D_L13_00178400[(unsigned char)ent[0xA4]].b29 = 1;
                    D_L13_00178400[(unsigned char)ent[0xA4]].s2A = *(unsigned short *)((char *)moby + 0xA6);
                    { struct L13Ent00178480_c306300 *t = &D_L13_00178400[(unsigned char)ent[0xA4]]; t->i24 = f2; }
                }
                break;
            }
        }
    }

    func_L00_00259888(work.hit, moby, 0x10001, 1.0f, &work.velocity);
    *(unsigned short *)(work.hit + 0x1A) = *(unsigned short *)((char *)moby + 0xA6);
    work.hit[0x18] = 1;
    work.hit[0x19] = 1;
    stationary = *(int *)(data + 0x24) == 0;
    if (*(float *)(data + 0x28) < func_001F9B80(&moby->pos, data + 0x10) || func_001F9740(data + 0x2C) != 0) {
        if (moby->unk31 != 0) {
            int i;
            for (i = 0; i < 3; i++) {
                *(OvlQuad_p_c306300 *)&work.random_direction = 0;
                work.random_direction.x = func_002132A8(-1.0f, 1.0f);
                work.random_direction.y = func_002132A8(-1.0f, 1.0f);
                work.random_direction.z = func_002132A8(-1.0f, 1.0f);
                *(OvlQuad_p_c306300 *)&work.direction = *(OvlQuad_p_c306300 *)&work.random_direction;
                func_L00_001FF500(&work.direction, &work.direction, func_001F9AF0(&work.random_direction) * 0.1f);
                func_001F9A10(&work.direction, &work.random_direction, &work.direction);
                func_L00_001FF500(&work.direction, &work.direction,
                    func_002132A8(D_0015ED6C + D_0015ED6C, D_0015ED6C * 4.0f));
                func_L07_0029A2A0(&moby->pos, &work.direction);
            }
        }
        goto remove;
    }
    if (func_L00_001EFC70(&work.old_pos, &moby->pos, stationary, *(void **)(data + 0x20), work.hit) == 0)
        goto bounds;
    qcopy(&moby->pos, &(*(L13Hit_p_c306300 *)D_L13_00174140).pos);
    if (moby->unk31 != 0) {
        int i;
        for (i = 0; i < 5; i++) {
            *(OvlQuad_p_c306300 *)&work.random_direction = 0;
            work.random_direction.x = func_002132A8(-1.0f, 1.0f);
            work.random_direction.y = func_002132A8(-1.0f, 1.0f);
            work.random_direction.z = func_002132A8(-1.0f, 1.0f);
            *(OvlQuad_p_c306300 *)&work.direction = *(OvlQuad_p_c306300 *)&work.random_direction;
            func_L00_001FF660(&work.random_direction, data, &(*(L13Hit_p_c306300 *)D_L13_00174140).matrix);
            func_L00_001FF500(&work.direction, &work.direction, func_001F9AF0(&work.random_direction) * 0.5f);
            func_001F9A10(&work.direction, &work.random_direction, &work.direction);
            func_L00_001FF500(&work.direction, &work.direction,
                func_002132A8(D_0015ED6C * 3.0f, D_0015ED6C * 6.0f));
            func_L00_0026DD20(&moby->pos, &work.direction, 0x7F2F4F6F,
                func_L00_00257B90(func_001F9688(10), func_001F9688(15)), 30000.0f);
        }
    }
    {
        struct Moby_c306300 *target = (*(L13Hit_p_c306300 *)D_L13_00174140).moby;
        if (target != 0 && *(short *)((char *)target + 0xA6) == 0)
            func_0022DA68(0, 0, (int)moby);
    }
remove:
    func_0020C828(moby);
    return;
bounds:
    if (moby->pos.x < 2.0f || moby->pos.x > 1021.0f ||
        moby->pos.y < 2.0f || moby->pos.y > 1021.0f ||
        moby->pos.z < 2.0f || moby->pos.z > 1021.0f)
        func_0020C828(moby);
}
