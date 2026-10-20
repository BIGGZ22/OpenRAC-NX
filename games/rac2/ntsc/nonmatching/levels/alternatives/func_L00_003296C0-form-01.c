/* Alternative source form for func_L00_003296C0.
   Retained from g1.c, entry func_L00_003296C0; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned char u8;

extern u8 *ref_L00_001B1AFC_00[];

void func_L00_003296C0(char *object, s32 index, u8 value)
{
    s32 offset;

    if (object == 0)
        return;
    offset = *(u8 *)(ref_L00_001B1AFC_00[index] + 4) * 8;
    *(char **)(ref_L00_001B1AFC_00[index] + offset) = object;
    *(u8 *)(ref_L00_001B1AFC_00[index] + offset + 5) = value;
    *(unsigned short *)(ref_L00_001B1AFC_00[index] + offset + 6) = 0;
    ++*(u8 *)(ref_L00_001B1AFC_00[index] + 4);
}
