/* 68-byte near miss: the target uses one gp-relative global load; this -G0 C build expands it to lui/lw (72 bytes). Remaining call/branch sequence aligns after that extra instruction. */
extern int D_001AA3B4 __attribute__((sda));
extern int func_002A8860(void *, int);
extern int func_00282978(short *);
extern void func_002B9180(void *);
void func_002C5020(unsigned char *self)
{
    func_002A8860(self + 4, D_001AA3B4);
    if (func_00282978(self + 0x0A) != 0)
        func_002B9180(self);
}
