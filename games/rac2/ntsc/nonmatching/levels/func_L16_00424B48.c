/* func_L16_00424B48 -- Not a C row: catalogue split in level 16.

   The 168-byte entry opens with a stack restore that cannot start any function:

     addiu $sp,$sp,0x190 ; nop ; addiu $sp,$sp,-0x10

   the first two words are the previous function's epilogue (the same shape the
   level catalogue already shows in levels 23 and 30). The real 160-byte body
   starts at 0x424B50 and ends where the next catalogue entry begins (0x424BF0):

     addiu $sp,$sp,-0x10 ; sd $s0,0($sp) ; sd $ra,8($sp) ; s0 = a0
     if (*(u8 *)(s0 + 0x20) == 0) {
         if (*(u8 *)(s0 + 0xBC) != 0) {
             *(u8 *)(s0 + 0x20) = 1;
             func_002E5DA0(0, 0, s0, 0xE71);
         }
         *(u16 *)(s0 + 0x32) = 0x80;
         v = *(u8 *)(s0 + 0x20);
     } else {
         v = *(u8 *)(s0 + 0x20);
     }
     if (v == 1) {
         float f = *(float *)(s0 + 0x18) - 0.13333334f;   /* 0x3E088889 */
         u16 n = *(u16 *)(s0 + 0x96) + 1;
         *(u16 *)(s0 + 0x96) = n;
         *(float *)(s0 + 0x18) = f;
         if ((u16)n >= 60)
             *(u8 *)(s0 + 0x20) = 2;
     }

   A C source cannot produce those two leading words, so no measurement is
   recorded here; the maintainers' catalogue needs the entry moved to 0x424B50
   with size 160 (as with func_L05_00334358 and its neighbours). */
