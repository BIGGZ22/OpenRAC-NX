/* 3 instructions differ, all store order (52 bytes): the field stores match retail (float b at +84, sh 1 at +62, float a at +76 of the block at +352) but GCC emits a, b, sh where retail has b, sh, a; missing: the source shape that makes the scheduler keep retail's order */
extern char *D_L00_001B8E10;

void func_L00_00424A50(float a, float b)
{
    char *p = D_L00_001B8E10;

    if (*(short *)(p + 134) == 0) {
        char *q = *(char **)(p + 112);
        char *r = q + 352;

        *(float *)(r + 84) = b;
        *(short *)(r + 62) = 1;
        *(float *)(r + 76) = a;
    }
}
