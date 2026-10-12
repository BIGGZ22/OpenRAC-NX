/* 8 instructions, 40 bytes emitted against 32: retail computes the data address gp-relative (`addiu $v0,$gp,-0x1388`) and stores it in the call delay slot; -G0 gives lui/addiu and a frame that saves a0 in $v1. */
extern char D_L00_001AEC68[];
extern void func_00335AE0(char *);

void func_L00_0042B6A8(char *o)
{
    *(char **)o = D_L00_001AEC68;
    func_00335AE0(o);
}
