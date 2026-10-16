/* functional: medium. Finds the entry (of 5, 8 bytes each) whose id equals the argument, clears bit 2 of its flags and returns 0; returns 1 if none. */
int func_L00_00284EB8(int id) {
    int *p = D_L00_001BA548 + 1;
    int i = 0;
    int mask = -5;
    do {
        if (p[-1] == id) {
            p[0] &= mask;
            return 0;
        }
        i++;
        p += 2;
    } while (i < 5);
    return 1;
}
