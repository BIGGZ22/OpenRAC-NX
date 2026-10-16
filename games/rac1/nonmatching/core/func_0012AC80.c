/* Stream/type key for type arg0 (0..9) and value arg1: the table entry
   D_00132FD8[arg0] holds a 64-bit value at +0 and a mask at +8; the
   value is shifted by 24 when the mask is 0xFFFF000000, by 32 when it
   is 0xFF00000000, and by 0 otherwise, then or-ed into the entry's
   value. Types 10 and up give 0. */
extern char D_00132FD8_tbl[] __asm__("D_00132FD8");

long func_0012AC80(int arg0, int arg1) {
    unsigned char *e;
    unsigned long val;
    unsigned long mask;
    unsigned long key;
    int shift;

    if ((unsigned int)arg0 >= 10) return 0;
    e = (unsigned char *)D_00132FD8_tbl + ((unsigned int)arg0 << 4);
    val = *(unsigned long *)e;
    mask = *(unsigned long *)(e + 8);
    if (mask == 0xFFFF000000UL) {
        shift = 24;
    } else if (mask == 0xFF00000000UL) {
        shift = 32;
    } else {
        shift = 0;
    }
    key = (unsigned long)(long)arg1 << shift;
    return (long)(val | key);
}
