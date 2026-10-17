/* functional: medium. Soft-double to unsigned 64-bit conversion (fixunsdfdi shape): negative -> 0; otherwise the high word is (unsigned)(d * 2^-32) shifted up, converted back to double and subtracted from d, and the remainder (as an unsigned 32-bit conversion) is added to it, or subtracted if the remainder went negative. Retail calls soft-double helpers (compare 0x120430, multiply 0x11FF08, to-uint 0x1205D0, from-int 0x11E7C8, add 0x11FE48, subtract 0x11FEA0); this uses host double arithmetic. Which helper is unsigned vs signed is inferred from the zero-extension of its result. */
long func_0011E6D8(double d) {
    unsigned long x;
    double r;
    if (d < 0.0) {
        return 0;
    }
    x = (unsigned long)(unsigned int)(d * 2.3283064365386963e-10) << 32;
    r = d - (double)x;
    if (r < 0.0) {
        x -= (unsigned int)(0.0 - r);
    } else {
        x += (unsigned int)r;
    }
    return (long)x;
}
