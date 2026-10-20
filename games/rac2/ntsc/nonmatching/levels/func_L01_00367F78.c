/* func_L01_00367F78, 92 bytes, 17 places.  ABANDON - the $gp table access.
   Read in full and the logic is measured: do { if (D_001A79F0 == *p && n < 8)
   D_00139A28[0x8D] |= 1 << n; n++; p++; } while (n < 5), walking the gp table at
   gp-0x1AB0 (= 0x001ADF40 with _gp 0x001AEFF0). The candidate is 96 bytes: the loop body
   itself matches word for word (bnel with the increment in the annulled slot, sltiu/slti
   pair), and the one extra word is the table base, which comes out ABSOLUTE
   (`lui %hi; addiu %lo`) where retail has `addiu $a1,$gp,-0x1AB0`.
   Measured here: `__attribute__((sda))`, `__attribute__((section(".sdata")))` and a -G8 unit
   all still gave the absolute pair for this array; only the 8-element size is wrong for -G8
   (the retail walk reads at least four words). Needs a maintainer measurement of what makes
   THIS compiler emit GPREL16 for a gp table (the matched r1_ declarations in
   src/levels/shared/L01_002A7248.c are the precedent that it is possible). */

#include "include_asm.h"
