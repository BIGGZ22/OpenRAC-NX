void SetMenuStateFromCardStatus(void) {
    char *b = D_0013D355 + 0x3B;
    if (*(int *)(b + 0xD4) == 2 && *(int *)(b + 0xDC) < 0) {
        if (*(int *)(b + 0xE4) != 0) {
            D_0015EFB0 = 17;
            D_0015EFB4 |= 0x40;
        } else {
            D_0015EFB0 = 14;
        }
    }
}
