/* near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Same wait-loop family as func_L00_0036BE28: retail loads through $at, hand-placed nops, jr ra then lui ra, 0x7000. */
void func_L00_0036C9AC(char *a0)
{
    while (*(volatile unsigned int *)(a0 - 0x2C00) & 0x100)
        ;
}
