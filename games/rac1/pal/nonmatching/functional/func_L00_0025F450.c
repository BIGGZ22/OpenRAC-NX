/* functional: medium. Returns 1 when the moby's class (short at +0xA6) is one of 0xE2, 0xCC, 0xDE, 0x3EE, 0xD6, 0xE1, 0xD5, 0xDF, else 0. */
int func_L00_0025F450(char *moby) {
    int c = *(short *)(moby + 0xA6);
    if (c == 0xE2 || c == 0xCC || c == 0xDE || c == 0x3EE ||
        c == 0xD6 || c == 0xE1 || c == 0xD5 || c == 0xDF) {
        return 1;
    }
    return 0;
}
