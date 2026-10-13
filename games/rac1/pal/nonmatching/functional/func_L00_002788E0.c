/* functional: medium. Menu pointer check: returns 0 unless B[1]->0x40 == a; then (pad word 0x13CA40+0x1C4 & 0xD00) with B+0x124 == 0 returns 1; else if pad bit 0x10 is set and B[1]->0x38 != 0 stores it at B+8 and returns 0; with B[1]->0x38 == 0 returns -1 if B+0x124 == 0 else 0. */
extern char D_L00_001BA070_788E0[] __asm__("D_L00_001BA070");
extern char D_0013CA40_788E0[] __asm__("D_0013CA40");
int func_L00_002788E0(int a) {
    char *b = D_L00_001BA070_788E0;
    char *p = *(char **)(b + 4);
    unsigned int pad;
    int v;
    if (*(int *)(p + 0x40) != a) return 0;
    pad = *(unsigned int *)(D_0013CA40_788E0 + 0x1C4);
    if (pad & 0xD00) {
        if (*(int *)(b + 0x124) == 0) return 1;
    }
    if ((pad & 0x10) == 0) return 0;
    v = *(int *)(*(char **)(b + 4) + 0x38);
    if (v == 0) {
        if (*(int *)(b + 0x124) == 0) return -1;
        return 0;
    }
    *(int *)(b + 8) = v;
    return 0;
}
