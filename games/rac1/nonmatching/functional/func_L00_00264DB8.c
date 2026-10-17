/* functional: high. Corrected from the void declarations at the callers: retail returns v0 from its last call, func_001166FC, which is a string copy that returns its destination. The text func_001FE540 gives for the id is copied into the buffer at D_L00_00179218 and that buffer's address is returned. A timeout of -1 becomes func_001F9850(180), and the timeout is stored to D_L00_0015F720 after the copy. */
extern char *func_001FE540(int);
extern int func_001F9850(int);
extern char D_L00_00179218[];
extern int D_L00_0015F720;

char *func_L00_00264DB8(int a0, int a1) {
    char *src;
    char *s;
    char *d;

    if (a1 == -1) {
        a1 = func_001F9850(180);
    }
    src = func_001FE540(a0);
    d = D_L00_00179218;
    s = src;
    for (;;) {
        char c = *s;
        *d = c;
        d++;
        s++;
        if (c == 0) {
            break;
        }
    }
    D_L00_0015F720 = a1;
    return D_L00_00179218;
}
