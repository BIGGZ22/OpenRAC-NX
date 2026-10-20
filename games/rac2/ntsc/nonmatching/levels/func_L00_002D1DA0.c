/* NEAR MISS - wave 11 S4.  308 bytes against retail's 312 (one instruction short);
   68 of the 78 instructions are byte-identical, and the remaining difference is a
   reordering of six instructions in one block plus one missing instruction.

   Retail body (level 00, 0x002D1DA0, 312 bytes, 27 places):

     002D1DA0  addiu  $sp,$sp,-0x80 / sd $s3..$s4 / sd $s1 / sd $s2 / sd $s0 / sd $ra
     002D1DC4  jal   func_L00_002A4EF0        (key = the current key)
     002D1DC8  daddu $s1,$zero,$zero           (count = 0)
     002D1DCC  daddu $s2,$v0,$zero             (key)
     002D1DD0  jal   func_L00_002ADEA0(3)      (mode)
     002D1DDC  bne   $v0,$v1 -> 0x002D1DEC     (mode != 2 -> the second test)
     002D1DE4  addiu $s0,$gp,-0x7290           (table = &D_L00_001A7D60)
     002D1DE8  b     -> 0x002D1E24
     002D1DF0  jal   func_L00_002ADEA0(3)
     002D1DFC  bne   $v0,$v1 -> 0x002D1E10     (mode != 3)
     002D1E04  addiu $s0,$gp,-0x72A8           (table = &D_L00_001A7D48)
     002D1E10  addiu $s0,$gp,-0x7278           (table = &D_L00_001A7D78)
     002D1E14  jal   func_L00_002ADEA0(3)
     002D1E1C  xori  $v0,$v0,0x4
     002D1E20  movn  $s0,$zero,$v0             (mode ^ 4 -> table = 0)
     002D1E24  beqz  $s0 -> 0x002D1E70
     002D1E2C  lh    $v1,0($s0)                (table->key)
     002D1E30  beq   $v1,$v0 -> 0x002D1E70     (table->key == -1 -> out)
     002D1E34  daddu $a1,$zero,$zero          (idx = 0)      <-- delay slot
     002D1E38  sll   $v0,$s1,2                (count * 4)
     002D1E3C  addiu $a2,$zero,-0x1           (the loop's -1)
     002D1E40  addu  $a0,$v0,$sp              (the &buf[count] pointer)
     002D1E44  daddu $v1,$s0,$zero            (p = table)
     002D1E48  lh    $v0,0($v1)
     002D1E50  bne   $s2,$v0 -> 0x002D1E64    (key != p->key -> skip the store)
     002D1E58  sw    $a1,0($a0)               (buf[count] = idx)
     002D1E5C  addiu $s1,$s1,0x1              (count++)
     002D1E60  addiu $a0,$a0,0x4              (the buf pointer + 4)
     002D1E64  lh    $v0,0($v1)
     002D1E68  bne   $v0,$a2 -> 0x002D1E50    (p->key != -1 -> loop)
     002D1E6C  addiu $a1,$a1,0x1              (idx++)
     002D1E70  bnez  $s1 -> 0x002D1E88        (count != 0)
     002D1E74  addiu $a1,$s1,-0x1             (idx = count - 1)  <-- delay slot
     002D1E78  addiu $v1,$zero,0x1
     002D1E7C  daddu $v0,$zero,$zero
     002D1E80  b     -> 0x002D1EB8
     002D1E84  sw    $v1,0($s3)               (*a1 = 1)
     002D1E88  jal   func_002A7820(0)         (the index into buf)
     002D1E90  sll   $v0,$v0,2
     002D1E94  addu  $a0,$sp,$v0
     002D1E98  lw    $v1,0($a0)               (v1 = buf[index])
     002D1EA0  sll   $v1,$v1,2
     002D1EA4  addu  $v1,$v1,$s0
     002D1EA8  lbu   $a0,2($v1) / sw $a0,0($s3)
     002D1EB0  lbu   $a1,3($v1) / sw $a1,0($s4)
     002D1EB8  the epilogue (sd $s0..$s4, $ra; jr $ra)

   The table is an array of 4-byte records {short key; unsigned char a; unsigned char b;}
   at 0x001A7D60 (mode 2), 0x001A7D48 (mode 3) and 0x001A7D78 (default); the function
   records the table positions whose key matches, then copies the two trailing bytes
   of the record the index selects.

   What is still missing (everything else is byte-identical):

   1. `addiu $a1,$s1,-0x1` at 0x002D1E74, in the delay slot of the `bnez $s1`.  It is
      `idx = count - 1` and its value is never read again - retail keeps it, this cc1
      removes it as a dead store.  Keeping it needs a later use of `idx`; every use
      tried (idx in the second phase's index, idx >= 0 as the test, a fresh `last`)
      either folds away or pushes `idx` into a callee-saved register, which changes the
      frame (144 bytes instead of 128) and the whole register assignment.
   2. The order of the six instructions of the loop's pre-header.  Retail:
      `daddu $a1,$zero,$zero` (idx = 0), `sll $v0,$s1,2`, `addiu $a2,$zero,-0x1`,
      `addu $a0,$v0,$sp`, `daddu $v1,$s0,$zero`, `lh $v0,0($v1)`.  This build:
      `sll`, `daddu $a1,$zero,$zero`, `addu $a0,$v0,$sp`, `daddu $v1,$s0,$zero`,
      `addiu $a2,$zero,-0x1`, `lh` - the same six instructions, scheduled differently
      (the `sll` takes the delay slot instead of `idx = 0`).

   Compiler habits measured while writing this (all confirmed on this cc1):

   * `movz` and `movn` ARE emitted: `if (c) x = 0;` gives `li $2,5; movn $2,$0,$4`
     and `x = c ? 0 : x;` gives the same.  `if (c == 0) x = 0;` gives `movz`.
   * The address of a small-data object comes out as ONE instruction,
     `addiu $reg,$gp,%lo(sym)`, when the object is a VAR_DECL of at most 8 bytes
     declared `__attribute__((sda))` and its address is taken (`&sym`); an array
     of the same name decays to `la` and gives `lui`+`addiu` instead.  So the three
     tables here are declared as single `Elem` objects, not arrays.
   * The value of a small-data object is `lw $reg,sym($gp)`; an unsized declaration
     never takes the gp-relative form (the brief's "needs a size" rule).
   * An `else if` chain whose middle branch assigns a pointer is cross-jumped by
     this cc1 (the assignment is hoisted in front of the test and the branch flips
     to `beq`); a chain of FOUR levels keeps the retail layout
     (`bne` to the else, the then-block falling through, `b` to the join).  The
     chain here is written with four levels to get retail's shape.
   * A call to an unprototyped function with an argument zero emits
     `daddu $a0,$zero,$zero` in the delay slot; with no argument it emits a `nop`.
   * A local declared inside a block keeps a scratch register; the same local at
     function scope gets a callee-saved one and the frame grows by 16 bytes.

   30 compiles.  The body above is the best of them.
*/
#include "include_asm.h"

typedef struct {
    short key;
    unsigned char a;
    unsigned char b;
} Elem;

extern Elem D_L00_001A7D60 __attribute__((sda));
extern Elem D_L00_001A7D48 __attribute__((sda));
extern Elem D_L00_001A7D78 __attribute__((sda));

extern int ref_L00_002D1DA0_00(void);
extern int ref_L00_002D1DA0_01(int);
extern int ref_L00_002D1DA0_02();

int func_L00_002D1DA0(int a0, int *a1, int *a2)
{
    int buf[20];
    Elem *table;
    Elem *p;
    int count = 0;
    int idx;
    int key;

    key = ref_L00_002D1DA0_00();
    if (ref_L00_002D1DA0_01(3) == 2) {
        table = &D_L00_001A7D60;
    } else if (ref_L00_002D1DA0_01(3) == 3) {
        table = &D_L00_001A7D48;
    } else if ((ref_L00_002D1DA0_01(3) ^ 4) != 0) {
        table = 0;
    } else {
        table = &D_L00_001A7D78;
    }
    if (table != 0) {
        if (table->key != -1) {
            idx = 0;
            p = table;
            do {
                if (p->key == key) {
                    buf[count] = idx;
                    count++;
                }
                p++;
                idx++;
            } while (p->key != -1);
        }
    }
    idx = count - 1;
    if (count != 0) {
        int n = ref_L00_002D1DA0_02(0);
        Elem *e = &table[buf[n]];
        *a1 = e->a;
        *a2 = e->b;
        return 1;
    }
    *a1 = 1;
    return 0;
}
