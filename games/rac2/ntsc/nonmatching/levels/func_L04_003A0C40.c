/* func_L04_003A0C40 (164 bytes, 5 places) -- NEAR MISS, not integrated: VU0 addresses.
   The body is a scalar loop, but every store goes through `lui $v1,0x7000 / ori $v1,0x2000`
   (0x70002000, the VU0 register area) and the row carries the same macro-mode words as its
   neighbour (`0x4BE1xxxx` was not in this one, but the `mult $v0,$a3` at +0x30 and the
   `0x7000` bases are): the code walks 0x11 (=17) entries of 0x60 bytes, adding four floats
   fetched from 0x1B0000-relative tables, and writes three of them into the VU0 window:
     003A0C48  lui   $v1, 0x7000 / ori $v1, $v1, 0x2000   <- VU0 memory
     003A0C70  mult  $v0, $a3        (0x60 stride, no mflo in the row)
     003A0C88  swc1  $f2, 0x0($v1)
   Writing C that reaches a VU0 window address as data is not Plain C; the same family is
   already recorded as "not C" for the other levels. Measured: not attempted beyond the
   decode. */
