/* functional: high. level 11. 1 for a moby in state 0xB, 0xC, 0x12 or 0x14, else 0. Written from the
 * retail instructions (48 bytes); not a match. */
int func_L11_00317598(char *m) {
    int state = *(unsigned char *)(m + 0x20);

    return state == 0x14 || state == 0xB || state == 0x12 || state == 0xC;
}
