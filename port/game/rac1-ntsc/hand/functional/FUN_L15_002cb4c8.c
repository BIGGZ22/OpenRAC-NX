/* Carried from the PAL decompilation's src/overlays/shared/vendor_00298BB8.c (matched C there) (games/rac1) through the version
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
extern void func_L15_002CBAC0(char *m, int a);
extern int func_001F9740(void *);
extern void func_00212F90(void *, int, int, int);
extern float func_001FA580(float, float);
extern int func_001FA6E0(int, int, float);
extern int D_L15_0015F5C4 MACRO_ADDR;
extern int D_L15_0015F5CC MACRO_ADDR;
extern float func_001F9B80(float *, float *);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern int func_001F9688(int);
extern char D_0013E533[];
extern f32 D_0015ED6C MACRO_ADDR;
extern f32 func_001F9DE0(f32);
typedef struct L15AlarmMoby_c2cb4c8 {
    char pad0[0x10];
    float position[3];
    char pad1[4];
    unsigned char state;
    char pad2[0x32];
    unsigned char animation;
    char pad3[0x1C];
    unsigned char status;
    char pad4[7];
    void *data;
    char pad5[0x14];
    int w90;
} L15AlarmMoby_c2cb4c8;
typedef struct L15AlarmData_c2cb4c8 {
    int mode;
    int idx;
    int f8;
    int c;
    int d10;
    float f14;
    char blk18[16];
} L15AlarmData_c2cb4c8;
extern int D_L15_00161B3C_gp SDATA(D_L15_00161AB4);
extern int D_L15_00161B40_gp SDATA(D_L15_00161AB8);
extern int D_L15_00161B44_gp SDATA(D_L15_00161ABC);
extern int func_L15_002A2868(int idx, void *pos);
extern void func_L15_002CBC30(void *arg);



/* Sentrybot alarm update (moby class 408, levels 15 and 17): turn, watch for the hero, raise the alarm. */
void FUN_L15_002cb4c8(L15AlarmMoby_c2cb4c8 *m) {
    L15AlarmData_c2cb4c8 *d = m->data;
    float f;
    int r;

    switch (m->state) {
    case 0: {
        int mode = d->mode;

        D_L15_00161B3C_gp = 0;
        D_L15_00161B40_gp = 0;
        D_L15_00161B44_gp = 0;
        if (mode == 2) {
            m->state = 6;
            if (m->animation != 4)
                func_00212F90(m, 4, 0, 0);
        } else {
            m->state = 3;
            if (m->animation != 2)
                func_00212F90(m, 2, 0, 0);
        }
        break;
    }
    case 1:
        f = func_001F9DE0(d->f14);
        r = func_001FA6E0(0x801414BC, 0x801414DC, (f + 1.0f) * 0.5f);
        m->w90 = r;
        d->f14 = func_001FA580(d->f14, D_0015ED6C * 6.2831855f);
        if (!d->f8)
            goto tail;
        if (D_L15_0015F5C4 == 2)
            break;
        if (d->mode != 1) {
            m->state = 5;
            break;
        }
        m->state = 6;
        if (m->animation != 4)
            func_00212F90(m, 4, 0, func_001F9688(30));
        break;
    case 2:
        if (m->status & 2) {
            m->state = 3;
            if (m->animation != 2)
                func_00212F90(m, 2, 0, func_001F9688(10));
        }
        break;
    case 3:
        if (!d->f8)
            goto tail;
        if (D_L15_0015F5C4 == 2)
            break;
        m->state = 4;
        if (m->animation != 3)
            func_00212F90(m, 3, 0, func_001F9688(10));
        break;
    case 4:
        if (!(m->status & 2))
            break;
        if (d->f8 && D_L15_0015F5C4 != 2) {
            if (d->mode == 1) {
                m->state = 7;
                if (m->animation != 5)
                    func_00212F90(m, 5, 0, func_001F9688(10));
            } else {
                if (m->animation != 8)
                    func_00212F90(m, 8, 0, func_001F9688(10));
                m->state = 5;
            }
            break;
        }
        if (m->animation != 8)
            func_00212F90(m, 8, 0, func_001F9688(10));
        m->state = 10;
        break;
    case 5:
        if ((m->status & 2) && m->animation != 0)
            func_00212F90(m, 0, 0, func_001F9688(10));
        f = func_001F9DE0(d->f14);
        r = func_001FA6E0(0x801414DC, 0x8004048C, (f + 1.0f) * 0.5f);
        m->w90 = r;
        d->f14 = func_001FA580(d->f14, D_0015ED6C * 12.566371f);
        if (func_001F9740(d->blk18) == 0 && D_L15_0015F5C4 != 2)
            break;
        func_L15_002CBAC0((char *)m, 0);
        d->f8 = 0;
        if (m->animation != 8)
            func_00212F90(m, 8, 0, func_001F9688(10));
        m->state = 10;
        break;
    case 6:
        if (!d->f8)
            goto tail;
        m->state = 7;
        if (m->animation != 5)
            func_00212F90(m, 5, 0, func_001F9688(10));
        break;
    case 7:
        if (!(m->status & 2))
            break;
        m->state = 8;
        if (m->animation != 6)
            func_00212F90(m, 6, 0, func_001F9688(10));
        break;
    case 8: {
        float *pos = m->position;
        char *h2 = D_0013E533 + 0xE9D;
        float tmp[4];

        if (func_001F9B80(pos, (float *)h2) < 16.0f && D_L15_0015F5C4 != 2 && func_001F9740(&d->c)) {
            qcopy(tmp, pos);
            tmp[2] = tmp[2] + 1.0f;
            if (func_L00_001EFC70(tmp, h2 + 0x50, 2, 0, 0) == 0 && func_L15_002A2868(d->idx, pos))
                d->c = d->d10;
        }
        r = func_001F9740(d->blk18);
        if (r != 0 || D_L15_0015F5C4 == 2) {
            func_L15_002CBAC0((char *)m, 0);
            d->f8 = 0;
            m->state = 9;
            if (m->animation != 7)
                func_00212F90(m, 7, 0, func_001F9688(10));
        }
        break;
    }
    case 9:
        if (!(m->status & 2))
            break;
        if (d->mode == 2) {
            m->state = 6;
            if (m->animation != 4)
                func_00212F90(m, 4, 0, func_001F9688(10));
        } else {
            m->state = 10;
            if (m->animation != 8)
                func_00212F90(m, 8, 0, func_001F9688(10));
        }
        break;
    case 10:
        if (!(m->status & 2))
            break;
        m->state = 2;
        if (m->animation != 1)
            func_00212F90(m, 1, 0, func_001F9688(10));
        break;
    }
    if (d->f8)
        D_L15_00161B40_gp = 1;
tail:
    if (D_L15_00161B3C_gp != D_L15_0015F5CC) {
        D_L15_00161B3C_gp = D_L15_0015F5CC;
        func_L15_002CBC30(m);
        D_L15_00161B40_gp = 0;
    }
}
