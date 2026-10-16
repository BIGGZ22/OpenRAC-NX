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
} Vec4f_c2e72c0;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2e72c0 {
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
struct Moby_c2e72c0 {
    Vec4f_c2e72c0 bsphere;
    Vec4f_c2e72c0 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2e72c0 *pclass;
    struct Moby_c2e72c0 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2e72c0 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c2e72c0 *moby);
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
    Vec4f_c2e72c0 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2e72c0 unkD0;
    Vec4f_c2e72c0 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2e72c0 Moby_c2e72c0;
typedef struct MobyClass_c2e72c0 MobyClass_c2e72c0;
extern float func_001F9B48(void *, void *);
extern char *D_L12_001B0930[];
extern f32 D_0015ED6C MACRO_ADDR;
extern s32 func_001F9688(s32);
extern s32 func_00213260(s32);
extern f32 func_001F9DC8(f32);
extern f32 func_001F9DE0(f32);
extern void func_L01_0026E090(s32, s32);
extern char D_0013E533[];
extern void func_001F9A28(void *, void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA580(float, float);
extern float func_001F99C0(float);
extern int func_001F9740(int *arg0);
extern void func_0020C828(void *);
extern float func_001F9B80(void *, void *);
extern float func_001FA688(float, float);
extern void func_00214DB0(void *, float, float, float);
extern float func_001FA5C8(float, float);
extern float func_001F9B20(void *);
extern float D_0015ED64 MACRO_ADDR;
extern char D_L12_00167240[];
extern int func_00214720(void *, int);
void func_L12_002E7290(void *a, void *b);
/* functional: high. Same reads, writes, calls and arguments as retail; the jump table is a switch, the shared exits are gotos, quad copies are four-float copies, and the two DeleteMoby calls pass the pointer that retail leaves in $a0. No COP2/VU0 instructions in this function. */
/* Level 12 (Hoven) moby update, one call per frame per moby. data = moby+0x78 is its state block, hero = the target
 * moby found by func_L00_0025FCB8 (or the hero global when none). Runs func_L00_00263B38 on data+0x120, hides or shows
 * the moby (flags +0x34 |= 0x41 or &= 0xFFBE) from the near test against the hero and the clip volume, runs the
 * shared update func_L12_002E81E0, then a state machine on moby+0x20: 0 setup, 1 walk along a waypoint row of
 * D_L12_001B0930, 2 wait/spawn check, 3 wander and face the hero, 4 and 9 dying/explode checks, 5 to 8 idle,
 * attack-ready and chase steering. The tail (state-independent) animates the two bone controllers. */
extern int D_L12_0015F5C4 MACRO_ADDR;
extern float D_L12_0015F580[] MACRO_ADDR;
extern char * D_L12_00178380[] ;
extern void func_L00_00263B38(void *, float);
extern void func_L12_002E81E0(char *moby);
extern void func_L00_0025FCB8(char *, float *, float);
extern void func_L01_0026D930(void *);
extern int func_001160D8(void);
extern int func_L00_00257B90(int, int);
extern float func_L00_00257C48(float, float);
extern float func_00213508(void *, int, float);
extern void func_002130D8(void *, int, int, int, int);
extern int func_L12_0027B8E0(void *, void *, float, float, float);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern int func_L01_0028B510(void *, void *, float);
extern void func_L12_002E8808(struct Moby_c2e72c0 *moby, float *target, float max_angle, float approach_speed);
extern void func_L12_002E8608(char *moby, float *pos);
extern void func_L12_002E87B0(char *moby);
extern void func_L12_002E86D0();
extern void func_L12_002E8720(unsigned char *moby);
extern int func_L00_001F2868(void *, int, void *, int, float);
extern int func_L00_0025E3B8(void *);
extern void func_L00_00259BC8(void *, int, int, void *, void *, float);
extern int func_00214CC8(void *, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_0022DA68(int, int, int);
extern int func_001FA6D0(float);
extern void func_L04_002AEE30(void *, void *, void *, int);
extern int func_L00_0025C698(void *, void *);
extern void func_L00_002628D8(char *, char *, int, float, float);
#define M_U8(p, o) (*(unsigned char *)((char *)(p) + (o)))
#define M_U16(p, o) (*(unsigned short *)((char *)(p) + (o)))
#define M_I(p, o) (*(int *)((char *)(p) + (o)))
#define M_F(p, o) (*(float *)((char *)(p) + (o)))
#define M_P(p, o) (*(char **)((char *)(p) + (o)))
#define COPY4(dst, src) qcopy((void *)(dst), (void *)(src))



void FUN_L12_002e72c0(char *moby) {
    char *data;
    char *hero;
    char *B;
    char *obj;
    char *row;
    char *arr;
    int *q;
    float buf[20] __attribute__((aligned(16)));
    float v50[4] __attribute__((aligned(16)));
    float v60[4] __attribute__((aligned(16)));
    float f20;
    float f21;
    float f;
    float g;
    int st;
    int n;
    int r;
    int i;
    char **pp;

    data = M_P(moby, 0x78);
    func_L00_00263B38(data + 0x120, 3.0f);

    if (D_L12_0015F5C4 == 2) {
        goto hide;
    }
    if (M_U8(moby, 0x20) >= 3) {
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        {
            char *o = M_P(data, 0x248);
            if (o != 0) {
                M_U16(o, 0x34) = M_U16(o, 0x34) & 0xFFBE;
            }
        }
    }
    if (M_U8(data, 0x269) != 0) {
        char *b0 = ((char *)D_0013E533) + 0xE1D;
        if (M_I(b0, 0x2084) == 0 && M_I(b0, 0x2090) == 0x67) {
            if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, b0 + 0x80) < 30.0f) {
                goto finish;
            }
        }
    }
    if (M_I(data, 0x270) != -1) {
        if (((int (*)(void *, int))func_00214720)(((char *)D_L12_00167240), M_I(data, 0x270)) == 0) {
        hide:
            M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x41;
            {
                char *o = M_P(data, 0x248);
                if (o != 0) {
                    M_U16(o, 0x34) = M_U16(o, 0x34) | 0x41;
                }
            }
            return;
        }
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        {
            char *o = M_P(data, 0x248);
            if (o != 0) {
                M_U16(o, 0x34) = M_U16(o, 0x34) & 0xFFBE;
            }
        }
    }
    if (M_U8(data, 0x2E) == 2) {
        goto finish;
    }

    ((void (*)(void *))func_L12_002E81E0)(moby);
    ((int (*)(void *, void *, float))func_L00_0025FCB8)(moby, buf, M_F(data, 0x230));
    hero = M_P(buf, 0x40);
    if (hero == 0) {
        char *B2 = ((char *)D_0013E533) + 0xE1D;
        hero = M_P(B2, 0x2080);
        COPY4(buf + 8, (float *)(B2 + 0xD0));
    }

    st = M_U8(moby, 0x20);
    switch (st) {
    case 0:
        f = M_F(data, 0x230);
        M_U8(data, 0x29) = 1;
        M_U8(data, 0x58) = 8;
        M_F(data, 0x20) = 2.0f;
        *(short *)(data + 0x24) = 2;
        M_U8(data, 0x5A) = 8;
        M_F(data, 0x28C) = f;
        M_I(data, 0x90) = 0x200;
        M_F(data, 0x98) = 0.5f;
        M_U8(data, 0x2E) = 1;
        func_L01_0026D930(data + 0xD0);
        {
        float k = (*(float *)&D_0015ED6C) * 5.5f;
        M_F(data, 0xDC) = 1.0f;
        M_I(data, 0xD0) = 0x38D;
        M_F(data, 0xD8) = 1.0f;
        M_I(data, 0x108) = 0x18;
        M_F(data, 0xF4) = k;
        }
        COPY4((float *)(data + 0x220), (float *)(moby + 0x10));
        if (M_U8(moby, 0x21) != 0xFF) {
            if (*(int *)M_P(data, 0x24C) == 1) {
                M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x8000;
            }
            *(int *)M_P(data, 0x24C) = (*(int *)M_P(data, 0x24C) == 0);
        } else {
            if (func_001160D8() & 1) {
                M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x8000;
            }
        }
        if (M_I(data, 0x274) != 0) {
            r = func_L00_00257B90(0, 300);
            M_I(data, 0x288) = ((int (*)(int))func_001F9688)(r);
        }
        M_I(moby, 0x40) = 0;
        M_I(moby, 0x44) = 0;
        if (M_F(data, 0x244) == M_F(moby, 0x40)) {
            if (M_U8(moby, 0x21) != 0xFF) {
                q = *(int **)(data + 0x250);
                M_F(data, 0x244) = (float)*q;
                r = *q;
                if (r > 0) {
                    *q = -r;
                } else {
                    *q = r + 0x14;
                }
            } else {
                M_F(data, 0x244) = func_L00_00257C48(15.0f, 35.0f);
            }
        }
        M_F(data, 0x244) = M_F(data, 0x244) * 0.0174532924f;
        M_F(moby, 0x18) = ((float (*)(void *, int, float))func_00213508)(moby + 0x10, 0, 0.5f);
        if (M_U8(data, 0x268) != 0) {
            M_U16(moby, 0x34) = M_U16(moby, 0x34) | 0x41;
            {
                char *o = M_P(data, 0x248);
                if (o != 0) {
                    M_U16(o, 0x34) = M_U16(o, 0x34) | 0x41;
                }
            }
        }
        M_U8(moby, 0x20) = 2;
        break;

    case 2:
        if (((float (*)(void *, void *))func_001F9B48)(moby + 0x10, ((char *)D_0013E533) + 0xE9D) > 64.0f) {
            return;
        }
        if (M_U8(moby, 0x53) != 0) {
            if (M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_002130D8(moby, 0, 0, ((int (*)(int))func_001F9688)(8), 5);
            }
        }
        r = func_L12_0027B8E0(moby, hero + 0x10, M_F(data, 0x230), 3.0f, 0.0f);
        if (r == 0) {
            if (M_I(data, 0x25C) != -1) {
                if (((int (*)(void *, int))func_00214720)(hero + 0x10, M_I(data, 0x25C)) == 0) {
                    if (M_U8(moby, 0xBC) == 0) {
                        goto tail;
                    }
                }
            } else if (M_U8(moby, 0xBC) == 0) {
                goto tail;
            }
        }
        if (M_U16(moby, 0x34) & 1) {
            COPY4(v50, (float *)(moby + 0x10));
            v50[2] = v50[2] + 0.5f;
            if (func_L00_001EFC70(v50, ((char *)D_L12_00167240), 2, moby, 0) != 0) {
                M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
                {
                    char *o = M_P(data, 0x248);
                    if (o != 0) {
                        M_U16(o, 0x34) = M_U16(o, 0x34) & 0xFFBE;
                    }
                }
            } else {
                obj = M_P(data, 0x248);
                if (obj != 0) {
                    ((void (*)(void *))func_0020C828)(obj);
                }
                ((void (*)(void *))func_0020C828)(moby);
                return;
            }
        }
        if (M_U8(moby, 0x21) != 0xFF) {
            ((void (*)(int, int))func_L01_0026E090)(M_U8(moby, 0x21), 1);
        }
        if (M_I(data, 0x254) == -1) {
            M_U8(moby, 0x20) = 3;
            break;
        }
        M_I(data, 0x260) = func_L01_0028B510(moby + 0x10, ((char * *)D_L12_001B0930)[M_I(data, 0x254)], 0.0f);
        M_U8(moby, 0x20) = 1;
        break;

    case 1:
        if (((int (*)(void *))func_001F9740)(data + 0x258) == 0) {
            break;
        }
        if (M_U8(moby, 0x53) != 8 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
            func_002130D8(moby, 8, 0, ((int (*)(int))func_001F9688)(11), 5);
        }
        row = ((char * *)D_L12_001B0930)[M_I(data, 0x254)];
        if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, row + ((M_I(data, 0x260) << 4) + 0x10)) < 3.0f) {
            M_I(data, 0x260) = M_I(data, 0x260) + 1;
        }
        {
            int last = *(int *)row - 1;
            if (last < M_I(data, 0x260)) {
                M_I(data, 0x260) = last;
            }
        }
        ((void (*)(void *, void *, float, float))func_L12_002E8808)(moby, row + ((M_I(data, 0x260) << 4) + 0x10), 0.0f, (*(float *)&D_0015ED6C) * 8.0f);
        if (M_I(data, 0x260) == *(int *)row - 1) {
            M_U8(moby, 0x20) = 3;
            break;
        }
        if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, hero + 0x10) < M_F(data, 0x264)) {
            M_U8(moby, 0x20) = 3;
        }
        break;

    case 3:
        if (M_U8(data, 0x268) == 1) {
            COPY4(v50, (float *)(moby + 0x10));
            v50[0] = v50[0] + ((float (*)(float))func_001F9DC8)(M_F(moby, 0x48));
            v50[1] = v50[1] + ((float (*)(float))func_001F9DE0)(M_F(moby, 0x48));
            n = func_L00_001F2868(v50, 0, moby, 0, 1.5f);
            if (n != 0) {
                COPY4(v60, D_L12_0015F580);
                if (n > 0) {
                    pp = D_L12_00178380;
                    B = ((char *)D_0013E533) + 0xE1D;
                    i = n;
                    do {
                        if (func_L00_0025E3B8(*pp) != 0) {
                            f = ((float (*)(float))func_001F99C0)(M_F(moby, 0x18) - M_F(*pp, 0x18));
                            if (f < 0.7f) {
                                ((void (*)(void *, void *, int, void *, void *, float))func_L00_00259BC8)(*pp, M_P(B, 0x2080), 0x10000, moby + 0x10, v60, 1.0f);
                            }
                        }
                        pp++;
                    } while (--i);
                }
            }
        }
        {
        float a = ((float (*)(float, float))func_001FA5C8)(((float (*)(float, float))func_L00_001FF8B0)(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)), M_F(moby, 0x48));
        if (a > 0.959931076f) {
            a = 0.959931076f;
        } else if (a < -0.959931076f) {
            a = -0.959931076f;
        }
        {
            float h = a * 0.5f;
            M_F(data, 0x188) = h;
            M_F(data, 0x208) = h;
        }
        }
        ((void (*)(void *, void *, float, float))func_L12_002E8808)(moby, hero + 0x10, M_F(data, 0x244), 0.0f);
        if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, hero + 0x10) < 5.5f || M_U8(data, 0x242) != 0) {
            f = ((float (*)(float, float))func_001FA688)(M_F(moby, 0x48), ((float (*)(float, float))func_L00_001FF8B0)(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)));
            if (f < 0.0872664601f) {
                M_U8(moby, 0x20) = 5;
                break;
            }
        }
        if (M_U8(moby, 0x53) != 8 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
            func_002130D8(moby, 8, 0, ((int (*)(int))func_001F9688)(11), 5);
        }
        goto tail;

    case 5:
        if (M_F(data, 0x28C) < 8.0f) {
            M_F(data, 0x28C) = 8.0f;
        }
        ((void (*)(void *, void *))func_L12_002E8608)(moby, hero + 0x10);
        ((void (*)(void *))func_L12_002E87B0)(moby);
        if (M_U8(moby, 0x53) != 2 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
            func_002130D8(moby, 2, 0, ((int (*)(int))func_001F9688)(8), 5);
        }
        if (M_U8(moby, 0x70) & 2) {
            if (M_U8(moby, 0x53) != 10 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_002130D8(moby, 10, 0, ((int (*)(int))func_001F9688)(5), 5);
            }
            M_U8(moby, 0x20) = 6;
        }
        break;

    case 6:
        ((void (*)(void *, void *))func_L12_002E8608)(moby, hero + 0x10);
        ((void (*)(void *))func_L12_002E87B0)(moby);
        if (M_P(data, 0x248) != 0) {
            if (func_00214CC8(moby, 0.0f) != 0) {
                ((void (*)(void *, int, void *))func_L00_0024F7C8)(M_P(data, 0x248), 0, v50);
                ((void (*)(void *, void *, void *))func_001F9A28)(v60, buf + 8, v50);
                f21 = ((float (*)(float, float))func_L00_001FF8B0)(v60[0], v60[1]);
                f20 = ((float (*)(float, float))func_001FA580)(M_F(moby, 0x48), M_F(data, 0x208));
                f21 = (((float (*)(float, float))func_001FA688)(f21, f20) > 0.226892799f) ? f20 : f21;
                g = ((float (*)(void *))func_001F9B20)(v60);
                g = ((float (*)(float, float))func_L00_001FF8B0)(g, v60[2]);
                ((void (*)(void *, float, float, float))func_00214DB0)(v60, (*(float *)&D_0015ED6C) * 15.0f, f21, g);
                ((int (*)(int, int, void *))func_0022DA68)(0, 0, moby);
                n = func_001FA6D0(18.0f / ((*(float *)&D_0015ED6C) * 15.0f));
                func_L04_002AEE30(moby, v50, v60, n);
            }
        }
        if (M_U8(moby, 0x70) & 2) {
            if (M_U8(moby, 0x53) != 5 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_002130D8(moby, 5, 0, ((int (*)(int))func_001F9688)(8), 5);
            }
            M_U8(moby, 0x20) = 7;
        }
        break;

    case 7:
        ((void (*)(void *, void *))func_L12_002E8608)(moby, hero + 0x10);
        ((void (*)(void *))func_L12_002E87B0)(moby);
        if (M_U8(moby, 0x53) != 5 || (M_U8(moby, 0x70) & 2)) {
            if (M_U8(moby, 0x53) != 0 && M_U8(moby, 0x52) == M_U8(moby, 0x53)) {
                func_002130D8(moby, 0, 0, ((int (*)(int))func_001F9688)(8), 5);
            }
        }
        if (((int (*)(int))func_00213260)(9) == 0) {
            M_U8(moby, 0x20) = 5;
            break;
        }
        if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, hero + 0x10) > 10.5f && M_U8(data, 0x242) == 0) {
            M_U8(moby, 0x20) = 3;
            break;
        }
        f = ((float (*)(float, float))func_001FA688)(M_F(moby, 0x48), ((float (*)(float, float))func_L00_001FF8B0)(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)));
        if (f > 0.488692194f) {
            M_U8(moby, 0x20) = 3;
        }
        break;

    case 8:
        ((void (*)(void *, void *, float, float))func_L12_002E8808)(moby, data + 0x220, 0.0f, 0.0f);
        f = M_F(data, 0x23C) * 30.0f;
        if (f > 0.785398185f) {
            f = 0.785398185f;
        } else if (f < -0.785398185f) {
            f = -0.785398185f;
        }
        M_F(data, 0x188) = f * 0.699999988f;
        M_F(data, 0x208) = f * 0.400000006f;
        if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, data + 0x220) < 1.0f) {
            M_U8(moby, 0x20) = 2;
        }
        goto tail;

    case 4:
        M_U16(moby, 0x34) = M_U16(moby, 0x34) & 0xFFBE;
        r = func_L00_0025C698(moby, data + 0x70);
        if (r & 0x60) {
            if (((float (*)(void *, void *))func_001F9B80)(moby + 0x10, hero + 0x10) < 6.0f || M_U8(data, 0x242) != 0) {
                f = ((float (*)(float, float))func_001FA688)(M_F(moby, 0x48), ((float (*)(float, float))func_L00_001FF8B0)(M_F(hero, 0x10) - M_F(moby, 0x10), M_F(hero, 0x14) - M_F(moby, 0x14)));
                if (f < 0.122173049f) {
                    M_U8(moby, 0x20) = 5;
                    break;
                }
            }
            M_U8(moby, 0x20) = 3;
            break;
        }
        if (!(M_F(moby, 0x18) < 2.0f)) {
            goto tail;
        }
        goto finish;

    case 9:
        r = func_L00_0025C698(moby, data + 0x70);
        if (r & 0x60) {
            func_L00_00257470(moby, 0, -1);
            ((void (*)(void *, void *))func_L12_002E7290)(moby, moby + 0x10);
            goto finish;
        }
        if (M_F(moby, 0x18) < 2.0f) {
            func_L00_00257470(moby, 0, -1);
        finish:
            ((void (*)(void *))func_L12_002E86D0)(moby);
            return;
        }
        break;
    }

tail:
    if (M_U8(moby, 0x31) == 0) {
        char *o = M_P(data, 0x248);
        if (o == 0) {
            return;
        }
        if (M_U8(o, 0x31) == 0) {
            return;
        }
    }
    ((void (*)(void *))func_L12_002E8720)(moby);
    if (M_U16(moby, 0x34) & 0x8000) {
        M_F(data, 0x188) = -M_F(data, 0x188);
        M_F(data, 0x208) = -M_F(data, 0x208);
    }
    ((void (*)(void *, void *, int, float, float))func_L00_002628D8)(moby, data + 0x120, 5, (*(float *)&D_0015ED64) * 0.0179999992f, (*(float *)&D_0015ED64) * 0.300000012f);
    ((void (*)(void *, void *, int, float, float))func_L00_002628D8)(moby, data + 0x1A0, 6, (*(float *)&D_0015ED64) * 0.0240000002f, (*(float *)&D_0015ED64) * 0.300000012f);
    return;

}
