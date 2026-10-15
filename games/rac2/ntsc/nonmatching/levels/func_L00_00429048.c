/* best attempt (-g8 sibling) -- last check.py line: "00_aranos_tutorial: 116 bytes, expected 120"
   The whole structure (bne, bnel, bc1tl, xori/sltiu) is in place; what is left is the
   %hi/%lo pair of D_L00_001B8E10 (retail: lui $v0,%hi / lw $v1,%lo($v0), cc1: single
   register) and one store missing. gp = 0x1AEFF0: 0x2854($gp) = D_L00_001B1844, 0x18E0
   itself being 0x001B18E0. */
int func_L00_00429048(int index)
{
    char *y;
    short s;

    if (*(short *)(D_L00_001B8E10 + 0x86) == 0x13) {
        y = (char *)(*(int *)(*(int *)(*(char **)(D_L00_001B1844 + index * 32 + 0x1C) + 4) + 0x70) + 0x40);
        s = *(short *)(y + 0x14);
        if (s != 1 || D_L00_001B18E0 != 0.0f) {
            return (s ^ 3) == 0;
        }
    }
    return 1;
}
