/* Carried from the PAL decompilation's nonmatching/functional/func_L02_002DA820.c (games/rac1) through the version
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
/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern void func_00233980(int, u64);
extern u64 func_001F44B8(int);
extern float func_001FA580(float, float);
extern float func_001F9DE0(float);
extern int func_001FA6E0(int, int, float);
extern short D_L02_00161B30;
extern short D_L02_00161B34;
extern short D_L02_00161B38;
extern short D_L02_00161B3C;
extern short D_L02_00161B40;
extern short D_L02_00161B44;
extern short D_L02_00161B48;
extern short D_L02_00161B4C;
extern short D_L02_00161B50;
extern short D_L02_00161B54;
extern short D_L02_00161B58;
extern short D_L02_00161B5C;
extern short D_L02_00161BA0;
extern short D_L02_00161BA4;
extern short D_L02_00161BA8;
extern short D_L02_00161BAC;
extern short D_L02_00161BB0;
extern short D_L02_00161BB4;
extern short D_L02_00161BBC;
extern short D_L02_00161BC0;
extern short D_L02_00161BC4;
extern int D_L02_0015F5CC MACRO_ADDR;
extern void func_L02_002DAE88(void);
extern void func_L02_002DA3D8(u64, u32, float, float, float, float);
extern void func_L02_002DA6C0(u64, int, int, float, float);
extern void func_L02_002DA938(u64, f32, f32, f32, s32);



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/entities/002a4038.c, FUN_L02_002d93e8. */
void FUN_L02_002d93e8(char *moby) {
    char *data = *(char **)(moby + 0x78);
    int *markers = (int *)(data + 0xb0);
    int *first;
    int *second;
    int *third;
    u64 texture;
    int i, state, first_step = 0, second_step = 0, third_step = 0;
    int index, other, slot, draw_second, color, degrees;
    float angle, distance, phase;

    func_00233980(0x47, 0x5380b);
    texture = func_001F44B8(0x10);
    func_L02_002DAE88();
    for (state = 11, first = markers; state >= 0; --state, ++first) {
        if (*first != 0) *first = 1;
    }
    if (*(float *)(data + 0xe4) > 0.31415927f) {
        state = *(int *)(data + 0xe0);
        if (state == 1) first_step = 1;
        else if (state == 2) second_step = 1;
        else third_step = 1;
    } else if (*(float *)(data + 0xe4) < -0.31415927f) {
        state = *(int *)(data + 0xe0);
        if (state == 1) first_step = -1;
        else if (state == 2) second_step = -1;
        else third_step = -1;
    }

    first = (int *)(data + 0x20);
    second = (int *)(data + 0x50);
    third = (int *)(data + 0x80);
    for (i = 0; i < 12; ++i) {
        phase = -0.31415927f;
        if (*(int *)(data + 0xe0) == 1) phase = func_001FA580(phase, *(float *)(data + 0xe4));
        angle = func_001FA580(phase, (float)(i * 30) * 0.017453292f);
        index = i + *(int *)(data + 0x14);
        slot = index % 12;
        if (first[slot] == 2) {
            other = i + *(int *)(data + 0x18) + first_step - second_step;
            draw_second = 1;
            color = (*(int *)&D_L02_00161BAC);
            if (second[other % 12] != 0) distance = (float)(*(int *)&D_L02_00161B44);
            else {
                int third_index = i + *(int *)(data + 0x1c) + first_step - third_step;
                if (third[third_index % 12] != 0) distance = (float)(*(int *)&D_L02_00161B38);
                else {
                    int extent = (*(int *)&D_L02_00161B38);
                    if (third[(third_index + 6) % 12] == 0 &&
                        (extent = (*(int *)&D_L02_00161B44), second[(other + 6) % 12] == 0) &&
                        (extent = (*(int *)&D_L02_00161B50), first[(index + 6) % 12] == 0)) {
                        distance = (float)-(*(int *)&D_L02_00161B5C);
                        draw_second = 0;
                        if (markers[(i + first_step + 6) % 12] != 0) {
                            first[slot] = 3;
                            markers[(i + first_step + 6) % 12] = 2;
                            color = (*(int *)&D_L02_00161BB0);
                        }
                    } else distance = (float)-extent;
                }
            }
            phase = func_001FA580(angle, 0.31415927f);
            func_L02_002DA938(texture, phase, (float)(*(int *)&D_L02_00161B50), distance, color);
            if (draw_second) {
                phase = func_001FA580(angle, 0.31415927f);
                func_L02_002DA938(texture, phase, distance, (float)(*(int *)&D_L02_00161B50), (*(int *)&D_L02_00161BB4));
            }
        }
    }
    for (i = 0; i < 12; ++i) {
        phase = -0.31415927f;
        if (*(int *)(data + 0xe0) == 2) phase = func_001FA580(phase, *(float *)(data + 0xe4));
        angle = func_001FA580(phase, (float)(i * 30) * 0.017453292f);
        index = i + *(int *)(data + 0x18);
        slot = index % 12;
        if (second[slot] == 2) {
            other = i + *(int *)(data + 0x1c) + second_step - third_step;
            draw_second = 1;
            color = (*(int *)&D_L02_00161BAC);
            if (third[other % 12] != 0) distance = (float)(*(int *)&D_L02_00161B38);
            else {
                int extent = (*(int *)&D_L02_00161B38);
                if (third[(other + 6) % 12] == 0 &&
                    (extent = (*(int *)&D_L02_00161B44), second[(index + 6) % 12] == 0) &&
                    (extent = (*(int *)&D_L02_00161B50),
                     first[(i + *(int *)(data + 0x14) + second_step - first_step + 6) % 12] == 0)) {
                    distance = (float)-(*(int *)&D_L02_00161B5C);
                    draw_second = 0;
                    if (markers[(i + second_step + 6) % 12] != 0) {
                        second[slot] = 3;
                        markers[(i + second_step + 6) % 12] = 2;
                        color = (*(int *)&D_L02_00161BB0);
                    }
                } else distance = (float)-extent;
            }
            phase = func_001FA580(angle, 0.31415927f);
            func_L02_002DA938(texture, phase, (float)(*(int *)&D_L02_00161B44), distance, color);
            if (draw_second) {
                phase = func_001FA580(angle, 0.31415927f);
                func_L02_002DA938(texture, phase, distance, (float)(*(int *)&D_L02_00161B44), (*(int *)&D_L02_00161BB4));
            }
        }
    }
    for (i = 0; i < 12; ++i) {
        phase = -0.31415927f;
        if (*(int *)(data + 0xe0) == 3) phase = func_001FA580(phase, *(float *)(data + 0xe4));
        angle = func_001FA580(phase, (float)(i * 30) * 0.017453292f);
        index = i + *(int *)(data + 0x1c);
        slot = index % 12;
        if (third[slot] == 2) {
            int extent = (*(int *)&D_L02_00161B38);
            draw_second = 1;
            color = (*(int *)&D_L02_00161BAC);
            if (third[(index + 6) % 12] == 0 &&
                (extent = (*(int *)&D_L02_00161B44),
                 second[(i + *(int *)(data + 0x18) + third_step - second_step + 6) % 12] == 0) &&
                (extent = (*(int *)&D_L02_00161B50),
                 first[(i + *(int *)(data + 0x14) + third_step - first_step + 6) % 12] == 0)) {
                distance = (float)-(*(int *)&D_L02_00161B5C);
                draw_second = 0;
                if (markers[(i + third_step + 6) % 12] != 0) {
                    third[slot] = 3;
                    markers[(i + third_step + 6) % 12] = 2;
                    color = (*(int *)&D_L02_00161BB0);
                }
            } else distance = (float)-extent;
            phase = func_001FA580(angle, 0.31415927f);
            func_L02_002DA938(texture, phase, (float)(*(int *)&D_L02_00161B38), distance, color);
            if (draw_second) {
                phase = func_001FA580(angle, 0.31415927f);
                func_L02_002DA938(texture, phase, distance, (float)(*(int *)&D_L02_00161B38), (*(int *)&D_L02_00161BB4));
            }
        }
    }
# 905 "/Users/flavy/Projects/OpenRAC/games/rac1/ntsc/src/overlays/shared/gameplay/entities/002a4038.c"
    for (i = 0; i < 12; ++i) { int value; int ring_index; angle = -0.2617994f; color = (*(int *)&D_L02_00161BA4); if (*(int *)(data + 0xe0) == (1)) { float wave = func_001F9DE0((float)((D_L02_0015F5CC % 60) * 6) * 0.017453292f); color = func_001FA6E0((*(int *)&D_L02_00161BA0), (*(int *)&D_L02_00161BA0) & 0xffffff, wave * 0.5f + 0.5f); if (*(float *)(data + 0xe4) != 0.0f) angle = func_001FA580(angle, *(float *)(data + 0xe4)); } angle = func_001FA580(angle, (float)(i * 30) * 0.017453292f); phase = func_001FA580(angle, 0.5235988f); func_L02_002DA3D8(texture, color, angle, phase, (float)((*(int *)&D_L02_00161B4C)), (float)((*(int *)&D_L02_00161B48))); ring_index = (i + *(int *)(data + (0x14))) % 12; value = (first)[ring_index]; if (value != 0) { if (value < 3) { phase = func_001FA580(angle, 0.2617994f); func_L02_002DA6C0(texture, (*(int *)&D_L02_00161BA8), value == 2 ? 1 : 2, phase, (float)((*(int *)&D_L02_00161B50))); } else { phase = func_001FA580(angle, 0.2617994f); func_L02_002DA6C0(texture, (*(int *)&D_L02_00161BA8), 0, phase, (float)((*(int *)&D_L02_00161B50))); (first)[(i + *(int *)(data + (0x14))) % 12] = 2; } } };
    for (i = 0; i < 12; ++i) { int value; int ring_index; angle = -0.2617994f; color = (*(int *)&D_L02_00161BA4); if (*(int *)(data + 0xe0) == (2)) { float wave = func_001F9DE0((float)((D_L02_0015F5CC % 60) * 6) * 0.017453292f); color = func_001FA6E0((*(int *)&D_L02_00161BA0), (*(int *)&D_L02_00161BA0) & 0xffffff, wave * 0.5f + 0.5f); if (*(float *)(data + 0xe4) != 0.0f) angle = func_001FA580(angle, *(float *)(data + 0xe4)); } angle = func_001FA580(angle, (float)(i * 30) * 0.017453292f); phase = func_001FA580(angle, 0.5235988f); func_L02_002DA3D8(texture, color, angle, phase, (float)((*(int *)&D_L02_00161B40)), (float)((*(int *)&D_L02_00161B3C))); ring_index = (i + *(int *)(data + (0x18))) % 12; value = (second)[ring_index]; if (value != 0) { if (value < 3) { phase = func_001FA580(angle, 0.2617994f); func_L02_002DA6C0(texture, (*(int *)&D_L02_00161BA8), value == 2 ? 1 : 2, phase, (float)((*(int *)&D_L02_00161B44))); } else { phase = func_001FA580(angle, 0.2617994f); func_L02_002DA6C0(texture, (*(int *)&D_L02_00161BA8), 0, phase, (float)((*(int *)&D_L02_00161B44))); (second)[(i + *(int *)(data + (0x18))) % 12] = 2; } } };
    for (i = 0; i < 12; ++i) { int value; int ring_index; angle = -0.2617994f; color = (*(int *)&D_L02_00161BA4); if (*(int *)(data + 0xe0) == (3)) { float wave = func_001F9DE0((float)((D_L02_0015F5CC % 60) * 6) * 0.017453292f); color = func_001FA6E0((*(int *)&D_L02_00161BA0), (*(int *)&D_L02_00161BA0) & 0xffffff, wave * 0.5f + 0.5f); if (*(float *)(data + 0xe4) != 0.0f) angle = func_001FA580(angle, *(float *)(data + 0xe4)); } angle = func_001FA580(angle, (float)(i * 30) * 0.017453292f); phase = func_001FA580(angle, 0.5235988f); func_L02_002DA3D8(texture, color, angle, phase, (float)((*(int *)&D_L02_00161B34)), (float)((*(int *)&D_L02_00161B30))); ring_index = (i + *(int *)(data + (0x1c))) % 12; value = (third)[ring_index]; if (value != 0) { if (value < 3) { phase = func_001FA580(angle, 0.2617994f); func_L02_002DA6C0(texture, (*(int *)&D_L02_00161BA8), value == 2 ? 1 : 2, phase, (float)((*(int *)&D_L02_00161B38))); } else { phase = func_001FA580(angle, 0.2617994f); func_L02_002DA6C0(texture, (*(int *)&D_L02_00161BA8), 0, phase, (float)((*(int *)&D_L02_00161B38))); (third)[(i + *(int *)(data + (0x1c))) % 12] = 2; } } };


    degrees = 0;
    for (i = 0; i < 12; ++i, degrees += 30) {
        angle = func_001FA580(-0.08726646f, (float)degrees * 0.017453292f);
        phase = func_001FA580(angle, 0.17453292f);
        if (markers[i] != 0) {
            color = (*(int *)&D_L02_00161BC4);
            if (markers[i] != 2) {
                color = (*(int *)&D_L02_00161BC0);
                if ((*(int *)0x0013c9e4 & 0x20) == 0) color = (*(int *)&D_L02_00161BBC);
            }
            func_L02_002DA3D8(texture, color, angle, phase, (float)(*(int *)&D_L02_00161B58), (float)(*(int *)&D_L02_00161B54));
        }
    }
    func_00233980(0x47, 0x5360b);
}
