/* func_L08_003D9208 (208 bytes) and func_L08_004237F0 (172 bytes) — best attempts, NOT MATCHED
 * (wave 6, w04, 2026-10-12)
 *
 * last check.py lines:
 *   func_L08_003D9208 : "08_tabora: instruction at +0x4 differs"
 *   func_L08_004237F0 : "08_tabora: 176 bytes, expected 172"
 *
 * Cause identified, the same for both: the "callee-saved register" allocation.
 *   retail: $s1 = the copy of the parameter (the input), $s0 = the moby pointer (a0->f68)
 *   my code: $s0 = the parameter, $s1 = the moby pointer
 * The body is identical everywhere else (same instructions, same offsets); the whole
 * deviation comes from the s0/s1 swap (the first diff is at +0x4, the `sd` of the
 * prologue). The variants tried with no effect: declaring the parameter copy
 * first (`Ent *a1 = a0;`), declaring it after the moby pointer, using the
 * parameter directly. Yet the same form had given the right order in
 * func_L08_003D8A28 (matched): it is an allocno-priority arbitration, not a
 * declaration-order rule.
 *
 * Another real deviation fixed in passing in func_L08_004237F0: the constant of
 * the saturation test is 0x7FFD (retail `sltiu $v0,$v1,0x7FFD`), not 0xFFFD.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned int u32;
typedef float f32;

typedef struct {
    u8 pad00[0x20];
    u8 f20;
    u8 f21;
    u8 pad22[0x24 - 0x22];
    void *f24;
    u8 pad28[0x68 - 0x28];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

typedef struct {
    u8  pad00[0x20];
    f32 f20;
    u8  pad24[0x50 - 0x24];
    u8  f50[0x88 - 0x50];
    u16 f88;
    u8  pad8A[0xE4 - 0x8A];
    f32 fE4;
    u8  padE8[0x144 - 0xE8];
    s32 f144;
    u8  pad148[0x18C - 0x148];
    u8  f18C;
    u8  pad18D[0x1B0 - 0x18D];
    f32 f1B0;
    u8  f1B8[0x1BA - 0x1B8];
    s16 f1BA;
    u8  f1BC[0x1BE - 0x1BC];
    u16 f1BE;
    u8  pad1C0[0x1CC - 0x1C0];
    s32 f1CC;
    f32 f1D0;
    f32 f1D4;
    s32 f1D8;
    s32 f1DC;
    u8  f1E0[0x254 - 0x1E0];
    s32 f254;
    u8  pad258[0x40E - 0x258];
    u16 f40E;
    u8  pad410[0x440 - 0x410];
    u8  f440[0x448 - 0x440];
    s32 f448;
    u8  f44C[0x484 - 0x44C];
    s32 f484;
    s32 f488;
    u8  f48C[8];
} Moby;

extern s32 func_2F9A60();
extern s32 func_327898(s32, s32);
extern void func_2F9A90();

/* func_L08_003D9208: only deviation = $s0/$s1 swapped (offset 0x7FFD correct here) */

void func_L08_003D9208(Ent *a0)
{
    Moby *s0 = a0->f68;
    u16 v1;

    if (a0->fBC != 0) {
        a0->fBC = 0;
        if (s0->f448 < 200) {
            s0->f448 = func_327898(300, 360);
            s0->f40E = 12;
        }
    }
    v1 = a0->f96;
    if (s0->f254 == 2) {
        if (func_2F9A60(&s0->f448)) s0->f40E = 3;
        v1 = a0->f96;
    }
    if (v1 <= 0xFFFDu) a0->f96 = v1 + 1;
    if (a0->f20 == 8) {
        if (a0->f95 == 0) func_2F9A60(&s0->f48C);
        func_2F9A60(&s0->f440);
    }
    func_2F9A60(&s0->f44C);
}
