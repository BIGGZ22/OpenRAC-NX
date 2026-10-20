/* func_L18_004273F0 (660 bytes, 3 places) -- NEAR MISS, not integrated. It belongs in
   L18_002C4000-g8.c: the count word at 0x1B2838, the entity bank pointer at 0x1B281C (a second,
   $gp-relative name for it in the tail) and the halfword at 0x1B125C are read through $gp, while
   the walk reads the bank pointer absolutely.

   This C compiles to 660 bytes, the same size as retail, with 11 words different, all of them in
   the first block (+0x48..+0xCC) and all caused by one wall. Retail copies the 16-byte vector at
   table[id] + 0x10 into the local at 0($sp) as
       lw $v1,0($a0) ; addiu $v1,$v1,0x10 ; lq $v0,0($v1) ; sq $v0,0($sp)
   and this cc1 folds the +0x10 into the load: lq $v0,0x10($a0) (the constant-offset wall of
   COMPILER-NOTES: cse's find_best_addr takes the sum over the register that holds it).
   Measured, none of them keeps the addiu: a direct deref, a TI128 pointer variable, a struct
   with a TI member, a TI128 temporary, ((TI128 *)rec)[1], a TI128 local whose address is taken,
   and an in-place `rec += 0x10` (cse records no equivalence for it, but combine merges the add
   into the load when it is the last use of rec). The add survives only when an insn that
   redefines rec, or a volatile access, sits between it and the load, which is not plain C. 24
   cc1 flags keep the folded lq as well (-fforce-addr, -fno-gcse, -fno-cse-follow-jumps,
   -fno-schedule-insns2, -O1, -mno-split-addresses, ...; -fforce-addr also rewrites every other
   address of the function, 700 bytes). The missing addiu then shifts the loop top by one word
   (the assembler pads a nop where retail has the addiu) and the allocator swaps $a0 and $v1 for
   the index and the record pointer. Quadword copies through a held pointer reproduce retail
   only when written as inline asm.

   Everything else is exact: the walk (unrotated, with the double `q != 0` guard), the tail copy
   and the three call endings. What it took:
   - extern const unsigned short D_L18_001B125C: without the const the scheduler issues the
     halfword load one slot after the first word copy; with it the load is independent of the
     stores through bs and goes first, as in retail;
   - the entity of the tail's bank index is spelled (idx << 8) + (int)base (an int sum), which
     gives retail's addu operand order (a pointer + int sum prints base first);
   - the order of the stores to bs (hill-climbed; the load of D_L18_001B125C goes right after
     the load of bs, the 0x38C copy comes after the 0x364 copy and the halfword store);
   - func_L18_0035D5A8 takes a fifth float argument in $f12 (0.0f), which its definition in
     L18_002C4000.c does not declare, so this unit declares it with the float parameter;
   - the callees by their catalogue names: func_0026FEB8 (0x2C27F0), func_002A7750 (0x335668),
     func_00282E80 (0x308AD0), func_L18_00427208. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int TI128 __attribute__((mode(TI)));

extern unsigned int D_L18_001FB320[];
extern char *D_L18_001B281C;
extern char *D_L18_001B281C_sda __attribute__((sda));
extern int D_L18_001B2838 __attribute__((sda));
extern const unsigned short D_L18_001B125C __attribute__((sda));
extern int D_L18_001B2348;
extern char D_L18_002C0A90[];
extern char D_00189EA0[];
extern char *D_001FE480[];

extern void func_0026FEB8(char *fmt, int a1, int a2);
extern float func_L18_0035D5A8(void *a0, void *a1, void *a2, int a3, float f12);
extern void func_002A7750(u8 *a0, int a1, int a2);
extern void func_L18_00427208(u8 *a0);
extern float func_00282E80(void *a0, void *a1);

void func_L18_004273F0(u8 *obj)
{
    u8 *state = *(u8 **)(obj + 0x68);
    u8 *best = 0;
    int found = 0;
    float best_d = 100000.0f;
    float vec[4];
    u16 *q;
    u8 *e;
    u8 *es;
    float d;

    if (*(int *)(state + 0x378) == -1) {
        if (*(int *)(state + 0x368) == -1) {
            func_0026FEB8(D_L18_002C0A90, *(s16 *)(obj + 0xB2), D_L18_001B2348);
        } else {
            func_L18_0035D5A8(vec, obj + 0x10, D_00189EA0, *(int *)(state + 0x368), 0.0f);
            found = 1;
        }
    } else {
        *(TI128 *)vec = *(TI128 *)(D_001FE480[*(int *)(state + 0x378)] + 0x10);
    }

    q = (u16 *)D_L18_001FB320[obj[0x21]];
    while (obj[0x21] != 0xFF && obj[0x21] <= D_L18_001B2838 && q != 0 &&
           ((q != 0 && (s16)q[-1] >= 0) || q == (u16 *)D_L18_001FB320[obj[0x21]])) {
        e = (u8 *)(D_L18_001B281C + ((*q & 0x7FFF) << 8));
        if (e == 0)
            break;
        es = *(u8 **)(e + 0x68);
        d = func_00282E80(e + 0x10, vec);
        if (es != 0 && (es[0x399] & 1) != 0 && d < best_d) {
            best_d = d;
            best = e;
        }
        q++;
    }

    if (best != 0) {
        u8 *bs;
        u16 hw;
        best[0x31] = 1;
        *(u16 *)(best + 0x34) &= 0xFFFE;
        bs = *(u8 **)(best + 0x68);
        hw = D_L18_001B125C;
        *(int *)(bs + 0x36C) = *(int *)(state + 0x36C);
        *(int *)(bs + 0x370) = *(int *)(state + 0x370);
        *(int *)(bs + 0x374) = *(int *)(state + 0x374);
        *(int *)(bs + 0x378) = *(int *)(state + 0x378);
        *(int *)(bs + 0x360) = *(int *)(state + 0x360);
        *(int *)(bs + 0x364) = *(int *)(state + 0x364);
        *(u16 *)(bs + 0x390) = hw;
        *(int *)(bs + 0x38C) = *(int *)(state + 0x378);
        bs[0x39A] = 1;
        bs[0x399] = 2;
        *(u16 *)(bs + 0x392) = state[0x398];
        if (found) {
            func_002A7750(best, 0, 4);
        } else if (bs[0x3EA] != 0 && *(int *)(bs + 0x3B4) != -1 &&
                   ((u8 *)((*(int *)(bs + 0x3B4) << 8) + (int)D_L18_001B281C_sda))[0x20] != 0xFE &&
                   ((u8 *)((*(int *)(bs + 0x3B4) << 8) + (int)D_L18_001B281C_sda))[0x20] != 0xFD) {
            func_L18_00427208(best);
        } else {
            func_002A7750(best, 0, 3);
        }
    }
}
