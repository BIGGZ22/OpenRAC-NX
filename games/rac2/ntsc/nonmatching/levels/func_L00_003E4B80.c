/* 0 of 27 exact: 100 bytes, retail 108. Missing two nops between cvt.s.w and div.s (retail: mtc1, nop, cvt.s.w, nop, nop, div.s); the early return and the join through a0 match. */
extern float func_00283CE0(void);

extern int func_00283CF0(float value);

int func_L00_003E4B80(int n)
{
    int unit;
    int k;
    int result;

    unit = 16;
    result = n;
    if (n % unit != 0) {
        k = func_00283CF0(func_00283CE0() / (float)unit);
        result = (k + 1) * unit;
    }
    return result;
}

