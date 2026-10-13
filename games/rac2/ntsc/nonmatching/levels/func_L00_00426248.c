/* NEAR MISS - 8 bytes over, single cause: the LICM wall called out in the BRIEF.
   Last check.py line: func_L00_00426248: 0 of 27 places match
   (00_aranos_tutorial: 256 bytes, expected 248)
   Every instruction of both loop bodies is reproduced; the only difference is where the
   address of the global at 0x1B1880 is materialised:
     mine:   lui  $s7,%hi(D_L00_001B1880)   once, before both loops (kept in a callee-saved
             register: one extra sd/ld pair = the 8 extra bytes), then lw $v1,0($s7)
     retail: lui  $v1,0x1B ; lw $v1,0x1880($v1)   re-materialised inside the inner loop
   The inner loop contains a call (func_00282DF0), so the *load* cannot be hoisted by
   loop.c - only the address computation can, and our compiler keeps it in a register
   where the retail re-materialises it. Declaring `char *base = D_L00_001B1880;` inside
   the inner loop body (tried) still hoists the lui, this time into $fp (268 bytes).
   Structure confirmed by the retail bytes: 64x64 table of int offsets at global 0x1B1880;
   offset 0 means empty; a non-empty block is base+off with an int count at +0, then
   count 48-byte records starting at +16 whose fields at +16 (type) and +20 (index) are
   read; a record with type == 5 dispatches func_00282DF0(table2 + (index << 7)) where
   table2 is the pointer at global 0x1B1F94. All counters are signed (slti/slt), loop
   tests at the bottom with the increment first. */

/* Measured partial layout; original type and game role are unknown. */
typedef struct Rac2Native_428d1e2cc4aa1f21_Entry {
    unsigned char unknown_00[16];
    int type_10;
    int index_14;
    unsigned char unknown_18[24];
} Rac2Native_428d1e2cc4aa1f21_Entry;

extern char *D_L00_001B1880;

extern char *D_L00_001B1F94;

extern float func_00282DF0(float *v);

void func_L00_00426248(void)
{
    int i, j, k;

    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            char *base = D_L00_001B1880;
            int off = ((int *)base)[i * 64 + j];
            char *hdr;
            int count;

            if (off == 0) {
                continue;
            }
            hdr = base + off;
            count = *(int *)hdr;
            if (count > 0) {
                for (k = 0; k < count; k++) {
                    Rac2Native_428d1e2cc4aa1f21_Entry *entry =
                        (Rac2Native_428d1e2cc4aa1f21_Entry *)(hdr + 16) + k;

                    if (entry->type_10 == 5) {
                        func_00282DF0((float *)(D_L00_001B1F94 + (entry->index_14 << 7)));
                    }
                }
            }
        }
    }
}
