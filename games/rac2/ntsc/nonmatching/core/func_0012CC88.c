/* from src/core/0012C9C8.c: an MPEG header dispatcher. It copies 0x2C bytes (11 words) of
   a read-only table at 0x13BBC8 onto the stack with an unaligned ldl/ldr copy, calls
   func_12C9C8(s), reads a two-bit code, then calls through the copied table with
   `jalr` - `table[code > 10 ? 0 : code](s)` - before a func_12C788/func_12C680 loop that
   compares the result against 0x1B5 and 0x1B2. The copy-to-stack of an 11-pointer local
   array with initializers is the shape to reproduce first. */
