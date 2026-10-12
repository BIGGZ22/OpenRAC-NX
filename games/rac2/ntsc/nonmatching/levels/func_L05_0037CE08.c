/* func_L05_0037CE08 (176 o) - best attempt, not a match.
   near miss: 180 of 176 bytes. Same wall as func_L05_003D02C8: the retail reloads lui+lw of D_L05_001B251C before EACH of the two calls in the loop; cc1 hoists the lui into a callee-saved register (s3), which also costs the two extra frame saves (s4 + bigger frame).
   Verified with: python3 tools/check.py func_L05_0037CE08 --diff
*/

int func_L05_0037CE08(char *obj, int value)
{
    int base = D_L05_001F4DE0[(unsigned char)obj[0x21]];
    unsigned short *p;
    int count = 0;

    if (base == 0)
        return 0;
    p = (unsigned short *)base;
    do {
        int tbl = D_L05_001B251C;
        int off = ((int)*p & 0x7FFF) * 256;

        if (func_L05_0037CEB8((char *)(tbl + off)) == value) {
            if (func_L05_0037AFD0((char *)(tbl + off)) != 0)
                count++;
        }
    } while (*(short *)p++ >= 0);
    return count;
}
