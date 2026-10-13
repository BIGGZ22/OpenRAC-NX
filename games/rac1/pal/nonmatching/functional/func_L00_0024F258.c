/* functional: medium. Returns 2 if byte 0x1F of the block at 0x0013D5C8 is set, else whether byte 0x21 is nonzero. */
extern unsigned char D_0013D5C8_4F258[] __asm__("D_0013D5C8");
int func_L00_0024F258(void) {
    if (D_0013D5C8_4F258[0x1F] != 0) {
        return 2;
    }
    return D_0013D5C8_4F258[0x21] != 0;
}
