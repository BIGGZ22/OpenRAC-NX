void TetheredMineUpdate(char *m) {
    float dir[4];
    float tmp[4];
    char *d = *(char **)(m + 0x78);
    void *hit = MobyGetHitMessage(m, 0x800000, 0);
    float s;
    ((unsigned char *)m)[0xA4] = 0xFF;
    if (((unsigned char *)m)[0x20] == 0) {
        *(float *)(d + 0x170) = *(float *)(m + 0x2C);
        *(float *)(d + 0x174) = *(float *)(d + 0xFC);
    }
    {
        float dist = FastVecDist(m + 0x10, camera_pos);
        float lo = *(float *)&D_L01_00161B84;
        float x0 = *(float *)&D_L01_00161B8C;
        float k = -((1.0f - lo) / (*(float *)&D_L01_00161B88 - x0));
        s = k * dist + (1.0f - k * x0);
        if (1.0f < s) s = 1.0f;
        if (s < lo) s = lo;
    }
    *(float *)(m + 0x2C) = *(float *)(d + 0x170) * s;
    *(float *)(d + 0xFC) = *(float *)(d + 0x174) * s;
    if (hit != 0) {
        int i;
        int *t;
        if (((unsigned char *)m)[0x20] == 0x65) return;
        if ((&D_0013D50F_k)[1] == 0) {
            (&D_0013D50F_k)[1] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x53D6, -1);
        }
        FastVecSub(tmp, d + 0xD0, d + 0xE0);
        m[0x20] = 0x65;
        *(u128 *)dir = *(u128 *)tmp;
        func_L00_0025F4A8(m, dir, m + 0x10, 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, 3.0f, 1, 15.0f, 1, 1, -1, 0);
        SpawnDebrisMoby(m, dir, s);
        t = (int *)(d + 0x140);
        for (i = 0; i < 4; i++) {
            if (t[i] != -1) {
                DeleteMoby((char *)(*(int *)&D_L01_00160058 + (t[i] << 8)));
            }
        }
        DeleteMoby(m);
        return;
    }
    if (((unsigned char *)m)[0x20] == 0x65) return;
    gunship_follow_path(m);
    *(unsigned short *)(m + 0x34) |= 0x1000;
    d[0x2B] = 1;
    d[0x2C] = 0x64;
    if (175.0f <= *(float *)(m + 0x18)) {
        m[0x23] = 0;
    } else if (125.0f < *(float *)(m + 0x18) && *(float *)(m + 0x18) < 175.0f) {
        m[0x23] = func_001FA898_r((175.0f - *(float *)(m + 0x18)) * 128.0f / 50.0f);
    } else if (*(float *)(m + 0x18) <= 125.0f) {
        ((unsigned char *)m)[0x23] = 0x80;
    }
}
