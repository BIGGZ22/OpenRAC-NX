/* Best attempt, 2 compiles. tools/check.py: 204 bytes against a 212-byte
   retail body. The whole linked-list block reproduces byte for byte (the
   `lh 0xAA / xor / movz` if-conversion, the two padding nops, the `bnel` with
   the next `lh` in its delay slot, the `sw` delay). Two measured gaps left:
   1. the 16-byte copy at +0x10: the retail materialises BOTH addresses
      (`addiu $a0,$s1,0x10` / `addiu $v1,$s0,0x10`, then `lq/sq` at 0) while
      gcc folds the offsets into the `lq/sq` (`lq $s1,0x10($s0)`), which is the
      2-instruction shortfall and the reason the size is 204, not 212. This is
      exactly what the repo's qcopy()/qzero() helpers in include/common.h exist
      for, but they are inline asm and AGENTS.md forbids inline assembly in a
      function, so the plain-C spelling below cannot reach it. A worker allowed
      to use the repo helper should close this in one try.
   2. `lw $v0,0x2310($gp)` vs the `swc1 $f0,0xF8($s0)` below it: the retail
      hoists the load and puts the store in the `bnez` delay slot; this source
      gets the opposite schedule.
   BELONGS IN THE -g8 SIBLING FILE: `lw $v0,0x2310($gp)` is a $gp access, and
   D_L05_001B2528 must stay a non-small `char *[]` so gas emits `lui+lw`
   (a scalar `char *` declaration under -G8 would go $gp-relative).
   The `if (p != 0) { ... return p; } return 0;` form is the one that produces
   the retail `beqz $s0` to the out-of-line return 0. */

extern void *func_0029EFD0(int);

extern float func_002A7940(void);

extern char *D_L05_001B2528[];

extern int D_L05_001B1300 __attribute__((sda));

extern void func_L05_0041C410(char *);

typedef int TI128 __attribute__((mode(TI)));

void *func_L05_0041C118(char *object)
{
    char *p = func_0029EFD0(0x10CF);
    char *e;
    char *found;

    if (p != 0) {
        *(short *)(p + 0x32) = 0xFF;
        *(char *)(p + 0x30) = 0xFF;
        *(char *)(p + 0x31) = 1;
        *(TI128 *)(p + 0x10) = *(TI128 *)(object + 0x10);
        func_00282C88(p + 0xF0);
        *(float *)(p + 0xF8) = func_002A7940();
        if (D_L05_001B1300 == 0) {
            e = D_L05_001B2528[0];
            if (e != 0) {
                found = 0;
                do {
                    if (*(short *)(e + 0xAA) == 0xFBF)
                        found = e;
                    e = *(char **)(e + 0x28);
                } while (e != 0);
                D_L05_001B1300 = (int)found;
            }
        }
        func_L05_0041C410(p);
        *(float *)(p + 0x2C) = *(float *)(*(char **)(p + 0x24) + 0x24);
        return p;
    }
    return 0;
}
