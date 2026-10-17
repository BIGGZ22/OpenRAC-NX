/* functional: high. Nothing differs but the trap: retail executes teq when a block is already free (the byte is unchanged), which traps; this returns there, after the earlier blocks of the run have been cleared, as the trap would stop before the store. The loop is retail's do/while on the count, one byte store per block. */
extern unsigned char D_001C7A60[];

void func_0020E990(int block, int count) {
    do {
        unsigned char *p = D_001C7A60 + ((unsigned int)block >> 3);
        unsigned int old = *p;
        unsigned int nw = old & ~(1u << (block & 7));

        block++;
        count--;
        if (nw == old) {
            return;
        }
        *p = (unsigned char)nw;
    } while (count > 0);
}
