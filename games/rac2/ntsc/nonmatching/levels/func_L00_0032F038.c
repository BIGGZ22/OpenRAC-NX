/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x4 differs.
   Exact size (68 bytes), 15 of 17 words identical, and the source is understood: the field at
   +0x190 is lowered to the argument when the argument is below it, the difference against the
   +0x18E short is wrapped modulo 30 and indexes the float table at +0x258.
   The two words left are the order of the first two instructions: retail emits
   `addiu $a3,$zero,0x1E` (the wrap constant, which the modulo needs in a register) BEFORE the
   `lui $a2,hi` half of the base address; this cc1 always emits it between the lui and the
   matching addiu. Five source shapes (wrap local, pointer local, operand order, decimal
   literals, named difference) all give the same pair of words.
   `if (value <= limit)` is what reproduces retail's `slt+movz` lowering of the clamp (a plain
   `if (limit > value)` gives `slt+movn` with the operands swapped). */

typedef struct {
    char gap_L00_0032F038[0x18E];
    short field_18E;
    int field_190;
    char gap2_L00_0032F038[0xC4];
    float table_258[0x1E];
} Table_L00_0032F038;

extern Table_L00_0032F038 D_00138180;

float func_L00_0032F038(int value)
{
    int limit = D_00138180.field_190;

    if (value <= limit)
        limit = value;
    return D_00138180.table_258[(D_00138180.field_18E - limit + 0x1E) % 0x1E];
}
