void process_unlock_code_input(void) {
    char *pad = (char *)D_0013A5E0 + 0x2460;
    int lim0, lim;
    int va, vb, vc;
    int pressed, direction, count, j, code, index, matched;
    if ((*(long *)(pad + 0x1C0) & 0xF) == 6) {
        pressed = *(int *)(pad + 0x1C4);
        if (!(pressed & 0xF0A0)) return;

        if (*(int *)&D_L00_001604B8 >= 20) return;
        if (pressed & 0x1000) direction = 0;
        else if (pressed & 0x4000) direction = 1;
        else if (pressed & 0x8000) direction = 2;
        else if (pressed & 0x2000) direction = 3;
        else direction = (pressed & 0x80) ? 4 : 5;
        count = *(int *)&D_L00_001604B8;
        D_L00_001BA838[count++] = direction;
        *(int *)&D_L00_001604B8 = count;
        if (count != 20) return;
        code = -1;
        index = 2;
        lim0 = 0x93;
        lim = *(int *)&lim0;
        for (; index < lim; index++) {
            matched = 1;
            for (j = 0; j < 20; j++) {
                if (D_L00_001BA838[j] != D_L00_001B9F20[(j * index + index) & 0xff]) {
                    matched = 0;
                    break;
                }
            }
            if (matched) {
                code = index - 2;
                break;
            }
        }
        if (code == -1) return;
        va = code - 0x24;
        vb = code - 0x37;
        vc = code - 0x3d;
        if (code < 0x25) {
            (menu_item_enabled_115 + 0xB9)[code] = 1;
            (discount_purchase_pricing + 5)[code] = 1;
        } else if ((unsigned)(code - 0x25) < 0x12) {
            unlock_planet(va);
        } else if ((unsigned)vb < 6) {
            char *p3 = D_0013D355 + 0x13B;
            p3[vb] = 1;
        } else if ((unsigned)vc < 0x1e) {
            unsigned char *q = menu_item_enabled_115 + 1 + vc;
            if (!*q) {
                *q = 1;
                PlayBankSound(1, 0, 0);
                ShowBanner(0x53D6, -1);
            }
        }
    } else {
        *(int *)&D_L00_001604B8 = 0;
    }
}
