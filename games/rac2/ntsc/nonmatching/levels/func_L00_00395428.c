/* MMI (SPECIAL2) instruction: no C spelling. 1428 bytes, level
   00_aranos_tutorial. Retail at 0x00395428 carries the SPECIAL2 words
   0x72821800, 0x72821800 (+0x030C, +0x049C) - opcode 0x1C, which rabbitizer
   prints as `INVALID`; it is the MMI family (pcpyld and its relatives) the
   sixth wave recorded and that no C spelling reaches. This cc1 emits
   `lq`/`sq` for a `mode(TI)` local and `ldl`/`ldr`/`sdl`/`sdr` for a struct
   assignment, never the packed copy. ASM_FUNC candidate, as for the rest of
   the family. */
