/* DRAFT w49 — best attempt, DOES NOT MATCH: 236 bytes expected, 236 bytes produced,
   only difference = swap of registers $s1/$s2 (pa/i).
   last check.py line: func_L00_003ECC90: 0 of 27 places match
   00_aranos_tutorial: instruction at +0x18 differs

   EVERYTHING else is identical to the retail word for word, including the
   lui/addiu pairs of the symbols, the movn, the loop and the tail block.
   Retail: $s1 = i, $s2 = a/pa. Here: $s1 = pa, $s2 = i. The loop counter must
   win the lowest callee-saved register before the pointer; neither the
   declaration order (done, i, pa, bv, base) nor explicit copies of the
   parameters (pa = a; bv = b;) nor the do/while form changed it.
   Remaining lead: the counter wins if its pseudo is created before the
   parameter's (local-alloc priority), e.g. by using `a` directly without a
   local copy AND declaring i before any use of a.

   The addresses 0x18A020, 0x189EA0 (resident) and 0x1D9F00, 0x1DA860 (level
   data) MUST be symbols: the retail emits `addiu` on the low half (ori would
   be emitted for a literal). The values above are those of level 0. */
extern char D_0018A020[];
extern char D_00189EA0[];
extern char D_L00_001D9F00[];
extern char D_L00_001DA860[];

int func_L00_003D3DE0(char *a, char *b);
int func_L00_003EB610(char *a, char *b);
void func_L00_0035F180(char *a, int b, int c, char *d, int e);

void func_L00_003ECC90(char *a, int b)
{
    int done = 0;
    int i;
    char *pa;
    int bv;
    char *base;

    i = 3;
    pa = a;
    bv = b;
    base = D_0018A020;
    do {
        if (bv == 0xB02) {
            if (func_L00_003D3DE0(*(char **)(base + 0x2290), pa) != 0)
                done = 1;
        } else {
            if (func_L00_003EB610(*(char **)(base + 0x2290), pa) != 0)
                done = 1;
        }
    } while (--i >= 0);
    if (done != 0)
        return;
    func_L00_0035F180((char *)(*(int *)(*(int *)(D_L00_001D9F00 + *(unsigned char *)(D_L00_001DA860 + bv) * 4) + 0x28) + 0xC0),
                      0, 0, D_00189EA0, 0x400);
}
