/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern void func_00234C98(int, u64);
extern u64 func_001F4868(int);
extern float func_001FA748(float, float);
extern float func_001F9FA8(float);
extern int func_001FA8A8(int, int, float);
extern short D_L02_00161BC0;
extern short D_L02_00161BC4;
extern short D_L02_00161BC8;
extern short D_L02_00161BCC;
extern short D_L02_00161BD0;
extern short D_L02_00161BD4;
extern short D_L02_00161BD8;
extern short D_L02_00161BDC;
extern short D_L02_00161BE0;
extern short D_L02_00161BE4;
extern short D_L02_00161BE8;
extern short D_L02_00161BEC;
extern short D_L02_00161C30;
extern short D_L02_00161C34;
extern short D_L02_00161C38;
extern short D_L02_00161C3C;
extern short D_L02_00161C40;
extern short D_L02_00161C44;
extern short D_L02_00161C4C;
extern short D_L02_00161C50;
extern short D_L02_00161C54;
extern int D_L02_0015F6B0 MACRO_ADDR;
extern void func_L02_002DC2C0(void);
extern void func_L02_002DB810(u64, u32, float, float, float, float);
extern void func_L02_002DBAF8(u64, int, int, float, float);
extern void func_L02_002DBD70(u64, f32, f32, f32, s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/entities/002a4038.c, FUN_L02_002d93e8. */
void func_L02_002DA820(char *moby) {
    char *data = *(char **)(moby + 0x78);
    int *markers = (int *)(data + 0xb0);
    int *first;
    int *second;
    int *third;
    u64 texture;
    int i, state, first_step = 0, second_step = 0, third_step = 0;
    int index, other, slot, draw_second, color, degrees;
    float angle, distance, phase;

    func_00234C98(0x47, 0x5380b);
    texture = func_001F4868(0x10);
    func_L02_002DC2C0();
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
        if (*(int *)(data + 0xe0) == 1) phase = func_001FA748(phase, *(float *)(data + 0xe4));
        angle = func_001FA748(phase, (float)(i * 30) * 0.017453292f);
        index = i + *(int *)(data + 0x14);
        slot = index % 12;
        if (first[slot] == 2) {
            other = i + *(int *)(data + 0x18) + first_step - second_step;
            draw_second = 1;
            color = (*(int *)&D_L02_00161C3C);
            if (second[other % 12] != 0) distance = (float)(*(int *)&D_L02_00161BD4);
            else {
                int third_index = i + *(int *)(data + 0x1c) + first_step - third_step;
                if (third[third_index % 12] != 0) distance = (float)(*(int *)&D_L02_00161BC8);
                else {
                    int extent = (*(int *)&D_L02_00161BC8);
                    if (third[(third_index + 6) % 12] == 0 &&
                        (extent = (*(int *)&D_L02_00161BD4), second[(other + 6) % 12] == 0) &&
                        (extent = (*(int *)&D_L02_00161BE0), first[(index + 6) % 12] == 0)) {
                        distance = (float)-(*(int *)&D_L02_00161BEC);
                        draw_second = 0;
                        if (markers[(i + first_step + 6) % 12] != 0) {
                            first[slot] = 3;
                            markers[(i + first_step + 6) % 12] = 2;
                            color = (*(int *)&D_L02_00161C40);
                        }
                    } else distance = (float)-extent;
                }
            }
            phase = func_001FA748(angle, 0.31415927f);
            func_L02_002DBD70(texture, phase, (float)(*(int *)&D_L02_00161BE0), distance, color);
            if (draw_second) {
                phase = func_001FA748(angle, 0.31415927f);
                func_L02_002DBD70(texture, phase, distance, (float)(*(int *)&D_L02_00161BE0), (*(int *)&D_L02_00161C44));
            }
        }
    }
    for (i = 0; i < 12; ++i) {
        phase = -0.31415927f;
        if (*(int *)(data + 0xe0) == 2) phase = func_001FA748(phase, *(float *)(data + 0xe4));
        angle = func_001FA748(phase, (float)(i * 30) * 0.017453292f);
        index = i + *(int *)(data + 0x18);
        slot = index % 12;
        if (second[slot] == 2) {
            other = i + *(int *)(data + 0x1c) + second_step - third_step;
            draw_second = 1;
            color = (*(int *)&D_L02_00161C3C);
            if (third[other % 12] != 0) distance = (float)(*(int *)&D_L02_00161BC8);
            else {
                int extent = (*(int *)&D_L02_00161BC8);
                if (third[(other + 6) % 12] == 0 &&
                    (extent = (*(int *)&D_L02_00161BD4), second[(index + 6) % 12] == 0) &&
                    (extent = (*(int *)&D_L02_00161BE0),
                     first[(i + *(int *)(data + 0x14) + second_step - first_step + 6) % 12] == 0)) {
                    distance = (float)-(*(int *)&D_L02_00161BEC);
                    draw_second = 0;
                    if (markers[(i + second_step + 6) % 12] != 0) {
                        second[slot] = 3;
                        markers[(i + second_step + 6) % 12] = 2;
                        color = (*(int *)&D_L02_00161C40);
                    }
                } else distance = (float)-extent;
            }
            phase = func_001FA748(angle, 0.31415927f);
            func_L02_002DBD70(texture, phase, (float)(*(int *)&D_L02_00161BD4), distance, color);
            if (draw_second) {
                phase = func_001FA748(angle, 0.31415927f);
                func_L02_002DBD70(texture, phase, distance, (float)(*(int *)&D_L02_00161BD4), (*(int *)&D_L02_00161C44));
            }
        }
    }
    for (i = 0; i < 12; ++i) {
        phase = -0.31415927f;
        if (*(int *)(data + 0xe0) == 3) phase = func_001FA748(phase, *(float *)(data + 0xe4));
        angle = func_001FA748(phase, (float)(i * 30) * 0.017453292f);
        index = i + *(int *)(data + 0x1c);
        slot = index % 12;
        if (third[slot] == 2) {
            int extent = (*(int *)&D_L02_00161BC8);
            draw_second = 1;
            color = (*(int *)&D_L02_00161C3C);
            if (third[(index + 6) % 12] == 0 &&
                (extent = (*(int *)&D_L02_00161BD4),
                 second[(i + *(int *)(data + 0x18) + third_step - second_step + 6) % 12] == 0) &&
                (extent = (*(int *)&D_L02_00161BE0),
                 first[(i + *(int *)(data + 0x14) + third_step - first_step + 6) % 12] == 0)) {
                distance = (float)-(*(int *)&D_L02_00161BEC);
                draw_second = 0;
                if (markers[(i + third_step + 6) % 12] != 0) {
                    third[slot] = 3;
                    markers[(i + third_step + 6) % 12] = 2;
                    color = (*(int *)&D_L02_00161C40);
                }
            } else distance = (float)-extent;
            phase = func_001FA748(angle, 0.31415927f);
            func_L02_002DBD70(texture, phase, (float)(*(int *)&D_L02_00161BC8), distance, color);
            if (draw_second) {
                phase = func_001FA748(angle, 0.31415927f);
                func_L02_002DBD70(texture, phase, distance, (float)(*(int *)&D_L02_00161BC8), (*(int *)&D_L02_00161C44));
            }
        }
    }
# 905 "/Users/flavy/Projects/OpenRAC/games/rac1/ntsc/src/overlays/shared/gameplay/entities/002a4038.c"
    for (i = 0; i < 12; ++i) { int value; int ring_index; angle = -0.2617994f; color = (*(int *)&D_L02_00161C34); if (*(int *)(data + 0xe0) == (1)) { float wave = func_001F9FA8((float)((D_L02_0015F6B0 % 60) * 6) * 0.017453292f); color = func_001FA8A8((*(int *)&D_L02_00161C30), (*(int *)&D_L02_00161C30) & 0xffffff, wave * 0.5f + 0.5f); if (*(float *)(data + 0xe4) != 0.0f) angle = func_001FA748(angle, *(float *)(data + 0xe4)); } angle = func_001FA748(angle, (float)(i * 30) * 0.017453292f); phase = func_001FA748(angle, 0.5235988f); func_L02_002DB810(texture, color, angle, phase, (float)((*(int *)&D_L02_00161BDC)), (float)((*(int *)&D_L02_00161BD8))); ring_index = (i + *(int *)(data + (0x14))) % 12; value = (first)[ring_index]; if (value != 0) { if (value < 3) { phase = func_001FA748(angle, 0.2617994f); func_L02_002DBAF8(texture, (*(int *)&D_L02_00161C38), value == 2 ? 1 : 2, phase, (float)((*(int *)&D_L02_00161BE0))); } else { phase = func_001FA748(angle, 0.2617994f); func_L02_002DBAF8(texture, (*(int *)&D_L02_00161C38), 0, phase, (float)((*(int *)&D_L02_00161BE0))); (first)[(i + *(int *)(data + (0x14))) % 12] = 2; } } };
    for (i = 0; i < 12; ++i) { int value; int ring_index; angle = -0.2617994f; color = (*(int *)&D_L02_00161C34); if (*(int *)(data + 0xe0) == (2)) { float wave = func_001F9FA8((float)((D_L02_0015F6B0 % 60) * 6) * 0.017453292f); color = func_001FA8A8((*(int *)&D_L02_00161C30), (*(int *)&D_L02_00161C30) & 0xffffff, wave * 0.5f + 0.5f); if (*(float *)(data + 0xe4) != 0.0f) angle = func_001FA748(angle, *(float *)(data + 0xe4)); } angle = func_001FA748(angle, (float)(i * 30) * 0.017453292f); phase = func_001FA748(angle, 0.5235988f); func_L02_002DB810(texture, color, angle, phase, (float)((*(int *)&D_L02_00161BD0)), (float)((*(int *)&D_L02_00161BCC))); ring_index = (i + *(int *)(data + (0x18))) % 12; value = (second)[ring_index]; if (value != 0) { if (value < 3) { phase = func_001FA748(angle, 0.2617994f); func_L02_002DBAF8(texture, (*(int *)&D_L02_00161C38), value == 2 ? 1 : 2, phase, (float)((*(int *)&D_L02_00161BD4))); } else { phase = func_001FA748(angle, 0.2617994f); func_L02_002DBAF8(texture, (*(int *)&D_L02_00161C38), 0, phase, (float)((*(int *)&D_L02_00161BD4))); (second)[(i + *(int *)(data + (0x18))) % 12] = 2; } } };
    for (i = 0; i < 12; ++i) { int value; int ring_index; angle = -0.2617994f; color = (*(int *)&D_L02_00161C34); if (*(int *)(data + 0xe0) == (3)) { float wave = func_001F9FA8((float)((D_L02_0015F6B0 % 60) * 6) * 0.017453292f); color = func_001FA8A8((*(int *)&D_L02_00161C30), (*(int *)&D_L02_00161C30) & 0xffffff, wave * 0.5f + 0.5f); if (*(float *)(data + 0xe4) != 0.0f) angle = func_001FA748(angle, *(float *)(data + 0xe4)); } angle = func_001FA748(angle, (float)(i * 30) * 0.017453292f); phase = func_001FA748(angle, 0.5235988f); func_L02_002DB810(texture, color, angle, phase, (float)((*(int *)&D_L02_00161BC4)), (float)((*(int *)&D_L02_00161BC0))); ring_index = (i + *(int *)(data + (0x1c))) % 12; value = (third)[ring_index]; if (value != 0) { if (value < 3) { phase = func_001FA748(angle, 0.2617994f); func_L02_002DBAF8(texture, (*(int *)&D_L02_00161C38), value == 2 ? 1 : 2, phase, (float)((*(int *)&D_L02_00161BC8))); } else { phase = func_001FA748(angle, 0.2617994f); func_L02_002DBAF8(texture, (*(int *)&D_L02_00161C38), 0, phase, (float)((*(int *)&D_L02_00161BC8))); (third)[(i + *(int *)(data + (0x1c))) % 12] = 2; } } };


    degrees = 0;
    for (i = 0; i < 12; ++i, degrees += 30) {
        angle = func_001FA748(-0.08726646f, (float)degrees * 0.017453292f);
        phase = func_001FA748(angle, 0.17453292f);
        if (markers[i] != 0) {
            color = (*(int *)&D_L02_00161C54);
            if (markers[i] != 2) {
                color = (*(int *)&D_L02_00161C50);
                if ((*(int *)0x0013cae4 & 0x20) == 0) color = (*(int *)&D_L02_00161C4C);
            }
            func_L02_002DB810(texture, color, angle, phase, (float)(*(int *)&D_L02_00161BE8), (float)(*(int *)&D_L02_00161BE4));
        }
    }
    func_00234C98(0x47, 0x5360b);
}
