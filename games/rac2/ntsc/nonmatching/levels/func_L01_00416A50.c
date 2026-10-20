/* func_L01_00416A50 (224 bytes, 1 place: 01_oozla) -- NEAR MISS, but the `$gp` wall is broken:
   the body reaches 232 bytes with the bare form for all four floats.
   Ruler (wslcmp) sizes: 248 in L01_003DA740.c (-G0), 252 moved to L01_003DA740-g8.c (-G8) alone,
   and **232 with the -g8 sibling plus `-mno-split-addresses`** - 8 bytes from retail. The lead's
   profile lever does what `sda` alone would not:
     plain -G0:  lwc1 $f3, 0x0($at)   (lui $at,%hi first)  |  retail: lwc1 $f1, 0x2A70($gp)
     -G8 -mno-split-addresses:  lwc1 $f1, 0x0($gp)         |  retail: lwc1 $f1, 0x2A70($gp)
   All four (`$gp+0x2A68`, `+0x2A6C`, `+0x2A70`, `+0x2A84`) take the one-instruction form with
   `flags="-O2 -G8 -ffunction-sections -mno-split-addresses"`, i.e. a `config/cflags.txt` line.
   This contradicts the earlier "the sda spelling changes nothing for loads" note for this unit.

   The 8 bytes left are two instructions, neither addressed:
   - the per-frame factor: retail's bits are 0x3998825C and the decimal 0.00029278849f lands one
     ULP high on 0x39998168. Take the bits from the disassembly (a union or an exact decimal);
   - retail stores the fourth component straight from `$f0` twice (`swc1 $f0,0xF8` in the last
     call's delay slot, then `swc1 $f0,0xFC`), where this cc1 inserts a `mov.s $f1,$f0` and an
     earlier redundant `swc1 $f0,0xF8`.

   Everything else is measured and right (the whole body below): the state-1 gate on byte 0x20, the
   rate folded from the owner's float 0x24 and the `$gp` word at +0x2A68, the sub-object float at
   +8 aged by exactly 1/240 (bits 0x3B888889), the two 16-byte pair updates, the three
   `func_2E4460` component re-derivations in the order 0xF0/0x68, 0xF4/0x64, 0xF8/0x60, and the
   fourth component re-armed and stored to 0xFC before 0xF8 - the reverse of the emitted order,
   which is what reproduces retail's delay slot. */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

extern f32 func_2E3A60(f32 x);
extern f32 func_2E4460(f32 x, f32 y);

extern f32 D_001B1A58 __attribute__((sda));
extern f32 D_001B1A5C __attribute__((sda));
extern f32 D_001B1A60 __attribute__((sda));
extern f32 D_001B1A74 __attribute__((sda));
extern s32 D_001B1A98 __attribute__((sda));

extern s32 func_3104E8(s32 mode);

extern void func_L01_00416C08(u8 *a0);
extern void func_L01_00417090(u8 *a0);

/* Ages the object's rate once, and in state 1 folds the sub-object's angles into the three
 * turning components and re-arms the fourth from the level's own per-frame factor. */
void func_L01_00416A50(u8 *a0)
{
    f32 f = D_001B1A58;
    u8 *h = *(u8 **)(a0 + 0x68);
    u8 *owner = *(u8 **)(a0 + 0x24);

    *(f32 *)(a0 + 0x2C) = *(f32 *)(owner + 0x24) * f;
    if (*(u8 *)(a0 + 0x20) == 1) {
        f = func_2E3A60(*(f32 *)(a0 + 0xFC));
        *(f32 *)(a0 + 0x18) = *(f32 *)(h + 0x18) + D_001B1A5C + D_001B1A60 * f;
        *(f32 *)(a0 + 0xF0) = func_2E4460(*(f32 *)(a0 + 0xF0), *(f32 *)(h + 0x68));
        *(f32 *)(a0 + 0xF4) = func_2E4460(*(f32 *)(a0 + 0xF4), *(f32 *)(h + 0x64));
        *(f32 *)(a0 + 0xF8) = func_2E4460(*(f32 *)(a0 + 0xF8), *(f32 *)(h + 0x60));
        f = func_2E4460(*(f32 *)(a0 + 0xFC), D_001B1A74 * 0.0002927885f);
        *(f32 *)(a0 + 0xFC) = f;
        *(f32 *)(a0 + 0xF8) = f;
    }
    func_L01_00416C08(a0);
    if (func_3104E8(D_001B1A98) == 0)
        func_L01_00417090(a0);
}
