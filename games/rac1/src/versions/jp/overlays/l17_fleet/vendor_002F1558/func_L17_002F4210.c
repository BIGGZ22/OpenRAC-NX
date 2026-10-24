void update_fleet_camera_box_watcher(char *moby) {
    int off;
    char *d = *(char **)(moby + 0x78);
    int st;
    int i;
    int j;
    int found;
    int tgt;
    int idx;
    char *m;
    int *p;
    int *q;
    float f;
    int n;
    int mask;
    unsigned char *sk;
    int wrap;

    st = *(int *)(d + 0x6C);
    switch (st) {
    case 0:
        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, *(int *)(d + 0x60))) {
            if (*(int *)(D_0013E633 + 0x2EA9) == 0x11) {
                *(int *)(d + 0x6C) = 1;
            }
        }
        break;
    case 1: {
        char *h = D_0013E633 + 0xE1D;
        int v = *(int *)(h + 0x2084);
        if (v == 0x76 || v == 0x32) {
            *(int *)(d + 0x6C) = 0;
        } else if (is_point_inside_clip_volume(h + 0x80, *(int *)(d + 0x64)) || is_point_inside_clip_volume(h + 0x80, *(int *)(d + 0x68))) {
            if (gSkillPoints[0x1C] == 0) {
                gSkillPoints[0x1C] = 1;
                PlayBankSound(1, 0, 0);
                ShowBanner(0x542D, -1);
            }
        }
        break;
    }
    }

    if (gSkillPoints[0x1B] == 0) {
        for (j = 0; j < 30; j++) {
            char *mb;
            sk = gSkillPoints;
            mb = D_L17_00160058 + (*(int *)(d + 0x70) << 8);
            if ((*(short *)(mb + 0xA6) == 0x2C || *(short *)(mb + 0xA6) == 0x566)
                && *(signed char *)(mb + 0x20) >= 0
                && !is_point_inside_clip_volume(mb + 0x10, *(int *)(d + 0x78))) {
                *(int *)(d + 0x74) += 1;
            }
            n = *(int *)(d + 0x70) + 1;
            wrap = D_L17_00160058 + (n << 8) < D_L17_0016005C;
            *(int *)(d + 0x70) = n;
            if (!wrap) {
                if (*(int *)(d + 0x74) == 0 && sk[0x1B] == 0) {
                    sk[0x1B] = 1;
                    PlayBankSound(1, 0, 0);
                    ShowBanner(0x542D, -1);
                }
                *(int *)(d + 0x70) = 0;
                *(int *)(d + 0x74) = 0;
            }
        }
    }

    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        *(unsigned char *)(moby + 0x30) = 0xFF;
        *(int *)(d + 0x38) = scale_ticks(600);
        *(unsigned char *)(d + 0x4C) = 0;
        if (*(int *)(d + 0x44) == -1 || *(int *)(d + 0x40) == -1 || *(int *)(d + 0x3C) == -1 || *(int *)(d + 0x48) == -1) {
            STUB_printf(D_L17_001E7300, *(int *)(d + 0x3C), *(int *)(d + 0x48), *(int *)(d + 0x40), *(int *)(d + 0x44));
            *(unsigned char *)(d + 0x4C) = 1;
        }
        *(unsigned char *)(moby + 0x20) = 1;
        break;
    case 1:
        tgt = -1;
        idx = -1;
        found = 0;
        for (i = 0; i < 1; i++) {
            off = i * 4;
            if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, *(int *)(d + off))) {
                found = 1;
                if (*(int *)(d + 0x30) != *(int *)(d + off)) {
                    tgt = *(int *)(d + off);
                    p = (int *)(d + 0x10);
                    idx = p[i];
                }
                break;
            }
        }
        if (found) {
            *(int *)(d + 0xC) += 1;
        } else {
            *(int *)(d + 0xC) = 0;
            *(int *)(d + 0x30) = -1;
        }
        if (idx != -1) {
            m = D_L17_00160058 + (idx << 8);
            if (m == 0) goto skip1;
            if (*(short *)(m + 0xA6) != 0x4E) goto skip1;
            if (*(unsigned char *)(m + 0x20) == 0xFE) goto skip1;
            if (*(unsigned char *)(m + 0x20) == 0xFD) goto skip1;
            if (*(unsigned char *)(m + 0x20) == 3) goto skip1;
        }
        if (*(int *)(d + 0xC) > *(int *)(d + 0x38) && tgt != -1
            && ((unsigned int)*(int *)(D_0013E633 + 0x2EA9) < 2 || *(int *)(D_0013E633 + 0x2EA9) == 9)
            && D_L17_00179D90[0] == 0 && D_L17_00179D90[9] == -1 && gHaveHologuise != 0) {
            *(int *)(d + 0x38) = scale_ticks(0x708);
            if (*(int *)(D_0014171B + 0x739) >= 0) {
                Help_Request(0x3A9A, 0x7D);
            }
            *(int *)(d + 0x30) = tgt;
            *(int *)(d + 0xC) = 0;
        }
    skip1:
        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, *(int *)(d + 0x20)) && D_L17_00179D90[0] == 0
            && D_L17_00179D90[9] == -1 && gHaveHeliPack[2] == 0) {
            int w = *(int *)(D_0014171B + 0x761);
            mask = 1 << D_0015EE84;
            if (!(w & mask)) {
                Help_Request(0x3A9D, 0x82);
            }
        }
        tgt = -1;
        found = 0;
        for (i = 0; i < 3; i++) {
            off = i * 4;
            q = (int *)(d + 0x20 + off);
            if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, *q)) {
                found = 1;
                if (*(int *)(d + 0x30) != *q) {
                    tgt = *q;
                }
                break;
            }
        }
        if (!found || (*(int *)(d + 0x2C) += 1, *(int *)(D_0013E633 + 0x2EA1) == 0x35)) {
            *(int *)(d + 0x2C) = 0;
            *(int *)(d + 0x30) = -1;
        }
        if (scale_ticks(0xE10) < *(int *)(d + 0x2C) && tgt != -1 && Hero_IsInSpecialState()
            && D_L17_00179D90[0] == 0 && D_L17_00179D90[9] == -1 && gHaveHeliPack[2] != 0) {
            Help_Request(0x4E2E, 0x78);
            *(int *)(d + 0x30) = tgt;
            *(int *)(d + 0x2C) = 0;
        }
        if (*(int *)(d + 0x58) != 0xFF
            && (D_0014171B + 0xAA35)[*(int *)(d + 0x58) + (D_0015EE84 << 4)] == 0xFF) {
            *(unsigned char *)(d + 0x4C) = 1;
        }
        if (*(unsigned char *)(d + 0x4C) == 0 && is_point_inside_clip_volume(D_0013E633 + 0xE9D, *(int *)(d + 0x3C))) {
            HelpPopup_Cancel();
            if (*(int *)(D_0014171B + 0x7A9) >= 0) {
                Help_Request(0x4268, 0x8B);
            }
            if (D_L17_00179D90[0] != 0 || D_L17_00179D90[9] != -1) {
                *(int *)(d + 0x50) = 0;
                *(unsigned char *)(moby + 0x20) = 2;
                *(int *)(d + 0x5C) = scale_ticks(0xB4);
                HeroL17_SetMoveState(0x72, 0);
            }
        }
        break;
    case 2: {
        char *cam;
        func_L00_002E9B30();
        set_follow_camera_angle_blend_from_position((int)(D_L17_0016016C + (*(int *)(d + 0x44) << 7) + 0x30), 0.02617994f, 0.0f);
        cam = D_L17_00167600;
        f = func_L00_001FF860(*(float *)(D_L17_0016016C + ((*(int *)(d + 0x44) << 7) + 0x30)) - *(float *)(cam + 0x140),
                              *(float *)(D_L17_0016016C + ((*(int *)(d + 0x44) << 7) + 0x34)) - *(float *)(cam + 0x144));
        if (FastDiffRots(*(float *)(cam + 0x158), f) < 0.034906585f) {
            *(unsigned char *)(moby + 0x20) = 3;
            *(int *)(d + 0x5C) = scale_ticks(0x3C);
        }
        break;
    }
    case 3:
        func_L00_002E9B30();
        if (func_001F9908_i((int *)(d + 0x5C))) {
            char *cam;
            set_follow_camera_angle_blend_from_position((int)(D_L17_0016016C + (*(int *)(d + 0x40) << 7) + 0x30), 0.02617994f, 0.0f);
            cam = D_L17_00167600;
            f = func_L00_001FF860(*(float *)(D_L17_0016016C + ((*(int *)(d + 0x40) << 7) + 0x30)) - *(float *)(cam + 0x140),
                                  *(float *)(D_L17_0016016C + ((*(int *)(d + 0x40) << 7) + 0x34)) - *(float *)(cam + 0x144));
            if (FastDiffRots(*(float *)(cam + 0x158), f) < 0.034906585f) {
                HeroL06_SetStateFromMode();
                *(unsigned char *)(d + 0x4C) = 1;
                *(unsigned char *)(moby + 0x20) = 1;
            }
        }
        break;
    }
}
