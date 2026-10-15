/* Near miss from src/levels/shared/L01_00397110-g8.c: the build refuses this body at its
   place — instruction at +0x0 differs. Kept as a starting point;
   not linked, not counted. Address casts are now symbolic; the refusal above
   describes the historical unit, not a fresh matching check. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110-g8.c. */
extern void func_00306250(char *);

extern char *D_001B326C __attribute__((nosda));

extern int D_001B3270 __attribute__((nosda));

extern int D_L01_001B3270 __attribute__((sda));

extern char *func_00305EE8(int);

extern void func_00307EF8(char *, int *, int *, int *);

extern char D_L01_002502C0[];

void func_L01_003E9CF8(char *a0, int a1, void (*a2)(char *))
{
    char *p = D_001B326C;
    int x;
    int y;
    int z;

    if (p == 0 || *(short *)(p + 0xAA) != 0xC41 ||
        (*(unsigned char *)(p + 0x20) != 0xFE && *(unsigned char *)(p + 0x20) != 0xFD)) {
        p = func_00305EE8(0xC41);
        D_001B326C = p;
        *(unsigned char *)(p + 0x30) = 0xFF;
    }

    *(unsigned short *)(a0 + 0x34) |= 2;
    *(int *)(a0 + 0x48) = 0;

    if (D_001B3270 < 0x40) {
        char *t;

        *(int *)(a0 + 0x44) = 0;
        func_00307EF8(a0, &x, &y, &z);
        t = D_L01_002502C0 + (D_L01_001B3270 << 5);
        *(char **)(t + 0x0) = a0;
        *(int *)(t + 0x4) = (z << 16) | (y << 8) | x;
        *(int *)(t + 0x8) = a1;
        *(float *)(t + 0xC) = 1.0f;
        *(int *)(t + 0x10) = (int)a2;
        D_L01_001B3270 = D_L01_001B3270 + 1;
    } else if (a2 != 0) {
        a2(a0);
    } else {
        func_00306250(a0);
    }
}
