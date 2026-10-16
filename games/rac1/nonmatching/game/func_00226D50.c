/* Fills the pause menu's table D_001D6448 (pairs of words) from the state at
   D_001D5F70: the first n entries get the word at +0x108 stepping by 0x11800,
   the next run gets the word at +0x10C stepping by 0x11800, the run after that
   gets the +0x108 word stepping by 0x4F000 with second word 1, and entries up to
   index 4 are cleared. n is 1 for a0 == 0 and 2 otherwise. Each run leaves the
   running index at its end, which is where the next run starts. */
extern int D_001D6448[];

void func_00226D50(int a0) {
    char *g = D_001D5F70;
    unsigned int v8 = *(unsigned int *)(g + 0x10C);
    unsigned int v7 = *(unsigned int *)(g + 0x108);
    int n1 = (a0 != 0) ? 2 : 1;
    int f2 = (a0 != 0) ? 1 : 0;
    int t10 = (a0 != 0) ? 2 : 0;
    int n9 = n1 + f2;
    int end2;
    int s = 0;
    int k;

    if (n1 > 0) {
        for (k = 0; k < n1; k++) {
            D_001D6448[2 * k] = (int)v7;
            D_001D6448[2 * k + 1] = 0;
            v7 += 0x11800;
        }
        s = n1;
    }

    if (s < n9) {
        for (k = s; k < n9; k++) {
            D_001D6448[2 * k] = (int)v8;
            D_001D6448[2 * k + 1] = 0;
            v8 += 0x11800;
        }
        s = n9;
    }

    end2 = n9 + t10;
    if (s < end2) {
        for (k = s; k < end2; k++) {
            D_001D6448[2 * k] = (int)v7;
            D_001D6448[2 * k + 1] = 1;
            v7 += 0x4F000;
        }
        s = end2;
    }

    if (s < 5) {
        for (k = s; k < 5; k++) {
            D_001D6448[2 * k] = 0;
            D_001D6448[2 * k + 1] = 0;
        }
    }
}
