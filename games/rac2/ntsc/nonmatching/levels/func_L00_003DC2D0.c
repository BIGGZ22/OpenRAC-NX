void func_L00_003DC2D0(int arg)
{
    int i;

    for (i = 0; i < 10; i++) {
        char *e = D_L00_001B2B28 + i * 16;

        if (*(int *)(e + 4) == arg) {
            if (func_002EDD48(e + 8)) {
                *(int *)(e + 4) = 0;
                *(int *)(e + 0) = 0;
            }
        }
    }
}
