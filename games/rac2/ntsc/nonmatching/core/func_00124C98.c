/* from src/core/00124C98.c: C that compiles but does not link at its places yet */
/* rac1-decomp src/core/001208E8.c: func_00120D28 */
/* _sceCd_scmd_prechk (libcdvd.a cdvd000.o) */
int func_00124C98(int arg0) {
    int i;
    volatile int *sem = &r1_D_001313E8;

    r1_func_00120B28();
    if (*sem != r1_func_00118CC0(*sem)) {
        if (r1_D_001313D0 > 0) {
            r1_func_0011A6C8(r1_D_00152FB0, arg0, r1_D_001313DC);
        }
        return 0;
    }
    r1_D_001313DC = arg0;
    r1_func_00118BF0(r1_D_00159850, r1_D_00159858);
    if (func_00124B88(1) != 0) {
        r1_func_00118C90(*sem);
        return 0;
    }
    r1_func_0011AE20(0);
    if (r1_D_001313F8 >= 0) {
        return 1;
    }
    while (1) {
        if (r1_func_0011B2F8(r1_D_00132590, 0x80000595, 0) < 0) {
            if (r1_D_001313D0 > 0) {
                r1_func_0011A6C8(r1_D_00152FD8);
            }
            for (i = 0x100000; i != -1; i--)
                ;
        } else {
            if (((SifRpcClientData *)r1_D_00132590)->serve != 0) {
                break;
            }
            for (i = 0x100000; i != -1; i--)
                ;
        }
    }
    r1_D_001313F8 = 0;
    return 1;
}
