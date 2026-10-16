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
} Vec4f_c2923b8;
/* A moby class header, pointed to by Moby.pclass. Size unknown;
   only fields some matched function reads are named. */
struct MobyClass_c2923b8 {
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
struct Moby_c2923b8 {
    Vec4f_c2923b8 bsphere;
    Vec4f_c2923b8 pos;
    u8 state;                         /* >= 0xFE: dead, waiting to respawn */
    u8 group;                         /* linked group: index into the level moby-list table D_Lxx_001ABCC0 (0xFF: none) */
    u8 unk22;                         /* class slot: pclass = D_L00_00196F80[unk22] (FUN_L00_002cf218) */
    u8 unk23;                         /* 0x40 for the smoke trail FUN_L09_00307ba8 spawns */
    struct MobyClass_c2923b8 *pclass;
    struct Moby_c2923b8 *next;
    f32 scale;                        /* draw scale (FUN_L01_002fa068 halves it, FUN_L00_00215ef8 divides by it) */
    u8 unk30;                         /* set to 0xFF (0x7F for beams) by spawners */
    u8 unk31;                         /* set to 1 by spawners */
    s16 unk32;                        /* set to 0xFF (0x7F for beams) by spawners */
    u16 flags;
    u16 unk36;                        /* set to 0x7F80 by spawners */
    u64 spawn_frame;                  /* frame count at which it may respawn */
    Vec4f_c2923b8 rot;                        /* z: yaw (FUN_L00_00266448 compares it with atan2 to the hero) */
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
    void (*update)(struct Moby_c2923b8 *moby);
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
    Vec4f_c2923b8 unkC0;                      /* 0xC0: first row of a matrix built from rot (FUN_001fa030) */
    Vec4f_c2923b8 unkD0;
    Vec4f_c2923b8 unkE0;
    u8 padF0[0x10];
};
typedef struct Moby_c2923b8 Moby_c2923b8;
typedef struct MobyClass_c2923b8 MobyClass_c2923b8;
typedef struct {
    unsigned char unk0;
    unsigned char attached;
    unsigned char pad2[0xE];
    float rot[4];
    unsigned char pad20[0x20];
} LegRigManip_p_c2923b8;
typedef struct {
    unsigned char joint;
    unsigned char joint2;
    unsigned char slot_a;
    unsigned char slot_b;
    unsigned char pad4[4];
    float yaw;
    float lower_turn;
    float lower_turn_speed;
    float upper_turn;
    float upper_turn_speed;
    float angle_min;
    float angle_max;
    unsigned char pad24[0xC];
    LegRigManip_p_c2923b8 manip_a;
    LegRigManip_p_c2923b8 manip_b;
} LegRigLeg_p_c2923b8;
typedef struct {
    float span[2];
    float from[2];
    float to[2];
} LegRigPhase_p_c2923b8;
typedef struct {
    int id;
    float unk4;
    float rate;
    float start;
    float len;
    unsigned char pad14[0xC];
    LegRigPhase_p_c2923b8 phase[2];
} LegRigAnim_p_c2923b8;
typedef struct {
    float pts[6][4];
    unsigned char pad60[0x10];
    LegRigAnim_p_c2923b8 *anims[13];
    unsigned char padA4[0xC];
    unsigned char unkB0;
    unsigned char unkB1;
    unsigned char unkB2;
    unsigned char unkB3;
    unsigned char jointB4;
    unsigned char jointB5;
    unsigned char mode;
    unsigned char unkB7;
    int unkB8;
    int unkBC;
    float speed;
    float unkC4;
    int unkC8;
    float unkCC;
    float unkD0;
    float unkD4;
    int unkD8;
    float unkDC;
    float unkE0;
    int unkE4;
    int unkE8;
    short unkEC;
    short unkEE;
    LegRigLeg_p_c2923b8 legs[2];
} LegRig_p_c2923b8;
extern char D_L04_001DB360[];
extern char D_L04_001DB3F0[];
extern void func_001E93B0();
extern void func_001F97B0(void);
extern float func_L04_002418B0(int, int);
extern float func_L04_00241910(int, int);
extern float func_001F96B0(float);
extern void func_0020CD48(struct Moby_c2923b8 *m, int count, int *joints, void *out);
extern void func_0020CCA8(struct Moby_c2923b8 *m, int joint, float *mtx);
extern float func_L00_001FF8B0(float, float);
extern void func_0020CB10(struct Moby_c2923b8 *m, int slot, void *manip);


void FUN_L04_002923b8(struct Moby_c2923b8 *moby, LegRig_p_c2923b8 *rig) {
    int joints[6];
    float matrix[4][4];
    int i;
    int j;
    LegRigLeg_p_c2923b8 *leg;
    int k;

    for (i = 0; i < 13; i++) {
        if (rig->anims[i]->id < 0) {
            func_001E93B0(D_L04_001DB360, i, moby->oclass);
            func_001F97B0();
        }
        rig->anims[i]->start = func_L04_002418B0(moby->unk22, rig->anims[i]->id);
        rig->anims[i]->len = func_L04_00241910(moby->unk22, rig->anims[i]->id) - rig->anims[i]->start;
        if (rig->anims[i]->len != 0.0f) {
            for (j = 0; j < 2; j++) {
                rig->anims[i]->phase[1].span[j] = rig->anims[i]->phase[1].from[j] <= rig->anims[i]->phase[1].to[j]
                    ? rig->anims[i]->phase[1].to[j] - rig->anims[i]->phase[1].from[j]
                    : rig->anims[i]->phase[1].to[j] + rig->anims[i]->len - rig->anims[i]->phase[1].from[j];
                rig->anims[i]->phase[0].span[j] = rig->anims[i]->phase[0].from[j] <= rig->anims[i]->phase[0].to[j]
                    ? rig->anims[i]->phase[0].to[j] - rig->anims[i]->phase[0].from[j]
                    : rig->anims[i]->phase[0].to[j] + rig->anims[i]->len - rig->anims[i]->phase[0].from[j];
            }
            rig->anims[i]->rate = rig->anims[i]->unk4 / func_001F96B0(rig->anims[i]->len + rig->anims[i]->len);
        }
    }
    joints[0] = rig->legs[0].joint;
    joints[1] = rig->legs[0].joint2;
    joints[2] = rig->legs[1].joint;
    joints[3] = rig->legs[1].joint2;
    joints[4] = rig->jointB4;
    joints[5] = rig->jointB5;
    func_0020CD48(moby, 6, joints, rig);
    for (k = 0; k < 2; k++) {
        char *p = (char *)rig + k * 0xB0;
        func_0020CCA8(moby, ((unsigned char *)p)[0xF0], matrix[0]);
        *(int *)(p + 0x104) = 0;
        *(int *)(p + 0xFC) = 0;
        *(float *)(p + 0xF8) = func_L00_001FF8B0(matrix[1][0], matrix[1][1]);
        if (((unsigned char *)p)[0x161] == 0)
            func_0020CB10(moby, ((unsigned char *)p)[0xF3], (LegRigManip_p_c2923b8 *)(p + 0x160));
        if (((unsigned char *)p)[0x121] == 0)
            func_0020CB10(moby, ((unsigned char *)p)[0xF2], (LegRigManip_p_c2923b8 *)(p + 0x120));
    }
    rig->unkEE = 0;
    rig->unkB7 = 0;
    rig->unkB8 = 0;
    rig->unkBC = 0;
    rig->speed = 0.0f;
    rig->unkC8 = 0;
    rig->unkE8 = 0;
    rig->unkD8 = 0;
    rig->unkE4 = 0;
    rig->unkEC = 0;
    rig->mode = 5;
    func_001E93B0(D_L04_001DB3F0, 0x250);
}
