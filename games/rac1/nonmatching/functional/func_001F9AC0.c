/* functional: high. FastMemOr16: dst[i] = a[i] | b[i] over 16-byte quadwords; the asm is a do-while on the byte count (count -= 16, loop while > 0), so at least one quadword is always written. Source and destination may alias (draw.c ORs into D_00194140 in place). */
void func_001F9AC0(unsigned int *dst, const unsigned int *a, const unsigned int *b, int bytes) {
    do {
        dst[0] = a[0] | b[0];
        dst[1] = a[1] | b[1];
        dst[2] = a[2] | b[2];
        dst[3] = a[3] | b[3];
        a += 4;
        b += 4;
        dst += 4;
        bytes -= 16;
    } while (bytes > 0);
}
