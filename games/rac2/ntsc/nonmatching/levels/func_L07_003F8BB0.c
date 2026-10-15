/* Plain C, and it matched at its place until 2026-10-13. It no longer does, and the cause is
   worth knowing: the four bodies above it in L07_003F4578.c called `qcopy` and had to come out
   (inline assembly is not a matching route), and taking them out moved this one's register
   allocation by a register - `mtc1 $v1,$f12` where retail has `mtc1 $v0,$f12`. The file is one
   compilation unit, so a body's allocation depends on the bodies around it. Kept here to be
   re-matched in place; the build reported it as "1 do not match" before it was moved out. */

void func_L07_003F8BB0(u8 *a0)
{
    u8 *s2 = *(u8 **)(a0 + 0x68);
    u8 a[16];
    u8 b[16];

    func_002E5CF8(3, 0, a0);
    func_0029FCC0(a0, 0, a);
    func_L07_003F8898(a0, b);
    func_L07_003F43E0(a0, *(u8 **)(s2 + 0x418), a, b, (f32)*(u8 *)(s2 + 0x35));
}
