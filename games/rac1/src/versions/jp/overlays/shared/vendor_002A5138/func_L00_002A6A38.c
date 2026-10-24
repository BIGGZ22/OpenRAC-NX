void vendor_update(char *moby) {
    char *p = *(char **)(moby + 0x78);
    char *c;
    int i;
    float f;
    float t2;
    float t3;
    char *m;

    *(float *)&D_L00_0016146C += 0.05f;
    if (3.14f < *(float *)&D_L00_0016146C) *(float *)&D_L00_0016146C = -3.14f;
    f = FastSin(*(float *)&D_L00_0016146C);
    *(int *)(moby + 0x90) = (((int)(f * 48.0f) + 0x60) * 0x10101) | 0x80000000;

    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        *(unsigned char *)(moby + 0x20) = 1;
        *(int *)(p + 0) = 0;
        *(int *)(p + 4) = 0;
        *(char **)(p + 0xC) = CreateMoby(0x477);
        *(unsigned short *)(*(char **)(p + 0xC) + 0x34) |= 2;
        *(short *)(*(char **)(p + 0xC) + 0x32) = 0x40;
        *(float *)(*(char **)(p + 0xC) + 0x10) = *(float *)(moby + 0x10);
        *(float *)(*(char **)(p + 0xC) + 0x14) = *(float *)(moby + 0x14);
        *(float *)(*(char **)(p + 0xC) + 0x18) = *(float *)(moby + 0x18) + 2.95f;
        c = *(char **)(p + 0xC);
        if (*(unsigned char *)(*(char **)(c + 0x24) + 6) != 0) *(unsigned char *)(c + 0x73) = 0x20;
        *(int *)(p + 0x90) = 0;
        for (i = 0; i < 2; i++) {
            AttachManipulator(*(char **)(p + 0xC), i, p + 0x10 + i * 0x40);
        }
        func_L00_00251E30(*(char **)(p + 0xC));
        break;
    case 1:
        if (*(float *)(p + 0x90) > 0.0f) *(float *)(p + 0x90) -= 0.1f;
        if (*(float *)(p + 0x90) <= 0.0f) *(unsigned short *)(*(char **)(p + 0xC) + 0x34) |= 1;
        if ((D_L00_0015F6B0 & 7) == 0) {
            char *g = D_0013E633 + 0xE9D;
            if (func_001F9D48(moby + 0x10, g) <= 16.0f) {
                if (FastAbsF(*(float *)(moby + 0x18) - *(float *)(g + 8)) <= 8.0f) {
                    *(unsigned char *)(moby + 0x20) = 2;
                    if (*(unsigned char *)(moby + 0x53) != 1) MobyBlendAnimation(moby, 1, 0, 10);
                    *(unsigned short *)(*(char **)(p + 0xC) + 0x34) &= 0xFFFE;
                }
            }
        }
        AddDrawCallback(func_L00_002A62D0, moby);
        axis_rotation_2d48(p + 0x20, 2, *(float *)&D_L00_00161464);
        axis_rotation_2d48(p + 0x60, 2, *(float *)&D_L00_00161468);
        *(float *)&D_L00_00161464 = FastAddRots(*(float *)&D_L00_00161464, 0.01f);
        t2 = FastAddRots(*(float *)&D_L00_00161468, -0.02f);
        m = *(char **)(p + 0xC);
        t3 = *(float *)(p + 0x90) * *(float *)(*(char **)(m + 0x24) + 0x24) * 2.5f;
        *(float *)&D_L00_00161468 = t2;
        *(float *)(m + 0x2C) = t3;
        func_L00_00251E30(*(char **)(p + 0xC));
        break;
    case 2: {
        float dist;
        float dz;
        char *g;
        int a;
        int b;
        int r;
        if (*(float *)(p + 0x90) < 1.0f) *(float *)(p + 0x90) += 0.1f;
        g = D_0013E633 + 0xE9D;
        dist = func_001F9D48(moby + 0x10, g);
        dz = FastAbsF(*(float *)(moby + 0x18) - *(float *)(g + 8));
        if ((D_L00_0015F6B0 & 7) == 0) {
            if (18.0f < dist || 10.0f < dz) {
                *(unsigned char *)(moby + 0x20) = 1;
                if (*(unsigned char *)(moby + 0x53) != 0) MobyBlendAnimation(moby, 0, 0, 10);
                return;
            }
        }
        AddDrawCallback(func_L00_002A62D0, moby);
        a = 1;
        b = 1;
        axis_rotation_2d48(p + 0x20, 2, *(float *)&D_L00_00161464);
        axis_rotation_2d48(p + 0x60, 2, *(float *)&D_L00_00161468);
        *(float *)&D_L00_00161464 = FastAddRots(*(float *)&D_L00_00161464, 0.01f);
        t2 = FastAddRots(*(float *)&D_L00_00161468, -0.02f);
        m = *(char **)(p + 0xC);
        t3 = *(float *)(p + 0x90) * *(float *)(*(char **)(m + 0x24) + 0x24) * 2.5f;
        *(float *)&D_L00_00161468 = t2;
        *(float *)(m + 0x2C) = t3;
        func_L00_00251E30(*(char **)(p + 0xC));
        if (G_pp.i208C != 0 && G_pp.i2084 != 3 && G_pp.i208C != 1) { a = 0; b = 0; }
        if (G_pp.i2084 == 0x1D || G_pp.i2084 == 0x32) { a = 0; b = 0; }
        if (G_pp.b20A4) { a = 0; b = 0; }
        if (*(unsigned char *)(moby + 0x52) != 1) a = 0;
        if (4.0f < dist || 2.0f < dz) { a = 0; b = 0; }
        if (FastDiffRots(G_pp.f98, func_L00_001FF860(*(float *)(moby + 0x10) - G_pp.f80, *(float *)(moby + 0x14) - G_pp.f84)) > 1.5707964f) { a = 0; b = 0; }
        r = 0;
        if (b != 0) r = try_set_help_message(1, 0x543A) != 0;
        if (*(int *)(D_0013A5E0 + 0x2604) & 0x10) {
            if (a != 0 && r != 0) {
                VendorOpenMenu(moby);
                *(unsigned char *)(moby + 0x20) = 3;
            }
        }
        break;
    }
    case 3:
        m = *(char **)(p + 0xC);
        if (m != 0) {
            *(int *)(p + 0x90) = 0;
            *(float *)(m + 0x2C) = *(float *)(*(char **)(m + 0x24) + 0x24) * *(float *)(p + 0x90) * 2.5f;
            func_L00_00251E30(*(char **)(p + 0xC));
        }
        break;
    }
}
