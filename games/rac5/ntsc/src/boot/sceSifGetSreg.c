extern unsigned int g_sceSifSreg[]; /* 0x01FE4040 */

unsigned int sceSifGetSreg(int sreg) {
    return g_sceSifSreg[sreg];
}
