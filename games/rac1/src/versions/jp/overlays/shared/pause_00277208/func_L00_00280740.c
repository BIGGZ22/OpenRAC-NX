int Menu_GoNextWithFlag(void) {
    int f = *(int *)(D_0013A5E0 + 0x2604);
    if (f & 0x60) {
        char *p = D_L00_001BA070;
        char *q;
        *(int *)(p + 0xD4) = 0;
        q = *(char **)(p + 4);
        *(int *)(p + 8) = *(int *)(q + 0x38);
        *(int *)(q + 0x84) = 1;
    } else if (f & 0x10) {
        char *p = D_L00_001BA070;
        char *q = *(char **)(p + 4);
        *(int *)(p + 8) = *(int *)(q + 0x38);
        *(int *)(q + 0x84) = 0;
    }
    return 0;
}
