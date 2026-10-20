/* func_L00_00419A80 (212 bytes, 2 places: 00_aranos_tutorial, 14_aranos_prison) -- NOT A C ROW:
   the extent opens on two stack restores.
   Retail at 0x00419A80, the same 16 bytes in both places:
      addiu $sp,$sp,0x50 ; nop      <- restores, the tail of the preceding function
      addiu $sp,$sp,0x30 ; nop
      addiu $sp,$sp,-0x20           <- the real entry at 0x00419A90
   No function can open on a stack restore, so the catalogue row starts 16 bytes early: the real
   body is 196 bytes at 0x00419A90 (same family as func_L00_004195C8's 8-byte opener and the
   func_L00_004263A8 / func_L00_00430A90 rows already recorded). Fix in `tools/levels.py`.

   Maintainer data: the recovered body, which matches no C that can cover the row.

   Scales the record's rate by the level's own factor, ages the sub-object's float by 1/240, runs
   the paired vector update on two 16-byte blocks, re-derives the three turning components from
   the sub-object's angles, and drops the record when its countdown byte reaches zero.

   float f = ref_L00_001B1444_00;                 /* $gp+0x2454 here, $gp+0x2B1C in level 14 */
   char *h = *(char **)(a0 + 0x68);
   char *p = *(char **)(a0 + 0x24);

   f = *(float *)(p + 0x24) * f;
   *(float *)(a0 + 0x2C) = f;
   *(float *)(h + 8) = *(float *)(h + 8) - (1.0f / 240.0f);   /* bits 0x3B888889 */
   func_00282CC0(a0 + 0x10, a0 + 0x10, h);
   func_00282CC0(a0 + 0xF0, a0 + 0xF0, h + 0x10);
   *(float *)(a0 + 0xF0) = func_00283B98(*(float *)(a0 + 0xF0), *(float *)(h + 0x10));
   *(float *)(a0 + 0xF4) = func_00283B98(*(float *)(a0 + 0xF4), *(float *)(h + 0x14));
   *(float *)(a0 + 0xF8) = func_00283B98(*(float *)(a0 + 0xF8), *(float *)(h + 0x18));
   if ((unsigned char)--*(unsigned char *)(a0 + 0xBC) == 0)
       func_0029F338(a0);

   The `$gp` float moves per place (0x2454 / 0x2B1C) and needs a ref_LNN_XXXXXXXX_kk name, not a
   literal. */
