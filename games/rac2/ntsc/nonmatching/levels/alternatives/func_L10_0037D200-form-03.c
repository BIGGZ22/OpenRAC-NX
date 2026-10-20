/* Alternative source form for func_L10_0037D200.
   Retained from batch3.c, entry func_L10_0037D200_v2; declarations preserved.
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

int func_L10_0037D200(unsigned char *a0) {
    unsigned short *p = D_L10_001FB1E0[a0[0x21]];
    unsigned short h;
    int n = 0;

    if (p == 0)
        return 0;
    do {
        h = *p;
        {
            char *m = (char *)(((h & 0x7FFF) << 8) + D_L10_001B251C);
            if ((*(unsigned short *)(m + 0x34) & 0x20) != 0) {
                float *f = *(float **)(m + 0x68);
                if (f != 0 && *f > 0.0f)
                    n++;
            }
        }
        p++;
    } while ((short)h >= 0);
    return n;
}
