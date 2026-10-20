/* func_L20_00447748 (444 bytes, level 20): NOT ATTEMPTED in this wave; the analysis is
 * recorded for the next attempt.
 *
 * The body is recovered from build/20_yeedil/asm/5B0B0.s:
 *  - the base is the boot table D_189E20; `lw $a0,0x1830($a1)` is a pointer at +0x1830;
 *    `lh $v1,0xAA($a0)` compared against 0x45, then the byte at +0x20 against 0xFE and
 *    0xFD, then the word at +0x2294 against 0x31: three early exits;
 *  - the state is arg0[0x68]; the word at +0x18 selects a table entry through
 *    `addiu $t7,$gp,0x3340` (a $gp array), the entry at +0x10 is added to it;
 *  - a fourteen-word structure is then built on the stack and its address (sp) is passed
 *    to one of four functions: func_L04_003A1A78, func_L04_003A5C08, func_L04_003A1C98,
 *    func_L04_003A1FE0 or func_L04_003A1FA0. The structure's words are 6, the computed
 *    value, &D_002AB980, &D_25DD00, &D_25DD60, &D_002B22E0, &f_00447908, &f_002FAE78,
 *    arg0 and four $gp words (gp+0x3340, gp+0x3348, gp+0x3354, gp+0x3358);
 *  - the choice between the four callees is a switch on arg0[0x20] (1, 2, 0 and the
 *    default), written as `beq`/`slti`/`beqz`;
 *  - the last loop walks three words at state+0x30 and calls func_L00_003294F0 with the
 *    $gp word at 0x3FAC.
 *
 * The wall: the fourteen stack stores come in the retail's scattered order (0x0, 0x4, 0x20,
 * 0x24, 0x8, 0xC, 0x10, 0x14, 0x18, 0x1C, 0x2C, 0x30, 0x28) - not the source order - and
 * "the store order is an arbitrary permutation of the source order" (docs/COMPILER-NOTES.md),
 * so the source order has to be found by trying the permutations. The unit also has to be
 * the -G8 sibling (it reads gp+0x3340 and gp+0x3FAC).
 */
