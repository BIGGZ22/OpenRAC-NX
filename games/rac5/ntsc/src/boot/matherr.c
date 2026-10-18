/* matherr 0x01EA2E40..0x01EA2E68 */
extern int dpcmp(double a, double b);
int matherr(void *x) {
    double d = *(double *)((char *)x + 8);
    dpcmp(d, d);
    return 0;
}
