/* MMI (SPECIAL2) instruction: no C spelling. 5716 bytes, level 18_damosel.
   Retail at 0x003F5650 carries the SPECIAL2 words 0x700014A9, 0x70001CA9
   (+0x0168, +0x0280) - opcode 0x1C, which rabbitizer prints as `INVALID`;
   it is the MMI family (pcpyld and its relatives) the sixth wave recorded
   and that no C spelling reaches. This cc1 emits `lq`/`sq` for a `mode(TI)`
   local and `ldl`/`ldr`/`sdl`/`sdr` for a struct assignment, never the
   packed copy. ASM_FUNC candidate, as for the rest of the family. */
