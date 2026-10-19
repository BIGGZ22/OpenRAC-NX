/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Handwritten in retail (asm/handwritten/text/func_001FAA28.s), written from its
   instructions. Run-length codes the bits of a 0x40000-bit map at bits into dst (size bytes),
   visiting only the bits that runs names: byte pairs (skip, count), skip bits passed over,
   count bits coded, until a skip brings the bit index to exactly 0x40000. Each output byte is the
   length of a run of equal bits, starting with a run of ones; the first run holds at most 0x7F,
   the others 0xFF, and a full run is followed by an empty run of the other value. dst[0] becomes
   dst[0] * 2 + 1 at the end. Returns the bytes written, or -1 when dst fills (the end is tested
   for equality only, as retail does). */
int func_001FAA28(unsigned char *dst, int size, unsigned char *runs, unsigned char *bits) {
    unsigned char *end = dst + size;
    unsigned char *out = dst;
    int limit = 0x7F;
    int index = 0;
    int value = 1;
    int run = 0;
    int left = 0;

    for (;;) {
        int bit;

        if (left == 0) {
            int skip = runs[0];

            left = runs[1];
            runs += 2;
            index += skip;
            if (index == 0x40000) {
                break;
            }
            continue;
        }
        bit = (bits[index >> 3] >> (index & 7)) & 1;
        index++;
        left--;
        if (bit != value) {
            value = bit;
            *out++ = run;
            if (out == end) {
                return -1;
            }
            limit = 0xFF;
            run = 1;
            continue;
        }
        if (run != limit) {
            run++;
            continue;
        }
        out[0] = run;
        out += 2;
        run = 1;
        if (out == end) {
            return -1;
        }
        limit = 0xFF;
        out[-1] = 0;
    }
    if (run != 0) {
        *out++ = run;
    }
    dst[0] = (unsigned char)((signed char)dst[0] * 2 + 1);
    return out - dst;
}
