/* func_L23_004256F0 (340 bytes, 1 place: 23:004256F0) -- NEAR MISS, not integrated.
   This C compiles to 332 bytes against 340; check.py reports "332 bytes, expected 340".
   Aligned on the instruction stream, 19 retail words are unmatched (the earlier record's body,
   with `if (s0 == 0) return s0;`, reached 340 bytes only because cse turned that path into a
   separate `return 0` (`b` + `daddu $v0,$zero,$zero`), two wrong words standing in for the two
   missing ones; 20 retail words unmatched). Retail has one return, `daddu $v0,$s0,$zero`, which
   the null test branches to: `if (s0 != 0) { ... } return s0;` as below.
   What still differs:
   - the 16-byte copy: retail computes `addiu $a2,$s0,0x10` early, copies with
     `lq $v0,0($s4) ; sq $v0,0($a2)` and leaves the first call's delay slot empty (`jal ; nop`);
     this cc1 folds the store (`sq $v0,0x10($s0)`) and puts it in that delay slot (-8 bytes);
   - sched1 moves the four stack-argument stores (a9..a12) ahead of the a3..a7 stores, where retail
     keeps source order;
   - the object pointer and a7 take $s3/$s4, retail $s4/$s3 (follows from the schedule: a7's life
     is longer in this order).
   Cause (measured): the copy is the volatile inline-asm macro
       __asm__ __volatile__("lq $2,0(%1)\n\tsq $2,0(%0)" :: "r"(s0 + 0x10), "r"(obj) : "$2")
   With it in place of the TI16 assignment, a scratch copy of the unit is exact (0 words differ,
   derive OK): register roles and store order included. The asm is a scheduling barrier, and it
   also resets sched1's register-use lists, so the stack-argument stores lose the two extra
   dependents described next and fall back to source order. Inline asm is not allowed in src/.
   Why the plain-C order differs: each stack argument is a REG_EQUIV pseudo of its arg slot, so
   sched_analyze counts a use of the arg pointer ($0) at each of its stores; patch 0029 keeps
   call-used hard-register uses across calls, so the 2nd and 3rd calls get anti-dependences on
   those four stores (8 dependents against 6), and rank_for_schedule's dependent count schedules
   them first. As u8 or short the stack arguments lose REG_EQUIV but are loaded at entry into
   callee-saved registers (earlier record).
   Tried without effect: a local copy of obj or of a7 (coalesced, roles unchanged). */

typedef unsigned char u8;

typedef int s32;

typedef int TI16 __attribute__((mode(TI)));

extern void *func_0029EFD0(int size);

extern float func_002A7940(void);

/* allocates the record, fills its sub-record from the arguments, then draws three samples into it */
void *func_L23_004256F0(u8 *obj, u8 *a1, u8 *a2, u8 a3, u8 a4, short a5, short a6, s32 a7, float f1, float f2,
                        float f3, float f4, s32 a9, s32 a10, s32 a11, s32 a12) {
    u8 *s0;
    u8 *v1;

    s0 = func_0029EFD0(0x11AC);
    if (s0 != 0) {
        v1 = *(u8 **)(s0 + 0x68);
        v1[0x28] = a3;
        v1[0x29] = a4;
        *(float *)(v1 + 0x10) = f1;
        *(float *)(v1 + 0x14) = f2;
        *(float *)(v1 + 0x18) = f3;
        *(float *)(v1 + 0x1C) = f4;
        *(short *)(v1 + 0xC) = a5;
        *(short *)(v1 + 0xE) = a6;
        *(s32 *)v1 = a7;
        *(s32 *)(v1 + 4) = a9;
        v1[0x2A] = a10;
        *(s32 *)(v1 + 0x2C) = a11;
        v1[0x2B] = a12;
        s0[0x30] = 0xFF;
        *(u8 **)(s0 + 0xB8) = a1;
        s0[0x20] = 0;
        *(u8 **)(v1 + 0x20) = a2;
        *(u8 **)(v1 + 0x24) = obj;
        *(TI16 *)(s0 + 0x10) = *(TI16 *)obj;
        *(float *)(s0 + 0xF0) = func_002A7940();
        *(float *)(s0 + 0xF4) = func_002A7940();
        *(float *)(s0 + 0xF8) = func_002A7940();
        s0[0x20] = 0;
    }
    return s0;
}
