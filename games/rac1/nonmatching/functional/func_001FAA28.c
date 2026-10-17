/* functional: high. Run-length encodes a bit mask into bytes: input is a stream of (skip, run) byte pairs over the mask at the last argument (bit index advances by skip, then run bits are read); the output (at dst, limited to size bytes) is alternating run lengths of equal bits starting with the 1-bit class, each run capped at 0x7F for the first and 0xFF for later runs (a full run is followed by a 0 byte). Stops when the bit index reaches 0x40000. The first output byte is then rewritten as (b << 1) + 1. Returns the output length, or -1 if the output buffer fills. Argument names a and b are game addresses, kept as ints as callers declare them. */
int func_001FAA28(void *dst, int size, int a, int b) {
    unsigned char *d = (unsigned char *)dst;
    unsigned char *end = d + size;
    unsigned char *start = d;
    unsigned char *src = (unsigned char *)a;
    unsigned char *bits = (unsigned char *)b;
    int limit = 0x7F;
    int pos = 0;
    int prev = 1;
    int cnt = 0;
    int run = 0;
    int bit;

    for (;;) {
        if (run == 0) {
            int skip = src[0];
            run = src[1];
            pos += skip;
            src += 2;
            if (pos == 0x40000) {
                break;
            }
            continue;
        }
        bit = (bits[pos >> 3] >> (pos & 7)) & 1;
        pos++;
        run--;
        if (prev == bit) {
            if (cnt != limit) {
                cnt++;
                continue;
            }
            *d = (unsigned char)cnt;
            d += 2;
            limit = 0xFF;
            cnt = 1;
            if (d == end) {
                return -1;
            }
            d[-1] = 0;
        } else {
            prev = bit;
            *d = (unsigned char)cnt;
            d++;
            limit = 0xFF;
            if (d == end) {
                return -1;
            }
            cnt = 1;
        }
    }
    if (cnt != 0) {
        *d = (unsigned char)cnt;
        d++;
    }
    *start = (unsigned char)(((signed char)*start << 1) + 1);
    return (int)(d - start);
}
