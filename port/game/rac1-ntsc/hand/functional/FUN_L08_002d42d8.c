/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), src/overlays/l08_batalia/vendor_002B9438.c (games/rac1): func_L08_002D55E0, matched C there.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* carry, not from a `same` function or an `exact` data address (read them):
 *   D_0013D355 -> 0013D255: between
 *   D_L08_00161938 -> 001618A8: between
 *   D_L08_0016195C -> 001618CC: between
 *   D_L08_00161960 -> 001618D0: between
 *   func_L08_002D5FA8 -> 002D4CA0: near
 */

typedef short s16_c08_2d42d8;
typedef int s32_c08_2d42d8;
typedef unsigned char u8_c08_2d42d8;
typedef unsigned short u16_c08_2d42d8;
typedef unsigned int u32_c08_2d42d8;
typedef unsigned long u64_c08_2d42d8;
typedef float f32_c08_2d42d8;
static __inline__ void qcopy_c08_2d42d8(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}
typedef struct {
    f32_c08_2d42d8 x;
    f32_c08_2d42d8 y;
    f32_c08_2d42d8 z;
    f32_c08_2d42d8 w;
} Vec4f_c08_2d42d8;
struct Moby_c08_2d42d8;
struct Manip_c08_2d42d8;
struct GifEntry_c08_2d42d8;
struct AnimSeq_c08_2d42d8;
struct MobyClass_c08_2d42d8 {
    u8_c08_2d42d8 pad_0[0xC];
    u8_c08_2d42d8 seq_count;
    u8_c08_2d42d8 pad_D[3];
    u32_c08_2d42d8 unk10;
    u8_c08_2d42d8 pad_14[0x8];
    void *unk1C;
    struct GifEntry_c08_2d42d8 *gifs;
    f32_c08_2d42d8 scale;
    s32_c08_2d42d8 unk28;
    void **callbacks;
    u8_c08_2d42d8 pad_30[0x14];
    u16_c08_2d42d8 flags;
    s16_c08_2d42d8 unk46;
    struct AnimSeq_c08_2d42d8 *seqs[1];
};
struct Moby_c08_2d42d8 {
    Vec4f_c08_2d42d8 bsphere;
    Vec4f_c08_2d42d8 pos;
    u8_c08_2d42d8 state;
    u8_c08_2d42d8 group;
    u8_c08_2d42d8 unk22;
    u8_c08_2d42d8 unk23;
    struct MobyClass_c08_2d42d8 *pclass;
    struct Moby_c08_2d42d8 *next;
    f32_c08_2d42d8 scale;
    u8_c08_2d42d8 unk30;
    u8_c08_2d42d8 unk31;
    s16_c08_2d42d8 unk32;
    u16_c08_2d42d8 flags;
    u16_c08_2d42d8 unk36;
    u64_c08_2d42d8 spawn_frame;
    Vec4f_c08_2d42d8 rot;
    u8_c08_2d42d8 frame;
    u8_c08_2d42d8 prev_frame;
    u8_c08_2d42d8 seq;
    u8_c08_2d42d8 prev_seq;
    f32_c08_2d42d8 unk54;
    f32_c08_2d42d8 unk58;
    u8_c08_2d42d8 pad5C[8];
    struct Manip_c08_2d42d8 *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8_c08_2d42d8 unk70;
    u8_c08_2d42d8 unk71;
    u8_c08_2d42d8 unk72;
    u8_c08_2d42d8 unk73;
    void (*update)(struct Moby_c08_2d42d8 *moby);
    u8_c08_2d42d8 *pvars;
    u8_c08_2d42d8 unk7C;
    u8_c08_2d42d8 pad7D;
    u8_c08_2d42d8 unk7E;
    u8_c08_2d42d8 unk7F;
    s32_c08_2d42d8 unk80;
    u8_c08_2d42d8 pad84[2];
    s16_c08_2d42d8 unk86;
    u8_c08_2d42d8 pad88[8];
    s32_c08_2d42d8 unk90;
    u32_c08_2d42d8 unk94;
    s32_c08_2d42d8 unk98;
    u8_c08_2d42d8 pad9C[8];
    u8_c08_2d42d8 unkA4;
    u8_c08_2d42d8 padA5;
    s16_c08_2d42d8 oclass;
    u8_c08_2d42d8 padA8[8];
    u8_c08_2d42d8 unkB0;
    u8_c08_2d42d8 unkB1;
    u16_c08_2d42d8 save_id;
    s16_c08_2d42d8 unkB4;
    s16_c08_2d42d8 unkB6;
    void *unkB8;
    u8_c08_2d42d8 unkBC;
    u8_c08_2d42d8 padBD[3];
    Vec4f_c08_2d42d8 unkC0;
    Vec4f_c08_2d42d8 unkD0;
    Vec4f_c08_2d42d8 unkE0;
    u8_c08_2d42d8 padF0[0x10];
};
typedef struct Moby_c08_2d42d8 Moby_c08_2d42d8;
typedef struct MobyClass_c08_2d42d8 MobyClass_c08_2d42d8;
extern char *D_L08_00160058_2B94B0_c08_2d42d8 __asm__("D_L08_0015FFD8") ;
extern char *func_0020C4F8(int);
extern int func_001F9688(int);
extern void func_L00_00250DF8(void *);
extern float func_001FA580(float, float);
extern int func_0022DA68(int, int, void *);
extern void func_001F99F8(void *);
extern void func_0020C828(void *);
extern float D_0015EE6C_36F8_c08_2d42d8 __asm__("D_0015ED6C") ;
extern char D_0013E533[] ;
extern char D_0013D255[] ;
extern unsigned char D_0013D3AD[] ;
extern unsigned char D_0013D4C2[] ;
extern char D_0014171B_55E0_c08_2d42d8[] __asm__("D_0014161B") ;
extern int D_L08_001B0CB0[];
extern const float D_L08_0016195C_c08_2d42d8 __asm__("D_L08_001618CC" "__gp") ;
extern float D_L08_00161960_c08_2d42d8 __asm__("D_L08_001618D0" "__gp") ;
extern float D_L08_00161938_c08_2d42d8 __asm__("D_L08_001618A8" "__gp") ;
extern void func_0020CB10(void *, int, void *);
extern void func_00213DE0_55E0_c08_2d42d8(void *, int, int, int) __asm__("func_00212F90");
extern int func_001FA898_55E0_c08_2d42d8(float) __asm__("func_001FA6D0");
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F9B80(void *, void *);
extern int func_L00_00203908(int, int);
extern int func_00214CC8(char *, float);
extern float func_0020D830_55E0_c08_2d42d8(void *) __asm__("func_0020C9E0");
extern void func_L00_00250800_55E0_c08_2d42d8(void *, int, void *) __asm__("func_L00_0024F7C8");
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_00260738(void *, void *, void *, void *);
extern void func_L08_002D5FA8_55E0_c08_2d42d8(void *) __asm__("func_L08_002D4CA0");
extern void func_L08_002D68D0_55E0_c08_2d42d8(void *, int) __asm__("func_L08_002D55C8");
extern void func_L08_002D7758_55E0_c08_2d42d8(void *) __asm__("func_L08_002D6450");
extern void func_L08_002D7010_55E0_c08_2d42d8(void *) __asm__("func_L08_002D5D08");
extern void func_L08_002D5950(char *moby, int a1, int count);
extern char *func_L08_002DC8A0(char *src, char *pos, char *vec);
void FUN_L08_002d42d8(Moby_c08_2d42d8 *moby) {
    float old[4];
    float rot[4];
    float vel[4];
    float tmp[4];
    unsigned char *d = moby->pvars;
    Moby_c08_2d42d8 *target;
    qcopy_c08_2d42d8(old, &moby->pos);
    qcopy_c08_2d42d8(rot, &moby->rot);
    if (d[0x2E] == 2) {
        if (*(Moby_c08_2d42d8 **)(d + 0x160) != 0) func_0020C828(*(Moby_c08_2d42d8 **)(d + 0x160));
        if (*(Moby_c08_2d42d8 **)(d + 0x164) != 0) func_0020C828(*(Moby_c08_2d42d8 **)(d + 0x164));
        func_0020C828(moby);
        return;
    }
    func_L08_002D5FA8_55E0_c08_2d42d8(moby);
    target = *(Moby_c08_2d42d8 **)(d + 0xF0);
    switch (moby->state) {
    case 0:
        if (*(Moby_c08_2d42d8 **)(d + 0x160) == 0) {
            Moby_c08_2d42d8 *p;
            *(Moby_c08_2d42d8 **)(d + 0x160) = (Moby_c08_2d42d8 *)func_0020C4F8(0x14B);
            (*(Moby_c08_2d42d8 **)(d + 0x160))->unk30 = 0x40;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->unk32 = 0x40;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->unk31 = 1;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->spawn_frame = (*(Moby_c08_2d42d8 **)(D_0013E533 + 0x2E9D))->spawn_frame;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->flags = 0x4000;
            p = *(Moby_c08_2d42d8 **)(d + 0x160);
            qcopy_c08_2d42d8(&p->pos, &moby->pos);
            p->unk94 = 0;
            func_00213DE0_55E0_c08_2d42d8(*(Moby_c08_2d42d8 **)(d + 0x160), 0, 0, 0);
            func_L00_00250DF8(*(Moby_c08_2d42d8 **)(d + 0x160));
        }
        if (*(Moby_c08_2d42d8 **)(d + 0x164) == 0) {
            Moby_c08_2d42d8 *p;
            *(Moby_c08_2d42d8 **)(d + 0x164) = (Moby_c08_2d42d8 *)func_0020C4F8(0x14B);
            (*(Moby_c08_2d42d8 **)(d + 0x164))->unk30 = 0x40;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->unk32 = 0x40;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->unk31 = 1;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->spawn_frame = (*(Moby_c08_2d42d8 **)(D_0013E533 + 0x2E9D))->spawn_frame;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->flags = 0xC000;
            p = *(Moby_c08_2d42d8 **)(d + 0x164);
            qcopy_c08_2d42d8(&p->pos, &moby->pos);
            p->unk94 = 0;
            func_00213DE0_55E0_c08_2d42d8(*(Moby_c08_2d42d8 **)(d + 0x164), 0, 0, 0);
            func_L00_00250DF8(*(Moby_c08_2d42d8 **)(d + 0x164));
        }
        func_0020CB10(moby, 0, d + 0x100);
        moby->flags |= 0x1000;
        *(float *)(d + 0x16C) = D_0015EE6C_36F8_c08_2d42d8 + D_0015EE6C_36F8_c08_2d42d8;
        if (*(int *)(d + 0x140) > 0) {
            moby->state = 11;
            moby->flags = (moby->flags & 0xEFFF) | 0x41;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->flags |= 0x41;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->flags |= 0x41;
            moby->unk94 = 0;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->unk94 = 0;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->unk94 = 0;
            if (moby->prev_seq != 0) func_00213DE0_55E0_c08_2d42d8(moby, 0, 0, 0);
        } else if (*(int *)(d + 0x144) > 0) {
            if (D_0013D3AD[9] == 0) {
                moby->state = 14;
                moby->flags |= 0x41;
                (*(Moby_c08_2d42d8 **)(d + 0x160))->flags |= 0x41;
                (*(Moby_c08_2d42d8 **)(d + 0x164))->flags |= 0x41;
                moby->unk94 = 0;
                (*(Moby_c08_2d42d8 **)(d + 0x160))->unk94 = 0;
                (*(Moby_c08_2d42d8 **)(d + 0x164))->unk94 = 0;
                if (moby->prev_seq != 0) func_00213DE0_55E0_c08_2d42d8(moby, 0, 0, 0);
            } else {
                func_L08_002D7758_55E0_c08_2d42d8(moby);
                qcopy_c08_2d42d8(&moby->pos, (char *)D_L08_001B0CB0[*(int *)(d + 0x150)] + 0x10);
                moby->state = 6;
                if (moby->prev_seq != 1) func_00213DE0_55E0_c08_2d42d8(moby, 1, 0, 0);
            }
        } else {
            qcopy_c08_2d42d8(&moby->pos, (char *)D_L08_001B0CB0[*(int *)(d + 0x150)] + 0x10);
            moby->state = 6;
            if (moby->prev_seq != 1) func_00213DE0_55E0_c08_2d42d8(moby, 1, 0, 0);
        }
        func_001F99F8(d + 0x40);
        d[0x29] = 0;
        d[0x28] = 3;
        *(float *)(d + 0x30) = 3.0f;
        *(float *)(d + 0x20) = 9.0f;
        d[0x2A] = 12;
        d[0x2E] = 1;
        *(unsigned short *)(d + 0x3E) |= 2;
        func_001F99F8(d + 0x40);
        *(float *)(d + 0x178) = 0.0f;
        *(float *)(d + 0x170) = moby->rot.z;
        d[0x58] = func_001FA898_55E0_c08_2d42d8(D_L08_0016195C_c08_2d42d8);
        d[0x5A] = func_001FA898_55E0_c08_2d42d8(D_L08_00161960_c08_2d42d8 * 8.0f);
        break;
    case 6:
        func_L08_002D68D0_55E0_c08_2d42d8(moby, *(int *)(d + 0x150));
        if (*(int *)(d + 0xF4) != 2 && func_001F9B80(&moby->pos, &target->pos) < 20.0f) {
            if (*(unsigned short *)(D_0014171B_55E0_c08_2d42d8 + 0x61D) == 0 && D_0013D4C2[9] == 0) {
                func_L00_00203908(0x1F44, 0x5A);
            }
            moby->state = 3;
            if (moby->prev_seq != 2) func_00213DE0_55E0_c08_2d42d8(moby, 2, 0, func_001F9688(10));
        }
        break;
    case 3:
        func_L08_002D68D0_55E0_c08_2d42d8(moby, *(int *)(d + 0x150));
        if (*(int *)(d + 0xF4) == 2 || func_001F9B80(&moby->pos, &target->pos) > 22.0f) {
            moby->state = 6;
            if (moby->prev_seq != 1) func_00213DE0_55E0_c08_2d42d8(moby, 1, 0, func_001F9688(10));
        }
        if (func_00214CC8((char *)moby, 4.0f) != 0 || func_00214CC8((char *)moby, 10.0f) != 0) {
            float k;
            char *shot;
            if (func_0020D830_55E0_c08_2d42d8(moby) == 4.0f) {
                func_L00_00250800_55E0_c08_2d42d8(moby, 1, tmp);
            } else {
                func_L00_00250800_55E0_c08_2d42d8(moby, 2, tmp);
            }
            k = D_L08_00161938_c08_2d42d8 * D_0015EE6C_36F8_c08_2d42d8;
            vel[0] = func_001F9DC8(func_001FA580(*(float *)(d + 0x170), moby->rot.z)) * (func_001F9DC8(*(float *)(d + 0x178)) * k);
            vel[1] = func_001F9DE0(func_001FA580(*(float *)(d + 0x170), moby->rot.z)) * (func_001F9DC8(*(float *)(d + 0x178)) * k);
            vel[2] = 0.0f;
            vel[2] = -func_001F9DE0(*(float *)(d + 0x178)) * k;
            shot = func_L08_002DC8A0((char *)moby, (char *)tmp, (char *)vel);
            if (shot != 0) shot[0x20] = 1;
        }
        break;
    case 11:
        if (*(float *)((Moby_c08_2d42d8 *)D_L08_00160058_2B94B0_c08_2d42d8 + *(int *)(d + 0x140))->pvars == 1.0f) {
            qcopy_c08_2d42d8(&moby->pos, (char *)D_L08_001B0CB0[*(int *)(d + 0x14C)] + 0x10);
            qcopy_c08_2d42d8(&(*(Moby_c08_2d42d8 **)(d + 0x160))->pos, &moby->pos);
            qcopy_c08_2d42d8(&(*(Moby_c08_2d42d8 **)(d + 0x164))->pos, &moby->pos);
            moby->flags &= 0xFFBE;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->flags &= 0xFFBE;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->flags &= 0xFFBE;
            moby->unk94 = moby->pclass->unk10;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->unk94 = (*(Moby_c08_2d42d8 **)(d + 0x160))->pclass->unk10;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->unk94 = (*(Moby_c08_2d42d8 **)(d + 0x164))->pclass->unk10;
            moby->flags |= 0x1000;
            if (*(int *)(d + 0xF4) != 2) {
                moby->state = 6;
                if (moby->prev_seq != 1) func_00213DE0_55E0_c08_2d42d8(moby, 1, 0, func_001F9688(10));
            }
        }
        break;
    case 14:
        if (*(int *)(d + 0xF4) != 2) {
            unsigned char *pers;
            func_L08_002D7010_55E0_c08_2d42d8(moby);
            pers = (unsigned char *)D_0013D255 + 0x133; /* 0x13D388 (lui/addiu at 0x2D4A38): PAL's 0x13D490 is 0x108 lower here, not 0x100 */
            if (pers[0x2E] != 0) func_0022DA68(6, 0, moby);
            pers[0x2E] = 1;
            qcopy_c08_2d42d8(&(*(Moby_c08_2d42d8 **)(d + 0x160))->pos, &moby->pos);
            qcopy_c08_2d42d8(&(*(Moby_c08_2d42d8 **)(d + 0x164))->pos, &moby->pos);
            moby->flags &= 0xFFBE;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->flags &= 0xFFBE;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->flags &= 0xFFBE;
            moby->unk94 = moby->pclass->unk10;
            (*(Moby_c08_2d42d8 **)(d + 0x160))->unk94 = (*(Moby_c08_2d42d8 **)(d + 0x160))->pclass->unk10;
            (*(Moby_c08_2d42d8 **)(d + 0x164))->unk94 = (*(Moby_c08_2d42d8 **)(d + 0x164))->pclass->unk10;
            moby->flags |= 0x1000;
            if (*(int *)(d + 0xF4) != 2) {
                moby->state = 6;
                if (moby->prev_seq != 1) func_00213DE0_55E0_c08_2d42d8(moby, 1, 0, func_001F9688(10));
            }
        }
        break;
    case 12:
        break;
    case 99: {
        float f;
        func_L08_002D68D0_55E0_c08_2d42d8(moby, *(int *)(d + 0x150));
        f = func_0020D830_55E0_c08_2d42d8(moby);
        if ((59.0f <= f && f <= 60.0f) || (119.0f <= f && f <= 120.0f)) {
            func_L08_002D5950((char *)moby, 4, 10);
        } else if ((29.0f <= f && f <= 30.0f) || (89.0f <= f && f <= 90.0f)) {
            func_L08_002D5950((char *)moby, 5, 10);
        }
        break;
    }
    }
    func_001F9A28(vel, &moby->pos, old);
    func_L00_00260738(d + 0x60, vel, &moby->rot, rot);
}
struct Manip_c08_2d42d8;
struct GifEntry_c08_2d42d8;
struct AnimSeq_c08_2d42d8;
extern int D_L08_001B0CB0[];
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F9B80(void *, void *);
extern int func_0022DA68(int, int, void *);
struct Manip_c08_2d42d8;
struct GifEntry_c08_2d42d8;
struct AnimSeq_c08_2d42d8;
extern void func_0020C828(void *);
extern float func_00213308(void);
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern void func_001F9A28(void *, void *, void *);
extern char D_0013E533[] ;
extern void func_0020CB10(void *, int, void *);
extern float func_001FA580(float, float);
extern void func_001F99F8(void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_00260738(void *, void *, void *, void *);
extern int D_L08_001B0CB0[];
extern float func_001F9B80(void *, void *);
extern float func_001FA580(float, float);
extern float func_001F9DE0(float);
extern int D_L08_001B0CB0[];
extern char *func_0020C4F8(int);
extern int func_001F9688(int);
struct Manip_c08_2d42d8;
struct GifEntry_c08_2d42d8;
struct AnimSeq_c08_2d42d8;
extern char D_0013D255[] ;
extern void func_0020C828(void*);
extern int func_0022DA68(int,int,void*);
extern float func_00213308(void),func_001F9DC8(float);
extern void func_0020C828(void *);
extern void func_001F9A28(void *dst, void *a, void *b);
