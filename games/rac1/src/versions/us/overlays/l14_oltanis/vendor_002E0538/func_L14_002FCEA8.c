void qwark_oltanis_update(char *m) {
    char *d = *(char **)(m + 0x78);
    AttachManipulatorsIfCheat(m);
    if (*(unsigned char *)(m + 0x31) != 0) {
        if (FastVecDist(m + 0x10, D_L14_001675C0) < 28.0f) {
            func_L00_0025B178(m);
            m[0x7F] = 0x16;
        }
    }
    switch (*(unsigned char *)(m + 0x20)) {
    case 0: {
        unsigned short id;
        if (D_L14_001BBCC0_2FCEA8.collected[(short)(id = *(unsigned short *)(m + 0xB2))] != 0
            || (*(int *)(D_0014171B_2FCEA8 + 0xAB75 + (((short)id >> 5) * 4 + (D_0015EE84_m << 8))) >> (id & 0x1F)) & 1) {
            DeleteMoby(m);
            return;
        }
        if (*(int *)(d + 0x44) == -1 || *(int *)(d + 0x48) == -1) {
            DeleteMoby(m);
            return;
        }
        *(char **)(d + 0x20) = D_L14_00161FF0;
        DialogRequest_Init(m, d);
        d[8] = 1;
        m[0x20] = 1;
        break;
    }
    case 1:
        if (*(unsigned char *)(d + 8) == 1 && is_point_inside_clip_volume(D_0013E633 + 0xE9D, *(int *)(d + 0x40)) != 0) {
            if ((D_0014171B_2FCEA8 + 0xAA35)[*(unsigned char *)(m + 0xB0) + (D_0015EE84_m << 4)] != 0xFF) {
                char *t;
                jet_clear_persistent((unsigned char)m[0xB0]);
                t = D_L14_001601AC_2EC3E8 + (*(int *)(d + 0x48) << 7);
                jet_reset_camera(t + 0x30, t + 0x70);
            }
            func_L14_002F0A00_2FCEA8(D_L14_00160098_2FCEA8 + (*(int *)(d + 0x44) << 8));
            *(float *)(d + 0xC) = 255.0f;
        } else {
            *(float *)(d + 0xC) = 1.7f;
        }
        if (Npc_TryStartDialog(m, d) != 0) {
            PlaceMarkerAheadOfMoby(3.0f, m);
            m[0x20] = 2;
        }
        break;
    case 2:
        if (D_L14_0015F6A8 == 2) break;
        m[0x20] = 1;
        if (*(short *)(d + 4) != 3 && *(short *)(d + 4) != 4) break;
        if (*(short *)(d + 4) == 4) {
            ShowBanner(0x53DD, -1);
        } else {
            ShowBanner(0x36B3, -1);
        }
        GiveItem(0x20, 1);
        mark_elevator_collected(D_L14_00160098_2FCEA8 + (*(int *)(d + 0x44) << 8));
        *(int *)(D_0014171B_2FCEA8 + 0xAB75 + (((short)*(unsigned short *)(m + 0xB2) >> 5) * 4 + (D_0015EE84_m << 8))) |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
        D_L14_001BAF60[(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
        memcard_Save(0, -1);
        DeleteMoby(m);
        return;
    case 3:
        DeleteMoby(m);
        return;
    }
    {
        float b[4];
        float sx = 0.02f;
        float sy = 0.3f;
        int doit = 0;
        if (*(unsigned char *)(m + 0x53) == 0) {
            char *g = D_0013E633 + 0xE9D;
            char *a_;
            doit = 1;
            if (func_001F9D48(m + 0x10, g) < 8.0f
                && FastDiffRots(*(float *)(m + 0x48),
                       func_L00_001FF860(*(float *)((a_ = g - 0x80) + 0xD0) - *(float *)(m + 0x10),
                                         *(float *)(a_ + 0xD4) - *(float *)(m + 0x14))) < 1.5707964f) {
                if (0.01f < FastVecLength(g + 0x80)) {
                    *(int *)(d + 0x194) = scale_ticks(0x78);
                } else {
                    func_001F9908((int *)(d + 0x194));
                }
            } else if (*(int *)(d + 0x194) != 0) {
                *(int *)(d + 0x194) = 0;
                qcopy(d + 0x180, D_0013E633 + 0xEED);
            }
            if (func_001F9908((int *)(d + 0x198)) != 0) {
                float y;
                float z;
                float x = random_float_between(180.0f, 300.0f);
                *(int *)(d + 0x198) = func_001FA898_r(func_001F9878(x));
                y = FastAddRots(*(float *)(m + 0x48), random_float_between(-90.0f, 90.0f) * 0.017453292f);
                z = random_float_between(0.0f, 30.0f) * 0.017453292f;
                BuildSphericalOffset(d + 0x180, 6.0f, y, z);
                FastVecAdd(d + 0x180, d + 0x180, m + 0x10);
            }
            if (*(int *)(d + 0x194) != 0) {
                qcopy(b, D_0013E633 + 0xEED);
                sx = 0.04f;
                sy = 0.3f;
            } else {
                qcopy(b, d + 0x180);
            }
        }
        if (doit != 0) {
            float w[4];
            float u[4];
            float f20;
            float f2;
            qcopy(w, m + 0x10);
            w[2] = w[2] + 1.0f;
            FastVecSub(u, b, w);
            f20 = FastSubRots(func_L00_001FF860(u[0], u[1]), *(float *)(m + 0x48));
            f2 = -func_L00_001FF860(func_001F9CE8(u), u[2]);
            if (f20 > 1.5707964f) f20 = 1.5707964f;
            else if (f20 < -1.5707964f) f20 = -1.5707964f;
            if (f2 > 0.5235988f) f2 = 0.5235988f;
            else if (f2 < -0.5235988f) f2 = -0.5235988f;
            *(float *)(d + 0xE4) = f2;
            *(float *)(d + 0xE8) = f20 * 0.6f;
            *(float *)(d + 0x168) = f20 * 0.4f;
        }
        if (gCheats[0] != 0) *(float *)(d + 0xF0) = 2.75f;
        EaseManipulatorOffsets(m, d + 0x80, 0, sx * D_0015EE64, sy * D_0015EE64);
        EaseManipulatorOffsets(m, d + 0x100, 1, sx * D_0015EE64, sy * D_0015EE64);
    }
}
