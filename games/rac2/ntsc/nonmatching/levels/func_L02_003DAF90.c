/* func_L02_003DAF90, 156 bytes, 1 placement (level 2, 02_maktar_nebula).
   ABANDONED, not compiled: three of the arguments come straight out of the small-data area as
   `addiu $a1,$gp,0x1450`, `lwc1 $f12,0x1448($gp)` and `lwc1 $f13,0x1444($gp)`, so the body
   needs the -G8 sibling unit of src/levels/02_maktar_nebula/L02_003D26B0.c and that unit's
   whole small-data block reproduced (the three offsets are ~5 kB in, so they depend on every
   preceding `sda` object in the unit).  That is its own piece of work, not a spelling of this
   function.
   Everything else is decoded and measured:

     frame 0x90; s0 -> 0x70, s1 -> 0x78, s2 -> 0x80, ra -> 0x88
     v0 = *(u32 *)(a0 + 0xB8);
     func_315618(a0, 4, sp);                    // fills 16 bytes at sp+0x00
     *(TI128 *)(sp + 0x10) = *(TI128 *)sp;      // lq $v0,0x0($sp); sq $v0,0x0($v1), v1 = sp+0x10
     s2 = *(u8 **)(v0 + 0x68);                  // loaded in the first jal's delay slot
     func_315578(a0, 4, sp + 0x20);
     func_2F2478(sp + 0x60, <$gp 0x1450>, sp + 0x20);
     func_2F1F70(sp, sp, sp + 0x60);
     func_32A8E8(sp, *(u32 *)(s2 + 0x250), <$gp 0x1448>, <$gp 0x1444>);

   The two float arguments are loaded in the last call's two delay slots, and the 16-byte copy
   reaches lq/sq through `typedef int TI128 __attribute__((mode(TI)));` (recipe measured in
   src/levels/shared/L08_003D3928.c).
   To get those three `$gp` references the symbol must be declared with the assembler name
   FIRST and the section attribute after it, e.g.
     extern int D_L02_001B0248 __asm__("D_L02_001B0248_alias") __attribute__((section(".sdata")));
   which leaves the reference naked in the .s (`lw $2,D_L02_001B0248_alias`) and so emits
   GPREL16 - verified with the lead's probe (gpprobe2.c).  The remaining unknown is the
   level's $gp base: the offsets 0x1444/0x1448/0x1450 fix each symbol's distance from it,
   so the three addresses have to be chosen against the base the build already uses for
   level 2 (readable from the -G8 unit src/levels/02_maktar_nebula/L02_003D26B0-g8.c,
   whose `sda` symbols already produce matching offsets).
   No C is deposited: without the $gp block a body only reproduces the shape, not the words. */

#include "include_asm.h"
