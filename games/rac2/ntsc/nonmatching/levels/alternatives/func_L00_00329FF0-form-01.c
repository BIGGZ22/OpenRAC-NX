/* Alternative source form for func_L00_00329FF0.
   Retained from f2.c, entry func_L00_00329FF0; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;

extern float func_002EF240(void);
extern int func_002EF250(float);

s32 func_L00_00329FF0(s32 value)
{
    s32 unit = 16;

    if (value % unit != 0)
        value = (func_002EF250(func_002EF240() / unit) + 1) * 16;
    return value;
}
