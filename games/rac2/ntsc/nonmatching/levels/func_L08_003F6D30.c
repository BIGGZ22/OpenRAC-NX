/* func_L08_003F6D30 (228 bytes, 1 place) - TI128 copies at offset 0 plus a div.s; no C
 * measured this wave.
 *
 * The four lq/sq pairs are 0x78C20000/0x7CE20000 for a0+0xF0 out of s0+0xF0 and
 * 0x7A020000/0x7C620000 for a0+0x10 out of s0+0x10, each with the pointers materialised
 * first (addiu $a3,$a0,0xF0, addiu $a2,$s0,0xF0, addiu $v1,$a0,0x10, addiu $s0,$s0,0x10) -
 * the reachable shape in principle, but func_L00_004282F0 measured that this cc1 folds the
 * displacements anyway.
 * The rest is plain: $s1 = *(void**)(a0+0x68), a -1 sentinel held in $s2, lw $s0,0x39EC($gp)
 * as the block base, func_31D1D8(0x131D) with the sll $v0,$v0,8 index, sh/sb/ld/sw field
 * copies, a div.s of *(f32*)(s0+0x2C) by *(f32*)(*(void**)(s0+0x24)+0x24) (note the two
 * nops cc1 leaves before it), func_31FC78, then func_31D540 guarded by the same $s2
 * sentinel and sw $s2,0x2C($s1).
 */
