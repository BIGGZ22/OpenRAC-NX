/* 0 of 27 places match - candidate is 64 bytes against 72.
   The threshold chain is right (value >= 90 -> 4, > 60 -> 3, > 30 -> 2, else 1) and retail keeps the
   result in $a0 with the default 1 stored in the first branch's delay slot. Two codegen differences:
   this cc1 folds the comparisons to `slti` (`value < 0x3D`, `value < 0x1F`) where retail materialises
   60 and 30 in $v0 and uses `slt $v0,$v0,$v1`; and the innermost arm becomes `movz $a0,$v0,$v1` here
   against retail's `slt $v0,$v0,$v1; addiu $v1,$zero,0x2; movn $a0,$v1,$v0`.
   Tried: early returns per threshold, `if/else if` with a result variable initialised to 1, nested
   compares against literals. The row is a leaf: `lw $v1,0x68($a0); lh $v1,0x1A($v1)`. */

int func_L00_00410840(char *object)
{
    int value = *(short *)(*(int *)(object + 0x68) + 0x1A);
    int result = 1;

    if (value >= 0x5A)
        result = 4;
    else if (value > 0x3C)
        result = 3;
    else if (value > 0x1E)
        result = 2;
    return result;
}
