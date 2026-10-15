/* Near miss from src/levels/shared/L05_00428350.c: the build refuses this body at its
   place — 164 bytes, expected 160. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L05_00428350.c. */
extern void func_2DCB60(int, int);

extern void func_2EB6E8(void);

extern int D_00221D94;

extern int D_001A79F0;

extern char *D_001B1EC4;

extern char D_00189E20[];

extern char D_0019B2E8[];

int func_L05_00445F90(char *a0)
{
    char *row;
    char *base = D_00189E20;

    if (D_00221D94 == 0) {
        row = *(char **)(D_001B1EC4 + ((*(short *)(a0 + 0x84)) << 5) + 0x1C);
        if ((*(unsigned char *)(*(char **)(row + 0x5C) + (D_001A79F0 << 4)
                                + (int)D_0019B2E8)
             & 0x80)
            == 0) {
            func_2DCB60(0x31, 1);
            *(int *)(*(char **)(base + 0x2290) + 0x98) = 0;
            func_2EB6E8();
            *(unsigned char *)(base + 0x22B5) = 1;
            return 1;
        }
    }
    return 0;
}
