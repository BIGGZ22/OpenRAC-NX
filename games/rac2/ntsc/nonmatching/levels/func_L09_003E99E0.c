/* func_L09_003E99E0, 188 bytes, 2 places, src/levels/shared/L09_002B7238.c (-O2 -G0).
   ABANDON - the TI quadword copy folds the offsets this cc1 will not materialise.
   Read in full: the object's sixteen-byte block at +0x10 is copied onto its frame at +0x330
   (`lq $v0`/`sq $v0`), then the two steering helpers `func_002AEE70(object, frame+0x360,
   frame+0x364, 0.2f, 0.0087266480550169945f, frame->f338)` and `func_002AEF28(object,
   frame+0x358, frame+0x35C, 0.087266474962234497f, 0.0072722062468528748f,
   0.0090175354853272438f)` re-seed the angles, and `frame->f340`/`frame->f344` are blended with
   the object's `+0xF0`/`+0xF4` through `func_002EB478` and stored back.
   Candidate is 176 bytes against 188: retail materialises both quadword addresses first
   (`addiu $a0,$s1,0x10` / `addiu $v1,$s0,0x330`, then `lq`/`sq` at offset zero, which also costs
   the reload of `$a0` before the first call), this cc1 folds the constant offsets straight into
   `lq $v0,0x10($s1)` / `sq $v0,0x330($s0)`. Measured: the assignment written inline, through two
   pointer locals, through a TI value local, and with the copy moved after the two calls - all
   four give the same folded pair, the same 176 bytes. Same class as the TI-copy wall recorded in
   docs/COMPILER-NOTES.md ("this cc1 folds a constant destination offset into the `sq`"). */

typedef float f32;
typedef int TI128 __attribute__((mode(TI)));

extern float func_002EB478(float a, float b);
extern void func_002AEE70(char *a0, void *a1, void *a2, f32 f12, f32 f13, f32 f14);
extern void func_002AEF28(char *a0, void *a1, void *a2, f32 f12, f32 f13, f32 f14);

void func_L09_003E99E0(char *object) {
    char *frame = *(char **)(object + 0x68);
    TI128 *src = (TI128 *)(object + 0x10);
    TI128 *dst = (TI128 *)(frame + 0x330);

    *dst = *src;
    func_002AEE70(object, frame + 0x360, frame + 0x364, 0.2f, 0.0087266480550169945f,
                  *(f32 *)(frame + 0x338));
    func_002AEF28(object, frame + 0x358, frame + 0x35C, 0.087266474962234497f,
                  0.0072722062468528748f, 0.0090175354853272438f);
    *(f32 *)(object + 0xF0) = func_002EB478(*(f32 *)(frame + 0x340), *(f32 *)(object + 0xF0));
    *(f32 *)(object + 0xF4) = func_002EB478(*(f32 *)(frame + 0x344), *(f32 *)(object + 0xF4));
}
