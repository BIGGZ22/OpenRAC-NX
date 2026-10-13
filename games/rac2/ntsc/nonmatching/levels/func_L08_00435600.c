/* DRAFT (non matching) - func_L08_00435600, 160 bytes, 3 places
   from src/levels/shared/L08_003D3928.c - belongs in the -g8 sibling
   (L08_003D3928-g8.c): its retail reads level data through $gp in ordinary
   flow (`lwc1 $f1,0x2FA4($gp)`), which needs .extern (sda + -G8).

   last check.py line:
     08_tabora: instruction at +0x3c differs
     func_L08_00435600: 0 of 3 places match

   Everything matches except one adjacent pair of stores: the retail emits
   `sb $v1,0x30($s0)` then `sh $a0,0x32($s0)`; this attempt emits the two
   swapped.  The two `addiu ..,0xFF` materialisations are already in the
   retail's order, so only the scheduler's choice of the two independent
   stores is off.  Tried source orders [0x31,0x30,0x32,0x20] (this one),
   [0x31,0x32,0x30,0x20] (156 bytes: folds the two 0xFF into one register),
   [0x20,0x31,0x30,0x32]. */
typedef int TI128 __attribute__((mode(TI)));

extern float D_L08_001B1F94 __attribute__((sda));
extern void *func_0029EFD0(s32);
extern void func_002A0F48(void *);

void *func_L08_00435600(u8 *a0, void *a1)
{
    u8 *s0 = func_0029EFD0(0x998);
    u32 *p;

    if (s0 != 0) {
        s0[0x31] = 1;
        s0[0x30] = 0xFF;
        *(u16 *)(s0 + 0x32) = 0xFF;
        s0[0x20] = 0;
        *(float *)(s0 + 0x2C) = *(float *)(*(u32 *)(s0 + 0x24) + 0x24) * D_L08_001B1F94;
        p = *(u32 **)(s0 + 0x68);
        p[4] = (u32)a0;
        qcopy(p, a1);
        qcopy(s0 + 0x10, a1);
        p[5] = 0;
        func_002A0F48(s0);
    }
    return s0;
}
