/* functional: high. Byte at p: 0 is left alone and returns 1; 1 is cleared and returns 2; n >= 2 is decremented and returns 0. The pmaxw is max(byte,1), and the handwritten addi are ordinary adds (no trap in practice: byte is 0..255). */
int func_L00_001FEF78(unsigned char *p) {
    if (*p == 0) {
        return 1;
    }
    if (*p - 1 > 0) {
        *p = *p - 1;
        return 0;
    }
    *p = 0;
    return 2;
}
