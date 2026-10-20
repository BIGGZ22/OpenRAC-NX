/* func_L00_004195C8 (156 bytes, 6 places) -- NOT A C ROW: the extent opens on a stack restore.
   Retail at 0x004195C8, the identical 8 bytes in all six places (levels 00, 03, 06, 09, 14, 18):
      addiu $sp,$sp,0x190        <- a deallocation, the tail of the preceding function
      nop
      addiu $sp,$sp,-0x10        <- the real entry
   The preceding instruction pair is a complete `jr $ra` / `addiu $sp,$sp,0xC0` epilogue, and the
   next row after the extent (func_L00_00419668, 0x00419668) opens on a clean `addiu $sp,$sp,-0x180`.
   No function can open on a stack restore, so the catalogue row starts 8 bytes early: the real body
   is 148 bytes at 0x004195D0 (re-cut there, the extent would be 4 + 4 + 148 = 156 as well).
   The same 8-byte opener sits in front of L00_004263A8, L00_00430A90, L00_00428EE0 and
   L00_004263A8-like rows already recorded, so it is a catalogue/overlay habit, not a one-off.

   Maintainer data: the recovered body, which matches no C that can cover the row.

   State: the handle at object+0x68 keeps a sub-object at +0x44; that sub-object's signed byte
   +0x20 decides whether the object is destroyed, otherwise the handle's float +0x3C is compared
   against zero and the object is re-armed through the level callback at 0x00419668.

   char *handle = *(char **)(object + 0x68);
   char *sub = *(char **)(handle + 0x44);
   if (sub != 0 && *(signed char *)(sub + 0x20) < 0) {
       func_0029F338(object);                       /* the destroy helper */
       return;
   }
   if (*(float *)(handle + 0x3C) != 0.0f) {
       if (func_00283DB8((void *)0x00420030, (float)*(short *)(object + 0x32)) == 0)
           return;
   }
   func_L00_002E6EF8((void *)0x00419668, object);   /* per-level, needs a ref_ name */

   Both 0x00420030 and 0x00419668 are level addresses that move per place: they need the
   ref_LNN_XXXXXXXX_kk spelling, not literals. */
