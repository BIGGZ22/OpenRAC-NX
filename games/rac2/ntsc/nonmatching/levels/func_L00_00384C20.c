/* 132 bytes, 4 instructions differ (128 bytes here): the two do-while loops have the right shape, but GCC loads each entry into $a0 and tests it there, while retail loads it into $v0, tests it, then copies it to $a0 in the branch delay slot, and it keeps the pointer increment before the test. Missing: the statement form that makes the value live in $v0. Tried: void * and int temporaries. */
extern void *D_L00_00232A60[24];

extern void ref_L00_00384C20_00(void *arg);

void func_L00_00384C20(char *obj)
{
    void **p = (void **)(*(char **)(obj + 0x68) + 0x48);
    int i = 1;
    do {
        int v = (int)*p;
        p++;
        i--;
        if (v) {
            ref_L00_00384C20_00((void *)v);
        }
    } while (i >= 0);
    p = D_L00_00232A60;
    i = 23;
    do {
        int v = (int)*p;
        p++;
        i--;
        if (v) {
            ref_L00_00384C20_00((void *)v);
        }
    } while (i >= 0);
}
