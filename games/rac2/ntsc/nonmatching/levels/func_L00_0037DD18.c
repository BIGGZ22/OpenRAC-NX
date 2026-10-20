/* 0 of 27 places match - candidate is 112 bytes against 100.
   The peeled first entry and the "first free entry" search are right (`lw $v1,0x4($v0)` then the
   fill with the triple a1/a0/a2, the search loop from 1 to 31). What differs is address
   materialisation and the loop induction: retail keeps 0x230000 in $t2 AND the table at 0x231340 in
   $v0 (the same page twice), stores the peeled entry as `sw $a1,0x1340($t2)`, and in the loop does
   `addiu $t1,$t2,0x1340` plus `sll $a3,$t0,4` (an index scaled per iteration, no walking pointer);
   this cc1 keeps one base, walks a pointer and adds a second copy of the address, for 12 extra bytes.
   Tried: the entry array at 0x231340 with three store orders, a struct page (char pad[0x1340] +
   entry[32]) over 0x230000, and peeled + `for (i = 1; ...)` against a single loop from 0. */

void func_L00_0037DD18(int a0, int a1, int a2)
{
    int i;

    if (D_00230000.entry[0].field1 == 0) {
        D_00230000.entry[0].field0 = a1;
        D_00230000.entry[0].field1 = a0;
        D_00230000.entry[0].field2 = a2;
        return;
    }
    for (i = 1; i < 32; i++) {
        if (D_00230000.entry[i].field1 == 0) {
            D_00230000.entry[i].field0 = a1;
            D_00230000.entry[i].field1 = a0;
            D_00230000.entry[i].field2 = a2;
            return;
        }
    }
}
