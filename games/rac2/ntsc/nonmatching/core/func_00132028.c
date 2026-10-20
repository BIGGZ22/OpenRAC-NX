/* from src/core/00131780.c: not attempted to a match; it needs the same -g8
 * sibling unit func_001326D8 now has, and the symbol map below.
 *
 * SDK snd_Pump (symbol_addrs's own name), 488 bytes. It reaches sixteen
 * globals through $gp at GP 0x001AEFF0 - `-0x7B70` is D_001A7480, `-0x7B68`
 * D_001A7488, `-0x7B60` D_001A7490, `-0x7B5C` D_001A7494, `-0x7B58`
 * D_001A7498, `-0x7B54` D_001A749C, `-0x7B50` D_001A74A0, `-0x7B40`
 * D_001A74B0, `-0x7B38` D_001A74B8, `-0x7B30` D_001A74C0, `-0x7B2C`
 * D_001A74C4, `-0x7B28` D_001A74C8, `-0x7B20` D_001A74D0, `-0x7B10`
 * D_001A74E0 and `-0x7B08` D_001A74E8 - and three absolutely: the sentinel
 * D_001A7180 (also the `-0x7E70` gp slot), D_001A7084 and the doubleword
 * D_001A74D8.
 *
 * Shape, for whoever picks it up: return 0 at once unless D_001A74C8 is set
 * and func_00132220() agrees; then walk the records of D_001A74B0/
 * D_001A74B8/D_001A74A0 - each entry is 16 bytes with a handler at +0 and an
 * argument at +8, called through `jalr` with the second table's word as the
 * argument - and drop the pending callback (D_001A74D0 with the doubleword
 * D_001A74D8) once the sentinel has moved. The tail sets D_001A74C8 for the
 * busy window the func_00132498/func_001326D8 pair tests, runs
 * func_001133108/func_00133580(1) and an indirect D_001A7490 callback, and
 * returns 1 only when D_001A7480 is clear and D_001A74C8 ends up set.
 *
 * Spent: the disassembly and the symbol map above, by the worker that matched
 * func_001326D8 in the same unit; no compile, because the -g8 route is a
 * prerequisite and the body is 488 bytes of indirect calls.
 */
