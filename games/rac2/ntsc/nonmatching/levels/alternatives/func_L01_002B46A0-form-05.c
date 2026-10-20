/* Alternative source form for func_L01_002B46A0.
   Retained from batch3.c, entry func_L01_002B46A0_v4; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
extern char D_L01_00189E20[];
extern float D_L01_001A7DC0;
extern void func_2B4380(int a0, float f12);

#define SDA __attribute__((section(".sdata")))
extern int D_L01_001A90C8_1 SDA;
extern float D_L01_001A90C4_1 SDA;
extern int D_L01_001A90D0_1 SDA;
extern int D_L01_001A90CC_1 SDA;
extern void func_3022C8(void *a0, void *a1, int a2);

extern int D_L01_001A90C8 SDA;
extern float D_L01_001A90C4 SDA;
extern int D_L01_001A90D0 SDA;
extern int D_L01_001A90CC SDA;

extern int func_37BA88(void *a0);
extern int func_37CCB0(void *a0, int a1);
extern int func_37EFB8(void *a0);

extern unsigned short *D_L10_001FB1E0[];
extern int D_L10_001B251C;

void func_L01_002B46A0(void) {
    char *b;
    float v;
    func_2B4380(0, 1.0f);
    b = D_L01_00189E20;
    v = *(float *)(b + 0x1B0);
    if (v > 0.0f)
        *(float *)(b + 0x1B0) = D_L01_001A7DC0 * (*(float *)(b + 0x24B8) * (1.0f / 60.0f));
}
