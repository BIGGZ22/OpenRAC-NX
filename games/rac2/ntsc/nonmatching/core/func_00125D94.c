/* Not matchable yet: the extent begins on the linker's fill word.
   Bytes 0x125D94..0x125D97 are 0xCDCDCDCD (the fill between two objects); the
   function itself is the 188 bytes from 0x125D98 to 0x125E54. check.py
   requires the compiled section to be exactly the catalogue's 192 bytes and to
   equal those bytes, and no C starts with that word. This needs the same
   correction docs/LEVELS.md describes for three Aranos entries -- move the
   extent start to +4 -- in tools/levels.py; then the body (call func_00125D96,
   test the halfword at +6, and store eight 64-bit GS registers under either
   the 0x12000000..0x120000C0 or the 0x12000000/20/90/A0/E0 set) is ordinary C.
   The same correction applies to func_001290BC. */
