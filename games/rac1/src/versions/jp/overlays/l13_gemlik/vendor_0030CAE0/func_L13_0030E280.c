void Moby1805Update(void *moby) {
    char *m = moby;
    char *data;
    char *item;
    char *p;
    int st;
    float tmp[4];
    int k, n;

    if ((level_moby_killed[D_0015EE84][(short)*(unsigned short *)(m + 0xB2) >> 5] >> (*(unsigned short *)(m + 0xB2) & 0x1F)) & 1) {
        DeleteMoby(moby);
        return;
    }
    data = *(char **)(m + 0x78);
    item = MobyGetHitMessage(moby, 0x10000, 0);
    ComputeHitResponse(moby, item, data + 0x20, 0, &st, 0, 0, 4);
    switch (((unsigned char *)m)[0x20]) {
    case 0:
        m[0x20] = 1;
        break;
    case 1:
        p = data + 0x60;
        if (item != 0) {
            float a = *(float *)(data + 0x20);
            float b = *(float *)(item + 0x2C);
            if (a < b) {
                m[0x20] = 2;
            } else {
                *(float *)(data + 0x20) = a - b;
                ((unsigned char *)data)[0x67] = 0xFA;
                RerollEffectCounter(moby, (short *)p);
            }
            ((unsigned char *)m)[0xA4] = 0xFF;
        }
        StepColorFade(moby, p);
        break;
    case 2: {
        if (((unsigned char *)m)[0x21] != 0xFF) {
            int r = count_group_e81e0(((unsigned char *)m)[0x21], -1);
            if (r == 1) {
                unsigned char *t = (unsigned char *)(menu_item_enabled_115 + 1);
                if (t[0x16] == 0) {
                    t[0x16] = r;
                    PlayBankSound(1, 0, 0);
                    ShowBanner(0x542D, -1);
                }
            }
        }
        Moby_PlaySound(0, 0, (int)moby);
        SpawnBreakFxByDistance(moby);
            level_moby_killed[D_0015EE84][(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
            D_L13_001BAAE0[(short)*(unsigned short *)(m + 0xB2) >> 5] |= 1 << (*(unsigned short *)(m + 0xB2) & 0x1F);
        for (n = 0; n < 2; n++) {
            for (k = 0x709; k < 0x70D; k++) {
                qzero(tmp);
                JitterVector(tmp, 1.5f);
                tmp[2] += 1.7f;
                FastVecAdd(tmp, tmp, m + 0x10);
                SpawnBreakFragment(m, k, tmp, m + 0x40, 0, 0, D_L13_0015F660, D_L13_0015F660, 0.0f, D_L13_0015F660);
            }
        }
        func_L00_0025F4A8(moby, m + 0x10, 0, 0.0f, 0.0f, 0xA, 3, 0x10, 6.0f, 3.0f, 9.0f, 0, 1.1f, 15.0f, 1, 3, -1, 0);
        DeleteMoby(moby);
        break;
    }
    }
}
