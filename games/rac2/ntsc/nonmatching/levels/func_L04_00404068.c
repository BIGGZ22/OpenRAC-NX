/* Not a C row as the catalogue splits it: catalogue defect. The 152-byte extent at
   0x00404068 opens with eight bytes that belong to the PREVIOUS function's epilogue
   (`addiu $sp,$sp,0x160` / `nop`); the real function opens at 0x00404070 with a 0x10 frame
   and runs 144 bytes to 0x004040FC.  Master's LEVEL_ENTRY_OFFSETS does not carry this entry
   (it only lists three boot cases), so the ruler still compares from 0x00404068 and no C body
   can match.  Maintainers need the entry moved to 0x00404070, size 144.
   The body itself is plain C and is written out below for whoever re-splits it: decays the
   object's 0x1FC float by 0.1, wraps it back by +8 when it falls to or under -8, and hands
   the record with three float arguments (0.08, 0.45 and the wrapped value) plus four
   integers (3, 0x60005AFF, 0xFF, 0x17) to the shared helper.  Same shape as the already
   known split entries func_L00_0031AFF0 / func_L00_00358670. */

typedef int s32;
typedef unsigned char u8;
typedef float f32;

extern void func_30DC78(u8 *, u8 *, s32, s32, s32, s32, f32, f32, f32);

void func_L04_00404068(u8 *a0) {
    u8 *a1 = *(u8 **)(a0 + 0x68);
    f32 v = *(f32 *)(a1 + 0x1FC) - 0.1f;

    if (v <= -8.0f) {
        *(f32 *)(a1 + 0x1FC) = v + 8.0f;
    } else {
        *(f32 *)(a1 + 0x1FC) = v;
    }
    func_30DC78(a0, a1 + 0x200, 3, 0x60005AFF, 0xFF, 0x17, 0.08f, 0.45f,
                *(f32 *)(a1 + 0x1FC));
}
