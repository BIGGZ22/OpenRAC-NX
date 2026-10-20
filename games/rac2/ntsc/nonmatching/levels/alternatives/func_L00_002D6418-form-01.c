/* Alternative source form for func_L00_002D6418.
   Retained from w17.c, entry y2; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern s32 D_L00_001A7908;
extern s32 D_L00_001B8E10;
extern s16 D_L00_00190086;

extern s32 D_L00_00189E20 __attribute__((sda));
extern u8 D_L00_00189E20_1[];

s32 func_L00_002D6418(void) {
    s32 v;

    if (D_L00_001A7908 == 0)
        return 0;
    v = D_L00_00189E20;
    if (v == 2)
        return 0;
    if (v == 6)
        return 0;
    if (D_L00_001B8E10 != 0 && D_L00_00190086 == 5)
        return 0;
    if (*(u8 *)(D_L00_00189E20_1 + 0x22B4) != 0)
        return 0;
    if (*(s32 *)(D_L00_00189E20_1 + 0x229C) == 0x14)
        return 0;
    if (*(s32 *)(D_L00_00189E20_1 + 0x24CC) == 0)
        return 0;
    v = *(s32 *)(D_L00_00189E20_1 + 0x2294);
    if (v == 0x6F || v == 0x31 || v == 0x1C || v == 0x8C)
        return 0;
    return 1;
}
