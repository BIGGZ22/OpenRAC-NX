/* MMI (SPECIAL2) instruction: no C spelling. 2572 bytes, level
   00_aranos_tutorial. Retail at 0x0038DE70 carries the SPECIAL2 words
   0x71936000, 0x70001CA9, 0x700044A9, 0x700044A9, 0x700024A9 (+0x0188,
   +0x01C8, +0x0308, +0x042C, +0x0430) - opcode 0x1C, which rabbitizer
   prints as `INVALID`; it is the MMI family (pcpyld and its relatives) the
   sixth wave recorded and that no C spelling reaches. This cc1 emits
   `lq`/`sq` for a `mode(TI)` local and `ldl`/`ldr`/`sdl`/`sdr` for a struct
   assignment, never the packed copy. ASM_FUNC candidate, as for the rest of
   the family. */
