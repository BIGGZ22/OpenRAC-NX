/* Near miss from src/levels/20_yeedil/L20_002DEBE0.c: the build refuses this body at its
   place — 88 bytes, expected 92. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/20_yeedil/L20_002DEBE0.c. */
extern void func_L00_002F3478(void);

extern void func_002B49E0(int a, int b, int c, int d, int e);

extern int D_001FFE40[];

extern int D_001A8FDC;

extern int D_00294F98;

void func_L20_0036E720(void)
{
    func_L00_002F3478();
    D_001A8FDC = 1;
    D_001FFE40[0x41] = 0xB;
    D_001FFE40[0x40] = (int)&D_00294F98;
    func_002B49E0(3, 1, 0xB, 0, 0);
}
