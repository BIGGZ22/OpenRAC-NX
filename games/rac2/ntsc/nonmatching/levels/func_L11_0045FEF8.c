/* from src/levels/shared/L11_002C5E80.c: two effect passes on a0 + 0x10 -
 * func_002A7820(0x2F,0x3F) then func_002A7940() feed func_L11_004600A0(a0+0x10,
 * 0x103080, v, 0xB, 2, 2.0f, f, 1.0f), and again with (0x1F,0x3F)/0x606060/1.0f.
 * Best of 2 compiles: exact 180 bytes with every instruction and delay slot matching,
 * including the v0 -> $s0 copy in the second call's slot and the shared 1.0f in $f20.
 * The only difference is the callee-saved pair: retail keeps the pointer in $s1 and
 * the int in $s0, this cc1 the other way round, which flips the two `sd` in the
 * prologue (+0x8). The `$s0/$s1` swap wall; a local `char *p` hoists the +0x10 (the
 * version without it is 184 bytes). */
extern int func_002A7820(int, int);

extern float func_002A7940(void);

extern void func_L11_004600A0(void *, int, int, int, int, float, float, float);

void func_L11_0045FEF8(void *a0)
{
    char *p = (char *)a0 + 0x10;
    int v = func_002A7820(0x2F, 0x3F);
    float f = func_002A7940();
    func_L11_004600A0(p, 0x103080, v, 0xB, 2, 2.0f, f, 1.0f);
    v = func_002A7820(0x1F, 0x3F);
    f = func_002A7940();
    func_L11_004600A0(p, 0x606060, v, 0xB, 2, 1.0f, f, 1.0f);
}
