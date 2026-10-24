void TrainWreckReaction(char *moby, void *vel_in, int flag) {
    float spot[4];
    float vel[4];
    float push[4];
    char *data;
    char *hit;
    int idx;
    unsigned char *e;

    data = *(char **)(moby + 0x78);
    hit = MobyGetHitMessage(moby, 0xA30000, 0);
    if (hit != 0 && *(unsigned short *)(hit + 0x28) != 0x102) {
        clear_u64_value(spot);
        if (*(short *)(moby + 0xA6) == 0x4B) {
            spot[0] = FastCos(*(float *)(moby + 0x48)) * -2.747f;
            spot[1] = FastSin(*(float *)(moby + 0x48)) * -2.747f;
            spot[2] = 0.0f;
        }
        *(Quad_0029E498 *)vel = *(Quad_0029E498 *)vel_in;
        FastVecScale(vel, vel, 0.5f);
        FastVecAdd(spot, spot, moby + 0x10);
        if (*(int *)(hit + 0x24) & 0x800000) {
            func_L00_00261B00_0029E498(moby, 10, 20, 2, -1);
        } else {
            func_L00_00261B00_0029E498(moby, 3, 5, 2, -1);
        }
        *(short *)(data + 0x14A) = scale_ticks(600);
        func_L00_0025F4A8(moby, vel, spot, 0.0f, 0.0f, 20, 3, 4, 4.0f, 2.0f, 100000.0f, -1, 3.0f, 15.0f, 1, 1, -1, 0);
        FastVecScale(push, moby + 0xC0, D_0015EE60 * 0.075f);
        push[2] += D_0015EE60 * 0.08f;
        switch (*(short *)(moby + 0xA6)) {
        case 0x73:
            spawn_debris_0029E498(moby, 0x6A9, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6AA, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6AB, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        case 0x74:
            spawn_debris_0029E498(moby, 0x6AC, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6AD, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6AE, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        case 0x75:
            spawn_debris_0029E498(moby, 0x6AF, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B0, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B1, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        case 0x76:
            spawn_debris_0029E498(moby, 0x6B2, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B3, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B4, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        case 0x77:
            spawn_debris_0029E498(moby, 0x6B5, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B6, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B7, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        case 0x78:
            spawn_debris_0029E498(moby, 0x6B8, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6B9, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6BA, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        case 0x84:
            spawn_debris_0029E498(moby, 0x6BC, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6BE, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            spawn_debris_0029E498(moby, 0x6BF, (float *)(moby + 0x10), moby + 0x40, scale_ticks(90), 0, D_0015EE70 * 12.0f, push, D_L03_0015F660, D_L03_0015F660);
            break;
        }
        if (*(short *)(moby + 0xA6) == 0x31B && D_0013D510_0029E498[4] == 0) {
            D_0013D510_0029E498[4] = 1;
            func_0022EE28_0029E498(1, 0, 0);
            func_L00_00264DB8_0029E498(0x542D, -1);
        }
        func_0022EEB8_0029E498(0, 0, moby);
    }
    D_L03_001607F0 = (void *)red_ship_1_update;
    ((unsigned char *)moby)[0xA4] = 0xFF;
    if (*(short *)(data + 0x14A) != 0) {
        if (FastDecTimer(data + 0x14A) && flag) {
            *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
            *(unsigned short *)(moby + 0x34) = (*(unsigned short *)(moby + 0x34) & 0xFFBE) | 0x1000;
        } else {
            *(int *)(moby + 0x94) = 0;
            *(unsigned short *)(moby + 0x34) |= 0x41;
            idx = *(int *)(data + 0x150);
            if (idx != -1) {
                e = (unsigned char *)D_0013E650_0029E498 + idx * 0x70;
                if (*(char **)(e + 0x88) == moby && e[0x74]) {
                    Sound_StopSlot(idx);
                }
            }
            *(int *)(data + 0x150) = -1;
            *(unsigned short *)(moby + 0x34) &= 0xEFFF;
        }
    }
}
