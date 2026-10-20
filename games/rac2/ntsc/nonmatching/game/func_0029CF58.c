/* from src/game/0029BCC8.c: 32-byte near miss; the candidate is 36 bytes and
   two instructions differ.  Retail materialises the register block in $at
   (lui $at,0x1000 ; ori $at,$at,0xD400) and keeps the constant 0x100 in $v0
   (ori $v0,$zero,0x100) until the last store, which sits in the jr $ra delay
   slot; this cc1 builds the address in $v0 (lui ; ori), materialises 0x100 as
   addiu $v1,$zero,0x100 before the address, and leaves a nop in the delay
   slot.  The address is the PS2 SPR block (0x1000D400) and it is the same in
   every program, so no level address is involved. */
void func_0029CF58(unsigned int a0, unsigned int a1, unsigned int a2) {
    volatile unsigned int *p = (volatile unsigned int *)0x1000D400;
    p[0x20] = a2;
    p[0x8] = a1;
    p[0x4] = a0;
    p[0] = 0x100u;
}
