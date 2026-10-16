/* The moby of hero item slot i: the slot's 0x50-byte record in the hero block (D_0013F450 +
   i * 0x50) holds its state at +0x10B4 and its moby at +0x1090; 0 unless the state is 2. Retail
   falls through into the next catalogue unit (func_L00_0020DB1C, the +0x1090 load) for the moby. */
extern char D_0013E633_DAF8[] __asm__("D_0013E633");

char *func_L00_0020DAF8(int i) {
    char *rec = D_0013E633_DAF8 + 0xE1D + i * 0x50;

    if (*(int *)(rec + 0x10B4) != 2) {
        return 0;
    }
    return *(char **)(rec + 0x1090);
}
