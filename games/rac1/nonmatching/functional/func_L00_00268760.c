/* functional: medium. Slot allocator: takes slot D_L00_00160230 (returns 0 when it is >= 0x800), sets its bit in the bitmap D_L00_001B1C00, advances the hint to the next clear bit (rescanning from the start of the hint's byte unless the slot was beyond the high-water mark D_L00_00160234), raises the high-water mark, counts D_L00_00160238, zeroes the first 0x20 bytes of the 64-byte record at D_L00_0016022C + slot*64, stores a0 in its first byte and returns the record. The unreachable alternate scan code after the first jump is not modelled. Not run against the capture. */
extern char *D_L00_0016022C_68760 __asm__("D_L00_0016022C");
extern int D_L00_00160230_68760 __asm__("D_L00_00160230");
extern int D_L00_00160234_68760 __asm__("D_L00_00160234");
extern int D_L00_00160238_68760 __asm__("D_L00_00160238");
extern unsigned char D_L00_001B1C00_68760[] __asm__("D_L00_001B1C00");

char *func_L00_00268760(int tag) {
    int cur = D_L00_00160230_68760;
    int hw = D_L00_00160234_68760;
    char *base = D_L00_0016022C_68760;
    int slot = cur;
    int *rec;
    unsigned char *bm = D_L00_001B1C00_68760;

    if (cur - 0x800 >= 0) {
        return 0;
    }
    bm[slot >> 3] |= (unsigned char)(1 << (slot & 7));
    cur = cur + 1;
    if (!(slot - hw > 0)) {
        int idx = cur >> 3;
        cur = idx << 3;
        for (;;) {
            unsigned int b, mask;
            if (cur - 0x800 >= 0) {
                break;
            }
            b = bm[idx];
            mask = 1;
            for (;;) {
                unsigned int t = b & mask;
                unsigned int nz;
                mask <<= 1;
                if (t == 0) {
                    goto done;
                }
                nz = mask & 0xFF;
                cur++;
                if (nz == 0) {
                    break;
                }
            }
            idx++;
        }
    }
done:
    D_L00_00160230_68760 = cur;
    if (hw < slot) {
        hw = slot;
    }
    D_L00_00160234_68760 = hw;
    D_L00_00160238_68760 = D_L00_00160238_68760 + 1;
    rec = (int *)(base + slot * 64);
    rec[0] = 0;
    rec[1] = 0;
    rec[2] = 0;
    rec[3] = 0;
    rec[4] = 0;
    rec[5] = 0;
    rec[6] = 0;
    rec[7] = 0;
    *(char *)rec = (char)tag;
    return (char *)rec;
}
