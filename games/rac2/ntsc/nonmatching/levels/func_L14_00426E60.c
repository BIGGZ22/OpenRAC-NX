/* Near miss from src/levels/14_aranos_prison/L14_003F4F98-g8.c: the build refuses this body at its
   place — instruction at +0x30 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/14_aranos_prison/L14_003F4F98-g8.c. */
extern float D_L14_001B17B8 __attribute__((sda));

extern float D_L14_001B17BC __attribute__((sda));

extern char D_00189EA0[];

extern int func_00282948(void *table);

extern void *func_L00_002D70A0(int size);

extern void func_L00_002D7140(void *block);

void func_L14_00426E60(char *object)
{
    char *s = *(char **)(object + 0x68);
    int i;

    for (i = 0; i < 4; i++) {
        if (*(int *)(s + 0x20 + i * 4) == -1)
            continue;
        if (func_00282948(D_00189EA0) == 0)
            continue;
        if (*(int *)(s + 0x14) == 0)
            *(int *)(s + 0x14) = (int)func_L00_002D70A0(5);
        *(float *)(*(int *)(s + 0x14)) = D_L14_001B17B8;
        *(float *)(*(int *)(s + 0x14) + 4) = D_L14_001B17BC;
        return;
    }
    if (*(int *)(s + 0x14) != 0) {
        func_L00_002D7140((void *)*(int *)(s + 0x14));
        *(int *)(s + 0x14) = 0;
    }
}
