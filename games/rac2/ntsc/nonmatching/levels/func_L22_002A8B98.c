/* Near miss from src/levels/shared/L22_002A8B98.c: the build refuses this body at its
   place — instruction at +0x4 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L22_002A8B98.c. */
extern char D_00189E20[];

extern int D_001A79F0;

extern int D_001A73A0;

extern char D_L22_001B1920[];

extern void func_L00_002D24B8(void);

extern void func_L22_002C9898(int mode, int flag);

extern void func_L00_003B09F8(void);

void func_L22_002A8B98(void)
{
    char *p = D_00189E20;
    unsigned char flag = D_001A79F0 == 0x1A;

    *(float *)(p + 0x264) = 0.45f;
    *(float *)(p + 0x258) = 0.8f;
    *(float *)(p + 0x25C) = 0.7f;
    *(float *)(p + 0xC24) = 1.0f;
    *(float *)(p + 0x250) = 0.8f;
    *(float *)(p + 0x254) = 0.7f;
    *(unsigned char *)(p + 0x24F0) = flag;
    D_001A73A0 = 0;
    func_L00_002D24B8();
    *(int *)(p + 0x2294) = 0;
    func_L22_002C9898(0, 1);
    func_L00_003B09F8();
    *(int *)(p + 0x1888) = (int)D_L22_001B1920;
}
