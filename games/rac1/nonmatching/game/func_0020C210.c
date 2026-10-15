/* DmaToSpr(madr, qwc, tadr): sets channel 8's MADR, QWC and TADR, then
   CHCR = 0x100 (STR) to start the transfer; returns 0x100. Stores in the
   retail order: TADR, QWC, MADR, CHCR. */
int func_0020C210(unsigned int madr, unsigned int qwc, unsigned int tadr)
{
    *(unsigned int *)0x1000D480 = tadr;
    *(unsigned int *)0x1000D420 = qwc;
    *(unsigned int *)0x1000D410 = madr;
    *(unsigned int *)0x1000D400 = 0x100;
    return 0x100;
}
