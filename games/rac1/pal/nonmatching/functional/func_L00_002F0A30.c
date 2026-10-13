/* functional: medium. s[0] start, s[1] base, s[2] count: returns 0 if a equals the end address ((count*16 + base + 16) & 0xFFFFFFF), else (a - s[0]) >> 11 as sector number. */
unsigned int func_L00_002F0A30(unsigned int *s, unsigned int a) {
    unsigned int end = ((s[2] << 4) + s[1] + 0x10) & 0xFFFFFFF;
    if (a == end) {
        return 0;
    }
    return (a - s[0]) >> 11;
}
