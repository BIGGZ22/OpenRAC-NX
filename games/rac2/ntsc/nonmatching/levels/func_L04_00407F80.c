/* Near miss: every instruction agrees but one, a single destination register.
   Retail at 0x00407F80, level 04 (164 bytes, 1 place) starts the object on its first update:

     void f(char *a0)
     {
         char *s0 = a0;
         char *sub = *(char **)(s0 + 0x68);          /* read unconditionally */

         if (*(unsigned char *)(s0 + 0x20) == 0) {
             func_002A0488(s0, D_L04_001B07B0, D_L04_001B07B4, D_L04_001B07B8);
             *(unsigned char *)(s0 + 0x30) = 0xC4;
             *(unsigned char *)(s0 + 0x20) = 1;
             *(unsigned short *)(s0 + 0x34) |= 1;
             *(unsigned short *)(s0 + 0x32) = 0;
         } else {
             if (*(int *)(sub + 8) != -1)
                 if (func_002ACBC8(D_L04_001B9680) == 0)
                     return;
             func_002A0488(s0, D_L04_001B07B0, D_L04_001B07B4, D_L04_001B07B8);
             func_L00_002E6EF8(func_L04_00407B80, s0);
         }
     }

   Its sibling func_L04_00407000 in src/levels/04_barlow/L04_003AB458-g8.c is the same shape and
   already matches, which is what fixed the three small-data words (D_L04_001B07B0/14/18, the
   second triple, not the D_L04_001B0710/14/18 func_L04_00407000 uses).

   probes "as measured": the field at +0x68 must be read unconditionally - that is what makes the
   branch a plain `bnez` with the load in its delay slot instead of the `bnel` an inline read
   produces; the constant at +0x30 must be `unsigned char` (a signed `char` folds 0xC4 to
   `addiu $v1,$zero,-0x3C`).

   Remaining gap, one register: the delayed load is `lw $a1,0x68($s0)` in retail and
   `lw $v1,0x68($s0)` for us. Tried and did not move it: hoisting the read as `int *` and as
   `char *`, and reading it inline. Register allocation, the wall already paid. */
