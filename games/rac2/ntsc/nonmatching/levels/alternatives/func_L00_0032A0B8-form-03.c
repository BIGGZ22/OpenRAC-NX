/* Alternative source form for func_L00_0032A0B8.
   Retained from v11.c, entry func_L00_0032A0B8; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned char u8;

extern u8 D_L00_00268E20;

s32 func_L00_0032A0B8(s32 key, s32 index)
{
    s32 base = (s32)&D_L00_00268E20;
    s32 offset = index * 0xA430;

    if (*(s32 *)(base + offset + 4) == key)
        return base + 8 + offset;
    else
        return 0;
}
