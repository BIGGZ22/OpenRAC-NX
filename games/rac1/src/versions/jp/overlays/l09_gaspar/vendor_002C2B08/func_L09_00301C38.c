void update_level09_item_unlock_and_frame_record(char *moby) {
    int *data = *(int **)(moby + 0x78);
    unsigned char *q;
    unsigned char *b;
    unsigned short *tp;
    int n;
    if (D_L09_0015F6B0 > 0) {
        if (D_L09_00161450 == 0) {
            q = menu_item_enabled_115 + 1;
            if (q[0xE] == 0) {
                q[0xE] = 1;
                PlayBankSound(1, 0, 0);
                ShowBanner(0x542D, -1);
                D_L09_00161450 = 0;
            } else {
                D_L09_00161450 = 0;
            }
        } else {
            D_L09_00161450 = 0;
        }
    }
    b = D_0014171B + 0x34D;
    tp = (unsigned short *)(b + 0x2D8);
    if (*tp == 0) {
        if (gHaveHeliPack[9] != 0) {
            (*tp)++;
        }
        n = scale_ticks(D_0015EFA4) / 600;
        if (*(unsigned short *)(b + 0x2DA) < n) {
            *(unsigned short *)(b + 0x2DA) = scale_ticks(D_0015EFA4) / 600;
        }
        *(int *)(b + 0x2DC) = *(int *)(b + 0x2DC) | (1 << D_0015EE84_m) | 0x80000000;
        if (is_point_inside_clip_volume(D_0013E633 + 0xE9D, *data) != 0) {
            Help_Request(0x2329, 0x5B);
        }
    }
}
