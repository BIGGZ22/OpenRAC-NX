/* best attempt -- last check.py line: "00_aranos_tutorial: instruction at +0x8 differs"
   Everything is exact except the %hi/%lo pair of D_L00_001B1844 (single register in retail).
   The order of the two stores is already the retail one. */
int func_L00_0042A460(char *a0)
{
    char *p = *(char **)(D_L00_001B1844 + *(short *)(a0 + 0x84) * 32 + 0x1C);

    if (*(short *)(p + 0x26) != 0) {
        return 0;
    }
    if (D_L00_001A8F00 != 0) {
        if (func_L00_00429CE8(a0) != 0) {
            *(short *)(p + 0x26) = 0;
            *(int *)(p + 0x20) = 0;
            return 0;
        }
    }
    func_L00_00429F68(a0);
    return 0;
}
