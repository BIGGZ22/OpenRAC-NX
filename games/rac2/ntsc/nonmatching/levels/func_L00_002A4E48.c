/* func_L00_002A4E48 (164 bytes, 14 places) -- NEAR MISS, not integrated.
   state: 156 bytes against 164; the control flow and the targets all line up, three
   codegen habits do not.
   check.py (00_aranos_tutorial):
     - retail materialises the resident base TWICE, `addiu $a1,$v0,-0x61E0` for the
       0x22B4/0x22BA/0x24EC probes and `addiu $a0,$a2,-0x61E0` (from the same $v0=lui
       0x19) for the 0x24CC/0x1492 tail: one C base gives one register (one instruction
       less). The source likely probes through a local pointer and answers through the
       global, or through two different struct views.
     - retail fills the three branch delay slots with `addiu $v1,$zero,1` / a nop and
       keeps plain `beqz/beqz/bnez`; this cc1 forms `beql/bnel` and duplicates
       `lw $v1,0x24CC($a1)` into all three slots.
     - the table index folds differently: retail `addiu $a0,$a0,0x2570` then
       `lw $v0,0x28($v1)`, ours `addiu $a0,$a0,0x28` (the 0x28 folded into the symbol
       constant). A row struct with the field at 0x28 and sizeof 0xE0 should put the
       0x28 back into the load - not tried.

   What is right and measured:
   - D_00139568 is resident (the same 0x139568 in level 02's copy) - name it D_, not
     D_L00_; D_L00_00262570 is per level (0x264A70 in level 02).
   - `b * 0xE0` compiles to the single R5900 `mult $v1,$a1` (rd = $v1) that retail has,
     and the `.word` rabbitizer flags is that instruction, alive.
   - the branch structure below (arg == 0 falls through to the tail, arg != 0 with both
     resident probes clear returns the table word) reproduces retail's targets, and
     `? 0x67 : 0` gives retail's `movz v0,zero,v1` with the `addiu v0,zero,0x67`
     hoisted into the beq's delay slot. */

extern char D_00189E20[];

extern int D_00139568[];

extern unsigned char D_L00_00262570[];

extern int func_L00_002ADEA0(int index);

/* picks a kind code from the resident status table and returns the matching record word,
   unless a resident flag or the caller's mode routes it to the two fixed answers */
int func_L00_002A4E48(int arg)
{
    char *base = D_00189E20;

    if (*(unsigned char *)(base + 0x22B4) == 1)
        return 0;
    if (arg != 0) {
        if (*(unsigned char *)(base + 0x22BA) == 0 && *(short *)(base + 0x24EC) == 0) {
            int kind = *((unsigned char *)D_00139568 + func_L00_002ADEA0(0));
            return *(int *)(D_L00_00262570 + kind * 0xE0 + 0x28);
        }
    }
    if (*(int *)(base + 0x24CC) == 1)
        return 0x51;
    return *(unsigned char *)(base + 0x1492) ? 0x67 : 0;
}
