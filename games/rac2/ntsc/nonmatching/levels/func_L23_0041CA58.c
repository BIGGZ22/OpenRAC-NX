/* func_L23_0041CA58 (564 bytes, 1 place: 23:0041CA58) -- NEAR MISS, not integrated.
   This C compiles to 564 bytes, the retail size (a -G0 unit: no $gp); check.py reports
   "instruction at +0x8 differs" (the prologue save order), 0 of 1 places; 7 mnemonics and the
   register assignment still differ.
   The function: it prepares the state at +0x68 (func_L23_0041CCC0, a func_002A77E0(3) whose result is
   dropped, func_00115484(picks, -1, 0x1C)), builds 47 running sums over the table
   D_L23_001FF300[state[+0x230 + 4i]] (each entry's first word is its count; the sum steps by
   count - 1), then seven times draws a number with func_002A77E0(total) until it differs from the
   picks kept so far, finds the bucket whose sum range holds it (the last bucket, i == 46, takes
   the rest and reads state[+0x2E8]) and copies the 16 bytes at bucket entry + 0x10 into a local
   vector, and for every bucket but 10, 15, 27, 28 and 31 passes that vector to
   func_L23_0041C7D8 (the `r % 3` is dead, only its divide-by-zero guard `beql $fp,$zero ; break`
   survives, which is why 3 sits in $fp). The 0x20 bytes of `pad` only reproduce retail's frame
   (it has 0x20 bytes after the vector that nothing here names).
   What measured, because it was the wall of this body: retail's quadword copy has no folded offset
   (`addiu $v1,$v1,0x10 ; lq $v0,0($v1) ; sq $v0,0($t2)` in one tail shared by both buckets), and
   this cc1 folds constant offsets into lq/sq. Two things get past it in plain C:
   - the source pointer is set in both arms and used after the join (`base += 0x10` in each arm,
     the copy under one label): a TImode access whose address is a register defined before a join
     keeps `lq $v0,0($v0)`; jump cross-jumping then merges the two `addiu 0x10` into the shared tail,
     exactly as retail has it;
   - the loop shape is retail's only when the loop test is rolled to the bottom (stmt.c moves the
     leading exit tests: `i < 47`, then `i == 46`, which is why the 46 case is a `break` first)
     and the loop-test duplication is refused (a label, here `copy:`, inside the rolled block), so the
     entry is retail's `b <bottom>`; `n = i + 1` before the tests is what gives its `addiu $a1,$a2,1`
     / `daddu $a2,$a1,$zero` pair, and one shared counter `i` for the first loop, the pick loop and
     the search is what puts all three in $a2.
   What still differs: retail's destination address `addiu $t2,$sp,0xE0` (hoisted into the search
   loop's preheader, a temporary) is the user variable `dst` here and stays in the loop's bottom
   block; retail's `daddu $t3,$s5,$zero` copy of the counts pointer in that preheader has no
   source spelling that survives copy propagation; and the registers ($s4/$s5 swapped, $v1 for j,
   $a1 for r where retail has $a2/$a3). On a private cc1 whose TImode addresses must be plain
   registers the temporary hoists as in retail (without `dst`, with the copy written inline) and
   the residue is the counts copy, a `nop` and the bottom block's order. */

typedef int TI128 __attribute__((mode(TI)));

extern char *D_L23_001FF300[];

extern void func_L23_0041CCC0(char *);

extern void func_L23_0041C7D8(char *, int);

extern int func_002A77E0(int);

extern void func_00115484(char *, int, int);

void func_L23_0041CA58(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int *cnt = (int *)(state + 0x230);
    int sums[47];
    int picks[7];
    float F[4];
    float pad[8];
    TI128 *dst;
    char *base;
    int total = 0;
    int found;
    int dead;
    int t;
    int r;
    int i;
    int j;
    int t46;
    int n;

    func_L23_0041CCC0(arg0);
    func_002A77E0(3);
    func_00115484((char *)picks, -1, 0x1C);
    for (i = 0; i < 47; i++) {
        sums[i] = total;
        t = total - 1;
        total = t + *(int *)D_L23_001FF300[cnt[i]];
    }
    for (j = 0; j < 7; j++) {
        found = 1;
        do {
            r = func_002A77E0(total);
            for (i = 0; i < 7; i++) {
                if (r == picks[i]) {
                    found = 1;
                    break;
                }
                if (i == 6)
                    found = 0;
            }
        } while (found == 1);
        picks[j] = r;
        t46 = (r - sums[46]) * 16;
        for (i = 0; i < 47; i++) {
            dst = (TI128 *)F;
            n = i + 1;
            if (i == 46) {
                base = D_L23_001FF300[*(int *)(state + 0x2E8)] + t46;
                base += 0x10;
    copy:
                *dst = *(TI128 *)base;
                break;
            }
            if (r >= sums[i] && r < sums[n]) {
                base = D_L23_001FF300[cnt[i]] + (r - sums[i]) * 16;
                base += 0x10;
                goto copy;
            }
        }

        if (i != 0xA && i != 0x1B && i != 0x1C && i != 0x1F && i != 0xF) {
            dead = r % 3;
            func_L23_0041C7D8((char *)F, 0);
        }
    }
}
