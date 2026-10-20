/* Best of twelve measured shapes: 196 bytes (retail's exact size) with 12 words differing.
   The whole body is understood - build the 0x28-byte record on the stack (16-byte copy from
   A1 through mode(TI) reaching lq/sq, the pointer at +0x10, 0x10000 at +0x14, 1 at +0x20,
   the float at A1+0x2C at +0x1C), scale it, then fill the tail (1.0f at +0x8, 0x45AFDF66 at
   +0xC, 3 and 2 as bytes, the halfword of A0+0xAA) and hand it to func_00277F48(A2, 1,
   D_0018C0B0, &rec, float at A1+0x30).
   What is left is schedule and allocation, not shape: retail keeps the four stores before the
   lq/sq and materialises the two small constants in $v0/$v1, this cc1 sinks the stores past
   the call set-up, keeps the 0x10000 pair in $v1/$a3 and hoists the lq into $v1 (so the sq
   cannot share the loaded register). Six permutations of the four stores and of the block
   copy were measured in one run (best: f14, f20, f1C, p10, block), plus four spellings of
   the 0x45AFDF66 constant.
   NOTE for whoever retries: the rebuilt assembler does not read the exact decimal of a float
   to its bits - 5627.906982421875f comes out 0x45AFDF42, not 0x45AFDF66 (same effect hit on
   3.141571044921875f, read as 0x40490F80 instead of 0x40490FD0); the constant needs a
   compensating spelling, measured, not computed. */
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
    s.f20 = 1;
    s.f1C = *(float *)(s0 + 0x2C);
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
