/* from src/levels/shared/L00_002FC7A0.c: C that compiles but does not link at its places yet */
/* Write state value 3 when bit 0x40 is clear in the absolute flag word; preserve the existing value otherwise. */
void func_L00_00309AE8(void)
{
    if ((D_001A7424 & 0x40u) == 0)
        D_001A7420_1 = 3;
}
