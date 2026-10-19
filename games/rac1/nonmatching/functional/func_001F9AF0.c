/* functional: high. fcheck: unchecked (the harness could not compile it in place; OpenRAC tools/fcheck_gate.py, 2026-10-18). Corrected from nonmatching/game/func_001F9AF0.c: the near miss wrote the control register (offset 0x00, 0x100 = start) before the count register (0x20); retail writes 0x80, 0x10, 0x20 and only then 0x00, so the transfer starts with the count already set. Registers and the status word are also accessed as volatile. */
void func_001F9AF0(unsigned int a0, unsigned int a1, unsigned int a2)
{
    volatile unsigned int *ch = (volatile unsigned int *)0x1000D000;
    unsigned int status;

    ch[0x80 / 4] = a1;
    ch[0x10 / 4] = a0;
    ch[0x20 / 4] = a2;
    ch[0x00 / 4] = 0x100;
    status = *(volatile unsigned int *)0x20100000;
    status |= 1;
}
