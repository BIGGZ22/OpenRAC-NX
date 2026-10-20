/* Not C: VU0 macro code, and the extent starts one word late.
   Bytes 0x1290BC..0x1290BF are 0xCDCDCDCD, the linker's fill between objects
   (docs/LEVELS.md); the 92 bytes that follow are sqc2/lqc2 plus a run of COP2
   macro-mode words, and end in `jr $ra; sqc2 $vf6,0($a0)`. Nothing here can
   come out of a C compiler, so the catalogue extent is either one word too
   early or the function is ASM_FUNC. */
