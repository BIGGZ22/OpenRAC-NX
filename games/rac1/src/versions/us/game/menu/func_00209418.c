void save_card_state_format_pending(void) {
    char *s = memory_card_state;
    *(int *)(s + 0x1C) = 0;
    if (*(int *)(s + 0xDC) < 0) {
        *(int *)(s + 0xE0) = 0;
        *(int *)(s + 0xDC) = 3;
    }
    D_0015EFB0 = 8;
}
