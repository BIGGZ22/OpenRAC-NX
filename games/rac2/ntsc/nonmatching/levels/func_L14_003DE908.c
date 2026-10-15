/* Near miss from src/levels/shared/L14_003DE908.c: the build refuses this body at its
   place — 132 bytes, expected 136. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L14_003DE908.c. */
typedef int TI128 __attribute__((mode(TI)));

extern int func_L00_00324120(char *object);

extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);

extern char D_00189EA0[];

extern int D_L14_001BF958;

int func_L14_003DE908(char *a0)
{
    char buf[0x20];

    if (func_L00_00324120(a0) != 0)
        return 1;
    *(TI128 *)(buf + 0x10) = *(TI128 *)D_00189EA0;
    *(TI128 *)(buf + 0) = *(TI128 *)D_00189EA0;
    *(float *)(buf + 0x18) = *(float *)(buf + 0x18) - 3.0f;
    if (func_002761F8(buf, buf + 0x10, 0x10, 0, 0) != 0 && D_L14_001BF958 == (int)a0)
        return 1;
    return 0;
}
