void save_card_state_good_save(void) {
    int flags = D_0015EFB4;
    char *b;
    int nf;
    D_0015EFB4 = flags & ~4;
    b = memory_card_state;
    nf = D_0015EFB4 & ~2;
    D_0015EFB4 = nf;
    if (*(int *)(b + 0xF4) == 0) {
        D_0015EFB0 = 3;
        return;
    }
    if (flags & 0x80) {
        D_0015EFB0 = 0x15;
        D_0015EFB4 = (nf ^ 0x80) | 0x40;
        return;
    }
    if (flags & 0x100) {
        D_0015EFB0 = 0x14;
        D_0015EFB4 = (nf ^ 0x100) | 0x40;
        return;
    }
    if (*(int *)(b + 0x1C) != 0) {
        *(int *)(b + 0xF4) = 0;
        D_0015EFB4 = nf | 1;
        D_0015EFB0 = 2;
        return;
    }
    if (flags & 0x200) {
        D_0015EFB4 = nf ^ 0x200;
        D_0015EFB0 = 0x16;
    }
}
