/* Record-chain validator plus heap sort, in 001232F0.c next to the record
   walker func_00123978 and this repository's func_00123930.

   One pointer in $a0 ($s1 = a0, $s2 = s1 + 8) and one expected value in
   $a1.  Two consistency checks, each ending in abort (func_00115E28), which
   is why symbol_addrs/usa-v2 lists this function among abort's callers:

     s0 = s[1];                      /* captured before the call *​/
     if (s0 != a1) abort();
     func_001242A0(s, s + 8);
     if (s[1] + s[3] != s0) abort();  /* s0 is the pre-call s[1] *​/

   The rest is a sift-down over an array of pointers at s[2]: `t5 = s2[1]`
   (a count, `beqz` and `sltiu ...,2` for the 0/1 exits), `sll $a3,$a0,3` /
   `sll $a1,$a0,2` for the two index scales, the order key at +8 of each
   pointee (`lw $v0,0x8($a0)`), and pointer-pair swaps.  GCC's heap-sort
   sift-down shape, not qsort's.

   Not attempted as C in this run.  The two abort-guarded invariants fix the
   struct's two counts and are the cheapest way in; the sift-down itself is
   mechanical once they are written down. */
