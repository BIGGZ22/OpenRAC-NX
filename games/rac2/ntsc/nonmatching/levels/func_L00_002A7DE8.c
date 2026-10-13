/* 0 of 27 places match - last line: 30_insomniac_museum: instruction at +0x30 differs
  33 of 36 instructions. Three `addu` put the scaled index first where retail puts the base first
  (`addu $a2,$s0,$a2` vs `addu $a2,$a2,$s0`). The 128-bit copy (qcopy_nc + a `ring` pointer) is exact. */

void func_L00_002A7DE8(void *src, int arg1, float f12, float f13)
{
    char *base = D_00189E20_2;

    char *ring = base + 0x2080;

    qcopy_nc(ring + *(short *)(base + 0x2160) * 16, src);
    *(int *)(base + 0x2100 + (*(short *)(base + 0x2160) << 2)) = arg1;
    *(float *)(base + 0x2120 + (*(short *)(base + 0x2160) << 2)) = f12;
    *(float *)(base + 0x2140 + (*(short *)(base + 0x2160) << 2)) = f13;
    func_L00_002A7A10();
    *(unsigned short *)(base + 0x2160) = *(unsigned short *)(base + 0x2160) + 1;
}
