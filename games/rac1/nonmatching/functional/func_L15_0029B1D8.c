/* functional: high. Checked against func_L15_0029B1D8 and the tail-called loop func_L15_0029B210 instruction by instruction (the loop is inlined here; the 0x29B208 return-0 stub is the beq target for a failed 0x13C test). Compile fix only: declared D_L15_001AC140 and D_L15_00160058 (a value, which the asm loads with lw before forming moby addresses). Returns 1 if the moby (byte 0x21 = list index) has no list or no listed class-0x2C moby that is alive, idle and has a lower +0x13C value than this one's data; returns 0 as soon as such a listed moby has a value not below this moby's. */
extern int D_L15_001AC140[];
extern int D_L15_00160058 MACRO_ADDR;

int func_L15_0029B1D8(void *mobyv) {
    char *moby = mobyv;
    unsigned char idx = (unsigned char)moby[0x21];
    unsigned short *list;
    char *d;
    if (idx == 0xFF) {
        return 1;
    }
    list = (unsigned short *)D_L15_001AC140[idx];
    if (list == 0) {
        return 1;
    }
    d = *(char **)(moby + 0x78);
    do {
        unsigned short v = *list;
        char *m = (char *)(((v & 0x7FFF) << 8) + D_L15_00160058);
        if (*(short *)(m + 0xA6) == 0x2C && m != 0 && (unsigned char)m[0x20] != 0xFE
            && (unsigned char)m[0x20] != 0xFD && (unsigned char)m[0xBC] == 0) {
            char *d2 = *(char **)(m + 0x78);
            if (*(int *)(d2 + 0x144) == 0 && *(int *)(d2 + 0x124) != -1) {
                if (!(*(int *)(d2 + 0x13C) < *(int *)(d + 0x13C))) {
                    return 0;
                }
            }
        }
        list++;
        if ((short)v < 0) {
            break;
        }
    } while (1);
    return 1;
}
