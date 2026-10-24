void save_card_state_creating_save(void) {
    char *s = memory_card_state;
    if (*(int *)(s + 0xD4) == 2 && *(int *)(s + 0xDC) < 0) {
        if (*(int *)(s + 0xE4) != 0) {
            D_0015EFB0 = 0x12;
            D_0015EFB4 |= 0x40;
            return;
        }
        *(int *)(s + 0xDC) = 7;
        *(int *)(s + 0xC0) = 0;
        *(int *)(s + 0x14) = 0;
        *(int *)(s + 0xE0) = 0;
        D_0015EFB0 = 0x10;
    }
}
