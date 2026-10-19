/* sceSifSetDma - submit an SIF DMA transfer, honouring the suspend flag. */

extern int sceSifDmaSuspendFlag;
extern int DIntr(void);
extern void EIntr(void);
extern int sceSifSetDmaSuspend();
extern int _sceSifSetDma(void *sdd, int len);

int sceSifSetDma(void *sdd, int len) {
    int intr;
    intr = DIntr();
    if (sceSifDmaSuspendFlag) {
        if (intr)
            EIntr();
        return sceSifSetDmaSuspend(sdd, len, 0);
    }
    if (intr)
        EIntr();
    return _sceSifSetDma(sdd, len);
}
