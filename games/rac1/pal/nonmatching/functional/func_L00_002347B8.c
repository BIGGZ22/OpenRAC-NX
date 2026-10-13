/* functional: medium. Classifies the value in D_001414D4: 0x65..0x66 -> 1, 0x67 -> 2, otherwise 0. Joined with the func_L00_002347F0 tail (return 2). */
extern int D_001414D4_47B8 __asm__("D_001414D4") NOT_SDA;
int func_L00_002347B8(void) {
    int v = D_001414D4_47B8;
    if (v >= 0x65) {
        if (v < 0x67) {
            return 1;
        }
        if (v == 0x67) {
            return 2;
        }
    }
    return 0;
}
