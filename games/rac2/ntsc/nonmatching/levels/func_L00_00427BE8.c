/* func_L00_00427BE8, 144 bytes, 27 placements.  Best candidate 128 bytes; five spellings
   (plain casts, a record view with the quad at +0 and the float at +0x18 so the base
   stays live, two named pointer locals, the +0xC8 store first) all give the IDENTICAL
   128-byte object.
   The two quadword copies DO reach `lq`/`sq` through `typedef int TQ __attribute__((mode(TI)));`
   - the recipe in src/levels/shared/L08_003D3928.c works - but this cc1 keeps $s0 as the
   base and folds both destinations into the store offsets (`sq $v0,0xB0($s0)` /
   `sq $v0,0x80($s0)`), where retail materialises each destination in a register
   (`addiu $a0,$s0,0xB0; ... sq $v0,0x0($a0)` then `addiu $v1,$s0,0x80; ... sq $v0,0x0($v1)`)
   and re-copies the blend weight (`mov.s $f12,$f0`) before the first call.  The four
   missing words are those two `addiu`, the `mov.s` and the schedule that follows.
   The body: */

typedef float f32;
typedef int TQ __attribute__((mode(TI)));

extern void func_L00_002D7958(float *dst, void *src, float x);
extern void func_L00_002D7940(char *p);
extern void func_L00_002EE0C0(char *p);
extern void func_L00_00427B40(char *p);

void func_L00_00427BE8(char *a0, void *a1, TQ *a2, f32 f12) {
    char *s0 = *(char **)(a0 + 0x70);

    *(TQ *)(s0 + 0xB0) = *a2;
    *(TQ *)(s0 + 0x80) = *a2;
    *(f32 *)(s0 + 0xC8) = f12;
    func_L00_002D7958((float *)(s0 + 0x14), a1, f12);
    func_L00_002D7958((float *)(a0 + 0x50), a1, 0.0f);
    func_L00_002D7940(s0);
    func_L00_002EE0C0(s0 + 0x90);
    func_L00_00427B40(a0);
}
