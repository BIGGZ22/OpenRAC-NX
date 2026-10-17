/* functional: high. No C existed for this function; written from asm/overlays/func_L00_002EBEE0.s. Same as func_L00_002EBE88 but copies the 16-byte quadword to offset 0x90 instead of 0x80 of the data at offset 0x70 of the moby D_L00_00166F00 points at (or of func_L00_001EB578(5)'s record when that moby's 0x86 halfword is not 5). */
extern char *D_L00_00166F00;
extern char *func_L00_001EB578(int);

void func_L00_002EBEE0(void *src) {
    char *p = D_L00_00166F00;
    char *base = (*(short *)(p + 0x86) != 5) ? func_L00_001EB578(5) : p;
    char *dst = *(char **)(base + 0x70) + 0x90;
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = ((char *)src)[i];
    }
}
