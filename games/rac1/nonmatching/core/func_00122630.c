/*
 * Builds a 6-quadword GS image transfer packet at buf: the BITBLTBUF, TRXPOS, TRXREG and
 * TRXDIR registers from seven shorts. arg3 selects a scale of arg6 * arg7 (a shift or
 * 3x shift); a result above 0x7FFF is reported and the function returns 0, otherwise 6.
 */
extern char D_001531B0[];
extern void func_0011A6C8();

int func_00122630(void *buf, short a1, short a2, short a3, short a4, short a5, short a6, short a7)
{
    u64 *q = (u64 *)buf;
    s64 s1 = a1, s2 = a2, s3 = a3, s4 = a4, s5 = a5, s6 = a6, s7 = a7;
    int p = (int)a6 * (int)a7;
    s64 v6 = 0;
    u64 v3, pat, r4, r5, r6, r9, r10, r11;

    if (s3 >= 0 && s3 < 0x3B) {
        switch (s3) {
        case 0: case 48:
            v6 = p >> 2;
            break;
        case 1: case 49:
            v6 = (int)((unsigned)p * 3u) >> 4;
            break;
        case 2: case 10: case 50: case 58:
            v6 = p >> 3;
            break;
        case 19: case 27:
            v6 = p >> 4;
            break;
        case 20: case 36: case 44:
            v6 = p >> 5;
            break;
        default:
            v6 = 0;
            break;
        }
    }

    if (v6 > 0x7FFF) {
        func_0011A6C8(D_001531B0);
        return 0;
    }

    v3 = (u64)v6 & 0x7FFF;
    q[10] = 0;                 /* sq zero, 0x50 */
    q[11] = 0;
    q[0] = 0;                  /* sq zero, 0x00 */
    q[1] = 0;
    pat = 0xFFFFFFFFF3FFFFFFULL;
    pat = (pat << 16) | 0xFFFFULL;
    pat = (pat << 16) | 0xFFFFULL;

    r4 = ((0 & ~0x7FFFULL) | v3) | 0x8000ULL;
    r4 &= pat;
    r4 |= 1ULL << 59;
    r5 = (0 & ~0x7FFFULL) | 4ULL;
    r5 &= 0x0FFFFFFFFFFFFFFFULL;
    r5 |= 1ULL << 60;
    r11 = (0 & ~0xFULL) | 0xEULL;
    r6 = ((u64)s1 << 32) | ((u64)s2 << 48) | ((u64)s3 << 56);
    r10 = (u64)s6 | ((u64)s7 << 32);
    r9 = ((u64)s4 << 32) | ((u64)s5 << 48);

    q[10] = r4;
    q[0] = r5;
    q[1] = r11;
    q[2] = r6;
    q[3] = 0x50;
    q[4] = r9;
    q[5] = 0x51;
    q[6] = r10;
    q[7] = 0x52;
    q[9] = 0x53;
    q[8] = 0;
    return 6;
}
