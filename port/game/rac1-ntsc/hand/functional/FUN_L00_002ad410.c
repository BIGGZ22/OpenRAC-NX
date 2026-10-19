/* Carried from the PAL decompilation's src/overlays/shared/vendor_002AB910.c (matched C there) (games/rac1) through the version
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
typedef int Q910_c2ad410 __attribute__((mode(TI)));
typedef union { Q910_c2ad410 q; float f[4]; } U910_c2ad410;
typedef struct M910_c2ad410 M910_c2ad410;
typedef struct W910_c2ad410 W910_c2ad410;
struct W910_c2ad410 {
    U910_c2ad410 vel;
    char p10[0x20];
    M910_c2ad410 *owner;
    short h34;
    short h36;
    char p38[8];
    float f40;
    char p44[4];
    int i48;
    M910_c2ad410 *p4C;
    short h50;
    short h52;
    M910_c2ad410 *p54;
    int i58;
    int i5C;
    float f60;
    float f64;
    int i68;
    float f6C;
    float f70;
    int i74;
    int i78;
};
struct M910_c2ad410 {
    char p0[0x10];
    U910_c2ad410 pos;
    unsigned char state;
    char p21[3];
    char *p24;
    char p28[0xC];
    unsigned short flags;
    char p36[0xA];
    float rx;
    float ry;
    float rz;
    char p4C[6];
    unsigned char b52;
    unsigned char anim;
    char p54[0x1C];
    unsigned char b70;
    char p71[7];
    W910_c2ad410 *v;
    char p7C[0x18];
    int i94;
    char p98[0xC];
    unsigned char bA4;
    char pA5;
    short cls;
    char pA8[0x14];
    unsigned char bBC;
    char pBD[3];
    float mtx[4][4];
};
typedef struct {
    char p0[0x18];
    M910_c2ad410 *moby;
    int i1C;
    U910_c2ad410 pos;
    char p30[0x10];
    U910_c2ad410 nrm;
} H910_c2ad410;
extern H910_c2ad410 D_H910 __asm__("D_L00_00173E40");
extern M910_c2ad410 *D_L00_00173E58;
extern float D_0015ED70 MACRO_ADDR;
extern s32 D_L00_00166DC0[];
/* NTSC-U calls FUN_L00_002603d0 here (jal 0x0C0980F4: returns the moby's word 0x78 -> +8), not
 * FUN_L00_0025c338 (the level's FUN_002141f8, word 0x78 -> +0) as the carry from PAL had it. */
extern unsigned char *func_L00_002603D0(unsigned char *);
extern void func_001F9A10(void *, void *, void *);
extern int func_001F9688(int);
extern void func_001F9A68(void *, void *, float);
extern float func_001F9DE0(float);
extern float func_001F9DC8(float);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern int func_L00_001F35D8(void);
extern float func_00213508(void *, int, float);
extern void func_001F9A28(void *, void *, void *);
extern int func_001F9740(int *);
extern float func_001FA580(float, float);
extern int D_L00_0015F5C4 MACRO_ADDR;
extern float func_L00_001FF8B0(float, float);
extern char D_0013E533[];
typedef struct { float f[4]; } __attribute__((aligned(16))) V_AE6B0_c2ad410;
typedef struct {
    V_AE6B0_c2ad410 vel;
    V_AE6B0_c2ad410 landing;
    V_AE6B0_c2ad410 normal;
    V_AE6B0_c2ad410 previous_hit;
    V_AE6B0_c2ad410 difference;
    char *holder;
    short timer;
    short holder_flag;
    char pad58[8];
    char *last_hit;
    int hit_count;
} Land_AE6B0_c2ad410;
extern void func_001F4600(void *, void *);
extern unsigned char D_0013E05A[];
extern void func_L00_002ACBF0(char *moby);
#define v ((Land_AE6B0_c2ad410 *)vp)


int FUN_L00_002ad410(void *vp, void *m, int mode) {
    V_AE6B0_c2ad410 position, velocity, previous, hero_position, offset, start;
    int timer, found = 0, object_hit = 0, i;
    char *hero = D_0013E533 + 0xE1D;
        char *last;
    char *cam;
    char *h;

    if (*(int *)(hero + 0x2084) == 0x72)
        return 0;
    if (D_L00_0015F5C4 != 0)
        return 0;
    qcopy(&previous, m + 0x10);
    if (*(unsigned char *)(m + 0x20) == 0) {
        position.f[0] = func_001F9DC8(func_001FA580(-0.36196801f, func_L00_001FF8B0(*(float *)(hero + 0x670), *(float *)(hero + 0x674)))) * 0.85903901f;
        position.f[1] = func_001F9DE0(func_001FA580(-0.36196801f, func_L00_001FF8B0(*(float *)(hero + 0x670), *(float *)(hero + 0x674)))) * 0.85903901f;
        position.f[2] = 0.0f;
        func_001F9A10(&position, &position, hero + 0x80);
        position.f[2] += 0.49082f;
        qcopy(&velocity, v);
        if (*(int *)(hero + 0x208C) == 0xF)
            func_001F9A10(&velocity, &velocity, hero + 0x100);
        if (*(int *)(hero + 0x2FC) != 0 && func_L00_002603D0(*(unsigned char **)(hero + 0x2FC)) && mode)
            func_001F9A10(&velocity, &velocity, hero + 0x100);
        timer = func_001F9688(300);
    } else {
        char *hp = *(char **)(v->holder + 0x78);
        int t50;
        qcopy(&position, m + 0x10);
        qcopy(&velocity, v);
        t50 = *(int *)(hp + 0x50);
        timer = v->timer;
        if (t50)
            v->holder_flag = 1;
    }
    h = D_0013E533 + 0xE1D;
    cam = *(char **)(h + 0x2080);
    hero_position = *(V_AE6B0_c2ad410 *)(cam + 0x10);
    hero_position.f[2] = position.f[2];
    if (func_L00_001EFC70(&hero_position, &position, 0x10, (int)cam, 0) &&
        (func_L00_001F35D8() || 0.0f < velocity.f[2])) {
        if (D_H910.moby != 0 && D_H910.i1C <= 0) {
            if (((char *)D_H910.moby != *(char **)(h + 0x2080) && (char *)D_H910.moby != v->holder) ||
                timer < func_001F9688(300) - func_001F9688(10)) {
                offset = (V_AE6B0_c2ad410){{0.0f, 0.0f, 0.0f, 0.0f}};
                offset.f[2] = 0.5f;
                {
                H910_c2ad410 *hh = &D_H910;
                if (v->last_hit != (char *)hh->moby) {
                    v->hit_count = 0;
                    v->last_hit = (char *)hh->moby;
                    v->previous_hit = *(V_AE6B0_c2ad410 *)&hh->moby->pos;
                } else {
                    v->hit_count++;
                    func_001F9A28(&start, &hh->moby->pos, &v->previous_hit);
                    v->difference = start;
                    v->previous_hit = *(V_AE6B0_c2ad410 *)&hh->moby->pos;
                }
                }
                object_hit = 1;
                qcopy(&v->landing, &D_H910.moby->pos);
                found = 1;
                func_001F9A10(&offset, &offset, &v->landing);
                func_00213508(&offset, 0, 0.5f);
            }
        } else if (D_H910.i1C > 0) {
            qcopy(&v->landing, &D_H910.pos);
            found = 1;
        }
    }
    if (D_0013E05A[0x4D0] != 0) {
        char *g = D_0013E533 + 0xE1D;
        if (*(int *)(g + 0x2084) == 1) {
            float z = velocity.f[2];
            func_001F9A68(&velocity, &velocity, 1.5f);
            velocity.f[2] = z / 1.5f;
        }
    }
    hero_position = position;
    while (!func_001F9740(&timer) && !found) {
        qcopy(&previous, &position);
        for (i = 9; i >= 0; i--) {
            func_001F9A10(&position, &position, &velocity);
            velocity.f[2] -= D_0015ED70 * 11.0f;
        }
        if (!func_L00_001EFC70(&previous, &position, 0x10, (int)m, 0))
            continue;
        if (!func_L00_001F35D8() && 0.0f < velocity.f[2])
            continue;
        if (D_H910.moby != 0 && D_H910.i1C <= 0) {
            if (!(position.f[2] - hero_position.f[2] < 2.0f))
                goto check_surface;
            {
                char *g = D_0013E533 + 0xE1D;
                if (((char *)D_H910.moby == *(char **)(g + 0x2080) || (char *)D_H910.moby == v->holder) &&
                    timer >= func_001F9688(300) - func_001F9688(10))
                    continue;
            }
            offset = (V_AE6B0_c2ad410){{0.0f, 0.0f, 0.0f, 0.0f}};
            offset.f[2] = 0.5f;
    {
    H910_c2ad410 *hh = &D_H910;
            if (v->last_hit != (char *)hh->moby) {
                v->hit_count = 0;
                v->last_hit = (char *)hh->moby;
                v->previous_hit = *(V_AE6B0_c2ad410 *)&hh->moby->pos;
            } else {
                v->hit_count++;
                func_001F9A28(&start, &hh->moby->pos, &v->previous_hit);
                v->difference = start;
                v->previous_hit = *(V_AE6B0_c2ad410 *)&hh->moby->pos;
            }
    }
            object_hit = 1;
            qcopy(&v->landing, &D_H910.moby->pos);
            func_001F9A10(&offset, &offset, &v->landing);
            func_00213508(&offset, 0, 0.5f);
            break;
        }
check_surface:
        if (D_H910.i1C > 0) {
            qcopy(&v->landing, &D_H910.pos);
            break;
        }
    }
    if (timer && (mode || (D_L00_00173E58 && func_L00_002603D0((unsigned char *)D_L00_00173E58)))) {
        if (mode != 2 || !D_L00_00173E58 || !func_L00_002603D0((unsigned char *)D_L00_00173E58))
            func_001F4600((void *)func_L00_002ACBF0, m);
        if (!object_hit && v->last_hit) {
            last = v->last_hit;
            if (*(unsigned char *)(last + 0x20) == 0xFE) {
                v->hit_count = 0;
            } else if (*(unsigned char *)(last + 0x20) == 0xFD) {
                v->hit_count = 0;
            } else if (!(*(unsigned short *)(last + 0x34) & 0x1000)) {
                v->hit_count = 0;
            } else {
                if (v->hit_count <= func_001F9688(30))
                    goto keep_hit;
                v->hit_count = 0;
            }
            v->last_hit = 0;
        }
    keep_hit:
        v->normal = *(V_AE6B0_c2ad410 *)&D_H910.nrm;
        func_001F9A28(&previous, &v->landing, D_L00_00166DC0);
        func_001F9A68(&previous, &previous, 0.95f);
        func_001F9A10(&v->landing, &previous, D_L00_00166DC0);
        return 1;
    }
    return 0;
}
