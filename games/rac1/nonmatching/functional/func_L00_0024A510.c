/* functional: medium. flag = (D_0013F450+0x208C in {0x11,0x12}) || byte D_0013F450+0x12E4 == 1. a < 351: x >= 200.0f ? flag : 0. a >= 351: branches into func_L00_0024A570's body (233.0f <= x <= 235.0f && D_0013D4EA[0] != 0), folded in here. */
extern char D_0013F450_4A510[] __asm__("D_0013F450");
extern unsigned char D_0013D4EA[];
int func_L00_0024A510(int a, float b, float c, float x) {
    int flag = 0;
    if ((unsigned int)(*(int *)(D_0013F450_4A510 + 0x208C) - 0x11) < 2) {
        flag = 1;
    } else if (*(unsigned char *)(D_0013F450_4A510 + 0x12E4) == 1) {
        flag = 1;
    }
    if (a >= 351) {
        return x >= 233.0f && x <= 235.0f && D_0013D4EA[0] != 0;
    }
    if (x >= 200.0f) return flag;
    return 0;
}
