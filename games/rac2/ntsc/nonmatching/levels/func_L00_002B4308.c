/* First plain-C attempt from worker w08. Last check: 316 bytes vs 336; 0/27. */
extern float func_00282E20(char *);
extern float func_00283248(float, float);
extern void func_00282D30(char *, char *, float);
extern void func_00282CC0(char *, char *, char *);
extern void func_001290E0(int *, int *);
extern unsigned char D_00189E20_1[];

void func_L00_002B4308(void)
{
    char *p = (char *)D_00189E20_1;
    float distance;
    float angle;
    float value;
    float vector[4];

    if (*(short *)(p + 0x34E) == 0)
        return;
    if (*(float *)(p + 0x31C) < 0.15f)
        return;
    if (*(float *)(p + 0x118) <= 0.0f)
        return;
    if (*(unsigned char *)(p + 0x287) == 0)
        return;
    if (*(float *)(p + 0x238) <= 0.0f)
        return;
    distance = func_00282E20(p + 0x230);
    angle = func_00283248(*(float *)(p + 0x238), distance);
    if (angle >= 0.87265f)
        return;
    func_001290E0((int *)vector, (int *)vector);
    value = -(*(float *)(p + 0xF0) * vector[0] + *(float *)(p + 0xF4) * vector[1]);
    if (value > 0.0f) {
        func_00282D30((char *)vector, (char *)vector, value);
        func_00282CC0(p + 0xF0, p + 0xF0, (char *)vector);
    }
    *(float *)(p + 0xF8) -= 0.029999f;
}
