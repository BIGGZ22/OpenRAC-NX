/* functional: high. Written from the retail instructions (32 bytes); not a match. */
/* Level 3: 1 for a class 0x330 moby in state 2 to 6, else 0. */
int func_L03_002D44A0(unsigned char *m) {
    if (*(short *)(m + 0xA6) != 0x330) {
        return 0;
    }
    return (unsigned int)(m[0x20] - 2) < 5;
}
