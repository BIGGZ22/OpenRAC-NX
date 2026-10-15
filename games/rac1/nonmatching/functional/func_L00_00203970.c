/* functional: medium. If a > 0x20000 tail-calls func_L00_002039C4 (clears *p and *q, returns -1); else *p = D_173F00[1] + D_16128C - a, *q = D_173F00[2] + D_16128C - a, returns 0. */
extern int D_L00_00173F00_03970[] __asm__("D_L00_00173F00");
extern int D_L00_0016128C_03970 __asm__("D_L00_0016128C");
int func_L00_002039C4(int a, int b, int *c, int d, int *e);
int func_L00_00203970(int a, int *p, int *q) {
    if ((unsigned long)a > 0x20000) {
        return func_L00_002039C4(a, 0, q, 0, p);
    }
    *p = D_L00_00173F00_03970[1] + D_L00_0016128C_03970 - a;
    *q = D_L00_00173F00_03970[2] + D_L00_0016128C_03970 - a;
    return 0;
}
