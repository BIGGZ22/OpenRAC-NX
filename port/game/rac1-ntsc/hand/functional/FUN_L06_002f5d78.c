/* Carried from the PAL decompilation's src/overlays/l06_blarg/vendor_002B5990.c (matched C there) (games/rac1) through the version
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
} Vec4f_c2f5d78;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2f5d78 {
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
struct Moby_c2f5d78 {
    Vec4f_c2f5d78 bsphere;
    Vec4f_c2f5d78 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2f5d78 *pclass;
    struct Moby_c2f5d78 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2f5d78 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c2f5d78 *moby);
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
    Vec4f_c2f5d78 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2f5d78 unkD0;
    Vec4f_c2f5d78 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2f5d78 Moby_c2f5d78;
typedef struct MobyClass_c2f5d78 MobyClass_c2f5d78;
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_00250DF8(void *);
typedef struct {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec_2F69D0_c2f5d78;
typedef struct Data_2F69D0_c2f5d78 {
    char p0[0x60];
    int i60;
    int i64;
    int i68;
    float f6C;
    Moby_c2f5d78 *a[20];
    Moby_c2f5d78 *b[20];
    Moby_c2f5d78 *c[10];
    Moby_c2f5d78 *d[10];
} Data_2F69D0_c2f5d78;
extern float D_L06_00161E08_2F69D0 SDATA(D_L06_00161D78);
extern float D_L06_00161E0C_2F69D0 SDATA(D_L06_00161D7C);
extern float D_L06_00161E10_2F69D0 SDATA(D_L06_00161D80);
extern float D_L06_00161E30_2F69D0 SDATA(D_L06_00161DA0);
extern float D_L06_00161E34_2F69D0 SDATA(D_L06_00161DA4);
extern int func_0022ED80_2F69D0(int, int, Moby_c2f5d78 *) __asm__("func_0022DA68");
extern void func_001F9CF8(void *, void *, void *);
extern float D_L06_00161D70 SDATA(D_L06_00161D70);
extern float D_L06_00161D74 SDATA(D_L06_00161D74);
extern Vec_2F69D0_c2f5d78 D_L06_00161D50 SDATA(D_L06_00161D50);
extern Vec_2F69D0_c2f5d78 D_L06_00161D60 SDATA(D_L06_00161D60);



/* Blarg drawbridge pieces: places and rotates the 20 + 10 chained segment mobys. */
void FUN_L06_002f5d78(Moby_c2f5d78 *m)
{
    Vec_2F69D0_c2f5d78 v, w, x;
    Data_2F69D0_c2f5d78 *s = (Data_2F69D0_c2f5d78 *)m->pvars;
    int i, j;
    float t, ang, a, b, c, d;

    for (i = 0; i < 20; i++) {
        t = D_L06_00161E08_2F69D0 - (float)i;
        if (t < 0.0f) t = 0.0f;
        func_L00_001FF500(&w, &m->unkC0, t);
        func_001F9CF8(&v, &D_L06_00161D50, &m->unkC0);
        func_001F9A10(&v, &v, &m->pos);
        func_001F9A10(&v, &v, &w);
        qcopy(&s->a[i]->pos, &v);
        func_001F9CF8(&v, &D_L06_00161D60, &m->unkC0);
        func_001F9A10(&v, &v, &m->pos);
        func_001F9A10(&v, &v, &w);
        qcopy(&s->b[i]->pos, &v);
        ang = D_L06_00161E0C_2F69D0 * 0.017453292f - (float)(i * 45) * 0.017453292f;
        if (ang > 1.5707964f) ang = 1.5707964f;
        else if (ang < 0.0f) ang = 0.0f;
        s->a[i]->rot.x = -ang;
        s->b[i]->rot.x = ang;
        func_L00_00250DF8(s->a[i]);
        func_L00_00250DF8(s->b[i]);
    }

    for (j = 0; j < 10; j++) {
        b = D_L06_00161D70 + (float)(4 - j) * D_L06_00161D74;
        t = D_L06_00161E08_2F69D0 - j * 2.0f;
        c = t - 1.0f;
        if (t < b) t = b;
        if (c < b) c = b;
        func_L00_001FF500(&w, &m->unkC0, t);
        func_L00_001FF500(&x, &m->unkC0, c);
        func_001F9CF8(&v, &D_L06_00161D50, &m->unkC0);
        func_001F9A10(&v, &v, &m->pos);
        func_001F9A10(&v, &v, &w);
        qcopy(&s->c[j]->pos, &v);
        func_001F9CF8(&v, &D_L06_00161D60, &m->unkC0);
        func_001F9A10(&v, &v, &m->pos);
        func_001F9A10(&v, &v, &x);
        qcopy(&s->d[j]->pos, &v);

        a = D_L06_00161E0C_2F69D0 * 0.017453292f - (float)(j * 2) * 0.7853982f;
        d = a - D_L06_00161E30_2F69D0 * 0.017453292f;
        if (a > 1.5707964f) a = 1.5707964f;
        else if (a < 0.0f) a = 0.0f;
        if (d > 1.5707964f) d = 1.5707964f;
        else if (d < 0.0f) d = 0.0f;
        if (a != 0.0f && d == 0.0f) func_0022ED80_2F69D0(1, 0, s->c[j]);

        c = D_L06_00161E0C_2F69D0 * 0.017453292f - (float)(j * 2) * 0.7853982f - 0.7853982f;
        d = c - D_L06_00161E30_2F69D0 * 0.017453292f;
        if (c > 1.5707964f) c = 1.5707964f;
        else if (c < 0.0f) c = 0.0f;
        if (d > 1.5707964f) d = 1.5707964f;
        else if (d < 0.0f) d = 0.0f;
        if (c != 0.0f && d == 0.0f) func_0022ED80_2F69D0(1, 0, s->d[j]);

        s->c[j]->rot.x = -a;
        s->d[j]->rot.x = c;

        a = D_L06_00161E10_2F69D0 * 0.017453292f - (float)(j * 2) * 0.7853982f;
        d = a - D_L06_00161E34_2F69D0 * 0.017453292f;
        if (a > 1.5707964f) a = 1.5707964f;
        else if (a < 0.0f) a = 0.0f;
        if (d > 1.5707964f) d = 1.5707964f;
        else if (d < 0.0f) d = 0.0f;
        if (a != 0.0f && d == 0.0f) func_0022ED80_2F69D0(2, 0, s->c[j]);

        c = D_L06_00161E10_2F69D0 * 0.017453292f - (float)(j * 2) * 0.7853982f - 0.7853982f;
        d = a - D_L06_00161E34_2F69D0 * 0.017453292f;
        if (c > 1.5707964f) c = 1.5707964f;
        else if (c < 0.0f) c = 0.0f;
        if (d > 1.5707964f) d = 1.5707964f;
        else if (d < 0.0f) d = 0.0f;
        if (c != 0.0f && d == 0.0f) func_0022ED80_2F69D0(2, 0, s->d[j]);

        s->c[j]->rot.y = -a;
        s->d[j]->rot.y = -c;
        func_L00_00250DF8(s->c[j]);
        func_L00_00250DF8(s->d[j]);
    }
}
