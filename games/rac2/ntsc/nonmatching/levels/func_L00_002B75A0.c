/* First plain-C attempt from worker w08. Last check: 360 bytes vs 372; 0/27. */
extern unsigned char D_00189E20_1[];
extern void func_L00_002B39F0(int, float);
extern char *func_L00_002AE888(char *, int, int, float, float, float, float);
extern float func_00283248(float, float);
extern void func_00282CF0(char *, char *, char *);
extern void func_L00_002D3B58(int, int, float);

void func_L00_002B75A0(float a, float b, float c, float d)
{
    char *p = (char *)D_00189E20_1;
    char *result;
    float x;
    float y;
    float scale;
    float vector[4];

    if (*(int *)(p + 0x1248) == 0x20 || *(int *)(p + 0xBE8) != 0)
        return;
    scale = *(float *)(p + 0x98);
    if (*(float *)(p + 0x24B8) <= 0.5f)
        func_L00_002B39F0(0, 1.0f);
    vector[0] = *(float *)(p + 0x190);
    vector[1] = *(float *)(p + 0x194);
    vector[2] = *(float *)(p + 0x198);
    vector[3] = *(float *)(p + 0x19C);
    *(int *)(p + 0xBE4) = 0;
    *(int *)(p + 0xBE8) = 1;
    result = func_L00_002AE888((char *)vector, -1, 0, a, b, c, d);
    if (result != 0) {
        if (*(int *)(p + 0xBE4) == 0)
            *(float *)(p + 0xBEC) = scale;
        if (*(int *)(p + 0xBE8) != 0) {
            x = func_00283248(*(float *)(result + 0x10) - *(float *)(p + 0x80),
                              *(float *)(result + 0x14) - *(float *)(p + 0x84));
            *(float *)(p + 0xBEC) = x;
            func_00282CF0(p + 0xBC0, result + 0x10, p + 0x80);
        } else {
            y = *(float *)(result + 0x10);
            *(float *)(p + 0xBEC) = y;
        }
    } else {
        *(float *)(p + 0xBEC) = scale;
    }
    func_L00_002D3B58(0, 0, 0.0f);
}
