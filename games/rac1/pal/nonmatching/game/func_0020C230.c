/* DmaToSprSync(void): waits while channel 8's CHCR has STR (0x100) set. */
void func_0020C230(void)
{
    /* fix: volatile, so the register is re-read every iteration as retail does (0020C238 lw in the loop) */
    while (*(volatile unsigned int *)0x1000D400 & 0x100) {
    }
}
