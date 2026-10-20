/* from src/core/0011C8B0.c: not attempted in this run; console/stdio init that installs
   the 0x134698/0x13469C hooks, reconstruction below */
/*
 * 0x11C8D8..0x11CB58, 640 bytes. Real body, ordinary C: no COP2/MMI/`sync`. It has no
 * `jr $ra` at all - it leaves through three sibling `j` tails: func_11F628 on the early
 * guard path, and func_11CD20 twice at the end.
 *
 * Profile: `addiu $sp,$sp,-0x60` with `sd $ra,0x50` / `sd $s4,0x40` / `sd $s3,0x30` /
 * `sd $s2,0x20` / `sd $s1,0x10` / `sd $s0,0x0` - 16-byte slots (sdk family). The first
 * call is `jal func_11F5E0` with `sd $s0,0x0($sp)` in its delay slot.
 *
 * Guard: `lui $t2,0x13 ; lw $v0,0x469C($t2)` - if 0x13469C is already non-zero the
 * function restores and tail-calls func_11F628; otherwise it stores 1 there. 0x13469C is
 * the second of the two hooks next to 0x134698 (the one func_0011C090 calls with '-').
 *
 * Then it fills a 0x80-byte structure passed as the argument: the words at +0x0, +0x4,
 * +0x8, +0xC, +0x10, +0x14, +0x18 and +0x1C, with the two pointers at +0x4 and +0x1C
 * ORed with 0x20000000 (the uncached alias) and a 0x20 stored at +0x10. Two 0x20-entry
 * loops clear 0x8-byte pairs from the +0x0 base and 4-byte words backwards from +0x7C;
 * a word-at-a-time 0x100-byte pair moves 0x12xxxx and 0x12xxxx code into it.
 *
 * Hardware pokes, all plain loads/stores: 0x1000E010 bit 0x20 then 0x10011FF0, and
 * 0x1000C000 bit 0x100.
 *
 * Then func_11AEA0(0), func_11B000(), the 0x5 / 0x12F28 pair to func_11A930 with a 0
 * third argument, func_11B6C0(5) whose result is stored at 0x13xxxx-0x30AC, and
 * func_11B030(0x80000000) whose result goes to +0x8 of the structure; two more
 * func_11B030 probes (0x80000004 / 0x80000002) feed func_11B020, then both exits store
 * 0x14 / 0 at the same offsets and tail-call func_11CD20.
 */
