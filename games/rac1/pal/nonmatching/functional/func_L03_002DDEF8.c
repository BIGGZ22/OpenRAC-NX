/* functional: high. Exact logic of retail func_L03_002DDEF8 in l03_kerwan. */
extern int func_L03_002DDF10(unsigned char *moby, char *data);

int func_L03_002DDEF8(unsigned char *moby) {
    if (*(short *)(moby + 0xA6) == 0x3F4) {
        return func_L03_002DDF10(moby, *(char **)(moby + 0x78));
    }
    return 0;
}
