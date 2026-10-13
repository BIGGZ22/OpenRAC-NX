/* functional: medium. Returns 0 if p is null or its word at +0x78 is zero, else that word + 0xD0. */
int func_L00_002D6CB0(char *p) {
    int v;
    if (p == 0) {
        return 0;
    }
    v = *(int *)(p + 0x78);
    if (v != 0) {
        return v + 0xD0;
    }
    return 0;
}
