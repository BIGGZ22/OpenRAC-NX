/* Form 1 of func_L00_003B3090: the float at +0xC is written before the vector call instead
   of after it (retail's schedule is ambiguous between the two). See
   nonmatching/levels/func_L00_003B3090.c for the primary near-miss and the measured wall.
   naming/extraction only: this form has not been recompiled or validated */
typedef int TI128 __attribute__((mode(TI)));

extern float func_00282F70(void *, void *, float);
extern void func_00277F48(void *, int, int, void *, float);
extern int D_L00_0018C0B0;

struct S {
    float f0;
    float f4;
    float f8;
    float fC;
    char *p10;
    int f14;
    signed char b18;
    signed char b19;
    unsigned short h1A;
    float f1C;
    int f20;
};

void func_L00_003B3090(char *a0, char *a1, char *a2) {
    struct S s;
    char *s0 = a1;
    char *s1 = a0;
    char *s2 = a2;

    s.f14 = 0x10000;
    s.f1C = *(float *)(s0 + 0x2C);
    s.f20 = 1;
    s.p10 = s1;
    *(TI128 *)&s.f0 = *(TI128 *)s0;
    func_00282F70(&s, &s, 1.0f);
    s.f8 = 1.0f;
    s.fC = 5627.925f;
    s.b18 = 3;
    s.b19 = 2;
    s.h1A = *(unsigned short *)(s1 + 0xAA);
    func_00277F48(s2, 1, D_L00_0018C0B0, &s, *(float *)(s0 + 0x30));
}
