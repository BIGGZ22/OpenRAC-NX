void func_L06_00464900(char *a0)
{
    char *o = *(char **)(a0 + 0x68);

    switch (*(unsigned char *)(a0 + 0x20)) {
    case 0:
        *(unsigned char *)(a0 + 0x30) = 0xFF;
        *(unsigned char *)(a0 + 0x20) = 1;
        break;
    case 1: {
        int i = *(int *)(o + 4);
        int base = D_L06_001B281C;
        char *p = (char *)(base + (i << 8));
        int t = *(unsigned char *)(p + 0x20);
        int j;
        char *q;

        if (t != 2) {
            break;
        }
        j = *(int *)(o + 8);
        q = (char *)(base + (j << 8));
        if (q == 0) {
            break;
        }
        if (*(unsigned char *)(q + 0x20) == 0xFE) {
            break;
        }
        if (*(unsigned char *)(q + 0x20) == 0xFD) {
            break;
        }
        func_L00_00429020(*(int *)o);
        *(unsigned char *)(a0 + 0x20) = t;
        break;
    }
    case 2: {
        int i = *(int *)(o + 4);
        char *p = (char *)(D_L06_001B281C + (i << 8));

        if (*(unsigned char *)(p + 0x20) == 2) {
            *(unsigned char *)(a0 + 0x20) = 1;
        }
        break;
    }
    }
}
