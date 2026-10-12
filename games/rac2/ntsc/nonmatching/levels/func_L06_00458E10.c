char *func_L06_00458E10(char *a0, char *a1, char *a2, int a3, float f)
{
    char *p = func_0029EFD0(0xFE5);

    if (p != 0) {
        char *q;
        float g;

        p[0x20] = 0;
        p[0x31] = 1;
        *(short *)(p + 0x32) = 0xFF;
        p[0x30] = 0xFF;
        *(long long *)(p + 0x38) = *(long long *)(a0 + 0x38);
        *(TI128 *)a2 = 0xFF;
        *(TI128 *)(p + 0x10) = 0xFF;
        D_L06_001B3760[a3] = 1;
        q = *(char **)(p + 0x68);
        *(TI128 *)a1 = 0xFF;
        *(TI128 *)q = 0xFF;
        *(int *)(q + 0x38) = 0xF0;
        *(int *)(q + 0x3C) = (int)a0;
        *(float *)(q + 0x40) = f;
        *(int *)(q + 0x48) = a3;
        *(int *)(q + 0x4C) = -1;
        g = func_00283248(*(float *)a1, *(float *)(a1 + 4));
        *(float *)(p + 0xF8) = g;
        *(float *)(p + 0xF4) = -func_00283248(func_00282E20(a1), *(float *)(a1 + 8));
        D_L06_001B3778[a3] = -1;
    }
    return p;
}
