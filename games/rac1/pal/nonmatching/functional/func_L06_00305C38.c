/* functional: medium. Returns 2 if byte at +0x20 is 7, else (byte > 3). Joined with the func_L00_002347F0 tail (return 2). */
int func_L06_00305C38(unsigned char *p) {
    unsigned int v = p[0x20];
    if (v == 7) {
        return 2;
    }
    return 3 < v;
}
