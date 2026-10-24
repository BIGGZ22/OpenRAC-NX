void save_card_state_formatting(void) {
    char *s = memory_card_state;
    if (*(int *)(s + 0xD4) == 2 && *(int *)(s + 0xDC) < 0) {
        if (*(int *)(s + 0xE4) != 0) {
            D_0015EFB0 = 0x11;
            D_0015EFB4 |= 0x40;
            return;
        }
        D_0015EFB0 = 0xE;
    }
}
