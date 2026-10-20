/* func_L00_004290C0 (220 bytes, 12 places) - measured 208-212 bytes against 220 for four
 * spellings (scratch/wslcmp.py).  check.py not run.
 *
 * Retained shape: every arm is exact.  The four obj+0xC/0x10/8/0x14 fields are a nested
 * `>= 0` test whose annulled `bltzl` slots carry the NEXT field's load - the exact chain
 * this cc1 produces from a plain `else if` chain.  The constants: 0x00189EA0 (the first
 * argument of all four arms) and the table base 0x001FDA80 must be declared as SYMBOLS;
 * as literals cc1 emits `lui $a0,0x18` / `ori $a0,$a0,0x9EA0` instead of retail's
 * `lui $a0,0x19` / `addiu $a0,$a0,-0x6160` (bit 15 of the low half is set), which is the
 * sixth wave's literal/symbol rule.
 * Residue (12 bytes, 3 words): retail puts the shared `r != 0` normalisation in its own
 * block after the arms (`beqz $v0` + `addiu $v0,$zero,1`, then the +0x38/+0x39 byte-flag
 * path, then a `daddu $v0,$zero,$zero` and the epilogue) and every arm branches there with
 * a bare `b` + `nop`; this cc1 sinks the normalisation into each arm's `b` delay slot as
 * `sltu $v0,$zero,$v0` and branches straight to the epilogue.  Three spellings of the join
 * (`return r != 0;`, `if (r == 0) return 0; return 1;`, a `goto` with one exit label) all
 * give the same sink, and the chain value also moves from retail's $a1 to $v0.
 * To land it: something that stops cc1 sinking the normalisation into the delay slots.
 */
typedef unsigned char u8;
typedef signed short s16;

extern int func_002ACBC8(void *a0);
extern int func_L00_003206B8(void *a0);
extern int func_002ACCA8(void *a0);
extern int func_002A8AF0(void *a0, void *a1, int a2);

extern u8 D_L00_00189EA0[];
extern u8 *D_L00_001FDA80[];

int func_L00_004290C0(u8 *a0) {
    u8 *t = *(u8 **)0x001B1844;
    u8 *p = *(u8 **)(t + (*(s16 *)(a0 + 0x84) << 5) + 0x1C);
    int r;

    if (*(int *)(p + 0xC) >= 0) {
        r = func_002ACBC8(D_L00_00189EA0);
    } else if (*(int *)(p + 0x10) >= 0) {
        r = func_L00_003206B8(D_L00_00189EA0);
    } else if (*(int *)(p + 8) >= 0) {
        r = func_002ACCA8(D_L00_00189EA0);
    } else if (*(int *)(p + 0x14) >= 0) {
        u8 *e = D_L00_001FDA80[*(int *)(p + 0x14)];
        r = func_002A8AF0(D_L00_00189EA0, e + 0x10, *(int *)e);
    } else {
        if (*(u8 *)(p + 0x38) == 0)
            return 0;
        if (*(u8 *)(p + 0x39) == 0)
            return 0;
        return 1;
    }
    return r != 0;
}
