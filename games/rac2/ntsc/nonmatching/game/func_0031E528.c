/* from src/game/003173B0.c: 52-byte near miss; 19 bytes differ. */
extern unsigned char *D_001B2754;

void func_0031E528(unsigned char *a0)
{
    unsigned char *p = D_001B2754;
    int i;
    for (i = 0x3F; i >= 0; i--) {
        if (*(unsigned char **)(p + 0x10) == a0)
            *(unsigned char **)(p + 0x10) = 0;
        p += 0x30;
    }
}
