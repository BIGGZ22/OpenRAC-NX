void save_card_state_check_save(void) {
    char *s = memory_card_state;
    if (*(int *)(s + 0xD4) == 2 && *(int *)(s + 0xDC) < 0) {
        *(int *)(s + 0xDC) = 7;
        *(int *)(s + 0xE0) = 0;
        D_0015EFB0 = 0xB;
    }
}
