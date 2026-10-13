/* functional: medium. Returns the int at +0x14 of the block a points to at +0x78, or 0 if a or that block is null. */
int func_L00_002CC390(char *a) {
    char *p;
    if (a == 0) return 0;
    p = *(char **)(a + 0x78);
    if (p == 0) return 0;
    return *(int *)(p + 0x14);
}
