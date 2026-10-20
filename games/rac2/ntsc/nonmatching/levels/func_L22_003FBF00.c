/* func_L22_003FBF00 (204 bytes, 2 places: 22_dobbo_orbit, 23_damosel_orbit) -- NEAR MISS.
   state: 208 bytes, exactly one instruction over; every other word is in retail's order.
   check.py (22_dobbo_orbit) with the source below:
     003FBF7C  lui   $at, %hi(D)   |  lwc1  $f12, 0x1C60($gp)
     003FBF80  lwc1  $f12, %lo($at)|  daddu $a0, $s1, $zero
   Retail reloads the fallback float at the join with the bare one-instruction `$gp` form; this
   cc1 materialises that same reload as the absolute `lui $at` + `lwc1` pair, so the join block is
   two instructions and everything after it shifts by four bytes.

   What is measured and right (all of it below):
   - the three 16-byte scratch vectors must be declared v, w, x in that order: retail's frame is
     sp+0x00 / sp+0x10 / sp+0x20 and its 0x60 frame with s0-s3 saved is reproduced exactly;
   - the slot base is a two-step expression (`q = handle + a2 * 4` then `*(int *)(q + 0x60)`):
     folded into one chain this cc1 emits `sll $s0,$s0,2` off the wrong base and the body grows
     to 216 bytes;
   - the level's own 16-byte table needs `ref_L22_00255820_00` (0x00255820 in 22_dobbo_orbit,
     0x002561E0 in 23_damosel_orbit), not a literal;
   - no named `p = x` is needed: retail materialises sp+0x20 into `$s1` in the two branch delay
     slots itself, which the plain `x` spelling reproduces (a named p is what pushed the body to
     216 bytes and added `$s4`).

   Tried and not enough for the remaining word: `volatile` on the fallback, a named `f32`
   fallback local, and two names for the same resident float (`D_001B0C50` / `D_001B0C50_1`) -
   all three give the same 208 bytes and the same absolute rematerialisation. */

typedef unsigned char u8;
typedef float f32;

extern char ref_L22_00255820_00[];

extern f32 D_001B0C50 __attribute__((sda));

extern void func_002ACEC0(void *out, char *table, char *in, f32 scale);

extern void func_00282F20_L22(void *out, void *in, f32 scale);

extern void func_00282D30_L22(void *out, char *in, f32 scale);

extern void func_00282CC0_L22b(void *out, char *in1, void *in2);

/* Builds the record's local frame from the level table entry the index selects, scales it by the
 * slot sub-object's rate when the slot is live and the record has not started, then folds the
 * frame into the caller's output through two bound vector helpers. */
void func_L22_003FBF00(char *a0, char *a1, int a2)
{
    char v[16];
    char w[16];
    char x[16];
    char *handle = *(char **)(a0 + 0x68);
    char *q;
    int slot;

    func_002ACEC0(v, ref_L22_00255820_00 + a2 * 16, handle + 0x10,
                  *(f32 *)(handle + 0x1C));
    q = handle + a2 * 4;
    slot = *(int *)(q + 0x60);
    if (slot != 0 && *(unsigned char *)(a0 + 0x20) == 0)
        func_00282F20_L22(v, v, *(f32 *)(slot + 0x28));
    func_00282D30_L22(x, a0 + 0xE0, D_001B0C50);
    func_00282CC0_L22b(w, a0 + 0x10, x);
    func_00282CC0_L22b(a1, w, v);
}
