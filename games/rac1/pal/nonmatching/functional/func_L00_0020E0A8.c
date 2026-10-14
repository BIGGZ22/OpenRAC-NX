/* functional: medium. Maps a small id to a text/sound id: 1 -> 0x79, 2 -> 0x77, 3 -> 0x75, otherwise 0. */
int func_L00_0020E0A8(int k) {
    if (k == 2) {
        return 0x77;
    }
    if (k == 1) {
        return 0x79;
    }
    if (k == 3) {
        return 0x75;
    }
    return 0;
}
