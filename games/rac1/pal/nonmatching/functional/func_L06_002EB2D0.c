/* functional: medium. Real function (head of a chain with func_L06_002EB2F8/002EB348 folded in): walks the moby index list D_L06_001AC340[index]; returns 0 on the first moby of class 0x33B whose state byte is not 0xFE, 0xFD or 0x10, else 1 (also 1 for an empty list). */
extern short *D_L06_001AC340[];
extern char *D_L06_00160058 MACRO_ADDR;
int func_L06_002EB2D0(int index) {
    short *p = D_L06_001AC340[index];
    if (p == 0) return 1;
    do {
        char *moby = D_L06_00160058 + ((*(unsigned short *)p & 0x7FFF) << 8);
        if (moby != 0 && *(short *)(moby + 0xA6) == 0x33B) {
            unsigned char s = moby[0x20];
            if (s != 0xFE && s != 0xFD && s != 0x10) return 0;
        }
    } while (*p++ >= 0);
    return 1;
}
