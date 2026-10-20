/* No attempt on the row as catalogued: its first eight bytes are not code.
   The catalogue puts func_L13_003C5908 at 0x3C5908, and the first two words
   there are 0x27BD0010 (addiu $sp,$sp,0x10) and 0x00000000 (nop) - the
   linker fill between two real functions, the same shape as func_L03_002B0958
   and the four-byte rows of src/levels/shared/L00_002F4400.c. The real body
   starts at 0x3C5910 with `addiu $sp,$sp,-0x20`, and the previous function
   (func_L00_003B79A0, 936 bytes at 0x3C5560) ends at 0x3C5904 with
   `jr $ra` / `addiu $sp,$sp,0x20`, so the eight words belong to neither.
   A catalogue split, to be fixed in tools/levels.py: the row should start at
   0x3C5910 and be 448 bytes long. */
