/* NOT A FUNCTION AS CATALOGUED: the row starts 16 bytes before the entry. Retail's bytes at 0x35BD60..0x35BD6F are
   `sw $t3,0x40($v1); nop; addiu $sp,$sp,0x20; nop` (they read registers this code never sets, and no jump, branch,
   call or data word of level 2 targets them: padding left between two functions); the real function begins at
   0x35BD70 (`lui $a1,0x14`), which is also what the data word at 0x289490 points to. Fix: add
   (2, 0x0035BD60): 16 to LEVEL_ENTRY_OFFSETS in tools/levels.py and rerun `tools/levels.py catalogue`. No C can
   produce those 16 bytes, so tools/check.py cannot pass before that.

   The body below is for the 272 bytes from 0x35BD70 (it needs the -g8 sibling: two `lw $a0,-0x47C0($gp)`). It is a
   near miss with the right size (272) and the first 36 words exact (the whole table-copy loop and its preheader).
   It copies up to 28 12-byte records from the table at 0x28DA20 into the one at 0x21E630, picking them by the
   zero-terminated index list at 0x139948 and ending the table with a zero; then it clears the word at +0x40 of the
   object, sets bit 15 of the word at +0x30, and looks for the first entry of the $gp-held list that equals the word
   at 0x1CAD10 (its index goes to +0x40).
   What differs, all in the search: (1) retail loads the list pointer (`lw $a0,-0x47C0($gp)`) before the stores to the
   object and reloads it (`lw $v1,-0x47C0($gp)`) in the loop; this cc1 loads it after the stores and keeps it in a
   register (`daddu $a1,$a0,$zero` in the first branch slot), so the loop head is `addiu $t1,$t1,1` instead of the
   reload and the first branch slot holds the copy instead of `addiu $t1,$t1,1`; (2) the kept `lui` of the compared
   word's base is in $a2 here and in $a1 there.
   Needed to get this far: the compared word's base as `char *g = D_L02_001CAAE0` declared inside the loop (gives
   `lui`, kept copy, `addiu` + `lw 0x230` per iteration), the two table bases as separate `ids` / `vals` pointers
   declared inside the copy loop's body (else they are computed before the guard), one variable `i` for both loops.
   Tried against (1): `int *p = D_L02_001AA830` before the stores (loads too early, before the copy loop), the pointer
   assigned twice, `const` on the $gp pointer (hoists everything), a struct-typed object argument, the search as
   `for`, `while`, `do-while`, `goto` and hand-peeled first iteration. */
typedef unsigned short u16;
typedef short s16;

typedef struct {
    u16 id;
    s16 kind;
    int ptr;
    u16 val;
    u16 pad;
} Dst12;

extern int D_L02_00139948[];

extern char D_L02_0028DA20[];

extern Dst12 D_L02_0021E630[];

extern char D_L02_002894E0[];

extern int *D_L02_001AA830 __attribute__((sda));

extern char D_L02_001CAAE0[];

int func_L02_0035BD60(char *self) {
    int i = 0;

    while (i < 0x1C && D_L02_00139948[i] != 0) {
        char *ids = D_L02_0028DA20;
        char *vals = ids + 4;

        D_L02_0021E630[i].kind = 3;
        D_L02_0021E630[i].ptr = (int)D_L02_002894E0;
        D_L02_0021E630[i].id = *(u16 *)(ids + D_L02_00139948[i] * 12);
        D_L02_0021E630[i].val = *(u16 *)(vals + D_L02_00139948[i] * 12);
        i++;
    }
    D_L02_0021E630[i].id = 0;
    *(int *)(self + 0x40) = 0;
    *(int *)(self + 0x30) |= 0x8000;
    i = 0;
    while (D_L02_001AA830[i] != 0) {
        char *g = D_L02_001CAAE0;

        if (*(int *)(g + 0x230) == D_L02_001AA830[i]) {
            *(int *)(self + 0x40) = i;
            break;
        }
        i++;
    }
    return 0;
}
