/* functional: medium. 0 if D_0013D9B4 is zero, else 1 when byte at D_0013D4AF+1 is zero, otherwise 2. */
extern int D_0013D9B4_4F1C8 __asm__("D_0013D9B4");
extern unsigned char D_0013D4AF_4F1C8[] __asm__("D_0013D4AF") MACRO_ADDR;
int func_L00_0024F1C8(void) {
    int r = 0;
    if (D_0013D9B4_4F1C8 != 0) {
        r = 2;
        if (D_0013D4AF_4F1C8[1] == 0) {
            r = 1;
        }
    }
    return r;
}
