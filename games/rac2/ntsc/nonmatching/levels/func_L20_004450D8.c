/* Near miss from src/levels/20_yeedil/L20_0043ECB0.c (wave 3, 20_yeedil, 2026-10-20,
   worker 02): 232 bytes of retail, 244 bytes of candidate.
 *
 * The body is recovered.  It is the id-list walk of L01_003CAFE0-g8.c /
 * L04_00422A00-g8.c with the walk's extra body:
 *
 *   void func_L20_004450D8(char *arg0) {
 *       u32 idx = *(u8 *)(arg0 + 0x21);          // the level's own index byte
 *       u16 *rec;
 *
 *       if (idx == 0xFF) return;
 *       rec = (u16 *)D_L20_001FBB20[idx];
 *       for (;;) {
 *           u32 idx2 = *(u8 *)(arg0 + 0x21);     // re-read every pass
 *           if (idx2 == 0xFF) break;
 *           if (D_L20_001B2FB8 < idx2) break;    // the limit through $gp
 *           if (rec == 0) break;
 *           if (*(s16 *)(rec - 1) < 0) {
 *               u32 *slot = (u32 *)((char *)D_L20_001FBB20 + idx2 * 4);
 *               if (rec != (u16 *)slot[0]) break;
 *           }
 *           {
 *               u16 w = *rec;
 *               char *e = (char *)D_L20_001B2F9C + ((w & 0x7FFF) << 8);
 *               rec++;
 *               if (e == 0) break;
 *               if (*(s16 *)(e + 0xAA) != 0xD0F) continue;
 *               {
 *                   char *o = *(char **)(e + 0x68);
 *                   if (o == 0) continue;
 *                   if (*(u8 *)(o + 0x302) & 1) {
 *                       if (*(u32 *)(o + 0x420) != 1) continue;
 *                       *(u8 *)(o + 0x302) = 2;
 *                       *(u16 *)(e + 0x34) &= 0xFFFD;
 *                   }
 *               }
 *           }
 *       }
 *   }
 *
 * The reading of the retail:
 *
 *   004450D8  daddu   $a3, $a0, $zero      the object
 *   004450DC  addiu   $v1, $zero, 0xFF
 *   004450E0  lbu     $v0, 0x21($a3)       the index byte
 *   004450E4  beq     $v0, $v1, END
 *   004450E8    lui    $v1, %hi(D_1FBB20) the table's %hi, in the delay slot
 *   004450EC  sll     $v0, $v0, 2
 *   004450F0  addiu   $a0, $v1, %lo(D_1FBB20)
 *   004450F4  addu    $v0, $v0, $a0
 *   004450F8  lw      $a2, 0x0($v0)        the record
 *   004450FC  b       .L00445170           the jump to the loop's test
 *   00445100    daddu  $t0, $v1, $zero     the %hi copied, in the delay slot
 *   .L00445108:                            the loop's body
 *   00445108  lui     $v1, %hi(D_1B2F9C)   the object bank
 *   0044510C  lw      $v1, %lo(D_1B2F9C)($v1)
 *   00445110  andi    $v0, $v0, 0x7FFF     the id
 *   00445114  sll     $v0, $v0, 8
 *   00445118  addu    $a1, $v1, $v0        the entry
 *   0044511C  beqz    $a1, END
 *   00445120    addiu  $v0, $zero, 0xD0F
 *   00445124  lh      $v1, 0xAA($a1)       the entry's id
 *   00445128  bnel    $v1, $v0, .L00445170
 *   0044512C    addiu  $a2, $a2, 0x2       the record advance, in the delay slot
 *   00445130  lw      $a0, 0x68($a1)       the entry's object
 *   00445134  beql    $a0, $zero, .L00445170
 *   00445138    addiu  $a2, $a2, 0x2
 *   0044513C  lbu     $v0, 0x302($a0)
 *   00445140  andi    $v0, $v0, 0x1
 *   00445144  beqz    $v0, .L0044516C
 *   00445148    addiu  $v0, $zero, 0x1
 *   0044514C  lw      $v1, 0x420($a0)
 *   00445150  bnel    $v1, $v0, .L00445170
 *   00445154    addiu  $a2, $a2, 0x2
 *   00445158  addiu   $v1, $zero, 0x2
 *   0044515C  sb      $v1, 0x302($a0)
 *   00445160  lhu     $v0, 0x34($a1)
 *   00445164  andi    $v0, $v0, 0xFFFD
 *   00445168  sh      $v0, 0x34($a1)
 *   .L0044516C:
 *   0044516C  addiu   $a2, $a2, 0x2       the record advance
 *   .L00445170:                            the loop's test
 *   00445170  lbu     $v1, 0x21($a3)       the index byte, read again
 *   00445174  addiu   $v0, $zero, 0xFF
 *   00445178  beq     $v1, $v0, END
 *   0044517C    lw     $v0, 0x3FC8($gp)   the limit through $gp, in the delay slot
 *   00445180  slt     $v0, $v0, $v1
 *   00445184  bnez    $v0, END
 *   00445188  nop
 *   0044518C  beqz    $a2, END
 *   00445190  nop
 *   00445194  lh      $v0, -0x2($a2)      the record just consumed
 *   00445198  bgezl   $v0, .L00445108
 *   0044519C    lhu    $v0, 0x0($a2)
 *   004451A0  sll     $v0, $v1, 2          the table lookup
 *   004451A4  addiu   $v1, $t0, %lo(D_1FBB20)
 *   004451A8  addu    $v0, $v0, $v1
 *   004451AC  lw      $a0, 0x0($v0)
 *   004451B0  beql    $a2, $a0, .L00445108
 *   004451B4    lhu    $v0, 0x0($a2)
 *   .L004451B8:
 *   004451B8  jr      $ra
 *   004451BC  nop
 *
 * What the candidate gets wrong is the LAYOUT, not the body: the retail's entry jumps
 * to the loop's TEST and the body's block sits BEFORE the test's block (the test is
 * the loop's header and the two back edges go back to the body), while this cc1
 * rotates the loop the other way and lays the test's block first.  The candidate is
 * 244 bytes against the retail's 232.
 *
 * Measured: the index byte must be read with `lbu` (`*(u8 *)`, not `char *` - the
 * compiler's `char` is signed and gives `lb`); the record advance has to sit BEFORE
 * the body's tests in the source (the retail duplicates it into the delay slots of
 * the three exits that skip it); the `& 0xD0F` test is on the SIGNED halfword at
 * 0xAA; the `0x302` byte test is `& 1`; the `0x420` word is compared against 1; the
 * `0x34` halfword is masked with 0xFFFD.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;

extern u32 D_L20_001FBB20[] __attribute__((nosda));

extern u32 D_L20_001B2F9C __attribute__((nosda));

extern u32 D_L20_001B2FB8 __attribute__((sda));

void func_L20_004450D8(char *arg0) {
    u32 idx = *(u8 *)(arg0 + 0x21);
    u16 *rec;
    u32 *slot;

    if (idx == 0xFF) {
        return;
    }
    rec = (u16 *)D_L20_001FBB20[idx];
    for (;;) {
        u32 idx2 = *(u8 *)(arg0 + 0x21);
        u16 w;

        if (idx2 == 0xFF) {
            break;
        }
        if (D_L20_001B2FB8 < idx2) {
            break;
        }
        if (rec == 0) {
            break;
        }
        if (*(s16 *)(rec - 1) < 0) {
            slot = (u32 *)((char *)D_L20_001FBB20 + idx2 * 4);
            if (rec != (u16 *)slot[0]) {
                break;
            }
        }
        w = *rec;
        rec++;
        {
            char *e = (char *)D_L20_001B2F9C + ((w & 0x7FFF) << 8);

            if (e == 0) {
                break;
            }
            if (*(s16 *)(e + 0xAA) != 0xD0F) {
                continue;
            }
            {
                char *o = *(char **)(e + 0x68);

                if (o == 0) {
                    continue;
                }
                if (*(u8 *)(o + 0x302) & 1) {
                    if (*(u32 *)(o + 0x420) != 1) {
                        continue;
                    }
                    *(u8 *)(o + 0x302) = 2;
                    *(u16 *)(e + 0x34) &= 0xFFFD;
                }
            }
        }
    }
}
