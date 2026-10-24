void save_card_state_create_save_pending(void) {
    char *s = memory_card_state;
    if (*(int *)(s + 0xD4) == 2 && *(int *)(s + 0xDC) < 0) {
        *(int *)(s + 0xDC) = 9;
        *(int *)(s + 0xE0) = 0;
        D_0015EFB0 = 0xF;
    }
}
