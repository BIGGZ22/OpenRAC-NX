/* from src/levels/shared/L00_002E5918.c: no C attempt;
 * VU0 macro mode. Retail, 104 bytes: ldc2 $1,0($a1); nine COP2 macro-mode words
 * (0x4B8108AA, 0x4B021081, 0x4A0203BD, 0x48A11800, 0x4A0003BF, 0x4B0000A0,
 * 0x4B001924, 0x48211000, 0x48222000) around an mfc1 $at,$f12 and two dsll32,
 * then beqz $at / bgtz with the $v0 spin, three COP2 words more (0x4A021BBC,
 * 0x4A0003BF, 0x4B80085C), sdc2 $1,0($a0); jr $ra returning 1, and a second exit
 * jr $ra returning 0. A candidate computed on the VU0 and stored back; plain C
 * cannot emit the COP2 ops: ASM_FUNC candidate. */
