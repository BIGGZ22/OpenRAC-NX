/* func_0033A520 (second pass): 364 bytes against retail's 360 (tools/check.py: 0 of 28 places), from
   src/game/003399B0.c, default flags.

   The four extra bytes are NOT a source problem. The pinned gas puts a nop between
   `mfc1 $v0,$f1` and `mtc1 $v0,$f0` (INSN_LOAD_COPROC_DELAY in gas/config/tc-mips.c: the next
   instruction reads the GPR that mfc1 just wrote). Retail never has that nop: a scan of the boot
   program finds 71 sites where an `mfc1 rX` is directly followed by an instruction reading rX,
   all of them in functions that are still markers, none in a function already matched. A tiny
   .s through build/compiler/tools/as confirms the nop. So this function (and the ~35 others with
   that pair) cannot match until the assembler stops inserting it after mfc1; the lead decides
   whether to patch tools/compiler.

   With that one nop deleted from the object, this C is 360 bytes and 12 words differ:
     +0x5c..+0x64  retail loads 300.0f ($f1) before D_001B23A4 ($f0); this puts D_001B23A4 first
     +0x74, +0x84  retail keeps the first obj->0x1FC pointer in $v0 (this build: $v1)
     +0x9c..+0xc4  order of the independent instructions in the block that holds the first call
   What got it that close: retail has a basic-block boundary right after the rounded value (the
   allocation, the unfilled nop between mtc1 and cvt.s.w and the unmerged conversion all show it).
   It is reproduced with a branch on a live register whose two arms hold the same code, which the
   post-scheduling cross-jump merges back into straight code (the same construction matched
   func_0033C028). The store order alloc[1],[2],[3],[0] is what gives retail's sw 0,4,8,0xC.
   Standalone: the typedefs and externs it uses are below. */
typedef float f32;
typedef unsigned char u8;
typedef int s32;
extern void func_00336710(void *a0, void *a1, void *a2);
extern void func_00335F88(void *a0, f32 a1, f32 a2, f32 a3, f32 a4);
extern int *func_00335E20(void *a0);
extern void func_003369A8(void *a0, void *a1, void *a2);
extern int *func_00336DA8(int a0, int a1);
extern int func_00336E40(void *a0);
extern char D_001ADC88[];
extern f32 D_001B23A4 __attribute__((sda));

void func_0033A520(u8 *obj, void *arg)
{
    f32 v;

    *(s32 *)(obj + 0x1F8) = 0;
    *(void **)(obj + 0x200) = arg;
    if (arg != 0) {
        s32 *alloc = func_00336DA8(16, func_00336E40(arg));
        *(s32 **)(obj + 0x1FC) = alloc;
        alloc[1] = 0;
        alloc[2] = 0;
        alloc[3] = 0;
        alloc[0] = 0;
    }
    if (obj) {
        v = (f32)(s32)(D_001B23A4 * 300.0f + 0.5f);
        ((f32 *)*(s32 **)(obj + 0x1FC))[0] = 70.0f;
    } else {
        v = (f32)(s32)(D_001B23A4 * 300.0f + 0.5f);
        ((f32 *)*(s32 **)(obj + 0x1FC))[0] = 70.0f;
    }
    ((f32 *)*(s32 **)(obj + 0x1FC))[1] = v;
    func_00336710(obj + 0x134, D_001ADC88, *(void **)(obj + 0x200));
    func_00335F88(obj + 0x134, 24.0f, 24.0f, 0.0f, 0.0f);
    *func_00335E20(obj + 0x134) = 0x60F0F0B0;
    *(f32 *)(obj + 0x204) = 65.0f;
    func_003369A8(obj + 0x170, D_001ADC88, *(void **)(obj + 0x200));
    *func_00335E20(obj + 0x170) = 0x80F0F0F0;
    func_00335F88(obj + 0x170, 0.7f, 0.7f, 0.0f, 0.0f);
}
