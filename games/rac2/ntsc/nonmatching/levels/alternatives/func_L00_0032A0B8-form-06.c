/* Alternative source form for func_L00_0032A0B8.
   Retained from v15.c, entry func_L00_0032A0B8; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned char u8;

extern u8 D_L00_00268E20[];

s32 func_L00_0032A0B8(s32 a0, s32 a1)
{
    s32 idx = a1 * 0xA430;
    s32 *e = (s32 *)(D_L00_00268E20 + idx);

    if (e[1] != a0) {
        return 0;
    }
    return (s32)((D_L00_00268E20 + 8) + idx);
}
