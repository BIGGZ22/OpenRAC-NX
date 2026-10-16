/* functional: medium. *out = 0. If state 0x16 and flag byte set and the object byte (+0x53) is 0x69..0x6C returns 1. Otherwise switch on a-0x1C (joined jump table): selected ids return 1, 0x37 also stores -0.95993f, 0x1C/0x1D/0x1F store -1.309f if the float at +0xAA8 > 25.0f; all else returns 0. */
extern char D_0013F450_3BA18[] __asm__("D_0013F450");
int func_L05_0023BA18(int a, float *out) {
    char *b = D_0013F450_3BA18;
    unsigned int idx;
    *out = 0.0f;
    if (*(int *)(b + 0x208C) == 0x16) {
        if (*(unsigned char *)(b + 0x8CE) != 0) {
            char *p = *(char **)(b + 0x2080);
            if ((unsigned int)((unsigned char)p[0x53] - 0x69) < 4) return 1;
        }
    }
    idx = (unsigned int)(a - 0x1C);
    if (idx >= 0x53) return 0;
    if (idx == 0 || idx == 1 || idx == 3) {
        if (*(float *)(b + 0xAA8) > 25.0f) *out = -1.3089969158172607f;
        return 1;
    }
    if (idx == 27) {
        *out = -0.9599310755729675f;
        return 1;
    }
    if (idx == 21 || idx == 22 || idx == 46 || idx == 47 || idx == 52 || idx == 68 || idx == 81 || idx == 82) return 1;
    return 0;
}
