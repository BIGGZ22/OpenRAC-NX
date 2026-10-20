/* Not a C row: COP2/VU0. The 72-byte extent at 0x0030FA30 loads eight VU registers from
   two vectors (`ldc2 $1..$8`), runs four COP2 ops (0x4BE50868, 0x4BE610A8, 0x4BE718E8,
   0x4BE82128 - a 4x4 vector/matrix operation) and stores four back with `sdc2`.
   Outside C, ASM_FUNC candidate. */
