/* func_L00_002B3D10 (172 bytes, 22 places) -- NEAR MISS, not integrated.
   state: 168 bytes / 42 instructions; only the first branch is wrong.
   check.py (00_aranos_tutorial): 168 bytes, expected 172
     002B3D40  bc1f  . + 4 + (0x9 << 2)   |  bc1fl . + 4 + (0xB << 2)
     002B3D44  lwc1  $f2, 0x0($gp)        |  lw    $v1, 0x2294($a0)      <- delay slot
     ... every other instruction is byte-identical (relocation sites included) ...

   What is right: the two call arguments, both float constants (1.0f, 1/60, 0.8f, 1/24), the
   compare operand order (c.olt.s $f1,$f0 with $f1 = 0.0f, i.e. the source spells `0.0f < x`),
   the multiply order (global * (f24B8 * C)), the f1B0 loads/stores, the state compare and
   the epilogue (ld $ra hoisted into the bne delay slot).

   What is missing: retail lets dbr fill the bc1f slot from the *branch target* (the
   `lw $v1,0x2294($a0)` block head, duplicated so the target keeps its copy at 0x2B3D6C) and
   therefore emits the branch-likely form `bc1fl`. Our compiler fills the slot from the
   fall-through instead and keeps `bc1f`, one instruction short. Tried and not enough:
   - `__attribute__((sda))` on D_001A7DC0  -> gas picks the one-instruction `lwc1 $f1,0($gp)`
     form (retail uses `lui $at,0x1A ; lwc1 $f1,0x7DC0($at)`, i.e. the plain declaration),
   - writing the product as `f24B8 * C * global` -> changes the FPR allocation of both muls,
   - plain two-statement shape without the sda attribute -> same 168 bytes, `lui $v0` in the
     delay slot instead of the target load.
   Next idea: nothing found that changes dbr's thread choice; this looks like a scheduler wall.

   Declarations used (struct D00189E20_Overlay needs i2294 at 0x2294 and f24B8 at 0x24B8,
   carved out of the existing pads):
       extern void  func_L00_002B39F0_1(int, float);      // address 0x2B39F0, 796 B, still a marker
       extern float D_001A7DC0;                            // absolute global, 0x1A7DC0
*/
void func_L00_002B3D10(void)
{
    func_L00_002B39F0_1(0, 1.0f);
    if (0.0f < D_00189E20.f1B0)
        D_00189E20.f1B0 = D_001A7DC0 * (D_00189E20.f24B8 * 0.016666668f);
    if (D_00189E20.i2294 == 0x70) {
        D_00189E20.f1B0 = D_00189E20.f1B0 * 0.8f;
        if (D_00189E20.f1B0 < 0.041666668f)
            D_00189E20.f1B0 = 0.041666668f;
    }
}
