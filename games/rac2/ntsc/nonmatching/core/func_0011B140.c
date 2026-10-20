/* from src/core/0011A830.c: store 4 to 0x1000F000, `sync`, poll bit 2 there until it is
   set or the caller's sp+0 flag turns non-zero, call func_0011F5E0 (and func_0011F628
   when it reports non-zero), store 4 to 0x1000F000 again with a second `sync`, and return
   the 8-byte value func_0011AF90 left at sp+8. The two `sync` barriers are in the retail
   bytes and this cc1 emits none from C, so the 164 bytes cannot match. The body below is
   the behaviour (for the port), not a candidate. */
extern void func_0011AF90(int *, long long *);

extern int func_0011F5E0(void);

extern void func_0011F628(void);

long long func_0011B140(void) {
    int flag = 0;
    long long value;
    volatile unsigned int *d = (volatile unsigned int *)0x1000F000;
    int r;

    func_0011AF90(&flag, &value);
    r = func_0011F5E0();
    *d = 4;
    if (r != 0)
        func_0011F628();
    while ((*d & 4) == 0 && flag == 0)
        ;
    r = func_0011F5E0();
    *d = 4;
    if (r != 0)
        func_0011F628();
    return value;
}
