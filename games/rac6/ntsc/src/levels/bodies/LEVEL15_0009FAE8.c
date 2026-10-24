/* LEVEL15_0009FAE8 -- LEVEL15.REL 0x0009FAE8, 192 bytes.
 *
 * Placement: module-relative offset of the disc module; the
 * other placements of the same body are listed in
 * config/levels/bodies.tsv.
 * Proof: the pinned chain compiles this source to the retail
 * bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 */


extern void func_09F4C0(unsigned long long handle, int index);

extern int D_25126C;
extern unsigned int D_251248[5];
extern int D_250DC0;
extern int D_250EA8;
extern int D_250F90;
extern int D_251078;
extern int D_251160;

void SCRNOPTIONS_SetMenuDataHackery__Fv(void)
{
    unsigned int *h0 = &D_250DC0;
    unsigned int *h1 = &D_250EA8;
    unsigned int *h2 = &D_250F90;
    unsigned int *h3 = &D_251078;
    unsigned int *h4 = &D_251160;

    D_25126C = 5;
    D_251248[0] = (unsigned int)h0;
    D_251248[1] = (unsigned int)h1;
    D_251248[2] = (unsigned int)h2;
    D_251248[3] = (unsigned int)h3;
    D_251248[4] = (unsigned int)h4;
    func_09F4C0((unsigned long long)h0, 0);
    func_09F4C0((unsigned long long)h1, 1);
    func_09F4C0((unsigned long long)h2, 2);
    func_09F4C0((unsigned long long)h3, 3);
    func_09F4C0((unsigned long long)h4, 4);
}
