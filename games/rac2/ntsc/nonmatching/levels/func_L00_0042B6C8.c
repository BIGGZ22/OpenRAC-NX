/* 8 instructions, 40 bytes emitted against 32: same shape as func_L00_0042B6A8 (`addiu $v0,$gp,-0x13A8`), same gp-relative address problem under -G0. */
extern char D_L00_001AEC48[];
extern void func_00335AE0(char *);

void func_L00_0042B6C8(char *o)
{
    *(char **)o = D_L00_001AEC48;
    func_00335AE0(o);
}
