/* func_L00_003EEE20: 100 bytes against 108; the whole body matches except that retail has two nops between cvt.s.w and div.s (the int divisor converted to float), which this compiler does not emit for any form tried (variable divisor, (float)d, reassigned parameter). Missing: the source form that makes the compiler pad the cvt latency. */
extern float func_00283CE0(int a0);

extern int func_00283CF0(float f);

int func_L00_003EEE20(int a0)
{
    int d = 16;

    if (a0 % d != 0) {
        a0 = (func_00283CF0(func_00283CE0(a0) / d) + 1) << 4;
    }
    return a0;
}
