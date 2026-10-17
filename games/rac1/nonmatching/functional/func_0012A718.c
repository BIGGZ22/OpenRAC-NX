/* functional: high. DMA channel 4 completion handler: acknowledges D_STAT, then sends the next piece of the record {remaining bytes, address}: a full 0xFFFF piece while more remains (advancing the address by 0xFFFF0 masked to 28 bits), else the remainder. Returns 1 when nothing remains, otherwise 0. Interrupt re-enable (ei) and the sync are dropped; they have no host meaning. */
int func_0012A718(int arg0, void *arg1) {
    unsigned int *rec = (unsigned int *)arg1;
    unsigned int v;

    *(volatile unsigned int *)0x1000E010 = 0x10;
    v = rec[0];
    if (v == 0) {
        return 1;
    }
    if (0xFFFF < v) {
        *(volatile unsigned int *)0x1000B410 = rec[1];
        *(volatile unsigned int *)0x1000B420 = 0xFFFF;
        *(volatile unsigned int *)0x1000B400 = 0x101;
        rec[1] = (rec[1] + 0xFFFF0) & 0x0FFFFFFF;
        rec[0] = rec[0] - 0xFFFF;
    } else {
        *(volatile unsigned int *)0x1000B410 = rec[1];
        *(volatile unsigned int *)0x1000B420 = rec[0];
        *(volatile unsigned int *)0x1000B400 = 0x101;
        rec[0] = 0;
    }
    return 0;
}
