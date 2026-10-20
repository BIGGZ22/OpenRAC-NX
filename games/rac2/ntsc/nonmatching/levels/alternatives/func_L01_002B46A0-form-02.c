/* Alternative source form for func_L01_002B46A0.
   Retained from batch1.c, entry func_L01_002B46A0_v1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
extern char D_L01_00189E20[];
extern char D_L01_00139A28[];
extern char D_L01_002637A0[];
extern char D_L01_001A79F0;
extern float D_L01_001A7DC0;
extern void func_2B4380(int a0, float f12);
extern int func_2AE958(int a0);
extern void func_3022C8(void *a0, void *a1, int a2);
extern int D_L01_001A90C8 __attribute__((sda));
extern float D_L01_001A90C4 __attribute__((sda));
extern int D_L01_001A90D0 __attribute__((sda));
extern int D_L01_001A90CC __attribute__((sda));
extern int D_L01_001ADF40[8] __attribute__((sda));

void func_L01_002B46A0(void) {
    char *b = D_L01_00189E20;
    float v;
    func_2B4380(0, 1.0f);
    v = *(float *)(b + 0x1B0);
    if (v > 0.0f)
        *(float *)(b + 0x1B0) = D_L01_001A7DC0 * (*(float *)(b + 0x24B8) * (1.0f / 60.0f));
}
