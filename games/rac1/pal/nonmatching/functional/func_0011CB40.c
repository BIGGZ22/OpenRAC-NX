/* functional: high. Retries func_0011B2F8(D_00158140, 0x80000003, 0) while it returns >= 0 and the client word at +0x24 is zero, spinning 0x100000 steps between tries (a delay loop; kept so timing matches). Returns -1 on error; otherwise clears D_0012FDAC and returns 0. */
extern int func_0011B2F8(void *, int, int);
extern char D_00158140[];
extern int D_0012FDAC;

int func_0011CB40(void) {
    int k;
    int r = func_0011B2F8(D_00158140, 0x80000003, 0);

    while (r >= 0 && *(int *)(D_00158140 + 0x24) == 0) {
        for (k = 0x100000; k != -1; k--) {
        }
        r = func_0011B2F8(D_00158140, 0x80000003, 0);
    }
    if (r < 0) {
        return -1;
    }
    D_0012FDAC = 0;
    return 0;
}
