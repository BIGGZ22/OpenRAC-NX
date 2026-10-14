/* functional: medium. Height offset by moby id (short at +0xA6): 0x5FB gives -2.55f, 0x44D or 0x5FC give 1.3f, else -2.05f. */
float func_L04_002E2B48(char *m) {
    short id = *(short *)(m + 0xA6);
    if (id == 0x5FB) {
        return -2.55f;
    }
    if (id == 0x44D || id == 0x5FC) {
        return 1.3f;
    }
    return -2.05f;
}
