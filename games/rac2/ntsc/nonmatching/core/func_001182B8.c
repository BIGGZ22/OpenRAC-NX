/* Double helper next door to func_00117650, in the same libc/heap object
   (001163B0.c) and with the same `-mastra-quad-saves` prologue
   (`addiu $sp,$sp,-0x40` then `sq $s1,0x10; sq $ra,0x30; sq $s2,0x20;
   sq $s0,0x0`), so it likewise needs its own unit with that flag instead of
   the object's `sdk` line.

   It splits the double with `dsra32 $s0,$v0,0` (high word) and
   `dsll32 $v1,$v0,0; dsra32 $v1,$v1,0` (low word) -- the named `s64` local
   pattern: an inline cast folds the second read to `lw`.  Then:
   `0x000FFFFF < high` guards, `(high & 0x7FFFFFFF) | low` for zero, a sign
   test, `dpmul` (func_00122B00) by `0x86A0 << 47` (2^54), an exponent
   re-bias through `litodp` (func_00123078), a call to func_00117CA0, and
   three `dpmul`/`dpadd` steps against 64-bit constants built as
   `ori/dsll` chains.  The two doubles it returns directly are the pair at
   0x0013A480/0x0013A488, which are -inf and +inf.

   Not attempted as C in this run: 424 bytes whose identity is not
   established.  2^54 scaling plus inf constants suggests a saturating
   conversion or a scaling helper for a decimal formatter rather than a
   libgcc entry; pinning the three magic constants (0xF567FBCC8F89B36 and
   friends) against a known source should come before writing C. */
