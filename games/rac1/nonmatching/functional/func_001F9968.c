/* functional: high. fcheck: unchecked. Bit length minus one of a word, from the EE's plzcw (count of leading bits equal to the sign bit, minus one): returns floor(log2(x)) for x > 0, -1 for 0 and -1, and floor(log2(~x)) for other negatives (30 for INT_MIN). Differential test: 100 random cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
int func_001F9968(int x) {
    unsigned int u = x < 0 ? ~(unsigned int)x : (unsigned int)x;
    int n = -1;
    while (u != 0) {
        n++;
        u >>= 1;
    }
    return n;
}
