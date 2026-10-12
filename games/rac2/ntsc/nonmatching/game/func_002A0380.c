/* 60-byte near miss: the global load and branch use v1 instead of v0. */
extern void func_0029FDF0(void);
extern void func_002A0180(void);
extern unsigned int D_L00_001B1E40[16];
extern void func_002A0048(void);

void func_002A0380(void)
{
    func_0029FDF0();
    func_002A0180();
    if (D_L00_001B1E40[32])
        func_002A0048();
}
