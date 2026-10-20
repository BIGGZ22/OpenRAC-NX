/* from src/levels/shared/L01_00397110.c: not written this wave - no C can place it.
 * 192 bytes, 5 places. Rule: jump table in .rodata. The body opens on the case index
 * (word at 0x001A79F0, minus one, `sltiu` against 0x14), then `lui $v0,0x2A` /
 * `sll $v1,$v1,2` / `addiu $v0,$v0,0x4420` / `addu` / `lw $a0,0x0($v1)` / `jr $a0`:
 * a 20-entry table at 0x002A4420 that the linker cannot place, so no C spelling reaches it.
 * The five arms read $gp+0x5A8 .. $gp+0x5E0 (five per-level pointer blocks, 8 bytes apart
 * for the first four and 0x10 for the last) and all converge on
 * func_2B0C28(*(block + (*(s32 *)(a1 + 0x10) << 2)), 4, 1) with three scalars.
 * Kept as data for the maintainers.
 */
