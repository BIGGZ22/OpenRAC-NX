/* Near miss from src/levels/14_aranos_prison/L14_003F4F98.c: the build refuses this body at its
   place — 140 bytes, expected 164. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/14_aranos_prison/L14_003F4F98.c. */
extern void func_002B5278(char *object, float factor);

extern void func_002B5258(char *object);

extern void func_002B50E0(char *object, char *bone, float factor);

extern void func_002B4FF0(char *object, char *bone, float factor);

void func_L14_00423B30(char *object)
{
    char *s = *(char **)(object + 0x68);

    switch (*(unsigned char *)(object + 0x42) - 1) {
    case 0:
        if (*(short *)(s + 0x776) != 0)
            func_002B5278(object, *(float *)(s + 0x744));
        else
            func_002B5258(object);
        break;
    case 1:
        if (*(short *)(s + 0x77A) != 0)
            func_002B50E0(object, s + 0x700, *(float *)(s + 0x768));
        else
            func_002B4FF0(object, s + 0x700, *(float *)(s + 0x740));
        break;
    }
}
