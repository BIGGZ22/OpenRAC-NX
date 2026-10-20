/* MMI (SPECIAL2) instruction: no C spelling. 3172 bytes, level
   00_aranos_tutorial. Retail at 0x003918C8 carries the SPECIAL2 words
   0x700014A9, 0x700024A9 (+0x0004, +0x0810) - opcode 0x1C, which rabbitizer
   prints as `INVALID`; it is the MMI family (pcpyld and its relatives) the
   sixth wave recorded and that no C spelling reaches. This cc1 emits
   `lq`/`sq` for a `mode(TI)` local and `ldl`/`ldr`/`sdl`/`sdr` for a struct
   assignment, never the packed copy. ASM_FUNC candidate, as for the rest of
   the family. */
