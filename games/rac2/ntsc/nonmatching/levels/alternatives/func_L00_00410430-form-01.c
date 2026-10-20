/* Alternative source form for func_L00_00410430.
   Retained from batch5.c, entry func_L00_00410430; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
extern void func_L03_0038B2E8(void *a0, void *a1, int a2);
extern void func_00282CF0(void *a0, void *a1, void *a2);
extern void func_00282CC0(void *a0, void *a1, void *a2);
extern char *D_L03_001FE700[];

extern void func_002E5CF8(int a0, int a1, void *a2);
extern void func_L00_00410508(void *a0, void *a1);
extern void func_L00_002D3F30(int a0, int a1);
extern int D_L00_001B2D58;
extern char D_L00_001B8F20[];
extern int func_L00_003AB9C0(void *a0, void *a1, int a2, void *a3,
                             float f12, float f13, float f14, float f15);

void func_L00_00410430(char *a0, void *a1, int a2) {
    char *p = *(char **)(a0 + 0x68);
    int r;

    if (a2 != 0) {
        *(int *)(p + 0x34) = a2;
        *(int *)(p + 0x2C) = a2 - 1;
        func_002E5CF8(5, 0, a0);
        *(int *)(p + 0x28) = a2 - 1;
        func_L00_00410508(a0, a1);
        return;
    }
    func_002E5CF8(5, 0, a0);
    r = func_L00_003AB9C0(a0, a1, *(int *)p, D_L00_001B8F20,
                          *(float *)(p + 8), 2.0f, 5.0f, 0.6f);
    if (r != 0) {
        func_L00_002D3F30(-1, 1);
        D_L00_001B2D58 = 1;
        *(void **)(r + 0xB8) = a0;
    }
}
