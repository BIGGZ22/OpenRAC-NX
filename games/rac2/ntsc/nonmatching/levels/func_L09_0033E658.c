/* attempt kept for the next worker - func_L09_0033E658
   closest attempt: layout is right, but gcc puts the -1 constant in the annulled
   delay slot (beql) where retail keeps a plain beqz + nop, and swaps lui/sll in the first delay slot.
   verify with: tools/check.py func_L09_0033E658 --diff
*/
extern unsigned int D_L09_001FE6C0[];

int func_L09_0033E658(int index)
{
    unsigned int entry;

    if (index == -1)
        return -1;
    entry = D_L09_001FE6C0[index];
    if (entry == 0)
        return -1;
    return *(int *)entry - 1;
}
