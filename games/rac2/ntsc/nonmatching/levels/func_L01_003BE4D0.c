/* func_L01_003BE4D0 */
void func_L01_003BE4D0(L01_003BE4D0_OBJ *a0)
{
    L01_003BE4D0_OBJ *p = a0;
    L01_003BE4D0_OWNED *q = p->f68;
    unsigned int x, y, z, c, mode;
    int r;

    if (p->fBC != 0) {
        p->fBC = 0;
        if (q->f238 < 200) {
            q->f238 = func_002A7820(300, 360);
        }
    }
    if (q->f114 != 1) {
        func_00282948((char *)q + 0x238);
    }
    if (p->f96 <= 0xFFFD) {
        p->f96 = p->f96 + 1;
    }
    func_00282948((char *)q + 0x230);
    func_00282948((char *)q + 0x23C);
    func_002A04B8(p, &x, &y, &z);
    c = 0x80000000u | (y << 8) | (z << 16) | x;
    if (q->f290 != 0 && p->f20 < 9) {
        mode = 0x8040E0F0;
        r = func_002E4600(c, mode, 0.05f);
    } else {
        mode = 0x00404040;
        r = func_002E4600(c, mode, 0.08f);
    }
    x = r & 0xFF;
    y = (r >> 8) & 0xFF;
    z = (r >> 16) & 0xFF;
    func_002A0488(p, x, y, z);
}
