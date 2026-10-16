/* Sets up a DMA channel at 0x1000D000 (the channel's address, count and
 * control registers) and then reads the status word at 0x20100000 (the
 * result is not returned). */
void func_001F9AF0(unsigned int a0, unsigned int a1, unsigned int a2)
{
    unsigned int *ch = (unsigned int *)0x1000D000;
    unsigned int status;

    ch[0x80 / 4] = a1;
    ch[0x10 / 4] = a0;
    ch[0x00 / 4] = 0x100;
    ch[0x20 / 4] = a2;
    status = *(unsigned int *)0x20100000;
    status |= 1;
}
