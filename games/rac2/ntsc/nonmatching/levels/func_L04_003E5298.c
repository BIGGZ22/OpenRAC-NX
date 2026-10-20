/* Near miss: 8 bytes short, the tag and the iterator land in different argument registers.
   Retail at 0x003E5298, level 04 (124 bytes, 1 place) runs the tag's list of sub-objects and
   marks each live one that carries handle 0x8BA:

     void f(char *a0)
     {
         unsigned char tag = *(unsigned char *)(a0 + 0x21);
         char *base = (char *)D_L04_001B24DC;      /* the $gp word, -G8 + sda */
         short *it;

         if (tag == 0xFF) return;
         it = ((short **)D_L04_001FB1E0)[tag];     /* the level's sub-object table */
         if (it == 0) return;
         do {
             char *object = base + ((*it & 0x7FFF) << 8);
             if (*(signed char *)(object + 0x20) >= 0 && *(short *)(object + 0xAA) == 0x8BA)
                 *(unsigned short *)(*(char **)(object + 0x68) + 0x2A6) = 1;
         } while (*it++ >= 0);
     }

   probes "as measured": the short pointer iterator with the increment inside the do/while
   condition is what reproduces the terminator test (`lh` in the two branch-likely delay slots,
   `bgez` looping back with `addiu $a0,$a0,2` in its own slot); a `char *` iterator with `it += 2`
   does not, and neither does a plain for/while. The gp word reaches $gp as `lw $a2,0x34EC($gp)`
   with `extern int D_L04_001B24DC __attribute__((sda))` in the -g8 sibling.

   The 8-byte gap is two `daddu` copies retail keeps and we drop: retail holds the loaded tag in
   $v0 and copies it to $v1 for the `sll` (`daddu $v1,$v0,$zero` in the beq delay slot), and
   copies the table entry to $a0 (`daddu $a0,$v0,$zero`); we load the tag straight into $v1 and
   use $a1 as the iterator, which is one instruction shorter. Tried and did not move it: swapping
   the compare to `0xFF == tag`. Register allocation, the wall already paid. */
