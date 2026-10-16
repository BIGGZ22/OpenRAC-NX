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
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c29a570 __attribute__((mode(TI), aligned(16)));
extern float func_001F9B48(void *, void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern void func_L15_0029A0D0(void *);
extern int D_L15_0015F5C4 MACRO_ADDR;
extern int *D_L15_001B0AB0[];
extern float D_0015ED6C MACRO_ADDR;
extern char D_0013E533[];
extern float D_0015ED70 MACRO_ADDR;
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern int D_L15_0015FFD8 MACRO_ADDR;
extern char *func_L00_0025A420(void *, int, int);
extern int func_L00_0025A478(void *, void *, void *, int, int *, float *, int, int);
extern void func_L00_0025AB48(void *, float *, void *, void *);
extern void func_L00_0025C558(void *, void *, int, int, int, float);
extern void func_L00_0025D458(void *, void *);
extern void func_L00_0025D538(void *, void *);
extern float func_002132A8(float, float);
#define func_002140F8_r func_002132A8
extern float func_001F96B0(float);
extern int func_L08_00211B08(void *);
#define func_L08_00211A38_r func_L08_00211B08
extern int func_001FA6D0(float);
#define func_001FA898_r func_001FA6D0
extern float func_001F99C0(float);
#define func_L00_001EFFF0_r func_L00_001EFC70
extern int func_L00_0025FF38(void *, void *, float, int, int, void *, int);
#define D_L15_00160058_m D_L15_0015FFD8
extern short D_L15_001613EC;
extern short D_L15_001613F0;
extern short D_L15_001613F4;
extern short D_L15_001613F8;
extern short D_L15_001613FC;
extern short D_L15_00161400;
extern short D_L15_00161404;
extern short D_L15_00161408;
extern short D_L15_0016140C;


void FUN_L15_0029a570(void *object, int flag) {
    unsigned char *moby = object;
    unsigned char *data = *(unsigned char **)(moby + 0x78);
    char *hit;
    float impact;
    int kind;
    int code;
    float angle;
    float fall_angle;
    float speed;
    int amount;
    float pos[4];
    int *path;
    int group;
    unsigned char seen;

    if (moby[0x20] == 0)
        return;
    *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L15_001613EC;
    impact = 0.0f;
    hit = func_L00_0025A420(moby, 0x330000, 0);
    code = func_L00_0025A478(moby, hit, data + 0x20, 0, &kind, &impact, 0, 4);
    if (kind != 1 && moby[0x20] != 0x17) {
        if (*(unsigned char *)(D_0013E533 + 0x2EC1) == 0)
            func_L15_0029A0D0(moby);
        *(float *)(data + 0x20) -= impact;
        if (*(short *)(*(char **)(hit + 0x20) + 0xA6) == 0x47)
            code = 3;
        if (*(float *)(data + 0x20) <= 0.0f)
            code = 1;
        amount = func_001FA898_r(512.0f);
        speed = *(float *)&D_L15_001613F0 * D_0015ED70;
        *(int *)(data + 0x90) = amount;
        *(float *)(data + 0x98) = 0.5f;
        *(int *)(data + 0x94) = 9;
        *(float *)(data + 0x80) = speed;
        data[0xAD] = 0;
        switch (code) {
        case 0:
        case 11:
            break;
        case 9:
        case 10:
            data[0x67] = 0xFA;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            {
                float step;
                float step_sq;
                float forward;
                float lateral;
                float vz;
                float fall_speed;
                step = D_0015ED6C;
                fall_speed = *(float *)&D_L15_001613F0;
                step_sq = D_0015ED70;
                fall_speed = fall_speed * step_sq;
                forward = *(float *)&D_L15_001613FC * step;
                lateral = *(float *)&D_L15_001613F8 * step;
                vz = *(float *)&D_L15_00161400 * step;
                *(float *)(data + 0x80) = fall_speed;
                *(float *)(data + 0x88) = forward;
                *(float *)(data + 0x8C) = lateral;
                *(float *)(data + 0x84) = vz;
                *(float *)(data + 0xC0) = 5.0f;
                *(float *)(data + 0xC4) = 10.0f;
            }
            {
                float heading = func_L00_001FF8B0(*(float *)(moby + 0x10) - *(float *)(*(char **)(hit + 0x20) + 0x10),
                                              *(float *)(moby + 0x14) - *(float *)(*(char **)(hit + 0x20) + 0x14));
                *(VecQuad_c29a570 *)pos = *(VecQuad_c29a570 *)(hit + 0x10);
                angle = heading;
            }
            func_L00_0025AB48(pos, &angle, data + 0x88, data + 0x8C);
            func_L00_0025C558(moby, data + 0x70, 7, 1, 0, angle);
            moby[0x20] = 0x15;
            moby[0xBC] = 2;
            data[0x67] = 0x78;
            break;
        case 1:
        case 2:
            {
                float step;
                float step_sq;
                float forward;
                float lateral;
                float vz;
                float fall_speed;
                step = D_0015ED6C;
                fall_speed = *(float *)&D_L15_001613F4;
                step_sq = D_0015ED70;
                fall_speed = fall_speed * step_sq;
                forward = *(float *)&D_L15_00161408 * step;
                lateral = *(float *)&D_L15_00161404 * step;
                vz = *(float *)&D_L15_0016140C * step;
                *(unsigned short *)(moby + 0x34) &= 0xEFFF;
                *(float *)(data + 0x80) = fall_speed;
                *(float *)(data + 0x88) = forward;
                *(float *)(data + 0x8C) = lateral;
                *(float *)(data + 0x84) = vz;
                *(float *)(data + 0xC0) = 8.0f;
                *(float *)(data + 0xC4) = 16.0f;
            }
            {
                float heading = func_L00_001FF8B0(*(float *)(moby + 0x10) - *(float *)(*(char **)(hit + 0x20) + 0x10),
                                              *(float *)(moby + 0x14) - *(float *)(*(char **)(hit + 0x20) + 0x14));
                *(VecQuad_c29a570 *)pos = *(VecQuad_c29a570 *)(hit + 0x10);
                fall_angle = heading;
            }
            func_L00_0025AB48(pos, &fall_angle, data + 0x88, data + 0x8C);
            func_L00_0025C558(moby, data + 0x70, 8, 1, 0, fall_angle);
            moby[0x20] = 0x17;
            moby[0xBC] = 2;
            data[0x67] = 0xF0;
            break;
        }
        func_L00_0025D458(moby, data + 0x60);
    }
    moby[0xA4] = 0xFF;
    func_L00_0025D538(moby, data + 0x60);
    if (flag != 0)
        return;
    if (*(int *)(data + 0x38) != 0) {
        float value;
        int rounded;
        value = func_002140F8_r(180.0f, 240.0f);
        rounded = func_001FA898_r(func_001F96B0(value));
        *(int *)(data + 0x38) = 0;
        *(float *)(data + 0x130) = (float)rounded;
    }
    func_L08_00211A38_r(data + 0x130);
    if (func_001F9B48(moby + 0x10, (D_0013E533 + 0xEED)) < 64.0f) {
        group = 0;
        qcopy(pos, moby + 0x10);
        pos[2] += 1.0f;
        if (*(int *)(data + 0x12C) != -1)
            group = D_L15_00160058_m + (*(int *)(data + 0x12C) << 8);
        data[0x15A] = func_L00_001EFFF0_r(pos, (D_0013E533 + 0xEED), 2, (void *)group, 0) == 0;
    } else {
        data[0x15A] = 0;
    }
    path = D_L15_001B0AB0[*(int *)(data + 0x120)];
    if (func_L00_0025FF38(moby, data + 0xD0, 128.0f, 0, 0, path + 4, *path) != 2) {
        float target_angle;
        seen = 0;
        target_angle = func_L00_001FF8B0(*(float *)(data + 0xD0) - *(float *)(moby + 0x10),
                              *(float *)(data + 0xD4) - *(float *)(moby + 0x14));
        if ((func_001FA688(*(float *)(moby + 0x48), target_angle) < 1.5707964f &&
             (*(short *)(D_0013E533 + 0x1125) == 0 || func_001F99C0(*(float *)(moby + 0x18) - *(float *)(data + 0xD8)) < 4.0f)) ||
            moby[0x20] == 0x10 || moby[0x20] == 0x11) {
            seen = data[0x15A];
        }
        if (*(int *)(data + 0x114) == 1)
            data[0x15E] = data[0x15B] + 1;
        else if (*(unsigned char *)(D_0013E533 + 0x2EC1) != 3)
            data[0x15E] = 0;
        else if (data[0x15E] >= 2)
            data[0x15B] = 1;
        data[0x19F] = 1;
        if (seen == 0) {
            if (data[0x15A] == 0)
                data[0x15B] = 1;
            data[0x19F] = 0;
            *(int *)(data + 0x114) = 2;
            *(short *)(data + 0x158) = 0;
        } else if (((*(short *)(data + 0x156) == 0 || data[0x15B] != 0) &&
                    *(unsigned char *)(D_0013E533 + 0x2EC1) == 3) ||
                   (data[0x15E] != 0 && *(int *)(data + 0x114) != 1)) {
            *(int *)(data + 0x114) = 2;
            *(short *)(data + 0x158) = 1;
        } else if (*(unsigned char *)(D_0013E533 + 0x2EC1) != 3 && *(int *)(data + 0x114) != 2) {
            data[0x15B] = 0;
        }
        if (D_L15_0015F5C4 == 2)
            *(int *)(data + 0x114) = 2;
    } else {
        data[0x19F] = 0;
        data[0x15B] = 1;
        *(short *)(data + 0x158) = 0;
    }
    if (*(int *)(data + 0x110) == 0)
        *(int *)(data + 0x110) = *(int *)(D_0013E533 + 0x2E9D);
}
