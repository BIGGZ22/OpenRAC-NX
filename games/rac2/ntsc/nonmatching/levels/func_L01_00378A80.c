/* Near miss from src/levels/01_oozla/L01_002BEB90-g8.c: the build refuses this body at its
   place — 144 bytes, expected 156. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/01_oozla/L01_002BEB90-g8.c. */
extern int D_L01_001B2088 __attribute__((sda));

extern int D_L01_001AEC38 __attribute__((sda));

extern unsigned char D_L01_001A7A6F;

extern int D_L01_001A907C;

extern void func_3558E0(int, int, int);

extern void func_31BEC0(int, int);

void func_L01_00378A80(char *arg0) {
    if (*(unsigned char *)(arg0 + 0x20) == 0)
        *(unsigned char *)(arg0 + 0x30) = 0xFF;
    if (D_L01_001B2088 == 2)
        D_L01_001AEC38 = 0x42;
    if (D_L01_001B2088 < 6)
        return;
    if (D_L01_001AEC38 > 0)
        return;
    if (D_L01_001A7A6F != 0)
        return;
    D_L01_001A7A6F = 1;
    func_3558E0(1, 0, 0);
    func_31BEC0(0x1233, -1);
    D_L01_001A907C = 1;
}
