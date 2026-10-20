/* from src/core/00131780.c: the two saved registers are the other way round */
/*
 * Two $gp stores (the second argument at gp - 0x7B6C and the first at
 * gp - 0x7B70), the two counters cleared at the head and the tail of a run of
 * (arg1 << 2) + 4 bytes, and the handler at func_0011B3D0 called with the
 * head and with (head + 3).
 *
 * The instruction sequence, the sizes and both calls are exact; what differs
 * is which saved register holds what. Retail keeps the object in $s0 (five
 * references) and the shift in $s1 (two); this cc1 gives $s0 to the shift.
 * The lever the notes record - a pointer local for the same value, so the
 * pointer carries the references - moves the difference to +0x1C instead of
 * settling it, so this is the allocator's own choice (issue #157, section 1).
 */
extern int D_001A7480 __attribute__((sda));

extern int D_001A7484 __attribute__((sda));

extern void func_0011B3D0(int arg0, int arg1);

void func_001322B8(int arg0, int arg1) {
    int offset = arg1 << 2;

    D_001A7484 = arg1;
    D_001A7480 = arg0;
    *(int *)(arg0 + offset + 4) = 0;
    *(int *)arg0 = 0;
    func_0011B3D0(arg0, arg0 + 3);
    arg0 += offset + 4;
    func_0011B3D0(arg0, arg0 + 3);
}
