/* func_L23_004265D0 (456 bytes, 1 place: level 23) -- NEAR MISS, not integrated.
   state: 456 bytes against retail's 456 (114 instructions); 24 words differ, the first at +0x28.
   Checked as tools/check.py does, in a -G8 unit (the float decrement at the end is a bare `$gp`
   access): it belongs in L23_0041E418-g8.c.
   What matches: the frame and the save set ($s0..$s3, $ra), the whole control structure, the four
   `c.olt.s` tests on func_00282E48's result (200, 100, 25, 0 and the stores 4.0, 2.0, 1.0, 0.5),
   the func_00282978 pair, the registration of func_L05_00428518, the `$gp` float read in the first
   branch's delay slot, the `sb` in the first call's delay slot, the order of the three stores.
   What differs: the registers of the first block's temporaries, and the order that follows from
   them; retail has the index in $v0, the first table in $a0, the second in $t0, 0xFF in $a3, 1 in
   $a2 and the reference's high half in $v1. Also the tail: retail issues `lui $v1; addiu $v1` and
   then `lw $v0,0x3C($s0)`, this cc1 puts the `lw` between the two (sched2 ranks the load at
   priority 8 against 6 for the `addiu`, so no tie-break reaches retail's order).

   Why it is not a spelling question (measured 2026-10-20, Opus retry):
   - An offline model of local-alloc's block_alloc for the first block (ties of each result with
     its first dying input, priority floor_log2(refs) * refs / (death - birth), lifetimes widened
     by one insn for the first try, REG_ALLOC_ORDER numeric) reproduces this cc1's allocation
     exactly from its sched1 order. Searched over every dependence-respecting order of the block's
     22 insns (random sampling and hill-climbing), it never gives the second table $t0 (the best is
     $a3): that quantity has 4 references, so it is placed before the 2-reference constants unless
     they live under a quarter of its length, and then they take the low registers themselves.
     With or without the widened lifetimes, and with the two constants set right before their
     stores (what update_equiv_regs does to a constant pseudo set in another block), the best is
     6 of retail's 7 choices. So retail's block had another structure at allocation time (the
     second table's address or the two constants in pseudos that live outside the block, for
     instance), not another order of the same insns.
   - One flag at a time on this C: -fno-schedule-insns (37 words), -fno-schedule-insns2 (33),
     -mno-split-addresses (444 bytes), -fno-expensive-optimizations (452 bytes); -fno-gcse, -g,
     -fno-strength-reduce, -fno-rerun-cse-after-loop, -fno-regmove, -fno-caller-saves,
     -fno-cse-follow-jumps change nothing.
   - `do { } while (0)` around each store, the first two, the last two and all three: 460 bytes
     each (the barrier moves the argument copies).
   - The reference point as `D_L23_001BA500 + 0x140` (it is that struct's field, see
     func_L23_00426248), struct views of the object and its vars, a switch on the state byte,
     the two call arguments as locals after the stores: 25 words each. The locals at the top of
     the block (this C) put 0xFF in retail's $a3: 24 words.
   The earlier ~350 compiles (all six store orders, index and pointer locals, &tab[i], *(tab + i),
   byte-offset spellings, a named pointer for the table address, positions of the locals) mostly
   gave the same bytes.
*/

typedef unsigned char u8;

typedef int s32;

extern float D_L23_001B38E8[6];

extern s32 D_L23_001B3900[6];

extern s32 D_L23_001B3918[6];

extern float D_L23_001BA640[];

extern float D_L23_001B19FC __attribute__((sda));

extern float func_00282E48(void *, void *);

extern s32 func_00282978(void *);

extern void func_L00_002E6DB8(void *, void *);

extern void func_L05_00428518(void *);

extern void func_0029F338(u8 *);

/* first update of a spawned effect: records the slot, grades the distance to the reference point,
   then either hands the object to the shared updater or releases it and gives the slot back */
void func_L23_004265D0(u8 *obj) {
    u8 *pl = *(u8 **)(obj + 0x68);

    if (obj[0x20] == 0) {
        u8 *pos = obj + 0x10;
        float *ref = D_L23_001BA640;

        D_L23_001B38E8[*(s32 *)(pl + 0x3C)] = 0.01f;
        D_L23_001B3900[*(s32 *)(pl + 0x3C)] = 0xFF;
        obj[0x20] = 1;
        if (200.0f < func_00282E48(pos, ref)) {
            *(float *)(pl + 0x40) = 4.0f;
        } else if (100.0f < func_00282E48(pos, ref)) {
            *(float *)(pl + 0x40) = 2.0f;
        } else if (25.0f < func_00282E48(pos, ref)) {
            *(float *)(pl + 0x40) = 1.0f;
        } else if (0.0f < func_00282E48(pos, ref)) {
            *(float *)(pl + 0x40) = 0.5f;
        }
    }
    if (func_00282978(pl) == 0) {
        func_00282978(pl + 4);
        func_L00_002E6DB8((void *)func_L05_00428518, obj);
    } else {
        D_L23_001B3918[*(s32 *)(pl + 0x3C)] = 0;
        D_L23_001B19FC = D_L23_001B19FC - 1.0f;
        func_0029F338(obj);
    }
}
