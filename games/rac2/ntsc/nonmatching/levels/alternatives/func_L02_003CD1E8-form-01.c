/* Form 1 of func_L02_003CD1E8: the state pointer is read into a local before the two float
   helpers instead of as the first statement (this is the shape the primary near-miss keeps).
   All twelve measured shapes are in the primary near-miss. naming/extraction only: this form
   has not been recompiled or validated */
typedef int TI128 __attribute__((mode(TI)));

extern float func_002A7878(void *, void *, float, float);
extern float func_002A7940(void);
extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00282CC0(void *, void *, void *);
extern int func_L02_0032EF10(void *, void *, void *, int, float);

void func_L02_003CD1E8(char *a0, char *a1) {
    char *s0 = a1;
    char *p = *(char **)(a0 + 0x68);
    TI128 q;
    float f21;
    float f20;

    f21 = func_002A7878(a0, a1, 2.0f, 5.0f);
    f20 = func_002A7940();
    *(float *)(s0 + 0x0) = func_00283180(f20) * f21;
    *(float *)(s0 + 0x4) = func_00283198(f20) * f21;
    *(int *)(s0 + 0x8) = 0;
    func_00282CC0(s0, s0, p + 0x340);
    q = *(TI128 *)(p + 0x340);
    func_L02_0032EF10(&q, s0, s0, *(int *)(p + 0x318), *(float *)(p + 0x100));
}
