/* Near miss from src/levels/shared/L03_002AB5C8.c: the build refuses this body at its
   place — 184 bytes, expected 192. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L03_002AB5C8.c. */
typedef int TI128 __attribute__((mode(TI)));

extern int D_001AEF60 __attribute__((sda));

void func_L03_0038B4E8(unsigned char *a0) {
    unsigned char *q = a0;
    unsigned char *p = *(unsigned char **)(q + 0x68);
    int v;

    v = *(int *)(p + 0x9C);
    *(int *)(p + 0x94) = 0;
    if (v < 0)
        *(int *)(p + 0x9C) = 0xE;
    v = *(int *)(p + 0x80);
    if (v < 0)
        *(int *)(p + 0x80) = 0x10;
    *(int *)(p + 0xBC) = 0x80402080;
    *(int *)(p + 0xC0) = 0x80004880;
    *(float *)(p + 0xA4) = 1.0f;
    *(float *)(p + 0xA0) = 0.25f;
    *(int *)(p + 0x68) = 0x807F0000;
    *(int *)(p + 0x6C) = 0x8000007F;
    *(int *)(p + 0xA8) = 0;
    *(int *)(p + 0x60) = 0x807F0000;
    *(int *)(p + 0x64) = 0x8000007F;
    q += 0x10;
    *(TI128 *)(p + 0xE0) = *(TI128 *)q;
    *(int *)(p + 0xD8) = -1;
    *(int *)(p + 0xDC) = *(int *)(p + 0xDC) << 3;
    *(float *)(p + 0xE8) = *(float *)(p + 0xE8) + 1.1f;
    D_001AEF60 = 0;
}
