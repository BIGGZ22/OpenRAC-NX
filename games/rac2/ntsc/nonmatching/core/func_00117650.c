/* Double bit-manipulation helper in the libc/heap object (001163B0.c), with
   the `-mastra-quad-saves` prologue: `addiu $sp,$sp,-0x60` then
   `sq $ra,0x50; sq $s4,0x40; ...; sq $s0,0x0` in descending order, so it
   needs its own unit with that flag rather than the object's `sdk` line.

   It never unpacks: it splits the raw double bits, `sra $v1,$s0,20` masked
   with 0x7FF then `-0x3FF` for the exponent, and branches on
   `exp < 0x14` and `exp < 0x34` (20 and 52 -- the FP fraction widths).  It
   then does `dpadd` (func_00122A40) against the double at 0x0013A3E8, which
   is 1e+300, `dpcmp` (func_00123028) against 0.0, and builds 0xFFFFFFFF as
   `addiu $a0,$zero,-1; dsrl32 $a0,$a0,0`.

   The `daddu $v0,$a1,$zero; dsll32 $s2,$v0,0; dsra32 $s2,$s2,0;
   dsra32 $s0,$v0,0` opening is the s64-local pattern this repository
   already records: the two `dsll32`/`dsra32` pairs need a named `s64`
   local, an inline cast folds the read to `lw`.

   Not attempted as C in this run.  Identity is not established: it is not
   one of the four RAC2 fp-bit rows (those are 0x122A40-0x1234F0), and 1e300
   next to 1.0 at 0x0013A3F0 points at a printf/`dtoa` decimal helper rather
   than at libgcc.  A writer should first identify it -- the 1e300 comparison
   and the 20/52 exponent split are the cheapest discriminators. */
