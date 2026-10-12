/* 84 bytes, 1 instruction differs: retail loads $a0 = 0 again before $a1/$a2 for the second call (the first call's delay slot holds one $a0 = 0 too), so retail has 21 instructions and this has 20; GCC puts that move in the delay slot of the second jal. Missing: a source shape that makes GCC re-emit the $a0 zero. Tried: int and pointer first argument on the second call; both drop it. */
extern short D_L00_001B267E;

extern int D_L00_001B18D4 __attribute__((sda));

extern void func_00284F40(int a);

extern void func_0027DDC0(int a, int b, int c, int d);

extern void func_0027BD70(void);

void func_L00_0037F790(void)
{
    func_00284F40(0);
    func_0027DDC0(0, 0, 0, 0x80);
    D_L00_001B18D4 = D_L00_001B267E ? 0x100FF : 0x70;
    func_0027BD70();
}
