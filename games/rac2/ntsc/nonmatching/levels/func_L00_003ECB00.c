/* DRAFT w49 — best attempt, DOES NOT MATCH: 52 bytes expected, 52 bytes produced,
   only the FIRST TWO instructions are swapped.
   last check.py line: func_L00_003ECB00: 0 of 27 places match
   00_aranos_tutorial: instruction at +0x0 differs
   retail  : addiu $v1,$zero,0x3 / lui $v0,0x1B / addiu $v0,$v0,0x7A4
   produced: lui $v0,0x1B / addiu $v1,$zero,0x3 / ori $v0,$v0,0x7A4

   The TWIN of src/levels/shared/L00_003F8E08.c (func_L00_003FF7B8, 0x003FF7B8,
   matched) has exactly the same descending loop `*w = c; w--; c -= 64;` and
   does emit the counter FIRST (addiu $a0,$zero,0x3 then the pointer): that is
   the source form copied here. The difference comes from the absolute address:
   the twin initialises its pointer from a register (addiu $v0,$s1,0x2C), here
   it needs lui+addiu on a constant, and cc1 hoists that pair before the
   counter initialisation whatever the order of the source assignments.
   Tried without success: plain assignments then `for (; i >= 0; i--)`,
   declaration initialisers, do/while form, reversed declaration order,
   intermediate variable `base`. */
void func_L00_003ECB00(void)
{
    int i;
    int *p;

    i = 3;
    p = (int *)0x1B07A4;
    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
