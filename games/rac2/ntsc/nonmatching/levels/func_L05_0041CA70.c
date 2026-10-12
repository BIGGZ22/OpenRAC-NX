/* Draft (best attempt) for func_L05_0041CA70 (244 bytes) - NOT a match.
   check.py: "05_feltzin_system: 244 bytes, expected 244" with 3 words differing:
   frame 0x40 and the callee-saved allocation ($s0 obj, $s1 table, $s2 sub, $s3 obj+0x10,
   $ra, $f20/$f21 at 0x30/0x38) now match retail exactly.  What remains:
     - the order of the two argument setups of func_37AEC0(obj,1): retail emits
       addiu $a1,$zero,1 before daddu $a0,$s0,$zero, we emit the reverse;
     - addu $s1,$s1,$v0 (finishing the table pointer) sits one slot later than retail.
   Both are scheduling-only differences (same instructions, same size).  Abandoned per
   the "pure register/schedule allocation on bodies >250 B" wall. */
void func_L05_0041CA70(char *obj)
{
    char *s2 = *(char **)(obj + 0x68);
    char *s1;
    float a, b, c;

    *(unsigned char *)(obj + 0x20) = 1;
    *(unsigned short *)(obj + 0x34) = *(unsigned short *)(obj + 0x34) | 0x100;
    *(int *)(func_379CA0(obj) + 0x148) = 5;
    func_37AEC0(obj, 1);
    s1 = D_L05_001B2614 + D_L05_00221D10[0] * 128;
    *(int *)(s2 + 0x268) = 0;
    *(float *)(s2 + 0x180) = 5.0f;
    *(float *)(obj + 0x2C) = 0.08f;
    a = func_332B50(0.0f, *(float *)s1);
    b = func_332C18();
    c = func_332C18();
    func_33B330(obj + 0x10, a, b, c);
    func_306630(obj + 0x10, s1 + 0x30, obj + 0x10);
}
