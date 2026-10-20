/* Alternative source form for func_L01_00367F78.
   Retained from L01_002A7248-g8.c, entry func_L01_00367F78; declarations preserved.
   Keep this in a -g8.c unit to preserve its recorded file-level profile.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
extern char D_L01_00189E20[];
extern char D_L01_00139A28[];
extern int D_L01_001A79F0 __attribute__((nosda));
extern int D_L01_001ADF40[8];
extern int D_L01_001A90C8;
extern float D_L01_001A90C4;
extern int D_L01_001A90D0;
extern int D_L01_001A90CC;
extern void func_3022C8(void *a0, void *a1, int a2);

void func_L01_00367F78(void) {
    char *b = D_L01_00139A28;
    int k = D_L01_001A79F0;
    int *p = D_L01_001ADF40;
    int n = 1;

    while (n < 5) {
        if (k == *p && n < 8)
            *(unsigned char *)(b + 0x8D) |= 1 << n;
        n++;
        p++;
    }
}
