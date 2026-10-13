void func_L11_00436850(char *p)
{
    char *q = *(char **)(p + 0x68);

    if (*(unsigned char *)(p + 0x20) < 4) {
        *(int *)(q + 0x270) = 0;
    } else if (*(short *)(q + 0x282) == 1) {
        *(short *)(q + 0x280) = func_33C800(p, *(int *)(q + 0x27C), 0, 0, 100.0f, 100.0f);
        *(int *)(q + 0x270) = *(int *)(q + 0x200);
    } else if (func_441868(p, *(short *)(p + 0xAA))) {
        *(short *)(q + 0x282) = 1;
        *(short *)(q + 0x280) = func_33C800(p, *(int *)(q + 0x27C), 0, 0, 100.0f, 100.0f);
        *(int *)(q + 0x270) = *(int *)(q + 0x200);
    }
    *(short *)(q + 0x282) = 0;
}
