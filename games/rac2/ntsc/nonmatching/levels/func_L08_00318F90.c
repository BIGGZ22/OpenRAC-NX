/* 0 of 2 places match - last line: 08_tabora: 68 bytes, expected 76 (two words short).
   The function is understood: the resident base pointer at 0x001A904C is read, the index must
   be below four (`sltiu` - the index is unsigned) and the pair is stored at eight-byte stride
   into the table at base + 0x38000 + 0x7370/0x7374, reporting 1 on success and 0 otherwise.
   The literal address is required for the base load: with a named symbol this cc1 splits the
   address into `lui $v0; lw $a3,lo($v0)` where retail keeps the fused `lui $a3; lw $a3,lo($a3)`.
   The two words left are retail's argument copies (`daddu $t0,$a0` before the test with
   `daddu $t1,$a1` in its delay slot) and the extra `daddu $v1,$a0` copy of the table pointer,
   which the nested-test shapes here do not produce. */

extern int D_L08_001A904C;

int func_L08_00318F90(int a0, int a1, unsigned index)
{
    int base = *(int *)0x001A904C;

    if (base != 0) {
        if (index < 4) {
            base += 0x38000 + index * 8;
            *(int *)(base + 0x7370) = a0;
            *(int *)(base + 0x7374) = a1;
            return 1;
        }
    }
    return 0;
}
