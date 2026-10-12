void func_L06_00458F30(char *a0)
{
    char *obj = *(char **)(a0 + 0x68);
    short *t1 = D_L06_001B3790;
    short *t2 = D_L06_001B37A0;
    char *p = D_L06_0025FFC0;
    int i;

    for (i = 5; i >= 0; i--) {
        if (*t1 != 0) {
            float f = func_00283CE0(*t2);
            int r = func_00283CF0(f);

            func_L06_00459020(p + *(int *)(obj + 0x48) * 0x180, r, r, 4, 0.1f, 0.5f, 0.2f);
        }
        t1++;
        t2++;
        p += 0x40;
    }
    func_L06_00459620(a0);
}

INCLUDE_ASM("asm/levels", func_L06_00459020);

INCLUDE_ASM("asm/levels", func_L06_00459620);

INCLUDE_ASM("asm/levels", func_L06_00459828);
