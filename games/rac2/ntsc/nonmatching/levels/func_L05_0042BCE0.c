/* Near miss from src/levels/shared/L05_00428350.c: the build refuses this body at its
   place — 188 bytes, expected 200. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L05_00428350.c. */
extern char *func_328508(char *);

extern void func_32AF08(char *);

extern void func_3073E0(char *, char *);

typedef int TI128 __attribute__((mode(TI)));

char *func_L05_0042BCE0(char *a0, char *a1, float f12, float f13)
{
    char *obj = func_328508((char *)0x1226);

    if (obj != 0) {
        char *sub;

        *(unsigned short *)(obj + 0x32) = 0x1FF;
        *(unsigned char *)(obj + 0x30) = 0xFF;
        *(unsigned char *)(obj + 0x31) = 1;
        *(float *)(obj + 0x2C) = *(float *)(*(char **)(obj + 0x24) + 0x24) * f12;
        *(unsigned short *)(obj + 0x34) =
            (*(unsigned short *)(obj + 0x34) & 0xFFF8) | 0x100;
        sub = *(char **)(obj + 0x68);
        *(TI128 *)(obj + 0x10) = *(TI128 *)a0;
        func_3073E0(a1, obj + 0xC0);
        *(float *)(sub + 0x10) = f13;
        func_32AF08(obj);
    }
    return obj;
}
