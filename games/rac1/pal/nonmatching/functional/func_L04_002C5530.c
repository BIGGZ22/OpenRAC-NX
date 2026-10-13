/* functional: medium. Returns the pointer a->+0x78 plus 0x180 (the sub-block inside it), or 0 if a is null or the pointer is null. */
char *func_L04_002C5530(char *a) {
    char *p;
    if (a == 0) return 0;
    p = *(char **)(a + 0x78);
    if (p != 0) return p + 0x180;
    return 0;
}
