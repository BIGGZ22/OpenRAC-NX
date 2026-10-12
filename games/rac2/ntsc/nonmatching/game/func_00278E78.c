int func_00278E78(void) {
    extern int D_001BAF80[8];
    int value = D_001BAF80[7];
    if (value < 0) return -1;
    value &= 31;
    if (value != 31) return value;
    return -1;
}
