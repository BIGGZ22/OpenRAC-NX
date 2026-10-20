/* 1 of 44 words differs, plus the 4 bytes that word adds: the whole body, both calls, the
   128-bit lq/sq copy and the epilogue are in retail's order and registers, but this cc1
   allocates the first parameter's pseudo to $v0, so the `lw $s1,0x68($a0)` that retail
   keeps in the first call's delay slot comes out as `daddu $v0,$a0,$zero` +
   `lw $s1,0x68($v0)` (180 bytes against 176).
   Twelve source shapes measured (state pointer loaded before/between/after the two calls,
   struct field, unsigned/int/void* loads, index expression, explicit local copies of both
   parameters, -O2 and -O2 -G8): the copy is invariant, so it is this cc1's local-alloc
   choice for the parameter pseudo, not a source-shape difference. */
typedef int TI128 __attribute__((mode(TI)));

extern float func_002A7878(void *, void *, float, float);
extern float func_002A7940(void);
extern float func_00283180(float);
extern float func_00283198(float);
extern void func_00282CC0(void *, void *, void *);
extern int func_L02_0032EF10(void *, void *, void *, int, float);

void func_L02_003CD1E8(char *a0, char *a1) {
    char *p = *(char **)(a0 + 0x68);
    char *s0 = a1;
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
