/* Near miss from src/levels/shared/L05_003008C0-g8.c: the build refuses this body at its
   place — instruction at +0x4 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L05_003008C0-g8.c. */
extern int D_L05_001AC4D8 __attribute__((sda));

extern int D_L05_00225DD0[];

int func_L05_00379ED0(int arg) {
    int n = D_L05_001AC4D8;
    int i = 0;

    for (; i < n; i++) {
        char *o = (char *)D_L05_00225DD0[i];

        if (*(short *)(o + 0xAA) == arg)
            return (int)o;
    }
    return 0;
}
