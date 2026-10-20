/* No attempt: COP2/VU0. The retail body is four `ldc2` transfers into $vf1-$vf4 from a1,
   an `mfc1 $at,$f12`, four raw COP2 words (0x48A12800, 0x4BE50858, 0x4BE51098, 0x4BE518D8,
   0x4BE52118 -- the `qmfc2`/`ctc2` family rabbitizer cannot name) and four `sdc2` stores to
   a0, then `jr $ra`. No C can be placed; the lead proposed ASM_FUNC for this class. */
