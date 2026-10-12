/* func_L05_003D02C8 (88 o) - best attempt, not a match.
   near miss: 84 of 88 bytes. The retail loads D_L05_001B251C with lui+lw INSIDE the loop; cc1 loop-invariant-motion hoists the lui (%hi of the symbol) into a2 and the loop loses one instruction, so the 8-byte loop-alignment nop before the loop label does not appear.
   Verified with: python3 tools/check.py func_L05_003D02C8 --diff
*/

void func_L05_003D02C8(char *obj)
{
    int base = D_L05_001F4DE0[(unsigned char)obj[0x21]];
    unsigned short *p;

    if (base == 0)
        return;
    p = (unsigned short *)base;
    do {
        int tbl = D_L05_001B251C;

        *(char *)(tbl + ((int)*p & 0x7FFF) * 256 + 0x20) = 1;
    } while (*(short *)p++ >= 0);
}
