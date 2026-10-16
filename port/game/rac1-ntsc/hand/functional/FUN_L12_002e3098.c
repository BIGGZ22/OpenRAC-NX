/* Carried from the PAL decompilation's src/overlays/l12_hoven/vendor_002C0310.c (matched C there) (games/rac1) through the version
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
} Vec4f_c2e3098;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2e3098 {
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
struct Moby_c2e3098 {
    Vec4f_c2e3098 bsphere;
    Vec4f_c2e3098 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2e3098 *pclass;
    struct Moby_c2e3098 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2e3098 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c2e3098 *moby);
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
    Vec4f_c2e3098 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2e3098 unkD0;
    Vec4f_c2e3098 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2e3098 Moby_c2e3098;
typedef struct MobyClass_c2e3098 MobyClass_c2e3098;
extern float D_0015ED70 MACRO_ADDR;
extern int func_001F9688(int);
extern float D_0015ED6C MACRO_ADDR;
extern float func_00213308(void);
extern float func_002132A8(float, float);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern char *func_00218888(void *, void *, void *, int, int, int, int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern short D_L12_0016183C;
extern short D_L12_00161840;
extern short D_L12_00161844;
extern short D_L12_00161848;
extern short D_L12_0016184C;
extern short D_L12_00161850;
extern short D_L12_00161854;
extern short D_L12_00161858;
extern short D_L12_0016185C;
extern short D_L12_00161860;
extern short D_L12_00161864;
extern short D_L12_00161868;
extern short D_L12_0016186C;
extern short D_L12_00161870;
extern short D_L12_00161874;
extern short D_L12_00161878;
extern short D_L12_0016187C;
extern short D_L12_00161880;
extern short D_L12_00161884;
extern void func_L00_0025F8E0(float *v, float s);
extern void func_001F9A68(void *, void *, float);
extern int func_001FA6E0(int, int, float);
extern float func_001F96B0(float);



/* Level 12 hoven: spawns a burst of particles around the moby (port of func_L10_002E3A90's shape). */
void FUN_L12_002e3098(struct Moby_c2e3098 *mp) {
    char *arg0 = (char *)mp;
    float v[4];
    float a[4];
    float b[4];
    float o[4];
    char *d;
    float f20;
    float f21;
    float f22;
    float r;
    int n = *(int *)&D_L12_0016183C;
    int i;
    int j;
    int k;
    int x16, x17, x18, x19, r7;
    d = *(char **)(arg0 + 0x78);
    if (((unsigned char)arg0[0x20]) == 0xF) {
        n = func_001FA898_r((float)n * 1.5f);
    }
    for (i = 0; i < n; i++) {
        { float *dst = v; char *src = arg0 + 0x10; float *d2 = dst; qcopy_nc(d2, src); }
        func_L00_0025F8E0(v, (*(float *)&D_L12_00161844));
        v[2] = v[2] + 0.55f;
        f22 = func_00213308();
        f20 = func_002132A8((*(float *)&D_L12_00161850), (*(float *)&D_L12_00161854));
        f21 = func_002132A8((*(float *)&D_L12_00161858), (*(float *)&D_L12_0016185C));
        if (((unsigned char)arg0[0x20]) == 0xF) {
            f20 = f20 * 0.55f;
            f21 = f21 * 0.55f;
            v[2] = v[2] - 0.2f;
        }
        r = func_001F9DC8(f22);
        a[0] = r * (f20 * D_0015ED6C);
        r = func_001F9DE0(f22);
        a[1] = r * (f20 * D_0015ED6C);
        a[2] = (f21 * D_0015ED6C) + 0.0f;
        func_001F9A68(o, d + 0x120, 0.5f);
        o[2] = 0.0f;
        func_001F9A10(a, a, o);
        qcopy(b, a);
        k = func_001F9688((*(int *)&D_L12_00161880));
        b[2] = b[2] - ((*(float *)&D_L12_00161860) * D_0015ED70) * (float)k;
        for (j = 0; j < (*(int *)&D_L12_00161840); j++) {
            r = func_002132A8(0.5f, 1.5f);
            a[3] = r * *(float *)&D_L12_00161864;
            b[3] = r * *(float *)&D_L12_00161868;
            func_L00_0025F8E0(v, (*(float *)&D_L12_00161848));
            func_L00_0025F8E0(b, (*(float *)&D_L12_0016184C) * D_0015ED6C);
            r = func_002132A8(0.0f, 1.0f);
            x19 = func_001FA6E0((*(int *)&D_L12_0016186C), (*(int *)&D_L12_00161874), r);
            r = func_002132A8(0.0f, 1.0f);
            x18 = func_001FA6E0((*(int *)&D_L12_00161870), (*(int *)&D_L12_00161878), r);
            x17 = func_001F9688((*(int *)&D_L12_0016187C));
            x16 = func_001F9688((*(int *)&D_L12_00161880));
            r = func_002132A8((float)(*(int *)&D_L12_00161884) * 0.5f, (float)(*(int *)&D_L12_00161884) * 2.5f);
            r = func_001F96B0(r);
            r7 = func_001FA898_r(r);
            func_00218888(v, a, b, x19, x18, x17, x16, r7, -1);
        }
    }
}
