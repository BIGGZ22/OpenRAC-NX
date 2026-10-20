/* Near miss: 160 against 148, the block pointer's address hoisted out of the loop.
   Retail at 0x0037B3A0, level 05 (148 bytes, 4 places) counts the tag's list of live
   sub-objects:

     int f(char *object)
     {
         int *p = D_001FB220[*(unsigned char *)(object + 0x21)];
         unsigned short *it;
         int count = 0;

         if (p == 0)
             return 0;
         it = (unsigned short *)p;
         do {
             char *r = func_379CA0(D_001B251C + ((*it & 0x7FFF) << 8));
             if (r != 0)
                 count = (*(int *)(r + 0x110) == 1) ? count + 1 : count;
         } while ((short)*it++ >= 0);
         return count;
     }

   probes "as measured": the `unsigned short *` iterator with `(short)*it++` in the do/while
   condition reproduces the table's mixed loads (`lhu` for the masked read, `lh` for the
   terminator), the `xori`/`movz` count is the ternary and not `count += (x == 1)` (that gives
   `sltiu`+`addu`, one word longer), and `extern int *D_001FB220[]` with `extern char
   D_001B251C[4]` reach their words with plain `lui`/`lw`.

   The 12 bytes are three words: this cc1 hoists the loop-invariant `lui` of the 0x1B251C address
   into a callee-saved register, which costs the extra `sd $s2,0x10($sp)` and turns the lookup's
   guard into `beql` with the terminator load in the annulled slot; retail re-materialises
   `lui`+`lw` inside the loop, keeps a plain `beqz` with `addiu $v1,$s1,0x1` in the slot, and needs
   no $s2. Fixing the count expression alone does not move the hoist. Register allocation of a
   constant address, the wall already paid. */
