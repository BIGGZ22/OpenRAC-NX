/* MMI (SPECIAL2) instruction: no C spelling. 1952 bytes, level
   00_aranos_tutorial. Retail at 0x0038E880 carries the SPECIAL2 words
   0x70004CA9, 0x70004CA9, 0x70002CA9, 0x70002CA9, 0x70004CA9, 0x70004CA9
   (+0x0134, +0x0254, +0x029C, +0x034C, +0x0424, +0x0538) - opcode 0x1C,
   which rabbitizer prints as `INVALID`; it is the MMI family (pcpyld and
   its relatives) the sixth wave recorded and that no C spelling reaches.
   This cc1 emits `lq`/`sq` for a `mode(TI)` local and
   `ldl`/`ldr`/`sdl`/`sdr` for a struct assignment, never the packed copy.
   ASM_FUNC candidate, as for the rest of the family. */
