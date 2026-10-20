/* MMI (SPECIAL2) instruction: no C spelling. 1068 bytes, level
   00_aranos_tutorial. Retail at 0x00388790 carries the SPECIAL2 word
   0x700024A9 (+0x0114) - opcode 0x1C, which rabbitizer prints as `INVALID`;
   it is the MMI family (pcpyld and its relatives) the sixth wave recorded
   and that no C spelling reaches. This cc1 emits `lq`/`sq` for a `mode(TI)`
   local and `ldl`/`ldr`/`sdl`/`sdr` for a struct assignment, never the
   packed copy. ASM_FUNC candidate, as for the rest of the family. */
