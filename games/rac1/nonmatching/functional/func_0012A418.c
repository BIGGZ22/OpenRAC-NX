/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). DMA channel 3 completion handler for the chunked copy: acknowledges D_STAT, counts the chunk, flags an error if the channel still has quadwords or is still running, otherwise starts the next full chunk (or the final, shorter one). Second argument is the progress record {done, err, size, next, total}. Interrupt re-enable (ei) and the sync are dropped; they have no host meaning. */

int func_0012A418(int arg0, void *arg1) {
    int *rec = (int *)arg1;

    *(volatile unsigned int *)0x1000E010 = 8;
    rec[0] = rec[0] + 1;
    if (*(volatile unsigned int *)0x1000B020 != 0 || (*(volatile unsigned int *)0x1000B000 & 0x100) != 0) {
        rec[1] = 1;
        return 0;
    }
    if (rec[0] < rec[4] - 1) {
        *(volatile unsigned int *)0x1000B010 = rec[3];
        *(volatile unsigned int *)0x1000B020 = 0xFFC0;
        *(volatile unsigned int *)0x1000B000 = 0x100;
        *(volatile unsigned int *)0x10002000 = 0x700003FF;
        rec[3] = (rec[3] + 0xFFC00) & 0x0FFFFFFF;
    } else if (rec[0] == rec[4] - 1) {
        rec[2] = rec[2] - (rec[0] * 0x400 - rec[0]);
        *(volatile unsigned int *)0x1000B010 = rec[3];
        *(volatile unsigned int *)0x1000B020 = rec[2] << 6;
        *(volatile unsigned int *)0x1000B000 = 0x100;
        *(volatile unsigned int *)0x10002000 = rec[2] | 0x70000000;
    }
    return 0;
}
