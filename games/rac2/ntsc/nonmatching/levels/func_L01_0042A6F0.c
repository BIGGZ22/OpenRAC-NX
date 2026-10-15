/* Near miss from src/levels/shared/L01_00397110.c: the build refuses this body at its place.
   Kept as a starting point; not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110.c. */
extern char D_001B9380[];

extern char D_00189EA0[];

extern void func_2CF938(char *, char *);

typedef int TI128 __attribute__((mode(TI)));

void func_L01_0042A6F0(int a0, int a1)
{
    char *p = *(char **)(D_001B9380 + 0x190);
    char *m = *(char **)(p + 0x70);

    *(unsigned char *)(p + 0x88) = a0;

    if (a0 == 2 || a0 == 3) {
        *(TI128 *)(m + 0x110) = *(TI128 *)D_00189EA0;
        func_2CF938(m + 0xFC, D_001B9380 + 0x140);
        *(int *)(m + 0xE4) = a1;
        *(int *)(m + 0xE0) = a1;
    }
}
