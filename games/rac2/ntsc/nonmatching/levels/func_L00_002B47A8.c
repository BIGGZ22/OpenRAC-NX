/* First plain-C attempt from worker w08. Last check: 260 bytes vs 344; 0/27. */
extern unsigned char D_00189EA0_1[];
extern void func_00282F20(char *, void *, float);
extern void func_00282CC0(char *, char *, char *);
extern void func_L00_0031EC88(char *, char *, char *);

void func_L00_002B47A8(char *first, char *second, float x, float y, int mode)
{
    char buffer[0x50];
    char work[0x10];
    char *matrix = (char *)D_00189EA0_1;
    float r;

    if (mode != 0)
        func_00282F20(buffer, matrix + 0x240, -y);
    else
        func_00282F20(buffer, matrix + 0x370, y);
    func_00282CC0(second, matrix, buffer);
    if (mode != 0)
        func_00282F20(buffer, matrix + 0x240, -x);
    else
        func_00282F20(buffer, matrix + 0x370, x);
    func_L00_0031EC88(work, second, buffer);
    r = *(float *)(second + 8) + x;
    if (r < 0.0f)
        r = 0.0f;
    *(float *)(second + 8) = r;
    *(float *)(first + 8) += y;
}
