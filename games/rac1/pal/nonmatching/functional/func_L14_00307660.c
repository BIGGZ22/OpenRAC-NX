/* functional: medium. State step on byte +0x20 of p (obj at +0x78 = q): state 0 -> q.b28=4, q.h3E=0, q.w20=0, q.h24=0, p.b20=1; state 2 -> q.h3E=5, p.b20=3; otherwise nothing. Includes the 0x307680 and 0x3076A4 pieces. */
void func_L14_00307660(char *p) {
    unsigned int k = (unsigned char)p[0x20];
    char *q;
    if (k == 0) {
        q = *(char **)(p + 0x78);
        q[0x28] = 4;
        *(short *)(q + 0x3E) = 0;
        *(int *)(q + 0x20) = 0;
        *(short *)(q + 0x24) = 0;
        p[0x20] = 1;
    } else if (k == 2) {
        q = *(char **)(p + 0x78);
        *(short *)(q + 0x3E) = 5;
        p[0x20] = 3;
    }
}
