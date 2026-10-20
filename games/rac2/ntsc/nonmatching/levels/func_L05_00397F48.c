/* 0 of 4 places match - last line: 05_feltzin_system: instruction at +0x40 differs
   Candidate is 208 bytes, retail's exact size, and everything up to +0x40 is retail's,
   including the three things this function was expected to be blocked on:
   (a) the FOUR `$gp` float accesses come out bare - `lwc1 $f0,0x0($gp)` with a GPREL16
       relocation against a sized `sda` float - under the combo
       `-O2 -G8 -ffunction-sections -mno-split-addresses` (the unit's own file is the -G0
       default profile, so the body needs its own file plus a config/cflags.txt line);
   (b) the 16-byte copy IS `lq $v0,0x0($s0)` + `sq $v0,0x0($sp)` - `mode(TI)` at offset 0 on
       both sides, exactly the shape the sixth wave said was reachable;
   (c) the 0x45AFDF66 literal encodes correctly from the decimal 5627.9248046875f: the object
       holds `lui $at,0x45AF` + `ori $at,$at,0xDF66`, retail's bits.
   WHAT IS LEFT is where the quadword lands: retail runs the f1C store (`swc1 $f0,0x1C($sp)`),
   the 1-into-0x20 store and only then the `lq`/`sq` pair, with `sq` in the `jal` delay slot;
   this cc1 hoists the `lq` above both stores, so the pair sits four words early and every
   following word shifts. Measured, no move: the copy written last (form 3 in the harness),
   the copy written first, and the copy moved after the first call - all land on +0x40. The
   second select was also tried as one variable read twice; that costs four bytes (212).
   Note the constant is 0x810000 (`lui $v0,0x81`), not 0x81 - as a bare 0x81 it becomes
   `addiu $v0,$zero,0x81` and the whole body shifts. */
#include "include_asm.h"

typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern f32 D_L05_001AEBE8 __attribute__((sda));
extern f32 D_L05_001AEBE4 __attribute__((sda));
extern f32 D_L05_001AEBE0 __attribute__((sda));
extern f32 D_L05_001AEBDC __attribute__((sda));

typedef struct { f32 a, b, c, d; } V16;

typedef struct {
    V16 v;
    s32 s10;
    s32 s14;
    u8 b18;
    u8 b19;
    u16 h1A;
    f32 f1C;
    s32 s20;
} T;

extern void func_00282F20(T *a, T *b, f32 c);
extern void func_00277F48(char *a, s32 b, s32 c, T *d, f32 e);

/* Builds the spawn record for slot A0 next to object A1: the 16 covered bytes of A1's own
   header, the owner, the mode word 0x810000, a distance chosen by A1's 0x24 state, then the
   object's 0xAA id, the two byte flags 3/2 and the 5627.92f radius, and hands it to the
   shared arming call with A2 as the owner. */
void func_L05_00397F48(s32 a0, char *a1, char *a2)
{
    char *s0 = a1;
    s32 s1 = a0;
    char *s2 = a2;
    T t;

    t.s14 = 0x810000;
    t.s10 = s1;
    t.f1C = *(s32 *)(s0 + 0x24) ? D_L05_001AEBE8 : D_L05_001AEBE4;
    t.s20 = 1;
    *(TI128 *)&t.v = *(TI128 *)s0;
    func_00282F20(&t, &t, 1.0f);
    t.h1A = *(u16 *)(s1 + 0xAA);
    t.b18 = 3;
    t.b19 = 2;
    t.v.d = 5627.9248046875f;
    func_00277F48(s2, 1, *(s32 *)(s0 + 0x20), &t,
                  *(s32 *)(s0 + 0x24) ? D_L05_001AEBE0 : D_L05_001AEBDC);
}
