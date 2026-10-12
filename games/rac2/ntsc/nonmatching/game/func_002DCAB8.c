/* From src/game/002D5E98.c: links at 26 of 28 places (instruction at +0x38 differs). */
/* Initialises an object: runs setup helpers, clears word +0x4C, stores one helper result at +0x48, then calls a final helper with (small-data global + 0x3CAA8) only when two globals are non-zero, and returns 0. */
int func_002DCAB8(char *a0)
{
    int v;

    func_002DE3A0(1);
    v = func_002DE550_1(0);
    *(int *)(a0 + 76) = 0;
    *(int *)(a0 + 72) = v;
    func_002C9A60();
    func_L00_002F3478();
    v = *(int *)0x1A8FD8;
    if (v != 0) {
        v = D_L00_001A904C_8;
        if (v != 0) {
            func_00339760(v + 0x3CAA8);
        }
    }
    return 0;
}
