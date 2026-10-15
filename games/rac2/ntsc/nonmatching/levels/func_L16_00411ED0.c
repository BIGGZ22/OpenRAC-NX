/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L16_00411ED0(u8 *arg)
{
    u8 *s0 = arg;
    u8 *s1 = *(u8 **)(arg + 0x68);
    s32 buf[4];
    s32 rec[4];
    float t;

    func_30AD48(s0, func_313EA0(2), buf);
    qzero(rec);
    t = func_313F38(5.0f, 10.0f);
    *(float *)((u8 *)rec + 8) = -(t * (1.0f / 60.0f));
    func_41F1F0(s0, buf, rec, 1, 1.0f / 180.0f, (float)*(u8 *)(s1 + 0x36));
}
