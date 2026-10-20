/* 0 of 4 places match - instruction at +0x14 differs; candidate is 60 bytes, exact size.
   The loop itself is exact: five iterations of `sw $zero,0($v1); addiu $v0,$v0,-1; sw $zero,0($a0);
   addiu $v1,$v1,4; nop; bgez $v0,loop; addiu $a0,$a0,4`, with both pointers starting at their bases
   and the counter counting 4 down to -1. What differs is the register allocation:
   retail puts the counter in $v0 and the two blocks in $a0 (0x1B2FA0) and $v1 (0x1B3018) and stores
   the $gp flag first; this cc1 gives the counter $a0, keeps the blocks in $v0/$v1 and materialises
   both block bases before the flag store (a v0-v1-a0 rotation of the same three registers).
   Measured in the -g8 unit (L05_0037B6D0-g8.c) with `extern int D_L05_001AFBF8
   __attribute__((sda));`: the 60-byte size and the $gp store are then exact, only the rotation is left.
   Tried: counter declared before/after the pointers, do/while against for, both store orders,
   array indexing, ascending index with `4 - i`, and counter/decrement positioned as retail's. */

void func_L05_003D1E20(void)
{
    int i = 4;
    int *q = D_L05_001B2FA0;
    int *p = D_L05_001B3018;

    D_L05_001AFBF8 = 0;
    do {
        *p = 0;
        *q = 0;
        p++;
        q++;
        i--;
    } while (i >= 0);
}
