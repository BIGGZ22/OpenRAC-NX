/* func_L16_002D2D10, 88 bytes, 1 placement (level 16).  Best candidate 72 bytes; six
   spellings (v1 source order, v2/v4 named reciprocal locals, v3 reversed, v5 pointer,
   v6 guard-first) give the IDENTICAL 72-byte object.
   Retail = 88: two 2-nop pairs (after `sw $a0,0x280($v0)` and after `swc1 $f0,0x27C($v0)`)
   and the two reciprocals in the other order - retail's first `div.s` is by $f13 into
   $f1 (stored at +0x278) and its second is by $f12 into $f0 (stored at +0x274); this cc1
   always divides by $f12 first into $f1 and stores +0x274 first, whatever the source
   order, and emits no nops.  So the gap is codegen, not spelling.
   The resident block is 0x001B99C0 (fields +0x274, +0x278, +0x27C float, +0x280 int);
   note the stale `T_001BFF99C0`/`D_001BFF99C0` declarations that used to sit in
   src/levels/16_snivelak/L16_002C6378.c named 0x001BFF99C0, which is 0x6640 too high -
   corrected to D_L16_001B99C0 there.  The body: */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad[0x274];
    f32 f274;
    f32 f278;
    f32 f27C;
    s32 i280;
} T_001B99C0;

extern T_001B99C0 D_L16_001B99C0;

void func_L16_002D2D10(s32 a0, f32 f12, f32 f13) {
    if (D_L16_001B99C0.f27C == 0.0f) {
        D_L16_001B99C0.i280 = a0;
        D_L16_001B99C0.f27C = 1.0f;
        D_L16_001B99C0.f278 = 1.0f / f13;
        D_L16_001B99C0.f274 = 1.0f / f12;
    }
}
