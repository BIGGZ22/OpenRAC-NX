/* Not a C row: COP2/VU0. The 24-byte extent at 0x0030EDE0 is `ldc2 $1` / `ldc2 $2` /
   a COP2 op (0x4BC2086C) / `nop` / `jr $ra` / `sdc2 $1`, i.e. vector register loads, a
   vector operation and a store - outside C, ASM_FUNC candidate. */
