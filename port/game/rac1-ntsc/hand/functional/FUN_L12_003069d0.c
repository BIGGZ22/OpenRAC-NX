/* Carried from the PAL decompilation's src/overlays/l12_hoven/vendor_002EDAA0.c (matched C there) (games/rac1) through the version
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
} Vec4f_c3069d0;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c3069d0 {
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
struct Moby_c3069d0 {
    Vec4f_c3069d0 bsphere;
    Vec4f_c3069d0 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c3069d0 *pclass;
    struct Moby_c3069d0 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c3069d0 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c3069d0 *moby);
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
    Vec4f_c3069d0 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c3069d0 unkD0;
    Vec4f_c3069d0 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c3069d0 Moby_c3069d0;
typedef struct MobyClass_c3069d0 MobyClass_c3069d0;
extern float func_L00_001FF8B0(float, float);
extern void func_001F9A28(void *, void *, void *);
/* Level 12 shot: flies with a smoke trail, damages the first moby it hits other than its owner, then pops.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/l12/gameplay/entities/002ec720.c, FUN_L12_002ece00. */
extern int D_0015EE84_e __asm__("D_0015ED84") MACRO_ADDR;
extern char D_0013E633_e[] __asm__("D_0013E533");
extern int func_00214720(void *, int);
extern void func_001F99F8(void *);
extern char D_L12_00167240[];
extern void func_001F9A10(void *, void *, void *);
extern void func_L00_002EA9D8(void *);
extern void func_L00_002EAA30(void *);
extern int func_001F9740(int *);
extern int D_L12_001619A0 MACRO_ADDR;
extern int D_L12_00161F24 MACRO_ADDR;
extern float func_001F9B80(void *, void *);
extern float func_001FA5C8(float, float);
extern float func_00213ED8(float *, float, float);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_00298840(int);
extern int func_001F9688(int);
extern float D_0015ED64 MACRO_ADDR;
extern void func_L00_0025B8C0(float *, float *, float, float, float, float);
extern int D_L12_00161F80_m __asm__("D_L12_00161F00") MACRO_ADDR;
extern int D_L12_00161F34 SDATA(D_L12_00161F34);
extern float func_001FA688(float, float);
extern int func_0022ED80_09590(int, int, void *) __asm__("func_0022DA68");
void func_L12_00306350(struct Moby_c3069d0 *moby, int slot);
extern void func_L12_003064E8(char *);
void func_L12_003054B0(char *moby);
extern f32 D_0015ED70 MACRO_ADDR;
extern void func_0020C828(void *);
extern float func_001FA580(float, float);
extern float func_L00_0025BC98(void *, float, void *, int, float, float, float);
extern int D_L12_001FB688[];
extern int D_L12_00160058_m __asm__("D_L12_0015FFD8") MACRO_ADDR;
extern float D_L12_00161F40 MACRO_ADDR;
extern float D_L12_00161F60 MACRO_ADDR;
extern float D_L12_00161F70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
void func_L12_00306828(char *moby);
typedef struct { float v[4]; } P_7DB0_c3069d0;
typedef struct { char pad[0x80]; float f80; } F80_7DB0_c3069d0;
typedef struct { char pad[0x34]; unsigned short flags; } M34_7DB0_c3069d0;
typedef struct { unsigned char pad0[0x74]; unsigned char active; unsigned char pad75[0x13]; char *owner; } Voice_7DB0_c3069d0;
extern P_7DB0_c3069d0 *D_L12_001B0C30_7DB0[] __asm__("D_L12_001B0930");
extern unsigned char D_0014C150_7DB0[][16] __asm__("D_0014C050");
extern int D_L12_001619B4 MACRO_ADDR;
extern char *D_L12_001600EC MACRO_ADDR;
extern int D_L12_0015F404 MACRO_ADDR;
extern short D_L12_00161F28;
extern short D_L12_00161F2C;
extern short D_L12_00161F30;
extern void func_L12_0027B268(char *, void *, int);
extern void func_L12_0027B370(char *, char *, int);
extern void func_L00_00262B00(float, float, char *, float *, float *);
extern void func_L00_00262B80(void *, char *, char *, float, float, float);
extern void func_L00_00258278(char *moby, float target, float *vel, float k, float d, float max);
extern float func_00213F38(float *, float *, float, float, float, float);
extern void func_001F9A40(void *, void *, void *, float);
extern void func_L00_002EAC18(int);
extern void func_L00_0028D918(int);



/* Planetbuster part update: rides its path, drops its parts and hands its voice slot back when done. */
void FUN_L12_003069d0(char *moby) {
    float v[4];
    float w[4];
    char *d = *(char **)(moby + 0x78);
    if (((unsigned char *)moby)[0x20] != 0 && D_L12_001FB688[17] != 0) {
        qcopy(moby + 0x10, (char *)&D_L12_00161F60);
        qcopy(moby + 0x40, (char *)&D_L12_00161F70);
    }
    switch ((unsigned char)moby[0x20]) {
    case 0:
        D_L12_001619B4 = 0;
        if (D_0014C150_7DB0[D_0015EE84_e][(unsigned char)moby[0xB0]] == 0xFF) {
            int i;
            for (i = 0; i < 17; i++) {
                int s = *(int *)(d + i * 0x30 + 0x20);
                if (s != -1) func_0020C828((char *)D_L12_00160058_m + (s << 8));
            }
            func_0020C828(moby);
            return;
        } else {
            P_7DB0_c3069d0 *path;
            int i;
            int j;
            D_L12_00161F24 = 0;
            func_L12_0027B268(moby, d, 0x11);
            path = D_L12_001B0C30_7DB0[*(int *)(d + 0x5C0)];
            qcopy(moby + 0x10, path + 1);
            *(float *)(d + 0x618) = *(float *)(moby + 0x18);
            *(float *)(moby + 0x48) = func_L00_001FF8B0(path[2].v[0] - path[1].v[0], path[2].v[1] - path[1].v[1]);
            if (*(float *)(D_0013E633_e + 0xEA5) < 52.0f) {
                *(float *)(moby + 0x18) = *(float *)(moby + 0x18) + 3.0f;
            }
            for (i = 0; i < 5; i++) {
                int k = *(int *)(d + 0x5D0 + i * 4);
                char *o = (char *)D_L12_00160058_m + (*(int *)(d + k * 0x30 + 0x20) << 8);
                (*(F80_7DB0_c3069d0 **)(o + 0x78))->f80 = 10.0f;
            }
            D_L12_00161F40 = -1.0f;
            for (j = 17; j >= 0; j--) D_L12_001FB688[j] = 0;
            *(int *)(d + 0x604) = -1;
            *(short *)(d + 0x5FC) = 100;
            moby[0x20] = 1;
        }
        break;
    case 1: {
        char *h;
        int r;
        if (func_00214720(D_L12_00167240, *(int *)(d + 0x624))) {
            int i;
            char *base = (char *)D_L12_00160058_m;
            *(unsigned short *)(moby + 0x34) |= 0x41;
            for (i = 0; i < 17; i++) {
                int s = *(int *)(d + i * 0x30 + 0x20);
                if (s != -1) ((M34_7DB0_c3069d0 *)(base + (s << 8)))->flags |= 0x41;
            }
        } else {
            int i;
            char *base = (char *)D_L12_00160058_m;
            *(unsigned short *)(moby + 0x34) &= ~0x41;
            for (i = 0; i < 17; i++) {
                int s = *(int *)(d + i * 0x30 + 0x20);
                if (s != -1) ((M34_7DB0_c3069d0 *)(base + (s << 8)))->flags &= ~0x41;
            }
        }
        h = D_0013E633_e + 0xE9D;
        r = func_00214720(h, *(int *)(d + 0x628));
        if (*(float *)(h + 8) > 52.0f && r == 0) {
            func_00213ED8((float *)(moby + 0x18), *(float *)(d + 0x618), D_0015ED6C + D_0015ED6C);
        } else {
            float add = 3.0f;
            if (r) add = 10.0f;
            if (func_00214720(D_0013E633_e + 0xE9D, *(int *)(d + 0x61C))) {
                *(int *)(d + 0x620) = func_001F9688(0x5A);
            }
            if (!func_001F9740((int *)(d + 0x620))) add = 19.0f;
            func_L00_0025B8C0((float *)(moby + 0x18), (float *)(d + 0x62C), *(float *)(d + 0x618) + add,
                              D_0015ED64 * 0.03f, D_0015ED64 * 0.3f, D_0015ED6C * 2.7f);
        }
        func_L00_00262B00(0.7f, D_0015ED6C * 1.2217305f, moby, (float *)(d + 0x5CC), (float *)(d + 0x5C8));
        func_L00_00262B80(moby, d + 0x5E4, d + 0x5E8, *(float *)&D_L12_00161F30,
                          *(float *)&D_L12_00161F28 * 0.017453292f * D_0015ED6C,
                          *(float *)&D_L12_00161F2C * 0.017453292f * D_0015ED6C);
        func_L12_0027B370(moby, d, 0x11);
        if (func_00214720(D_0013E633_e + 0xE9D, *(int *)(d + 0x5EC)) || D_L12_00161F80_m) {
            int i;
            D_L12_001619B4 = 1;
            for (i = 0; i < 5; i++) {
                int k = *(int *)(d + 0x5D0 + i * 4);
                ((unsigned char *)D_L12_00160058_m + (*(int *)(d + k * 0x30 + 0x20) << 8))[0xA4] = 0xFF;
            }
            moby[0x20] = 2;
        }
        break;
    }
    case 2: {
        P_7DB0_c3069d0 *path;
        int nx;
        float f;
        func_L00_00262B00(0.7f, D_0015ED6C * 1.2217305f, moby, (float *)(d + 0x5CC), (float *)(d + 0x5C8));
        func_L00_00262B80(moby, d + 0x5E4, d + 0x5E8, *(float *)&D_L12_00161F30,
                          *(float *)&D_L12_00161F28 * 0.017453292f * D_0015ED6C,
                          *(float *)&D_L12_00161F2C * 0.017453292f * D_0015ED6C);
        path = D_L12_001B0C30_7DB0[*(int *)(d + 0x5C0)];
        if (*(short *)(d + 0x5F8) < *(int *)path - 11) {
            func_00213ED8((float *)(d + 0x5F0), D_0015ED6C * 8.0f, D_0015ED70 * 10.0f);
        } else {
            func_00213ED8((float *)(d + 0x5F0), 0.0f, D_0015ED70 * 8.0f);
            if (*(float *)(d + 0x5F0) == 0.0f) {
                moby[0x20] = 3;
                break;
            }
        }
        nx = (*(short *)(d + 0x5F8) + 1) % *(int *)path;
        func_001F9A28(v, &path[nx + 1], &path[*(short *)(d + 0x5F8) + 1]);
        func_L00_001FF500(v, v, *(float *)(d + 0x5F0));
        func_001F9A10(moby + 0x10, moby + 0x10, v);
        f = func_L00_001FF8B0(*(float *)((char *)path + (nx << 4) + 0x10) - *(float *)((char *)path + (*(short *)(d + 0x5F8) << 4) + 0x10),
                              *(float *)((char *)path + (nx << 4) + 0x14) - *(float *)((char *)path + (*(short *)(d + 0x5F8) << 4) + 0x14));
        if (*(int *)path - 20 < *(short *)(d + 0x5F8)) f = 3.0543261f;
        func_L00_00258278(moby, f, (float *)(d + 0x5F4), D_0015ED64 * 0.0015f, D_0015ED64 * 0.3f, D_0015ED6C * 0.17453292f);
        if (func_001F9B80(moby + 0x10, &path[*(short *)(d + 0x5F8) + 1]) < 5.0f
            || func_001FA688(*(float *)(moby + 0x48),
                             func_L00_001FF8B0(*(float *)((char *)path + (*(short *)(d + 0x5F8) << 4) + 0x10) - *(float *)(moby + 0x10),
                                               *(float *)((char *)path + (*(short *)(d + 0x5F8) << 4) + 0x14) - *(float *)(moby + 0x14))) > 1.5707964f) {
            *(short *)(d + 0x5F8) = (*(short *)(d + 0x5F8) + 1) % *(int *)path;
        }
        func_L12_0027B370(moby, d, 0x11);
        func_L12_003054B0(moby);
        break;
    }
    case 3:
        D_L12_00161F34++;
        func_L00_0025BC98(moby + 0x48, 3.0543261f, d + 0x5F4, -1, D_0015ED64 * 0.0015f, D_0015ED64 * 0.3f, D_0015ED6C * 0.17453292f);
        func_L00_00262B00(0.7f, D_0015ED6C * 1.2217305f, moby, (float *)(d + 0x5CC), (float *)(d + 0x5C8));
        func_L00_00262B80(moby, d + 0x5E4, d + 0x5E8, *(float *)&D_L12_00161F30,
                          *(float *)&D_L12_00161F28 * 0.017453292f * D_0015ED6C,
                          *(float *)&D_L12_00161F2C * 0.017453292f * D_0015ED6C);
        func_L12_0027B370(moby, d, 0x11);
        func_L12_003054B0(moby);
        break;
    case 4:
        if (*(int *)(d + 0x608) != -1 && *(int *)(d + 0x60C) != -1) {
            switch ((unsigned char)moby[0xBC]) {
            case 0:
                moby[0xBC] = 1;
                func_L00_002EA9D8(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x30);
                func_L00_002EAA30(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x70);
                D_L12_0015F404 = 1;
                D_L12_001619A0 = 1;
                break;
            case 1:
                if (*(float *)(d + 0x610) < 1.0f) {
                    func_00213F38((float *)(d + 0x610), (float *)(d + 0x614), 1.0f,
                                  D_0015ED70 * 0.666f, D_0015ED70 * 0.666f, D_0015ED6C * 0.4f);
                    func_001F9A40(v, D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x30,
                                  D_L12_001600EC + *(int *)(d + 0x60C) * 128 + 0x30, *(float *)(d + 0x610));
                    func_001F99F8(w);
                    w[0] = func_001FA5C8(*(float *)(D_L12_001600EC + *(int *)(d + 0x60C) * 128 + 0x70),
                                         *(float *)(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x70)) * *(float *)(d + 0x610);
                    w[0] = func_001FA580(*(float *)(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x70), w[1]);
                    w[1] = func_001FA5C8(*(float *)(D_L12_001600EC + *(int *)(d + 0x60C) * 128 + 0x74),
                                         *(float *)(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x74)) * *(float *)(d + 0x610);
                    w[1] = func_001FA580(*(float *)(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x74), w[1]);
                    w[2] = func_001FA5C8(*(float *)(D_L12_001600EC + *(int *)(d + 0x60C) * 128 + 0x78),
                                         *(float *)(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x78)) * *(float *)(d + 0x610);
                    w[2] = func_001FA580(*(float *)(D_L12_001600EC + *(int *)(d + 0x608) * 128 + 0x78), w[2]);
                    func_L00_002EA9D8(v);
                    func_L00_002EAA30(w);
                } else {
                    moby[0xBC] = 2;
                    D_L12_0015F404 = 0;
                    func_L00_002EAC18(3);
                }
                break;
            case 2: {
                int i;
                for (i = 0; i < 17; i++) {
                    int s = *(int *)(d + i * 0x30 + 0x20);
                    if (s != -1) {
                        char *o = (char *)D_L12_00160058_m + (s << 8);
                        if (o[0x20] >= 0) func_0020C828(o);
                        *(int *)(d + i * 0x30 + 0x20) = -1;
                    } else if (D_L12_001FB688[i] > 0) {
                        char *o = (char *)D_L12_00160058_m + (D_L12_001FB688[i] << 8);
                        if (o[0x20] >= 0) func_0020C828(o);
                    }
                }
                D_L12_001619B4 = 0;
                func_L00_00298840(7);
                func_0020C828(moby);
                return;
            }
            }
        }
        if (*(short *)(d + 0x5FA) == 0) {
            int i;
            for (i = 0; i < 17; i++) {
                if (D_L12_001FB688[i] == 0) D_L12_001FB688[i] = *(int *)(d + i * 0x30 + 0x20);
                *(int *)(d + i * 0x30 + 0x20) = -1;
            }
            D_L12_001FB688[17] = 1;
            for (i = 0; i < 18; i++) func_L12_00306350((struct Moby_c3069d0 *)moby, i);
        }
        *(short *)(d + 0x5FA) = *(short *)(d + 0x5FA) + 1;
        func_L12_003064E8(moby);
        break;
    }
    func_L12_00306828(moby);
    if ((unsigned char)moby[0x20] >= 1 && (unsigned char)moby[0x20] <= 3) {
        if (*(int *)(d + 0x604) == -1) *(int *)(d + 0x604) = func_0022ED80_09590(0, 4, moby);
    } else {
        int s = *(int *)(d + 0x604);
        if (s != -1) {
            Voice_7DB0_c3069d0 *e = (Voice_7DB0_c3069d0 *)(D_0013E633_e + 0x1D + s * 0x70);
            if (e->owner == moby && e->active != 0) func_L00_0028D918(s);
        }
        *(int *)(d + 0x604) = -1;
    }
}
