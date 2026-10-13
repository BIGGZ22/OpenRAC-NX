/* functional: medium. m[0x21] is a list index (0xFF: none, returns 0). Walks D_L02_001AC140[idx] for the first moby of class 0x244 in state 0x1A: sets its +0xB8 to m, lowers its +0xB4 to m's if m's is smaller, sets m+0xB4 = m+0xB4 - moby+0xB4 (at least 1) and returns the moby; 0 if none (list ends on a negative entry). */
extern short *D_L02_001AC140_DE6E0[] __asm__("D_L02_001AC140");
extern char *D_L02_00160058_DE6E0 __asm__("D_L02_00160058") MACRO_ADDR;
char *func_L02_002DE6E0(char *m) {
    short *p;
    char *base;
    unsigned int idx = ((unsigned char *)m)[0x21];
    if (idx == 0xFF) return 0;
    p = D_L02_001AC140_DE6E0[idx];
    base = D_L02_00160058_DE6E0;
    for (;;) {
        unsigned short v = *(unsigned short *)p;
        char *moby = base + ((v & 0x7FFF) << 8);
        if (*(short *)(moby + 0xA6) == 0x244 && ((unsigned char *)moby)[0x20] == 0x1A) {
            int d;
            *(char **)(moby + 0xB8) = m;
            if (*(short *)(m + 0xB4) < *(short *)(moby + 0xB4)) {
                *(unsigned short *)(moby + 0xB4) = *(unsigned short *)(m + 0xB4);
            }
            d = *(unsigned short *)(m + 0xB4) - *(unsigned short *)(moby + 0xB4);
            *(short *)(m + 0xB4) = d;
            if ((short)d <= 0) *(short *)(m + 0xB4) = 1;
            return moby;
        }
        if ((short)v < 0) return 0;
        p++;
    }
}
