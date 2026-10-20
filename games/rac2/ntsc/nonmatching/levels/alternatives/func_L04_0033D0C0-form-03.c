/* Alternative source form for func_L04_0033D0C0.
   Retained from w15.c, entry u3; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef char s8;
typedef int s32;
typedef int TI128 __attribute__((mode(TI)));

extern float func_00283180(char *a, char *b, float f);
extern float func_00283198(float f);
extern void func_L04_0033CC00(char *a, char *b, float *v, float *w, float f);

void func_L04_0033D0C0(char *a0, char *a1) {
    float v[4];
    float w[4];

    *(TI128 *)v = *(TI128 *)(a0 + 0x10);
    v[0] += func_00283180(a0, a1, *(float *)(a0 + 0xF8)) * 100.0f;
    v[1] += func_00283198(*(float *)(a0 + 0xF8)) * 100.0f;
    func_L04_0033CC00(a0, a1, v, w, 1.0f);
}
