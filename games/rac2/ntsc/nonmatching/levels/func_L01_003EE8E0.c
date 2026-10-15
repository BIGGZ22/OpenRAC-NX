/* w13 ABANDON — func_L01_003EE8E0, 112 bytes, 7 places.
 * last check.py line (3rd compilation):
 *   func_L01_003EE8E0: 0 of 7 places missing -> "instruction at +0x1c differs"
 *   (116 bytes produced against 112 expected)
 * The whole structure fits: the three beql/bne with `addiu $v0,$zero,0xFF`
 * in the delay slots, the call, the final store `sb $v0,0xA8($s0)` and the
 * 0xFF return do share $v0. ONE instruction too many is left:
 *   retail : addiu $v0,$0,0xFF ; ld $ra ; sb $v0,0xA8($s0) ; ld $s0 ; jr
 *   here   : addiu $v0,$0,0xFF ; sb $v0,0xA8($s0) ; ld $s0 ; addiu $v0,$0,0xFF(!!) ; ld $ra ; jr
 * gcc rematerialises the constant for the return instead of reusing $v0.
 * Also tried: literal `return 0xFF;` (gives $v1 for the store, 4
 * materialisations), and a single `r = 0xFF; return r;`.
 */
extern char *func_002A9640(char *, int, int);

extern void func_002A8598(int, int);

int func_L01_003EE8E0(char *a0)
{
    char *s0 = a0;
    char *p = func_002A9640(a0, 0x10000, 0);
    int r;

    if (p == 0) {
        r = 0xFF;
    } else {
        p = *(char **)(p + 0x20);
        if (p == 0) {
            r = 0xFF;
        } else if (*(short *)(p + 0xAA) != 0x825) {
            r = 0xFF;
        } else {
            if (*(unsigned char *)(s0 + 0x21) != 0xFF)
                func_002A8598(*(unsigned char *)(s0 + 0x21), 2);
            r = 0xFF;
        }
    }
    *(unsigned char *)(s0 + 0xA8) = r;
    return r;
}
