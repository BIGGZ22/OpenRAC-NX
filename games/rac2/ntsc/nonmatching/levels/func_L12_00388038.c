/* Near miss from src/levels/12_todano/L12_002CF7B0-g8.c: the build refuses this body at its
   place — instruction at +0x4 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/12_todano/L12_002CF7B0-g8.c. */
extern int D_L12_001A8F00 __attribute__((nosda));

extern int D_L12_001BDB30 __attribute__((nosda));

extern char *D_L12_001B2EA8 __attribute__((sda));

void func_L12_00388038(void)
{
    char *o;

    if (D_L12_001A8F00 != 2)
        return;
    if (D_L12_001BDB30 != 5)
        return;
    o = D_L12_001B2EA8;
    if (o == 0)
        return;
    do {
        *(int *)(o + 0x98) = 0;
        *(unsigned short *)(o + 0x34) = (*(unsigned short *)(o + 0x34) | 0x41) & 0xEFFF;
        o = *(char **)(o + 0x28);
    } while (o != 0);
}
