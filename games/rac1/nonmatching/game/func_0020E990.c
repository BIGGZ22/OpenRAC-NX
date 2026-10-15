extern unsigned char D_001C7A60[];

/* Frees `count` 32-byte blocks of the moby grid's list storage from block `block`: clears their
   bits in the block bitmap D_001C7A60 (bit n of byte n >> 3). Retail traps (teq) on a block that
   is already free; this returns there instead. */
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
