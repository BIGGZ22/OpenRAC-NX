/* Carried from the PAL decompilation's src/overlays/l04_eudora/vuchain_00293490.c (matched C there) (games/rac1) through the version
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
} Vec4f_c295778;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c295778 {
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
struct Moby_c295778 {
    Vec4f_c295778 bsphere;
    Vec4f_c295778 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c295778 *pclass;
    struct Moby_c295778 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c295778 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c295778 *moby);
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
    Vec4f_c295778 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c295778 unkD0;
    Vec4f_c295778 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c295778 Moby_c295778;
typedef struct MobyClass_c295778 MobyClass_c295778;
extern float func_L04_002418B0(int, int);
extern void func_0020CD48(struct Moby_c295778 *m, int count, int *joints, void *out);
extern void func_001F9BC0_2939E8(float *) __asm__("func_001F99F8");
extern float func_001F9B88_2939E8(float) __asm__("func_001F99C0");
extern void func_001F9BD8_2939E8(void *, void *, void *) __asm__("func_001F9A10");
extern float func_001F9F90_2939E8(float) __asm__("func_001F9DC8");
extern float func_001F9FA8_2939E8(float) __asm__("func_001F9DE0");
extern float func_001F9CE8_2939E8(void *) __asm__("func_001F9B20");
extern float func_L00_00258110(float, float, float *, float, float, float);
extern f32 D_0015ED60 MACRO_ADDR;
extern float func_0020C9E0(struct Moby_c295778 *);
extern float func_001FA6C0(int);
extern int func_L04_00293990_96938(char *, char *, float) __asm__("func_L04_002927D0");
extern void func_L04_00293F08_96938(char *, char *, int, float) __asm__("func_L04_00292D48");
extern int func_L04_002939E8_96938(char *, char *) __asm__("func_L04_00292828");
extern void func_001F9A28(void *dst, void *a, void *b);
extern void func_L00_001FF378(void *, void *, float);
extern void func_L00_002A2900_96938(char *, char *, float, float, float, int) __asm__("func_L00_002A1670");
extern void func_L01_00263A28(void *, int, int *, float *, float *, float *);
extern int func_L00_001F19A0(float, float, void *, int, void *);
extern float func_001FA688(float, float);
extern char D_L04_00173FF0[];
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED68 MACRO_ADDR;



/* Level 04 walker update: blends heading and speed, places the legs, probes the ground and sets the anim rate; returns flags. */
int FUN_L04_00295778(char *m, char *s, float speed, float ang, float accel) {
    float buf[8];
    float v20[24];
    float v80[4];
    int a90;
    float a94;
    float a98;
    float t;
    int flags;
    int res;
    int changed;
    int mask;
    int bit;
    int k;
    int i;
    float cnt;
    float one;
    char **pp;
    float *f;

    if (*(unsigned char *)(m + 0x52) != 0xFF) {
        t = func_0020C9E0((struct Moby_c295778 *)m);
    } else {
        t = func_001FA6C0(*(unsigned char *)(m + 0x51)) + func_L04_002418B0(*(unsigned char *)(m + 0x22), *(unsigned char *)(m + 0x53));
    }
    flags = func_L04_00293990_96938(m, s, t);
    if (*(unsigned short *)(s + 0xEE) & 4) {
        *(float *)(m + 0x48) = func_L00_00258110(*(float *)(m + 0x48), *(float *)(s + 0x6C), (float *)(s + 0xC8),
                                                 *(float *)(s + 0xCC), *(float *)(s + 0xD0), *(float *)(s + 0xD4));
    } else if (flags & 0xF) {
        *(float *)(m + 0x48) = func_L00_00258110(*(float *)(m + 0x48), ang, (float *)(s + 0xC8),
                                                 *(float *)(s + 0xCC), *(float *)(s + 0xD0), *(float *)(s + 0xD4));
    } else {
        *(float *)(m + 0x48) = func_L00_00258110(*(float *)(m + 0x48), ang, (float *)(s + 0xC8),
                                                 0.0f, *(float *)(s + 0xD0) + *(float *)(s + 0xD0), *(float *)(s + 0xD4));
    }
    func_L04_00293F08_96938(m, s, flags, ang);
    *(float *)(m + 0x44) = func_L00_00258110(*(float *)(m + 0x44), *(float *)(s + 0xB8), (float *)(s + 0xBC),
                                             0.005f, 0.01f, D_0015ED60 * 0.017453292f);

    ((int *)buf)[0] = *(unsigned char *)(s + 0xF0);
    ((int *)buf)[1] = *(unsigned char *)(s + 0xF1);
    ((int *)buf)[2] = *(unsigned char *)(s + 0x1A0);
    ((int *)buf)[3] = *(unsigned char *)(s + 0x1A1);
    ((int *)buf)[4] = *(unsigned char *)(s + 0xB4);
    ((int *)buf)[5] = *(unsigned char *)(s + 0xB5);
    func_0020CD48((struct Moby_c295778 *)m, 6, (int *)buf, v20);

    if (!(*(unsigned short *)(s + 0xEE) & 4)) {
        mask = flags | (*(unsigned char *)(s + 0xB6) & 0x30);
        cnt = 0.0f;
        one = 1.0f;
        func_001F9BC0_2939E8(v80);
        bit = 1;
        for (i = 0; i < 6; i++) {
            if (mask & bit) {
                cnt = cnt + one;
                func_001F9BD8_2939E8(v80, v80, s + i * 0x10);
                func_001F9A28(v80, v80, &v20[i * 4]);
            }
            bit <<= 1;
        }
        func_L00_001FF378(v80, v80, cnt);
    } else {
        v80[0] = func_001F9F90_2939E8(*(float *)(s + 0x6C)) * *(float *)(s + 0xC0);
        v80[1] = func_001F9FA8_2939E8(*(float *)(s + 0x6C)) * *(float *)(s + 0xC0);
        v80[2] = 0.0f;
    }
    for (i = 0; i < 6; i++) {
        func_001F9BD8_2939E8(s + i * 0x10, &v20[i * 4], v80);
    }
    func_001F9BD8_2939E8(m + 0x10, m + 0x10, v80);

    changed = (flags ^ *(unsigned char *)(s + 0xB6)) & flags;
    *(unsigned char *)(s + 0xB6) = flags;
    if (*(float *)(s + 0xC4) < accel) accel = *(float *)(s + 0xC4);
    if (*(float *)(s + 0xC0) < speed) {
        *(float *)(s + 0xC0) = *(float *)(s + 0xC0) + accel;
        if (speed < *(float *)(s + 0xC0)) *(float *)(s + 0xC0) = speed;
    } else if (speed < *(float *)(s + 0xC0)) {
        *(float *)(s + 0xC0) = *(float *)(s + 0xC0) - accel;
        if (*(float *)(s + 0xC0) < speed) *(float *)(s + 0xC0) = speed;
    }

    res = func_L04_002939E8_96938(m, s);
    func_L00_002A2900_96938(m, s, speed, ang, accel, changed);

    if (!(*(unsigned short *)(s + 0xEE) & 4)) {
        func_L01_00263A28(m, 0, &a90, (float *)buf, &a94, &a98);
        if (*(unsigned char *)(m + 0x53) == **(int **)(s + 0x7C) ||
            *(unsigned char *)(m + 0x53) == **(int **)(s + 0x74) ||
            *(unsigned char *)(m + 0x53) == **(int **)(s + 0x78)) {
            *(float *)(s + 0xE8) = *(float *)(s + 0xE0) + *(float *)(s + 0xE0);
        } else if (*(unsigned char *)(m + 0x53) == **(int **)(s + 0x8C) && (*(unsigned char *)(s + 0xB6) & 0xF)) {
            *(float *)(s + 0xE8) = *(float *)(s + 0xE0) + *(float *)(s + 0xE0);
        } else if (*(unsigned char *)(m + 0x53) == *(unsigned char *)(m + 0x52)) {
            *(float *)(s + 0xE8) = *(float *)(s + 0xE8) * 0.9f;
        }
        buf[2] = buf[2] + *(float *)(s + 0xE8);
        a98 = a98 - *(float *)(s + 0xE8);
        func_001F9A28(buf + 4, m + 0x10, buf);
        for (k = 0; k < 8 && func_L00_001F19A0(a94, a98, buf, 4, m); k++) {
            qcopy(buf, D_L04_00173FF0);
            res |= 1;
        }
        func_001F9A28(v20, buf, m + 0x10);
        func_001F9BD8_2939E8(m + 0x10, buf, buf + 4);
        if (func_001F9CE8_2939E8(v20) < *(float *)(s + 0xC0) * 0.5f) res |= 2;
    }

    ((int *)buf)[0] = *(unsigned char *)(s + 0xF0);
    ((int *)buf)[1] = *(unsigned char *)(s + 0xF1);
    ((int *)buf)[2] = *(unsigned char *)(s + 0x1A0);
    ((int *)buf)[3] = *(unsigned char *)(s + 0x1A1);
    ((int *)buf)[4] = *(unsigned char *)(s + 0xB4);
    ((int *)buf)[5] = *(unsigned char *)(s + 0xB5);
    func_0020CD48((struct Moby_c295778 *)m, 6, (int *)buf, s);

    if (*(unsigned char *)(s + 0xB7) == 0x12 && *(unsigned char *)(m + 0x52) == **(int **)(s + 0xA0)) {
        return res;
    }
    *(float *)(m + 0x58) = 1.0f;
    if (*(unsigned char *)(s + 0xB7) != 0) {
        pp = (char **)(s + 0x70);
        for (i = 0; i < 13; i++, pp++) {
            if (*((int **)(s + 0x70))[i] == *(unsigned char *)(m + 0x52)) {
                if (!(*(float *)(s + 0xC0) < D_0015ED6C * 0.19f) ||
                    !(func_001FA688(*(float *)(m + 0x48), ang) > 0.092399783f) ||
                    !(speed < D_0015ED6C * 0.19f)) {
                    f = ((float **)(s + 0x70))[i];
                    t = *(float *)(s + 0xC0);
                } else {
                    t = D_0015ED6C * 0.19f;
                    f = (float *)*pp;
                }
                *(float *)(m + 0x58) = D_0015ED68 * func_001F9B88_2939E8(t / f[2]);
                break;
            }
        }
    }
    return res;
}
