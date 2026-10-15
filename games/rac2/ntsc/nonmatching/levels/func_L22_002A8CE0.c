/* Near miss from src/levels/shared/L22_002A8B98.c: the build refuses this body at its
   place — 168 bytes, expected 184. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L22_002A8B98.c. */
extern char D_00189E20[];

extern unsigned char D_00139568[];

extern char D_L22_002645F0[];

extern int func_L00_002ADEA0(int index);

int func_L22_002A8CE0(int a0)
{
    char *p = D_00189E20;

    if (*(unsigned char *)(p + 0x22B4) == 1 || *(unsigned char *)(p + 0x22B4) == 2)
        return 0;
    if (a0 != 0 && *(unsigned char *)(p + 0x22BA) != 0 && *(short *)(p + 0x24EC) == 0) {
        int index = D_00139568[func_L00_002ADEA0(0)];

        return *(int *)(D_L22_002645F0 + index * 0xE0 + 0x28);
    }
    if (*(int *)(p + 0x24CC) == 1)
        return 0x51;
    return *(unsigned char *)(p + 0x1492) != 0 ? 0x67 : 0;
}
