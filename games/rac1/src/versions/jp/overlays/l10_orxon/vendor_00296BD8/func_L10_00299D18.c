void update_hero_relocation_trigger(char *m) {
    char *d = *(char **)(m + 0x78);
    unsigned short id;
    ((unsigned char *)m)[0x30] = 0xFF;
    if (*(int *)&D_L10_00161410 == 1) {
        *(short *)(D_0014171B + 0x32D) = 0;
        *(int *)&D_L10_00161410 = 0;
    }
    if (*(int *)(d + 0x18) != -1) {
        int b = *(int *)(d + 0x1C);
        if (b != -1) {
            if (*(short *)((*(int *)(d + 0x18) << 8) + (int)D_L10_00160058_00299818 + 0xA6) == 0x325 &&
                *(short *)((b << 8) + (int)D_L10_00160058_00299818 + 0xA6) == 0x325) {
                char *f1 = *(char **)((*(int *)(d + 0x18) << 8) + (int)D_L10_00160058_00299818 + 0x78);
                char *f2 = *(char **)((b << 8) + (int)D_L10_00160058_00299818 + 0x78);
                if (D_0014C150[((unsigned char *)m)[0xB0] + (D_0015EE84 << 4)] == 0xFF) {
                    *(int *)(f1 + 8) |= 8;
                    *(int *)(f2 + 8) &= ~8;
                } else {
                    *(int *)(f1 + 8) &= ~8;
                    *(int *)(f2 + 8) |= 8;
                }
            }
        }
    }
    id = *(unsigned short *)(m + 0xB2);
    if (D_L10_001BB9C0.collected[(short)id] != 0 ||
        ((*(int *)(D_0014171B + 0xAB75 + ((((short)id >> 5) * 4) + (D_0015EE84 << 8))) >> (id & 0x1F)) & 1) != 0) {
        if (gHaveHeliPack[4] != 0) {
            char *bs = D_L10_00160058_00299818;
            *(unsigned short *)((*(int *)d << 8) + (int)bs + 0x34) |= 1;
            *(unsigned short *)((*(int *)d << 8) + (int)bs + 0x34) |= 2;
            *(int *)((*(int *)d << 8) + (int)bs + 0x94) = 0;
        } else {
            if (*(int *)&D_L10_0015F6A8 == 0) {
                unsigned char *T = D_0014171B + 0x22D;
                unsigned short t0 = *(unsigned short *)(T + 0x100);
                int r = scale_ticks(0x12C);
                if (t0 < r) {
                    unsigned short t1 = *(unsigned short *)(T + 0x100);
                    int r2;
                    if (t1 <= 0xFFFE)
                        *(short *)(T + 0x100) = t1 + 1;
                    r2 = scale_ticks(D_0015EFA4);
                    if (r2 / 600 > *(unsigned short *)(T + 0x102)) {
                        *(short *)(T + 0x102) = scale_ticks(D_0015EFA4) / 600;
                    }
                    {
                        int w = *(int *)(T + 0x104);
                        w |= 1 << D_0015EE84;
                        w |= 0x80000000;
                        *(int *)(T + 0x104) = w;
                    }
                    try_set_help_message(1, 0x5448);
                    if (((unsigned char *)D_0013E633)[0x2EC1] == 1) return;
                } else {
                    if (((unsigned char *)D_0013E633)[0x2EC1] == 1) return;
                }
            } else {
                if (((unsigned char *)D_0013E633)[0x2EC1] == 1) return;
            }
        }
    }
    *(int *)(D_0014171B + 0xAB75 + ((((short)*(unsigned short *)(m + 0xB2) >> 5) * 4) + (D_0015EE84 << 8)))
        |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
    D_L10_001BAC60[(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
    if (*(int *)(d + 8) == -1 || *(int *)d == -1) return;
    if (gHaveHeliPack[4] == 0) {
        char *e = (char *)((*(int *)d << 8) + (int)D_L10_00160058_00299818);
        int eo = D_0015EE84 << 4;
        unsigned char byte;
        int k;
        *(int *)(e + 0x94) = *(int *)(*(char **)(e + 0x24) + 0x10);
        byte = D_0014C150[((unsigned char *)m)[0xB0] + eo];
        if (byte == 0xFF || *(int *)(d + 0x14) == -1) {
            k = *(int *)(d + 0x14);
            Hero_ResetToPosition(D_L10_0016016C + (k << 7) + 0x30, D_L10_0016016C + (k << 7) + 0x70, 0, 1);
        } else {
            k = *(int *)(d + 8);
            Hero_ResetToPosition(D_L10_0016016C + (k << 7) + 0x30, D_L10_0016016C + (k << 7) + 0x70, 0, 1);
        }
        Hero_BeginObjectControl(1, 0x43, D_L10_00160058_00299818 + (*(int *)d << 8));
        MusicRequestTrack(2, 4);
    } else {
        unsigned char *h = (unsigned char *)D_0013E633 + 0xE1D;
        if (h[0x20A4] == 1) {
        ResetHelpState();
        {
            Hero_ResetToPosition(D_L10_0016016C + (*(int *)(d + 4) << 7) + 0x30, D_L10_0016016C + (*(int *)(d + 4) << 7) + 0x70, 0, 1);
        }
        MusicRequestTrack(0, 5);
        {
            char *bs = D_L10_00160058_00299818;
            *(unsigned short *)((*(int *)d << 8) + (int)bs + 0x34) |= 1;
            *(unsigned short *)((*(int *)d << 8) + (int)bs + 0x34) |= 2;
            *(int *)((*(int *)d << 8) + (int)bs + 0x94) = 0;
        }
            h[0x22CB] = 1;
            DeleteMoby(m);
        } else {
            h[0x22CB] = 1;
            DeleteMoby(m);
        }
    }
}
