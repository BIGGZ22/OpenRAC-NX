/* MMI (SPECIAL2) instruction: no C spelling. 4972 bytes, level
   00_aranos_tutorial. Retail at 0x0041CB80 carries the SPECIAL2 words
   0x70832000, 0x70432800, 0x70432800, 0x70822000, 0x70832000 (+0x04A8,
   +0x0958, +0x09B0, +0x1248, +0x12A4) - opcode 0x1C, which rabbitizer
   prints as `INVALID`; it is the MMI family (pcpyld and its relatives) the
   sixth wave recorded and that no C spelling reaches. This cc1 emits
   `lq`/`sq` for a `mode(TI)` local and `ldl`/`ldr`/`sdl`/`sdr` for a struct
   assignment, never the packed copy. ASM_FUNC candidate, as for the rest of
   the family. */
