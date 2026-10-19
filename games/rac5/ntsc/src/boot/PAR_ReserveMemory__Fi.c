typedef struct {
    int pad00[2];
    int pages;
    char pad0C[0x5C8 - 0x0C];
    unsigned char locked;
} RPAR;

extern RPAR gPar;

void PAR_ReserveMemory__Fi(int size) {
    int pages = (size + 0xFFF) / 0x1000;
    if (!gPar.locked) {
        if (gPar.pages < pages) {
            gPar.pages = pages;
        }
    }
}
