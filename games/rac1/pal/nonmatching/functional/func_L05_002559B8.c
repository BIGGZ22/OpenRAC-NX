/* functional: medium. Picks a code from hero unk884 (zero or not) and unk8CE (nonzero or not): 0x52/0x7D when unk884 is 0, 0x68/0x7E otherwise. */
int func_L05_002559B8(void) {
    int r;
    int b = D_0013F450_h2523C8.unk8CE;
    if (D_0013F450_h2523C8.unk884 == 0) {
        r = 0x52;
        if (b != 0) {
            r = 0x7D;
        }
    } else {
        r = 0x68;
        if (b != 0) {
            r = 0x7E;
        }
    }
    return r;
}
