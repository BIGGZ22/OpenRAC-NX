/* Alternative source form for func_L10_004190F0.
   Retained from batch2.c, entry func_L10_004190F0; declarations preserved.
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

void func_L10_004190F0(char *a0) {
    char *p = *(char **)(a0 + 0x68);
    int r = func_37BA88(a0);

    *(int *)(r + 0x148) = 10;
    func_37CCB0(a0, 1);
    *(float *)(p + 0x180) = 10.0f;
    *(unsigned char *)(a0 + 0x20) = 1;
    *(unsigned char *)(a0 + 0x30) = 0xFF;
    *(unsigned short *)(a0 + 0x32) = 0x1FF;
    *(unsigned short *)(a0 + 0x34) |= 0x100;
    *(float *)(a0 + 0x2C) = 0.08f;
    *(int *)(p + 0x268) = 0;
    *(int *)(p + 0x26C) = -1;
    func_37EFB8(a0);
}
