/* Not attempted: no source on this machine, and the row is walled by a
 * jump table. What the 900 bytes do, measured from the retail disassembly:
 *
 *  - two globals guard it: 0x134730 is read first and, when non-zero, the
 *    work is skipped; 0x134728 holds the slot index it computes
 *    (`lui $v0,0x13` / `lw $v1,0x4730($v0)`, `sw $a1,0x4728($v0)`);
 *  - the index is the caller's `a0->[0xC]` scaled by 0x440
 *    (`addiu $v1,$zero,0x440` then the three-operand `mult1 $a1,$a1,$v1`);
 *  - the slot record starts at 0x13F600 + index and is reached through the
 *    uncached alias, `lui $a0,0x14` / `addiu $a0,$a0,-0xA00` /
 *    `lui $a1,0x2000` / `or $s0,$v1,$a1`, i.e. 0x20000000 | (0x13F600 + i);
 *  - a 16-byte command is read out of it with lwl/lwr (four unaligned word
 *    copies, 0,4,8,0xC), and the first word's sign bit selects one of two
 *    paths (`bltz $v0`); the taken path memcpy's (func_1153D4, the
 *    hand-written SDK memcpy) from record+0x10;
 *  - then `addiu $a0,$v0,-0x2` / `sltiu $v1,$a0,0x19` / `sll` / jump table:
 *    a 25-case switch on (command word - 2), whose table lives at 0x14xxxx
 *    and which is why C cannot place this row, exactly as for func_00115FC0
 *    and func_0011C1F8;
 *  - it ends by writing -1 into a slot field and calling func_0011AC50, which
 *    config/datamine/ntsc-u-sdk-matches.tsv names: libkernl.a klib.o
 *    iSignalSema. So this is the completion side of a SIF/kernel command
 *    transaction: it posts a semaphore when the packet's command comes back.
 *
 * RAC1 carries the same body at 0x11B868 (RAC2 = RAC1 + 0x2158 in this
 * object, the delta pinned by the matched func_0011D950 = rac1-decomp
 * src/core/00119D88.c func_0011B7F8 next to it), but rac1-decomp has it as
 * `INCLUDE_ASM` too, so there is no upstream C to transcribe either side.
 *
 * What would unblock it: nothing short of the original SDK source for this
 * libsif/libkernl file, plus a placement mechanism for the switch table.
 * The nine-argument SIF convention recorded in the func_0011D620 near miss
 * next door (a0-a3, t0-t3, then 0(sp)) applies to the siblings, not here -
 * this row takes only a0.
 */
