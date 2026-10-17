/* functional: high. Unpacks a single-precision float into a record: [1] = sign, then by exponent: 0 -> [0]=2 (zero, denormals included); 0xFF with zero fraction -> [0]=4 (infinity); 0xFF otherwise -> [0]= 1 if fraction bit 20 is set else 0, [3]=fraction; normal -> [0]=3, [2]=exponent-127, [3]=(fraction<<7)|0x40000000. Same stores as retail, same order. */
void func_001206B0(float *arg0, int *out) {
    unsigned int x = *(unsigned int *)arg0;
    unsigned int frac = x & 0x7FFFFF;
    unsigned int exp = (x >> 23) & 0xFF;
    out[1] = x >> 31;
    if (exp == 0) {
        out[0] = 2;
        return;
    }
    if (exp == 0xFF) {
        if (frac == 0) {
            out[0] = 4;
            return;
        }
        if ((frac & 0x100000) != 0) {
            out[0] = 1;
        } else {
            out[0] = 0;
        }
        out[3] = frac;
        return;
    }
    out[3] = (frac << 7) | 0x40000000;
    out[2] = exp - 0x7F;
    out[0] = 3;
}
