

extern int D_00138344[4];

extern int D_L04_001ADF14[4] __attribute__((sda));

extern int D_L04_001ADF10 __attribute__((sda));

extern int D_L04_001ADF18 __attribute__((sda));

extern void func_L00_00430948(char *, float, float);

extern void func_0033AE28(char *);

extern int func_002A9588_1(int, int, int, int, int);

extern void *func_00335E20(char *);

extern void func_00335E38_3(char *, float, float, float, float);

int func_003398A8(char *p)
{
    char *a = p + 8;
    char *b;
    void *r;
    float x;
    float y;

    func_L00_00430948(a, ((float *)(*(int *)(p + 756)))[0], ((float *)(*(int *)(p + 756)))[1]);
    func_0033AE28(a);
    if (D_00138344[0] & 0x5000)
        func_002A9588_1(0, 0, 1, 0, 1);
    b = p + 680;
    r = func_00335E20(b);
    *(int *)r = func_002A9588_1(0x60442D00, 0x70FFFEED, 20, 0, 0);
    x = ((float *)(*(int *)(p + 756)))[0] + (float)D_L04_001ADF10;
    y = ((float *)(*(int *)(p + 756)))[1]
        + (float)(*(int *)(p + 760) * D_L04_001ADF18);
    y = y + (float)D_L04_001ADF14[0];
    func_00335E38_3(b, x, y, 0.0f, 0.0f);
    return 0;
}
