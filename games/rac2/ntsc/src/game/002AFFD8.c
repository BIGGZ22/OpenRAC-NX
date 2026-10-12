

extern int D_L00_001A8FF0 __attribute__((sda));

extern int D_L00_001A8FF4_1 __attribute__((sda));

extern char D_00189E20_11[];

extern void func_00282C98(char *a);

extern void func_002AFF60(char *a, char *b, int c);

extern void func_00282CF0_5(char *a, char *b, char *c);

extern void func_00282F20_4(char *a, char *b, float f12);

extern void func_00282D30_8(char *a, char *b, float f12);

/* three-way branch on two 32-bit engine flags: one arm calls a helper, one arm calls a three-argument helper, and the third reads a float from a bound table to pick a +1/-1 scale and issues two helper calls; a final call runs only when the third argument is zero. */
void func_002AFFD8(char *a0, char *a1, int a2)
{
    char *p;
    float f20;

    if (D_L00_001A8FF0 == 0) {
        func_00282C98(a1);
    } else if (D_L00_001A8FF4_1 != 0) {
        func_002AFF60(a0, a1, 1);
    } else {
        p = D_00189E20_11;
        f20 = 1.0f;
        if (0.0f < *(float *)(p + 188))
            f20 = -1.0f;
        func_00282CF0_5(a1, p + 176, a0);
        func_00282F20_4(a1, a1, f20);
    }
    if (a2 == 0)
        func_00282D30_8(a1, a1, -1.0f);
}
