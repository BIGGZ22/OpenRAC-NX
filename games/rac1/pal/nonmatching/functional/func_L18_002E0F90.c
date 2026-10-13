/* functional: medium. Maps the byte at +0x20: 2 -> 1, 3 -> 2, otherwise 0. */
int func_L18_002E0F90(unsigned char *p) {
    unsigned int v = p[0x20];
    if (v == 2) {
        return 1;
    }
    if (v == 3) {
        return 2;
    }
    return 0;
}
