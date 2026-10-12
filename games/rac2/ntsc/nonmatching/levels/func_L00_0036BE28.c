/* near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Retail loads through $at with hand-placed nops and ends in jr ra + lui ra, 0x7000 (looks like hand asm); plain C gives $v0 and fewer nops. */
void func_L00_0036BE28(char *a0)
{
    while (*(volatile unsigned int *)(a0 - 0x3000) & 0x100)
        ;
}
