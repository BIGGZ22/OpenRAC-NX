/* func_L01_003BDB28 */
void func_L01_003BDB28(L01_003BDB28_OBJ *a0)
{
    L01_003BDB28_OWNED *q = a0->f68;
    unsigned int v = a0->f21;

    if (v != 255) {
        func_002A8598(v, 1);
    }
    if (q->f280 != -1) {
        func_002A8598(q->f280, 1);
    }
    if (q->f284 != -1) {
        func_002A8598(q->f280, 1);
    }
    a0->fBC = 1;
}
