/* 0 of 1 places match - last line: 02_maktar_nebula: 204 bytes, expected 184.
   184 bytes = 46 instructions; the shape reproduced here is 204 (the `s32 i` spelling is
   196). The bulk is right: the `f280 != 0` guard, the `lhu` read of the 0x282 counter, the
   `lh 0x24` / `cvt.s.w` scale, `3.0f - (float)(s16)counter`, the 0x3EAAAAAB (1/3) fold, the
   `c.olt.s` against the 0x20 float, and the `bc1f` that either stores the incremented counter
   and enters the walk or sets `f280 = 1`. What differs is the peeled walk: retail stores
   `sh $a0,0x282($a1)` once BEFORE the loop and once AFTER it (`sh $v1,0x282($a1)` at 0x3DB924)
   with an empty three-instruction loop between, where the cc1 keeps a store per iteration.
   Below: the candidate that measures 204. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad0[0x20];
    f32 f20;
    s16 f24;
    u8 pad1[0x25A];
    s16 f280;
    u16 f282;
} Rec;

void func_L02_003DB880(u8 *arg0)
{
    Rec *r = *(Rec **)(arg0 + 0x68);
    u16 i;

    if (r->f280 != 0)
        return;
    i = r->f282;
    if (r->f20 < (f32)r->f24 * (3.0f - (f32)(s16)(i + 1)) * 0.33333334f) {
        i++;
        r->f282 = i;
        while (r->f20 < (f32)r->f24 * (3.0f - (f32)(s16)(i + 1)) * 0.33333334f)
            i++;
        r->f282 = i;
    } else {
        r->f280 = 1;
    }
}
