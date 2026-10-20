/* Near miss - not attempted as C.  1556 bytes at 0x00117CA0, the largest body
   of the libc/double pocket that func_00117650 and func_001182B8 belong to
   (markers in `001163B0.c`); `-mastra-quad-saves` prologue `addiu $sp,$sp,-0xA0`
   then `sq $ra,0x90; sq $fp,0x80; sq $s7,0x70; ...; sq $s0,0x0` in descending
   order, eleven saves.  It needs its own unit with that flag instead of the
   object's `sdk` line.

   Its call inventory is new evidence and it is what makes this the arithmetic
   engine of the pocket: 27 x func_122B00 (dpmul), 12 x func_122A40 (dpadd),
   12 x func_122A98 (the sub twin; name not checked), 3 x func_123078 (litodp),
   1 x func_123028 (dpcmp) and 1 x func_122DA8.  func_001182B8 calls it - see
   that near-miss, which also records its 2^54 `dpmul` scaling and the
   -inf/+inf pair at 0x0013A480/0x0013A488.

   Identity is not established; a balanced 27-multiply / 12-add / 12-sub body
   over litodp and dpcmp points at a scaled conversion or a series evaluation
   rather than at a raw libgcc entry (the four RAC2 fp-bit rows are
   0x122A40-0x1234F0).  Whoever resumes it should compare the three magic
   64-bit constants against newlib's dtoa/mprec and the SDK's own decimal
   helpers before writing C. */
