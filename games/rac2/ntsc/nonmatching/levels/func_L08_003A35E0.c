/* MMI (SPECIAL2) instruction: no C spelling. 5868 bytes, level 08_tabora.
   Retail at 0x003A35E0 carries the SPECIAL2 words 0x72A21800, 0x72A22000,
   0x72A21800 (+0x124C, +0x1278, +0x1400) - opcode 0x1C, which rabbitizer
   prints as `INVALID`; it is the MMI family (pcpyld and its relatives) the
   sixth wave recorded and that no C spelling reaches. This cc1 emits
   `lq`/`sq` for a `mode(TI)` local and `ldl`/`ldr`/`sdl`/`sdr` for a struct
   assignment, never the packed copy. ASM_FUNC candidate, as for the rest of
   the family. */
