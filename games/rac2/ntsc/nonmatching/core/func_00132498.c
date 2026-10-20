/* from src/core/00131780.c: not attempted to a match; the row needs the
 * small-data unit route before any C can be written for it.
 *
 * snd_BankLoadAsync (symbol_addrs's own SDK-library name), 328 bytes. It
 * reaches five globals through $gp in ordinary flow - `lw $v0,-0x7B28($gp)`,
 * `sw $zero,-0x7B68($gp)`, `lw $v0,-0x7AF8($gp)`, `sw $s0,-0x7E30($gp)` and
 * `sw $s2,-0x7B20($gp)` - which per docs/COMPILER-NOTES.md needs a `-G8`
 * sibling unit plus `__attribute__((sda))` on each symbol, and a sixth
 * (`sd $s3,...`, 64 bits) absolutely. With the pinned GP 0x001AEFF0 those are
 * D_001A74C8, D_001A7488, D_001A74F8, D_001A71C0, D_001A74D0 and the
 * doubleword D_001A74D8; the routine itself also stores to D_001A71C4 and
 * D_001A7180 absolutely (through $at) and passes the address 0x001A71C0 to
 * its last call, which is what pins the GP value.
 *
 * Shape, for whoever picks it up: if the busy flag (gp-0x7B28) is set, report
 * through func_00115D48 with the string at 0x001A7600 unless gp-0x7AF8 is
 * set, and return. Otherwise store arg0 in gp-0x7E30, ask func_00133580(1)
 * for the driver state (a 1 is the refusal, reported with 0x001A7630), latch
 * arg1 in D_001A71C4, the sentinel -1 in D_001A7180, arg2 in gp-0x7B20 and
 * the 64-bit arg3 in D_001A74D8, then poll: while func_0011D810(0x001A7140)
 * is non-zero, print 0x001A7578 when gp-0x7AF8 is clear, then func_00132028()
 * and func_0011AEA0(0). It finishes by setting the busy flag, zeroing a stack
 * word and calling func_0011D620(0x001A7140, 3, 1, 0x001A71C0, 8, 0x001A7180,
 * 4, 0) - eight register arguments and that stack word, the EE ABI's $t0-$t3
 * slots.
 *
 * Spent: the disassembly and symbol map above; no compile, because the -g8
 * unit is a prerequisite (the same recipe func_00133640 documents).
 */
