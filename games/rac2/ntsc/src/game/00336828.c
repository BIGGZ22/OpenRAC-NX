

extern int D_L04_001ADC6C __attribute__((sda));

extern int D_L04_001ADC70 __attribute__((sda));

extern int func_0028E588(int a, int b);

extern int func_0028EA58(int a, int b, int c, int d, int e, int *p);

void func_00336828(int *obj)
{
    int tmp[4];
    float f;

    f = *(float *)obj[4];
    if (f == 0.0f)
        return;
    if (D_L04_001ADC6C != 0)
        *(int *)obj[3] = (*(int *)obj[3] & 0xff000000) | (D_L04_001ADC70 & 0x00ffffff);
    tmp[0] = *(int *)obj[3];
    tmp[1] = *(int *)obj[3];
    tmp[2] = *(int *)obj[3];
    tmp[3] = *(int *)obj[3];
    func_0028EA58(func_0028E588((int)((float *)obj[13])[0], (int)((float *)obj[13])[1]),
             (int)((float *)obj[0])[0], (int)((float *)obj[0])[1],
             (int)((float *)obj[1])[0], (int)((float *)obj[1])[1], tmp);
}
