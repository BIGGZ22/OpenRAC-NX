/* 116 bytes, 3 instructions differ: the loop table pointer and the counter swap registers (retail: pointer in $s0 loaded with lui/addiu, counter in $s1; this gives counter $s0, pointer $s1 via a copy from $v0). Same result with for, do-while and index forms. Missing: whatever statement order makes GCC allocate the pointer first. */
extern void *D_L00_001B26B8[16];

extern int func_00303E20(void *object, void *arg, int a, int b);

int func_L00_00380058(void *arg)
{
    int i = 0;
    void **p = D_L00_001B26B8;
    while (i < 16) {
        if (*p == 0) {
            return 0;
        }
        if (func_00303E20(*p, arg, 0, 0)) {
            return 1;
        }
        p++;
        i++;
    }
    return 0;
}
