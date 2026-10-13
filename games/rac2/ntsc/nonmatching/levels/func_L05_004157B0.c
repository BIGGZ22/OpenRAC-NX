/* Draft (best attempt) for func_L05_004157B0 (256 bytes) - NOT a match.
   Twin of func_L05_0041CA70.  check.py: "05_feltzin_system: 256 bytes, expected 256"
   with the frame/registers matching and only the store/setup order differing:
   retail emits swc1 $f1,0x180($s1) first, then sw $a0,0x270 / sw $a0,0x26C / sw $zero,0x268,
   and materialises 20.0f in $f1 and 0.02f in $f0 (we get the two FP constants in the
   opposite registers).  Measured: swapping the source order of those four stores does not
   move the emitted order (sched2 dominates); the FP-constant register assignment follows
   the creation order in the RTL, so the 0.02f store must be written before the 20.0f one. */
void func_L05_004157B0(char *obj)
{
    char *s1 = *(char **)(obj + 0x68);
    char *s2;
    float a, b, c;

    *(unsigned char *)(obj + 0x20) = 1;
    *(unsigned short *)(obj + 0x34) = *(unsigned short *)(obj + 0x34) | 0x100;
    *(int *)(func_379CA0(obj) + 0x148) = 0x14;
    func_37AEC0(obj, 1);
    s2 = D_L05_001B2614 + D_L05_00221D10[0] * 128;
    *(int *)(s1 + 0x268) = 0;
    *(int *)(s1 + 0x26C) = -1;
    *(int *)(s1 + 0x270) = -1;
    *(float *)(s1 + 0x180) = 20.0f;
    *(float *)(obj + 0x2C) = 0.02f;
    a = func_332B50(0.0f, *(float *)s2);
    b = func_332C18();
    c = func_332C18();
    func_33B330(obj + 0x10, a, b, c);
    func_306630(obj + 0x10, s2 + 0x30, obj + 0x10);
}
