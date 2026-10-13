/* DRAFT (non matching) - func_L08_003D4600, 144 bytes, 3 places
   from src/levels/shared/L08_003D3928.c

   last check.py line:
     08_tabora: instruction at +0x2c differs
     func_L08_003D4600: 0 of 3 places match

   Size and structure are right (36 words); two things are off.
   +0x2c is the delay slot of the `beqz $s0`: the retail fills it with the
   qcopy destination (`addiu $a2,$s0,0x10`) where this attempt has the
   constant 1.  The retail store order inside the guard is
   sb 0x30 / sb 0x31 / sh 0x32 / sb 0x20; this attempt emits
   sb 0x31 / sb 0x30 / sb 0x20 / sh 0x32 from the source order below, and
   neither the source order nor its inverse rotation changed it.
   Tried: early `return s0` instead of the guard (152 bytes), source orders
   [0x30,0x31,0x32,0x20], [0x20,0x30,0x31,0x32], [0x31,0x30,0x20,0x32],
   TI128 stores instead of qcopy. */
extern void *func_0029EFD0(s32);
extern void func_002A0F48(void *);

void *func_L08_003D4600(u8 *a0, void *a1)
{
    u8 *s0 = func_0029EFD0(0x9A1);

    if (s0 != 0) {
        s0[0x20] = 0;
        s0[0x30] = a0[0x30];
        s0[0x31] = 1;
        *(u16 *)(s0 + 0x32) = *(u16 *)(a0 + 0x32);
        *(u32 *)(*(u32 *)(s0 + 0x68)) = (u32)a0;
        qcopy(s0 + 0x10, a1);
        qcopy(s0 + 0xF0, a0 + 0xF0);
        func_002A0F48(s0);
    }
    return s0;
}
