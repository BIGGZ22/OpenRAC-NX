/* MMI (SPECIAL2) instruction: no C spelling. 2048 bytes, level 07_siberius.
   Retail at 0x00416510 carries the SPECIAL2 word 0x70431000 (+0x0448) -
   opcode 0x1C, which rabbitizer prints as `INVALID`; it is the MMI family
   (pcpyld and its relatives) the sixth wave recorded and that no C spelling
   reaches. This cc1 emits `lq`/`sq` for a `mode(TI)` local and
   `ldl`/`ldr`/`sdl`/`sdr` for a struct assignment, never the packed copy.
   ASM_FUNC candidate, as for the rest of the family. */
