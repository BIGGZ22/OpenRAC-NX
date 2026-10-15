/* best attempt -- last check.py line: "00_aranos_tutorial: %hi at +0x38 differs"
   The seven stores are exact (the source order must be 0,2,1,3,4,5,6: cc1 applies a
   rotation that sends the last store to the front and the second-to-last into the jal slot).
   Only the %hi/%lo pair of D_L00_001B1844 is left. */
void func_L00_00426680(char *a0)
{
    int *p = *(int **)(a0 + 0x70);
    char *e;

    p[0] = 0;
    p[2] = 0;
    p[1] = 0;
    p[3] = 0;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0;
    func_L00_00426560(a0);
    e = *(char **)(D_L00_001B1844 + *(short *)(a0 + 0x84) * 32 + 0x1C);
    *(short *)(e + 0x26) = 0;
    *(short *)(a0 + 0x7E) = 0;
}
