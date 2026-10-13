/* func_L00_002B5A88 (212 bytes, 27 places) -- NEAR MISS, not integrated.
   state: 200 bytes / 50 instructions against 212 / 53.
   check.py (00_aranos_tutorial): 200 bytes, expected 212

   Retail shape (fully decoded, this part is believed right):
     s1 = D_00189EA0 (table base), s2 = -1, s3 = 0x190000 (the lui of D_00189E20),
     s4 = s1 + 0x1D10 (ring base), s5 = 1, s0 = i = 0
   loop:
     jal func_2B55F0 with a0 = i ; if (v0 == 1) goto epilogue
     if (D_00189E20.i23C8 >= 2) { D.i23C8 -= 1; D.i23C4 = (D.i23C4 + 31) % 32; }
     lq/sq copy of ring[D.i23C4] (16 bytes) into table[0]
     i++ ; if (i < 16) goto loop

   Ours is 3 instructions short: gcc rotates the loop (the producer call moves to the bottom
   and the break test becomes the back-edge test) and computes the D_00189E20 base inline with
   `la D_00189E20_1` + 0x23C8 displacement instead of keeping the `lui $s3,0x19` / `addiu
   $a1,$s3,-0x61E0` pair. Tried: `for` with a top `break` (rotated), a `tbl`/`ring` pointer
   pair (kept s1/s4 but not s3), `%` written as `(x + 31) % 32` (this part matches exactly:
   the slt/addiu/movn/sra/sll/subu sequence is right).
   Next: try a shape that stops the loop rotation (e.g. an explicit `i = 0; for (;;)` with the
   test written out), and reach the D_00189E20 fields through a struct whose base symbol is
   D_00189E20 (offset 0) rather than D_00189E20_1.

   Struct fields needed on D_00189E20_Overlay (carved out of the existing pads, offsets of all
   other members unchanged): int i23C4 at 0x23C4, int i23C8 at 0x23C8.
   func_L00_002B55F0 is a marker in the same file, so the call needs an alias name
   (func_L00_002B55F0_1); tools/check.py and tools/build.py resolve the value from the retail
   relocation, the name only has to be unique.
*/
extern int func_L00_002B55F0_1(int);

void func_L00_002B5A88(void)
{
    V4_F6b9e759f *tbl = D_00189EA0_1;
    V4_F6b9e759f *ring = tbl + 465;
    int i;

    for (i = 0; i < 16; i++) {
        if (func_L00_002B55F0_1(i) == 1)
            break;
        if (D_00189E20.i23C8 >= 2) {
            D_00189E20.i23C8 = D_00189E20.i23C8 - 1;
            D_00189E20.i23C4 = (D_00189E20.i23C4 + 31) % 32;
        }
        *(V4_Fa012328c *)tbl = *(V4_Fa012328c *)&ring[D_00189E20.i23C4];
    }
}
