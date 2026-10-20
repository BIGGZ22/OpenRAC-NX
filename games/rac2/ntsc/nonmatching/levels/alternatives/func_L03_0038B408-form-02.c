/* Alternative source form for func_L03_0038B408.
   Retained from batch5.c, entry func_L03_0038B408_v1; declarations preserved.
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

void func_L03_0038B408(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    int buf[4];
    int i;
    int j;
    char *q;
    char *v;
    int n;

    for (i = 0; i < 4; i++) {
        q = D_L03_001FE700[*(int *)(p + 0x28 + i * 4)];
        if (q == 0)
            continue;
        func_L03_0038B2E8(buf, q, *(int *)(p + 0x38));
        n = *(int *)(p + 0x38);
        if (n <= 0)
            continue;
        v = q;
        for (j = 0; j < n; j++) {
            v += 0x10;
            func_00282CF0(v, v, buf);
            func_00282CC0(v, v, p + 0xE0);
        }
    }
}
