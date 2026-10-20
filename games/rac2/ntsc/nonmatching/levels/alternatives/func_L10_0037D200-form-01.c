/* Alternative source form for func_L10_0037D200.
   Retained from batch2.c, entry func_L10_0037D200; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
extern unsigned short *D_L10_001FB1E0[];
extern int D_L10_001B251C;

extern int func_37BA88(void *a0);
extern int func_37CCB0(void *a0, int a1);
extern int func_37EFB8(void *a0);

extern int func_311B48(void *a0, void *a1, int a2);
extern int func_3163C8(void *a0, void *a1);
extern int func_316558(void *a0, void *a1);
extern int func_3164A0(void *a0, void *a1);
extern char *D_L01_001FE180[];

int func_L10_0037D200(unsigned char *a0) {
    unsigned short *p = D_L10_001FB1E0[a0[0x21]];
    int n = 0;

    if (p == 0)
        return 0;
    do {
        unsigned short h = *p;
        char *m = (char *)(((h & 0x7FFF) << 8) + D_L10_001B251C);
        if ((*(unsigned short *)(m + 0x34) & 0x20) != 0) {
            float *f = *(float **)(m + 0x68);
            if (f != 0 && *f > 0.0f)
                n++;
        }
    } while (*(short *)p++ >= 0);
    return n;
}
