/* functional: high. fcheck: unchecked. Quadword clear: zeroes 16 bytes at dst, dst + 16, ... while n (bytes) counts down by 16, always clearing at least one quadword. Retail uses sq $0, which ignores the low four address bits; callers pass 16-byte-aligned buffers. Differential test: 8 cases (n from -1 to 400) agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
void func_001F99D8(void *dst, int n) {
    unsigned int *p = (unsigned int *)dst;
    do {
        p[0] = 0;
        p[1] = 0;
        p[2] = 0;
        p[3] = 0;
        p += 4;
        n -= 16;
    } while (n > 0);
}
