/* functional: medium. Scans the 0x100-byte records from *D_L00_00160098 up to *D_L00_0016009C for the first whose short at +0xB2 equals key; returns it or 0. Joined with the func_L00_002864CC loop tail. */
extern char *D_L00_00160098_86498 __asm__("D_L00_00160098");
extern char *D_L00_0016009C_86498 __asm__("D_L00_0016009C");
int func_L00_00286498_impl(int unused, int key) __asm__("func_L00_00286498");
int func_L00_00286498_impl(int unused, int key) {
    char *p = D_L00_00160098_86498;
    char *end = D_L00_0016009C_86498;
    while (p != end) {
        if (*(short *)(p + 0xB2) == key) {
            return (int)p;
        }
        p += 0x100;
    }
    return 0;
}
