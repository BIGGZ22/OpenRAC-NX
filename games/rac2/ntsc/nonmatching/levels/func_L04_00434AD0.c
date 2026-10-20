/* func_L04_00434AD0, 224 bytes, 2 placements.  NOT RETRIED - the twin of func_L04_00423178.
   Byte for byte the same body as its neighbour (same 224 bytes): D_001A8F00 == 2, D_001A7A40,
   the count at 0x001BD380 + 0x44, the +0x18C pointer-array walk, the +0xAA short comparison,
   the D_001B1DE1 guard, func_3325E0(record, 0, D_001B1DE0) and the three 4.0f publishes into
   D_001B1E00/0x1E04/0x1E08 (f20 = 0x4030).  It therefore hits the same measured wall - retail
   rematerialises every absolute address at its own site (`lui $v1,0x1B` /
   `lw $v1,-0x7100($v1)`, three separate `lui $at,0x1B` / `swc1`), this cc1 CSEs them and the
   frame grows 0x40 -> 0x60 with $s4-$s7/$fp (260 bytes nearest).  Read
   nonmatching/levels/func_L04_00423178.c for the four spellings measured on this exact shape. */
