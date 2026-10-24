int check_memory_card(void) {
    int type, free, format, cmd, result;

    result = func_00124068(D_0013D390_i[0], D_0013D390_i[1], &type, &free, &format);
    while (func_00123F30(1, &D_0013D390_i[0x30], &D_0013D390_i[0x31]) == 0) {
        func_00122598(0);
    }
    if (D_0013D390_i[0x31] == -5 || D_0013D390_i[0x31] < -9 || type != 2) {
        return 1;
    }
    if (D_0013D390_i[0x31] != -2 && format != 0) {
        result = func_001241F0(D_0013D390_i[0], D_0013D390_i[1], D_0013D2D0, 0, -1, 0);
        while (func_00123F30(1, &cmd, &result) == 0) {
            func_00122598(0);
        }
        if (result <= 0) {
            int t = free < 351;
            return t << 1;
        }
    }
    return 0;
}
