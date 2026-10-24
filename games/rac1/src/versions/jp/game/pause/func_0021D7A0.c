int update_menu_item_slots(SelList_0021D7A0 *arg0) {
    char *q = *(char **)(D_001D5F74 + 0x40);
    int item = *(short *)(*(int *)(q + 0x3C) * 10 + *(char **)(q + 0x48) + 6);
    int old = arg0->idx;
    char *pad = D_0013CA40;
    int i;

    if (*(int *)(pad + 0x1C4) & 8) {
        arg0->idx = (old + 1) % 8;
    }
    if (*(int *)(pad + 0x1C4) & 4) {
        arg0->idx = (arg0->idx + 7) % 8;
    }
    if (arg0->idx != old) {
        Moby_PlaySound(1, 0x11, arg0->id);
    }
    if (item != 0 && ((unsigned char *)&D_0013D5C8)[item] != 0
        && (*(int *)(pad + 0x1C4) & 0x60)) {
        Stats_0021D7A0 *s = &usage_stats;

        if (s->count <= 0xFFFE) {
            s->count++;
        }
        if (func_001F98C0(D_0015EFA4) / 600 > s->best) {
            s->best = func_001F98C0(D_0015EFA4) / 600;
        }
        s->mask = s->mask | (1 << D_0015EE84) | 0x80000000;
        for (i = 0; i < 8; i++) {
            if (arg0->list[i] == item) {
                break;
            }
        }
        if (i < 8) {
            arg0->list[i] = 0;
        }
        arg0->list[arg0->idx] = item;
        arg0->idx = (arg0->idx + 1) % 8;
    }
    return 0;
}
