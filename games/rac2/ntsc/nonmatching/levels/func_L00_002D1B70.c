/* Not attempted - body read in full, no C written: a table lookup, 208 bytes, 27 places.

   func_L00_002D1B70(id, out): `func_2ADEA0(3)` selects one of three $gp tables
   -0x7290 / -0x72A8 / -0x7278, i.e. 0x001A7D60 / 0x001A7D48 / 0x001A7D78 (all above
   LEVEL_BASE, so level-local small data). The third case is the default and a
   `movn` zeroes it when the selector is not 4 (`xori` against 4 then movn), which is the
   same merge shape as func_L00_002CB2D8's blocker. A null table stores 0 to *out and
   returns 0; a table whose first short is -1 stores 0 and returns 0. Otherwise it walks
   the table in 4-byte strides (a `short *` advanced by 4 bytes, i.e. +2 shorts) comparing
   *(u8 *)(q+2) with the id; on a hit it stores the table's first short to *out and returns
   1, and the walk ends when the short at the next stride is -1 (then *out = 0, return 0).
   Blocking requirement: the three $gp tables declared with a size so `sda` takes effect. */
