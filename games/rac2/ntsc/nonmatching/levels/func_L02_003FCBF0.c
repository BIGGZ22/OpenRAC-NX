/* Near miss from src/levels/02_maktar_nebula/L02_003F5430-g8.c: the build refuses this body at its
   place — 180 bytes, expected 172. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/02_maktar_nebula/L02_003F5430-g8.c. */
extern char *D_L02_001B1190[] __attribute__((sda));

int func_L02_003FCBF0(char *a0) {
    char *obj = *(char **)(a0 + 0x68);
    char *sub = D_L02_001B1190[*(short *)(obj + 0x120)];
    char *e = *(char **)(sub + 0x4) + *(short *)(obj + 0x122) * 0x14;
    int ok;

    ok = *(short *)(obj + 0x90) == *(short *)(e + 0)
      && *(short *)(obj + 0x94) == *(short *)(e + 2)
      && *(short *)(obj + 0x98) == *(short *)(e + 4)
      && *(short *)(obj + 0x9C) == *(short *)(e + 6);
    if (*(unsigned char *)(e + 0x10))
        ok &= *(int *)(obj + 0xA0);
    if (*(unsigned char *)(e + 0x11))
        ok &= *(int *)(obj + 0xA4);
    if (*(int *)(obj + 0x60) != 0)
        return 0;
    return ok != 0;
}
