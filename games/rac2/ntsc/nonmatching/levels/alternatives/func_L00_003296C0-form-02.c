/* Alternative source form for func_L00_003296C0.
   Retained from g2.c, entry func_L00_003296C0; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned char u8;

extern u8 *ref_L00_001B1AFC_00[];

void func_L00_003296C0(char *object, s32 index, u8 value)
{
    u8 **cell = &ref_L00_001B1AFC_00[index];
    u8 *block;
    s32 offset;

    if (object == 0)
        return;
    block = *cell;
    offset = *(u8 *)(block + 4) * 8;
    *(char **)(*cell + offset) = object;
    *(u8 *)(*cell + offset + 5) = value;
    *(unsigned short *)(*cell + offset + 6) = 0;
    ++*(u8 *)(*cell + 4);
}
