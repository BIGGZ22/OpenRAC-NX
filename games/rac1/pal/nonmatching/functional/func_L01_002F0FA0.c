/* functional: medium. Returns 0 if p is null or its word at +0x78 is zero, else that word + 0xC0. */
int func_L01_002F0FA0(char *p) {
    int v;
    if (p == 0) {
        return 0;
    }
    v = *(int *)(p + 0x78);
    if (v != 0) {
        return v + 0xC0;
    }
    return 0;
}
