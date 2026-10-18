typedef struct {
    void *src;
    void *dest;
    unsigned int size;
    unsigned int attr;
} SifDmaTransfer;

extern int sceSifSetDma(SifDmaTransfer *transfer, int count);

void SND_DMAtoIOP__FPvT0Ui(void *src, void *dest, unsigned int size) {
    SifDmaTransfer transfer;
    transfer.src = src;
    transfer.dest = dest;
    transfer.size = size;
    transfer.attr = 0;
    sceSifSetDma(&transfer, 1);
}
