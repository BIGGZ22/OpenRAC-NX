/* Near miss from src/levels/shared/L05_003D33E8-g8.c: the build refuses this body at its
   place — 196 bytes, expected 212. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L05_003D33E8-g8.c. */
typedef int TI128 __attribute__((mode(TI)));

extern int func_328508(int);

extern void func_3065F8(int);

extern float func_332C18(void);

extern int D_L05_001B14FC __attribute__((sda));

extern int D_L05_001B2528 __attribute__((nosda));

extern void func_426270(int);

int func_L05_00425FE0(int a0)
{
    int s1 = a0;
    int s0 = func_328508(0x117C);

    if (s0 != 0) {
    *(unsigned short *)(s0 + 0x32) = 0xFF;
    *(unsigned char *)(s0 + 0x30) = 0xFF;
    *(unsigned char *)(s0 + 0x31) = 1;
    *(TI128 *)(s0 + 0x10) = *(TI128 *)(s1 + 0x10);
    func_3065F8(s0 + 0xF0);
    *(float *)(s0 + 0xF8) = func_332C18();
    if (D_L05_001B14FC == 0) {
        int p = D_L05_001B2528;

        if (p != 0) {
            int r = 0;

            do {
                if (*(short *)(p + 0xAA) == 0xFBF)
                    r = p;
                p = *(int *)(p + 0x28);
            } while (p != 0);
            D_L05_001B14FC = r;
        }
    }
        func_426270(s0);
        *(float *)(s0 + 0x2C) = *(float *)(*(int *)(s0 + 0x24) + 0x24);
        return s0;
    }
    return 0;
}
