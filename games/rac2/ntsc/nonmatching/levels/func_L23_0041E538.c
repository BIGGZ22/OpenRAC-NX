#include "include_asm.h"

typedef unsigned int u32;
typedef int s32;

/*
 * func_L23_0041E538 -- catalogue defect, no C can be placed.
 *
 * 12 bytes at 0x0041E538 in level 23 (23_damosel_orbit), between func_L23_0041E418
 * (which ends at 0x0041E534 with its own `jr $ra` / `addiu $sp,$sp,0x20` epilogue,
 * then a nop) and func_L23_0041E548 (`lw $v0,0x2494($gp)`, a fresh body).
 *
 * retail:
 *   0x0041E538  sltiu $v0, $v0, 0x1     0x2C420001
 *   0x0041E53C  nop                     0x00000000
 *   0x0041E540  sltu  $v0, $zero, $v0   0x0002102B
 *   0x0041E544  nop
 *
 * Reads $v0 (never $a0) and has no `jr $ra` in its extent: it is the tail of an
 * expression (`v0 = (v0 == 0)`) that a neighbour reaches by falling or jumping in,
 * not a function entry. check.py needs the exact 12 bytes, and a C function that
 * takes its input in $v0 and returns nothing cannot produce them. The preceding
 * body ends with its own return, so the extent is a tools/levels.py boundary
 * question (the same shape as func_L01_002E1670 / func_L03_003245B0 in README).
 */

INCLUDE_ASM("asm/levels", func_L23_0041E538);
