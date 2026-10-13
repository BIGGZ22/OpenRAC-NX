/* 72-byte near miss: C emits absolute lui/lwc1 for the threshold; retail uses one gp-relative lwc1 (76 bytes). Remaining sequence aligns after that extra instruction. */
extern void func_002FC808(void);
extern void func_00327D50(char *);
extern float D_001B00F4 __attribute__((sda));
void func_00327D08(char *self)
{
    func_002FC808();
    if (*(float *)(*(char **)(self + 0x68) + 0x24) < D_001B00F4)
        func_00327D50(self);
}
