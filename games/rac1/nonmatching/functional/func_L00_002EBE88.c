/* functional: high. No C existed for this function; written from asm/overlays/func_L00_002EBE88.s. Copies the 16-byte quadword at its argument to offset 0x80 of the data pointed to by the record at offset 0x70 of the moby D_L00_00166F00 points at, or of the record func_L00_001EB578(5) returns when that moby's 0x86 halfword is not 5 (the asm calls func_L00_001EB578 then, and uses its result as the base). */
extern char *D_L00_00166F00;
extern char *func_L00_001EB578(int);

void func_L00_002EBE88(void *src) {
    char *p = D_L00_00166F00;
    char *base = (*(short *)(p + 0x86) != 5) ? func_L00_001EB578(5) : p;
    char *dst = *(char **)(base + 0x70) + 0x80;
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = ((char *)src)[i];
    }
}
