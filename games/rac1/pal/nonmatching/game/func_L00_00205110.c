/* Searches the halfword table at D_L00_00179550 (150 entries, 4 bytes apart, starting at
 * halfword a1) for id. Returns the index of the first match, or -1. When found and out is not
 * null, *out is the halfword at table + 4*index if a1 != 0, else at table + 2. */
extern unsigned char D_L00_00179550[];

int func_L00_00205110(short id, int a1, unsigned short *out) {
    char *base = (char *)D_L00_00179550;
    char *p = base + 2 * a1;
    int i;
    for (i = 0; i < 150; i++, p += 4) {
        if (*(short *)p == id) {
            /* FIX (rv02): retail's $7 starts at 2 and gains 4 per iteration (asm 0x0020512C, func_L00_00205158 addiu $7,$7,4), and `movn $7,$3,$5` only replaces it when a1 != 0, so for a1 == 0 the offset is 2 + 4*i, not 2. */
            int off = i * 4 + 2;
            if (a1 != 0) {
                off = i * 4;
            }
            if (out != 0) {
                *out = *(unsigned short *)(base + off);
            }
            return i;
        }
    }
    return -1;
}
