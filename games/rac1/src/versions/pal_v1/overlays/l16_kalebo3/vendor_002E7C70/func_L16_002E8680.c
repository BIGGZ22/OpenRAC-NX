void update_two_region_trigger_latch(char *moby) {
    char *data = *(char **)(moby + 0x78);
    switch (*(int *)(data + 8)) {
    case 0: {
        char *g = D_0013E633 + 0xE9D;
        if (is_point_inside_clip_volume(g, *(int *)data) != 0) {
            if (*(int *)(g + 0x200C) == 0xF) {
                *(int *)(data + 8) = 1;
            }
        }
        break;
    }
    case 1: {
        char *h = D_0013E633 + 0xE1D;
        if (*(int *)(h + 0x2084) == 0x42) {
            *(int *)(data + 8) = 0;
        } else if (is_point_inside_clip_volume(h + 0x80, *(int *)(data + 4)) != 0) {
            unsigned char *f = (unsigned char *)menu_item_enabled_115 + 0x1;
            if (f[0x19] == 0) {
                f[0x19] = 1;
                PlayBankSound(1, 0, 0);
                ShowBanner(0x53D6, -1);
            }
        }
        break;
    }
    }
}
