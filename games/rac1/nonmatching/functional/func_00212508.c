/* functional: high. fcheck: unchecked. Fills word slots from the 16-byte record table D_001C8480, D_00160038 bytes long: each record is { value, b, c, k }; for each of its k entries (k >= 1 always runs once), the word pointer at c + 4 receives the record's value in n consecutive words, n being the byte at b + 0xF; b steps by 16 and c by 8 per entry. As in retail's unrolled loop, n = 0 still stores two words. The table walk is a do-while: a zero-length table is not expected. Differential test: 3 tables of 1-3 records with 1-3 entries and n in {0, 1, 2, 3, 5} agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
extern unsigned char D_001C8480[];
extern int D_00160038;

void func_00212508(void) {
    unsigned int *rec = (unsigned int *)D_001C8480;
    unsigned int *end = (unsigned int *)(D_001C8480 + D_00160038);
    do {
        unsigned int value = rec[0];
        unsigned char *b = (unsigned char *)rec[1];
        unsigned int *c = (unsigned int *)rec[2];
        int k = (int)rec[3];
        do {
            unsigned int *dst = (unsigned int *)c[1];
            int n = b[0xF];
            for (;;) {
                *dst++ = value;
                n--;
                if (n == 0) {
                    break;
                }
                *dst++ = value;
                n--;
                if (n <= 0) {
                    break;
                }
            }
            k--;
            c += 2;
            b += 16;
        } while (k > 0);
        rec += 4;
    } while (rec != end);
}
