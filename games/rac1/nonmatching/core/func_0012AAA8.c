/* Bitstream peek: the top n bits of the 64-bit accumulator at +0x0, as an
   int. The shift count is taken mod 64 as the MIPS dsrlv does. */
int func_0012AAA8(void *arg0, int n) {
    unsigned long acc = *(unsigned long *)arg0;
    return (int)(acc >> ((64 - n) & 63));
}
