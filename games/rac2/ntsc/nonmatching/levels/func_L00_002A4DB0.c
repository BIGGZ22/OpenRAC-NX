/* func_L00_002A4DB0 (152 bytes, 27 places) -- NEAR MISS, not integrated.
   state: 152 bytes, 8 words differ; 3 are the jal relocations, the other 5 are one
   register-assignment swap.
   check.py (00_aranos_tutorial): every instruction is right except
     002A4DE8  lbu   $v0, 0x10($v1)   |  lbu   $a0, 0x10($v1)
     002A4DF0  lbu   $a0, 0x13($v1)   |  lbu   $v0, 0x13($v1)
     002A4DFC  sll   $a0, $a0, 2      |  (retail: sll $a0,$a0,2 of the 0x10 byte)
   Retail keeps the 0x10 byte in $a0 (the accumulator and the result register) and the
   0x13 byte in $v0; this cc1 picks the two the other way round. The sum, the tree
   ((row + (b10*4 + 0x1C)) + b13*4) and every other instruction agree.

   What is right and measured:
   - typed array indexing is what puts the base first in the two addu:
     `char **rows = (char **)(owner + 0x48); row = rows[a1];` and
     `int *words = (int *)(row + 0x1C); entry = words[a2];`
     flat char* arithmetic gives `addu $a1,$a1,$t0` where retail has `addu $a1,$t0,$a1`
     (2 more words differ; `char *row = *(char **)(owner + 0x48 + a1 * 4);` alone = 10 words).
   Tried and not enough (10 spellings, all 152 bytes / 8 words): explicit parentheses
   around the 0x1C term, the constant written last, the byte terms in the other order,
   latched unsigned char locals, `* 4` instead of `<< 2`, `((unsigned char *)row)[0x10]`,
   struct-typed row/owner, and an int-typed base (`row + 7 * 4`, which is much worse:
   it changes the whole address chain).
   Next idea: a lever that swaps the two byte temporaries' registers - a pure allocation
   tie, the tree and the instruction sequence are already retail's. */

extern void func_002A1130(void *a0, int a1, int a2, int a3);

extern void func_00282B50(int a0, int a1, int a2);

extern void func_0011AEA0(int a0);

/* walks one loaded entry of an owner's list and hands it to the vector routine when the
   tag bits are set, else to the count-driven copy, then polls and returns the caller's
   handle */
int func_L00_002A4DB0(char *a0, int a1, int a2, int a3)
{
    char *owner = *(char **)(a0 + 0x24);
    char **rows = (char **)(owner + 0x48);
    char *row = rows[a1];
    int *words = (int *)(row + 0x1C);
    int entry = words[a2];

    if (entry & 0xF0000000) {
        func_002A1130(row + 0x1C + (*(unsigned char *)(row + 0x10) << 2)
                       + (*(unsigned char *)(row + 0x13) << 2),
                       entry, a3, *(unsigned char *)(owner + 8));
    } else {
        func_00282B50(a3, entry, (*(short *)(entry + 6) + 1) << 4);
    }
    func_0011AEA0(0);
    return a3;
}
