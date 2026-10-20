/* 0 of 1 places match - last line: 30_insomniac_museum: instruction at +0x24 differs
   Claims the first free slot of four parallel 32-entry tables: stores the three
   arguments in 0x260E30[i] / 0x260EB0[i] / 0x1B3D30[i], the two trailing ones in
   0x260F30[i*2] / 0x260F30[i*2+1], bumps the owner's counter at (0x68-word)+4 and
   returns; falls through when all 32 slots are taken. Retail materialises the
   bases in the order t3 = 0x260F30, v1 = 0x260EB0, a0 = 0x260E30, i(t2) = 0,
   t5 = 0x1B3D30, and runs the counter in $t2 in the `bnel` delay slot. Array
   spelling, pointer spelling and a re-ordered pointer spelling all reach the same
   `+0x24` divergence (this cc1 emits the four `lui`s first and swaps t2/t3), so
   the pointer bases and the loop counter land in each other's registers.
   Blocker: that allocation, not the shape.
   Wave 9 (p1) re-measured it with the pointer spelling and six orders of the six
   locals: the declaration order does NOT decide the allocation (two different orders
   give the same two allocations), and the two allocations differ only in which of
   $v1/$a0/$t2/$t3 the slot, tag, pair and counter pointers get.  With
   `int *slot`, `int *tag`, `int i`, `char *name`, `int *pair` declared in that order
   the slot pointer lands in $a0 as retail has it, but the tag lands in $t2 where
   retail has $v1, and the pair in $v1 where retail has $t3.  Note the addresses
   above are 0x260E30 / 0x260EB0 / 0x260F30 (lui 0x26 + addiu 0xE30 etc.).  A `char`
   third argument costs a sign-extension pair, so it must be an int. */
typedef char *ptr;
typedef unsigned char u8;
typedef int s32;

extern s32 D_L30_00260E30[];
extern s32 D_L30_00260EB0[];
extern s32 D_L30_00260F30[];
extern u8 D_L30_001B3D30[];

void func_L30_00413528(char *a0, s32 a1, s32 a2, s32 a3, s32 a5, s32 a6)
{
    char *s = *(char **)(a0 + 0x68);
    s32 i;

    for (i = 0; i < 0x20; i++) {
        if (D_L30_00260E30[i] == 0) {
            D_L30_00260E30[i] = a1;
            D_L30_00260EB0[i] = a2;
            D_L30_001B3D30[i] = a3;
            D_L30_00260F30[i * 2 + 1] = a6;
            D_L30_00260F30[i * 2] = a5;
            *(s32 *)(s + 4) = *(s32 *)(s + 4) + 1;
            return;
        }
    }
}
