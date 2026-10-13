void func_L00_003DF6B0(int arg0, int arg1)
{
    char *p = (char *)D_L00_001B043C;
    int a = arg1;
    char *q;
    int v;

    if (p == 0)
        return;
    q = *(char **)(p + 0x68);
    switch (arg0) {
    case 0:
        v = 7;
        break;
    default:
        v = 5;
        break;
    }
    p[0x20] = v;
    func_L00_003B8138();
    if (a != -1)
        *(int *)(q + 0x20) = a;
    else
        *(int *)(q + 0x20) = 10;
}
