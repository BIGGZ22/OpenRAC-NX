int func_L00_003DB0A8(int arg)
{
    int n = 16;
    int r = arg;

    if (arg % n != 0)
        r = (func_002EF250(func_002EF240(arg) / n) + 1) * n;
    return r;
}
