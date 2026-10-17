/* functional: medium. Two passes of an MMI loop that turns 8-bit samples into 16-bit halfwords: for each 16-byte output block, the 16 source bytes start at byte (shift & 0xF) of the pair (srcA, srcB) (qfsrv), and each byte is zero-extended to a halfword (pextlb/pextub). The source pointers advance by the stride, the destination by twice the stride. Between the passes both sources advance by 0x80. The assumed MMI byte order is the low byte of each halfword from the first (data) operand; if that is backwards, every sample would be shifted up by 8 bits. */
void func_001265C8(void *arg0) {
    int *f = (int *)arg0;
    unsigned int sa = (unsigned int)f[1] & 0xF;
    unsigned int stride = (unsigned int)f[4];
    unsigned int dst = (unsigned int)f[0];
    unsigned int srcA = (unsigned int)f[5];
    unsigned int srcB = (unsigned int)f[6];
    int n = f[2];
    int i;

    do {
        unsigned short *o = (unsigned short *)dst;
        const unsigned char *A = (const unsigned char *)srcA;
        const unsigned char *B = (const unsigned char *)srcB;
        for (i = 0; i < 16; i++) {
            unsigned int k = sa + (unsigned int)i;
            o[i] = (unsigned short)(k < 16 ? A[k] : B[k - 16]);
        }
        srcA += stride;
        srcB += stride;
        dst += 2 * stride;
    } while (--n > 0);

    srcA += 0x80;
    srcB += 0x80;
    n = f[3];
    if (n == 0) {
        return;
    }

    do {
        unsigned short *o = (unsigned short *)dst;
        const unsigned char *A = (const unsigned char *)srcA;
        const unsigned char *B = (const unsigned char *)srcB;
        for (i = 0; i < 16; i++) {
            unsigned int k = sa + (unsigned int)i;
            o[i] = (unsigned short)(k < 16 ? A[k] : B[k - 16]);
        }
        srcA += stride;
        srcB += stride;
        dst += 2 * stride;
    } while (--n > 0);
}
