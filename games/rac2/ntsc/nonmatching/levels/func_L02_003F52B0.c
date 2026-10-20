/* Near miss (small-function wave, level 2): 216 bytes against retail's 220, level 2 only.
 *
 * Everything but the 16-byte copy is exact: the 0x20-byte frame with $s0/$ra/$f20, the
 * switch on the byte at +0x20 (case 0 first, case 1 after it, both leaving through the shared
 * epilogue), the four float stores, the `sb 0xFF`, the four calls and the 1.0f kept in $f20
 * across them.
 *
 * What differs is the quadword copy. Retail holds both pointers in registers and copies
 * through them with zero displacements:
 *     addiu $a1,$v1,0xB0      ($v1 = D_00189E20, the base the four stores use)
 *     lui   $a0,%hi(D_L02_001B3210) ; addiu $a0,$a0,%lo(D_L02_001B3210)
 *     lq    $v0,0($a1)
 *     sq    $v0,0($a0)
 * This cc1 folds the constant 0xB0 into the load (`lq $v0,0xB0($v1)`), so the `addiu $a1`
 * is gone (the 4 missing bytes), the destination pointer is built earlier, and the rest of
 * the scheduling and the $v1/$a1/$a2 choices shift with it (38 differing lines of the diff).
 * This is the known constant-offset tie of cse's find_best_addr (docs/COMPILER-NOTES.md,
 * "The constant-offset wall"): a register and a register plus a small constant cost the same
 * and the sum wins.
 *
 * Tried, all 216 bytes: the base as a pointer variable `g` with `*(TI128 *)(g + 0xB0)`; the
 * source as its own `TI128 *src` variable set before or after the stores; `char *s = g + 0xB0`;
 * the source as the constant `D_00189E20 + 0xB0`; all 24 source orders of the four float
 * stores (retail emits B4, B8, B0, BC; the scheduler gives only B0 B4 B8 BC, B0 B4 BC B8,
 * B8 B4 B0 BC or B8 B4 BC B0 here, never retail's, because in retail the separate
 * `addiu $a1` is what lets the stores move).
 *
 * What would unblock it: a spelling of the copy that makes cse keep `$v1 + 0xB0` as a value
 * (retail's shape is what an asm quadword-copy macro with "r" operands produces, which
 * src/ does not allow).
 */

typedef unsigned char u8;
typedef int TI128 __attribute__((mode(TI)));

extern char D_00189E20[];
extern char D_L02_001B3210[];
extern char *D_L02_001AD410;
extern void func_L02_003F5390(char *);
extern void func_L02_003F54D8(char *);
extern void func_L02_003F56C0(char *);
extern void func_L02_003F5710(char *);
extern void func_002833B8(char *, float);
extern void func_002A7750();

void func_L02_003F52B0(char *arg0) {
    switch (*(u8 *)(arg0 + 0x20)) {
    case 0: {
        char *g = D_00189E20;

        *(float *)(g + 0xB4) = 200.0f;
        *(float *)(g + 0xB8) = 500.0f;
        *(float *)(g + 0xB0) = 500.0f;
        *(float *)(g + 0xBC) = 1.0f;
        *(TI128 *)D_L02_001B3210 = *(TI128 *)(g + 0xB0);
        *(u8 *)(arg0 + 0x30) = 0xFF;
        func_L02_003F5390(arg0);
        func_002833B8(D_L02_001AD410 + 0x40, 1.0f);
        func_002A7750(arg0, 1, -1);
        break;
    }
    case 1:
        func_L02_003F54D8(arg0);
        func_L02_003F56C0(arg0);
        func_L02_003F5710(arg0);
        break;
    }
}
