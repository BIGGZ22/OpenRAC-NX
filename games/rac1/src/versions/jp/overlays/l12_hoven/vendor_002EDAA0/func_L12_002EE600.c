void update_hoven_help_and_dialogue(char *m) {
    int *d = *(int **)(m + 0x78);
    if (count_group_e81e0(d[3], -1) == 0) {
        unsigned char *s = D_0013D50F_e + 1;
        if (s[0x14] == 0) {
            s[0x14] = 1;
            PlayBankSound(1, 0, 0);
            ShowBanner(0x542D, -1);
        }
    }
    {
        char *h = D_0014171B_e + 0x34D;
        if (*(unsigned short *)(h + 0x368) == 0) {
            char *q = D_0013E633_e + 0xE1D;
            if (*(int *)(q + 0x2084) == 0x81) {
                *(unsigned short *)(h + 0x368) = *(unsigned short *)(h + 0x368) + 1;
            }
            if (scale_ticks(D_0015EFA4_e) / 600 > *(unsigned short *)(h + 0x36A)) {
                *(unsigned short *)(h + 0x36A) = scale_ticks(D_0015EFA4_e) / 600;
            }
            *(unsigned int *)(h + 0x36C) = *(unsigned int *)(h + 0x36C) | (1 << D_0015EE84_e) | 0x80000000;
            if (is_point_inside_clip_volume(q + 0x80, d[0])) {
                Help_Request(0x2EE3, 0x6D);
            }
        }
    }
    {
        char *h = D_0014171B_e + 0x34D;
        if (*(unsigned short *)(h + 0x370) == 0
            || (*(unsigned short *)(h + 0x370) < 2
                && scale_ticks(D_0015EFA4_e) - *(unsigned short *)(h + 0x372) * 600 > scale_ticks(72000))) {
            char *q = D_0013E633_e + 0xE1D;
            if (*(int *)(q + 0x2084) == 0x81) {
                d[1]++;
                if ((D_0013A5E0_e[0x2600 / 4] & 3) && 0.3f < *(float *)(q + 0x229C)) {
                    d[2]++;
                } else {
                    d[2] = 0;
                }
                if (scale_ticks(0x28) < d[2]) {
                    char *g = D_0014171B_e + 0x34D;
                    d[1] = 0;
                    if (*(unsigned short *)(g + 0x370) <= 0xFFFE) {
                        *(unsigned short *)(g + 0x370) = *(unsigned short *)(g + 0x370) + 1;
                    }
                    if (scale_ticks(D_0015EFA4_e) / 600 > *(unsigned short *)(g + 0x372)) {
                        *(unsigned short *)(g + 0x372) = scale_ticks(D_0015EFA4_e) / 600;
                    }
                    *(unsigned int *)(g + 0x374) = *(unsigned int *)(g + 0x374) | (1 << D_0015EE84_e) | 0x80000000;
                }
                {
                    int a = scale_ticks(0x1C20);
                    int b = scale_ticks(0x708);
                    char *k = D_0014171B_e + 0x34D;
                    if (*(unsigned short *)(k + 0x370) * a + b < d[1]) {
                        Help_Request(0x2EE4, 0x6E);
                    }
                }
            }
        }
    }
    if (D_0013D5CA_e[2] != 0) {
        char *k = D_0014171B_e + 0x34D;
        if (*(unsigned short *)(k + 0x1F0) == 0) {
            Help_Request(0x2EE0, 0x3E);
        }
    }
}
