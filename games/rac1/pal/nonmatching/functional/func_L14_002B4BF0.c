/* functional: medium. Returns 1 unless the list at D_L14_001AC2C0[obj->byte21] holds a class-28 object in state 2 (then 0); no list or index 0xFF gives 1. Includes the continuation piece func_L14_002B4C2C. */
int func_L14_002B4BF0(char *obj) {
    unsigned short *ids;
    char *base;
    unsigned short id;
    if (*(unsigned char *)(obj + 0x21) == 0xFF) return 1;
    ids = (unsigned short *)D_L14_001AC2C0[*(unsigned char *)(obj + 0x21)];
    if (!ids) return 1;
    base = D_L14_00160098;
    do {
        char *m;
        id = *ids;
        m = base + (id & 0x7FFF) * 256;
        if (*(short *)(m + 0xA6) == 28 && (unsigned char)m[0x20] == 2) return 0;
        ++ids;
    } while ((short)id >= 0);
    return 1;
}
