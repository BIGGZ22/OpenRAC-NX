/* functional: medium. For a < 0xDD returns 1 if x >= 242.0f else 0; otherwise returns 0 if x > 180.0f, else whether the byte at D_0013D4E7 is nonzero. */
extern unsigned char D_0013D4E7_4A430[] __asm__("D_0013D4E7") MACRO_ADDR;
/* FIX (rv02): retail reads x from $f14 (asm 0x0024A448 c.le.s $f0,$f14), the third float argument; the sibling hit tests take (a, b, c, x). */
int func_L00_0024A430(int a, float b, float c, float x) {
    if (a < 0xDD) {
        return 242.0f <= x;
    }
    if (!(x <= 180.0f)) {
        return 0;
    }
    return D_0013D4E7_4A430[0] != 0;
}
