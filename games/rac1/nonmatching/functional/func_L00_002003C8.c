/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Run-length encodes a 0x40000-bit bitmap (bits, LSB first) into out[0..size): the (skip,count) byte pairs in rle say which bit ranges to read; runs of equal bits are written as a byte count (split with a 0 byte after a full run of 0x7F, then 0xFF); returns -1 (func_L00_001F3980) if out fills up, else the length written; the first byte is turned into (b << 1) + 1. */
extern int func_L00_001F3980(void);
int func_L00_002003C8(unsigned char *out, int size, unsigned char *rle, unsigned char *bits) {
    unsigned char *end = out + size;
    unsigned char *start = out;
    int limit = 0x7F, idx = 0, prev = 1, run = 0, cnt = 0, bit;
    for (;;) {
        if (cnt == 0) {
            int skip = rle[0];
            cnt = rle[1];
            idx += skip;
            rle += 2;
            if (idx != 0x40000) continue;
            break;
        }
        bit = (bits[(unsigned int)idx >> 3] >> (idx & 7)) & 1;
        idx++;
        cnt--;
        if (prev == bit) {
            if (run != limit) {
                run++;
                continue;
            }
            *out = run;
            out += 2;
            run = 1;
            limit = 0xFF;
            if (out == end) return func_L00_001F3980();
            out[-1] = 0;
        } else {
            prev = bit;
            *out = run;
            out++;
            limit = 0xFF;
            run = 1;
            if (out == end) return func_L00_001F3980();
        }
    }
    if (run != 0) {
        *out = run;
        out++;
    }
    *start = (*(signed char *)start << 1) + 1;
    return out - start;
}
