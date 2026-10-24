void background_ship_2_update(char *moby) {
    char *d;
    char *t;
    int r;
    int flag = 0;
    float tv[4];
    float f70;
    float p2;
    float x;
    int r1;
    int r2;
    int r3;
    d = *(char **)(moby + 0x78);
    t = D_L02_001B0DB0[*(int *)(d + 0x60)];
    r = (int)MobyGetHitMessage(moby, 0x10000, 0);
    switch (*(unsigned char *)(moby + 0x20)) {
    case 0: {
        int m1;
        *(unsigned char *)(moby + 0x30) = 0xFF;
        *(short *)(moby + 0x32) = 0xFF;
        if (D_0015EE84_2D9F08 == 10) {
            *(float *)(moby + 0x2C) = *(float *)(moby + 0x2C) * 0.5f;
        }
        m1 = -1;
        if (*(int *)(d + 0x60) == m1) {
            STUB_printf(D_L02_001FB0A0, *(short *)(moby + 0xB2));
            DeleteMoby(moby);
            return;
        }
        if (*(int *)D_L02_001B0DB0[*(int *)(d + 0x60)] == 0) {
            STUB_printf(D_L02_001FB0C0, *(short *)(moby + 0xB2));
            DeleteMoby(moby);
            return;
        }
        f70 = FastVecDist(D_L02_001B0DB0[*(int *)(d + 0x60)] + 0x10, D_L02_001B0DB0[*(int *)(d + 0x60)] + 0x20);
        *(unsigned char *)(d + 0x2C) = 0x28;
        *(float *)(d + 0x70) = f70;
        *(unsigned char *)(d + 0x2B) = 1;
        qcopy(moby + 0x10, D_L02_001B0DB0[*(int *)(d + 0x60)] + 0x10);
        *(float *)(d + 0x68) = random_angle_radians();
        *(int *)(d + 0x6C) = m1;
        *(unsigned char *)(moby + 0x20) = 1;
        *(unsigned short *)(moby + 0x34) |= 0x10;
        *(float *)(d + 0x64) = *(float *)(d + 0x74) * (float)*(int *)t;
        break;
    }
    case 1:
        if (*(unsigned char *)(D_0013E633 + 0x2EC1) == 1 && D_0015EE84_2D9F08 == 10) {
            DeleteMoby(moby);
            return;
        }
        qcopy(tv, moby + 0x10);
        if (Sound_IsPlaying(moby, *(int *)(d + 0x6C)) == 0) {
            *(int *)(d + 0x6C) = func_0022ED80_r(1, 4, (int)moby);
        }
        p2 = *(float *)&D_L02_00162050 * D_0015EE6C;
        if (D_0015EE84_2D9F08 == 10) {
            p2 = p2 * 0.25f;
        }
        *(float *)(d + 0x64) = *(float *)(d + 0x64) + p2 / *(float *)(d + 0x70);
        if ((float)*(int *)t < *(float *)(d + 0x64)) {
            *(float *)(d + 0x64) = *(float *)(d + 0x64) - (float)*(int *)t;
        }
        if ((unsigned int)(D_0015EE84_2D9F08 - 9) < 2) {
            SamplePathAt((int *)t, 1, moby + 0x10, (float *)(moby + 0x40), 0, *(float *)(d + 0x64));
        } else {
            SamplePathAt((int *)t, 0, moby + 0x10, (float *)(moby + 0x40), 0, *(float *)(d + 0x64));
        }
        if (*(float *)(moby + 0x10) < 8.0f) {
            *(int *)(moby + 0x94) = 0;
        } else if (*(float *)(moby + 0x14) < 8.0f) {
            *(int *)(moby + 0x94) = 0;
        } else {
            *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        }
        if (*(unsigned char *)(moby + 0x31) != 0) {
            if (D_0015EE84_2D9F08 != 10) {
                SpawnJointParticles(moby, tv);
            } else {
                SpawnTwoParticles(moby, tv);
            }
        }
        if (r != 0 && 0.0f < *(float *)(r + 0x2C)) {
            flag = 1;
        }
        if (flag) {
            *(unsigned char *)(moby + 0x20) = 2;
        }
        break;
    case 2:
        if (D_0015EE84_2D9F08 == 2) {
            D_0015EEF4 = D_0015EEF4 + 1;
            if (D_0015EEF4 >= 3) {
                unsigned char *p = menu_item_enabled_115 + 1;
                if (p[2] == 0) {
                    p[2] = 1;
                    PlayBankSound(1, 0, 0);
                    ShowBanner(0x542D, -1);
                }
            }
        }
        if (D_0015EE84_2D9F08 == 9) {
            if (*(int *)(D_0013E633 + 0x2EA1) == 0x32) {
                D_0015EEFC = D_0015EEFC + 1;
                if (D_0015EEFC >= 5) {
                    unsigned char *p = menu_item_enabled_115 + 1;
                    if (p[0xF] == 0) {
                        p[0xF] = 1;
                        PlayBankSound(1, 0, 0);
                        ShowBanner(0x542D, -1);
                    }
                }
            }
        }
        func_0022ED80_r(0, 0, (int)moby);
        FastVecScale(tv, moby + 0xC0, *(float *)&D_L02_00162060 * D_0015EE60);
        tv[2] = tv[2] + *(float *)&D_L02_00162064 * D_0015EE60;
        r1 = scale_ticks(90);
        SpawnBreakFragment(moby, 0x5F6, (float *)(moby + 0x10), (void *)(moby + 0x40), r1, 0, D_0015EE70 * 12.0f, tv, D_L02_0015F660, D_L02_0015F660);
        r2 = scale_ticks(90);
        SpawnBreakFragment(moby, 0x5F8, (float *)(moby + 0x10), (void *)(moby + 0x40), r2, 0, D_0015EE70 * 12.0f, tv, D_L02_0015F660, D_L02_0015F660);
        r3 = scale_ticks(90);
        SpawnBreakFragment(moby, 0x5F9, (float *)(moby + 0x10), (void *)(moby + 0x40), r3, 0, D_0015EE70 * 12.0f, tv, D_L02_0015F660, D_L02_0015F660);
        DeleteMoby(moby);
        return;
    default:
        break;
    }
    if (*(unsigned char *)(moby + 0x31) != 0) {
        x = FastSin(*(float *)(d + 0x68)) * 4.0f - 3.0f;
        if (1.0f < x) {
            x = 1.0f;
        } else if (x < 0.0f) {
            x = 0.0f;
        }
        {
            int res;
            float k;
            res = func_001FA8A8_2ebce8(*(int *)&D_L02_00162058, *(int *)&D_L02_0016205C, x);
            k = *(float *)&D_L02_00162054 * 0.017453292f * D_0015EE6C;
            *(int *)(moby + 0x90) = res;
            *(float *)(d + 0x68) = FastAddRots(*(float *)(d + 0x68), k);
        }
        RefreshMobyAttachPoints6(moby);
    }
}
