/* 19 instructions (76 bytes) against retail's 23 (92 bytes): the frame is 0x30 not 0x40 and s1 is never saved; retail keeps &a in s1 across the first call (hoisted before it), GCC rematerializes sp+0 after it. Copy/zero order and the calls match. */
extern void func_L00_002D3798(V4_Fa012328c *, V4_Fa012328c *, float);

extern void func_00282DB0(V4_Fa012328c *, V4_Fa012328c *);

void func_L00_002B5590(V4_Fa012328c *p) {
    V4_Fa012328c a;
    V4_Fa012328c b;
    V4_Fa012328c *pa = &a;
    V4_Fa012328c *pb = &b;

    *pa = *p;
    *pb = 0;
    func_L00_002D3798(pb, pb, 1.0f);
    func_00282DB0(pa, pb);
}
