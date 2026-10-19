/* functional: high. fcheck: unchecked. Word fill: stores value at dst, dst + 4, ... while n (bytes) counts down by 4, always storing at least one word (retail's two-way unrolled loop checks n only after each store, so n <= 4 stores one word and n = 5..8 two). Differential test: 10 cases (n from -4 to 255) agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
void func_001F99B0(void *dst, int value, int n) {
    int *p = (int *)dst;
    do {
        *p++ = value;
        n -= 4;
    } while (n > 0);
}
