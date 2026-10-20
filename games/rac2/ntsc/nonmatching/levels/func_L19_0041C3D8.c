/* func_L19_0041C3D8 -- catalogue defect: the entry starts eight bytes early.
 *
 * The 148-byte extent at 0x0041C3D8 opens with `addiu $sp,$sp,0x10` / `nop` and the real
 * body's own prologue is at 0x0041C3E0 (`addiu $sp,$sp,-0x20` ... `jr $ra` /
 * `addiu $sp,$sp,0x20` at 0x0041C464/0x0041C468). So the real function is 140 bytes at
 * 0x0041C3E0 and the row's address and size are both off; the same started-early defect the
 * levels 23/30 rows show. No C of the catalogued extent can exist.
 *
 * The real body (state gate on byte 0x20, then a float chain):
 *   if (self->f20 == 0) {
 *       t = func_1234F0(self->f0C);                 // *((float *)self + 3)
 *       a = func_131430(0x400000000000ULL);         // ori 0x8000 ; dsll32 15
 *       b = func_122B00(a, 0x409000000000ULL);      // ori 0x8120 ; dsll32 15
 *       c = func_122DA8(t, b);
 *       d = func_123298(c);
 *       ctx->f10 = d;                               // *((float *)ctx + 4)
 *       func_002A7750(self, 1, -1);
 *   }
 * ctx is *(void **)(self + 0x68); the two 64-bit operands are materialised with
 * ori/dsll32, so the declaration needs `long long` (unsigned long long) parameters.
 *
 * Needs the corrected extent from the maintainers (same fix as func_L00_00304AE8 and the
 * other started-early rows).
 */
