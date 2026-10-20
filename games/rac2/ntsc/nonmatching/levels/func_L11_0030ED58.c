/* Near miss: the guard and the band structure are recovered, the C spelling is not.
   Retail at 0x0030ED58, level 11 (156 bytes, 1 place) reads the shared state word, and only when
   it is 11 and the block at 0x001BE5C0 holds 2 at +0x30 does it classify the id at +0x34 into
   one of three bands and hand the result to the string/asset loader:

     void f(void)
     {
         int id, which;

         if (D_001A79F0 != 11) return;            /* absolute lui/lw, so no -G8 for this unit */
         if (*(int *)(0x001BE5C0 + 0x30) != 2) return;
         id = *(int *)(0x001BE5C0 + 0x34);
         if ((unsigned)(id - 0x1B9) < 0xB3)       /* id in [0x1B9,0x26B] */
             which = 0x46;
         else if ((unsigned)(id - 0x321) < 0x95)  /* id in [0x321,0x3B5] */
             which = 0x47;
         else if (id <= 0x44B)
             which = 0x48;
         else
             which = -1;
         if (which == -1) return;
         func_2FF678((void *)(0x001AEFF0 - 0x5BD0), which, 2, 0x80808080, 0);
     }

   probes "as measured": the two `sltiu` after `addiu $v0,$a0,-<base>` are one `unsigned` range
   test each, so the bands are closed ranges and not a switch this cc1 bisected (a switch would
   have shown several pivots, not two full-width bands); the trailing `addiu $v1,$zero,0x48;
   slt $v0,$v0,$a0; movz $a1,$v1,$v0` is the `id <= 0x44B` fallback with -1 as the sentinel, and
   the loader's five arguments are the $gp string at 0x001A9420, the band value, 2, the 0x80808080
   mask and a zero.

   Not written out: the band constants are three literals with no derivation in the bytes, so the
   C would be a transcription rather than a reconstruction, and every spelling of the range test
   tried landed on a different compare order. Worth one more pass with the band bounds as named
   constants. */
