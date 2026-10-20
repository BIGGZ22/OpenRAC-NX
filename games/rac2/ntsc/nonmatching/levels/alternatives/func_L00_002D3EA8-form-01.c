/* Alternative source form for func_L00_002D3EA8.
   Retained from v2d3ea8.c, entry a1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int V4 __attribute__((mode(TI)));

typedef struct {
    float f0;
    float f1;
    float f2;
    float f3;
} Q;

extern char g_base[];

extern float func_2EE268(char *);
extern float func_2EE7A8(float, float);
extern void func_2EE5D8(void *, void *, char *);

float func_L00_002D3EA8(char *in)
{
    char *p = g_base;
    int mode = *(unsigned char *)(p + 8899);
    V4 local;

    if (mode == 0)
        return func_2EE7A8(*(float *)(in + 8), func_2EE268(in));
    if (mode < 0)
        return 0.0f;
    if (mode >= 3)
        return 0.0f;
    local = *(V4 *)in;
    func_2EE5D8(&local, &local, p + 64);
    return func_2EE7A8(*(float *)((char *)&local + 8), func_2EE268((char *)&local));
}
