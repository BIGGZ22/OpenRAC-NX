/* functional: high. level 3. -1 while the hero is in state 6 (the one of his state ids the level waits
 * out), or in a state other than 0x11 while the object id at +0x2284 of the hero block is 0x50;
 * 0 otherwise. Written from the retail instructions (56 bytes); not a match. */
int func_L03_002ED138(void) {
    char *hero = (char *)0x13F450;
    int state = *(int *)(hero + 0x2084);

    if (*(int *)(hero + 0x2284) == 0x50 && state != 0x11) {
        return -1;
    }
    if (state != 6) {
        return 0;
    }
    return -1;
}
