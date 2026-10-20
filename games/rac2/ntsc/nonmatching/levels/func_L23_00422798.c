/* func_L23_00422798 (532 bytes, 1 place: level 23) -- NEAR MISS, not integrated.
   state: 528 bytes against retail's 532. Aligned word by word, three words differ and nothing else:
   the whole body, its call order, its constants and its `$gp` words are right (the float words
   are 0x001B18E0..0x001B190C; `$gp` is 0x001AEFF0). Checked in the -G8 unit; it belongs in
   L23_0041E418-g8.c.
   What differs is the single quad zero of the local direction {0, 0, 0, 1}:
   - retail zeroes it with `por $v0,$zero,$zero` (the MMI word 0x700014A9) at +0x4 and stores the
     register (`sq $v0,0($sp)`); this cc1 folds the zero into the store (`sq $zero,0($sp)`), one word
     short, because the compiler's zero-fold pass folds a materialised TI zero whenever the register
     is dead after its single store;
   - the `lui $at,0x3F80; mtc1 $at,$f0` pair for 1.0f then issues at +0x4 here and at +0x2C in
     retail, a consequence of the missing word.
   Every spelling of the zero reaches the same fold or a worse shape: `*(TI128 *)v = 0` (this
   file) and a struct or union with a TI member initialised `{0}` or assigned all give
   `sq $zero`;`float v[4] = {0,0,0,1}` gives a `memset` call (540 bytes, 15 words differ); four
   scalar stores give 536 bytes; an `int` return value that keeps $v0 live (also with `return` on the
   last call) does not stop the fold. Retail has the por form for a single quad zero stored to the
   stack in 62 compiled-looking functions of level 23 (func_00281EB8, func_002A7EA0, func_002AE838,
   func_002D36A0, func_0030DE88, func_00315E20 ...) against 13 that store `sq $zero` to the stack;
   func_L23_0041C578 and func_L23_003BFE90 are the same wall. Unblocking it is a change to the
   zero-fold pass (rac2_fold_zero_ti_store), not to the C.
   Tuning that did matter: func_00282DF0's result and `D_L23_001B1908 * 0.2f` are computed before
   the store to +0x18 (retail's registers), and func_L00_002ECFE8 is declared with its integer
   arguments first. The registers are the same as for the declaration in L02_002B5F50.c (floats in
   the middle) but the evaluation order differs; the floats-in-the-middle order gives 5 differing
   words instead of 3.
*/

typedef unsigned char u8;

typedef unsigned int u32;

typedef int s32;

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B1908 __attribute__((sda));

extern float D_L23_001B190C __attribute__((sda));

extern float D_L23_001B18E0 __attribute__((sda));

extern u32 D_L23_001B18E4 __attribute__((sda));

extern u32 D_L23_001B18E8 __attribute__((sda));

extern u32 D_L23_001B18EC __attribute__((sda));

extern u32 D_L23_001B18F0 __attribute__((sda));

extern s32 D_L23_001B18F4 __attribute__((sda));

extern u32 D_L23_001B18F8 __attribute__((sda));

extern s32 D_L23_001B18FC __attribute__((sda));

extern u32 D_L23_001B1900 __attribute__((sda));

extern s32 D_L23_001B1904 __attribute__((sda));

extern float func_00282DF0(void *);

extern float func_002A7940(void);

extern void func_00281EB8(void *, s32, u32, s32, float, float, float);

extern void func_L00_002ECFE8(void *, void *, s32, u32, s32, s32, float, float, float);

/* scrolls the effect's phase, spawns three particles around it, and once the phase passes the
   limit emits three more through the shared emitter with the local direction {0, 0, 0, 1} */
void func_L23_00422798(u8 *obj) {
    float v[4];
    u8 *pl = *(u8 **)(obj + 0x68);
    float f;
    float t;
    float s1;

    *(TI128 *)v = 0;
    v[3] = 1.0f;
    f = *(float *)(pl + 0x14) - 0.1f;
    *(float *)(pl + 0x14) = f;
    if (f <= -8.0f) {
        *(float *)(pl + 0x14) = f + 8.0f;
    }
    t = func_00282DF0(pl);
    s1 = D_L23_001B1908 * 0.2f;
    *(float *)(pl + 0x18) = *(float *)(pl + 0x18) + t;
    func_00281EB8(obj + 0x10, 0x31, D_L23_001B18E4, 1, s1, 0.0f, func_002A7940());
    func_00281EB8(obj + 0x10, 0x31, D_L23_001B18E8, 1, D_L23_001B1908 * 0.1f, 0.0f, func_002A7940());
    func_00281EB8(obj + 0x10, 0x2F, D_L23_001B18EC, 1, D_L23_001B1908 * 0.05f, 0.0f, func_002A7940());
    if (*(float *)(pl + 0x18) > D_L23_001B18E0) {
        func_L00_002ECFE8(obj, v, 1, D_L23_001B18F0, D_L23_001B18F4, 0x17, D_L23_001B1908 * 0.02f,
                          D_L23_001B190C * D_L23_001B1908, *(float *)(pl + 0x14));
        func_L00_002ECFE8(obj, v, 1, D_L23_001B18F8, D_L23_001B18FC, 0x17, D_L23_001B1908 * 0.07f,
                          D_L23_001B190C * D_L23_001B1908, *(float *)(pl + 0x14));
        func_L00_002ECFE8(obj, v, 1, D_L23_001B1900, D_L23_001B1904, 0x17, D_L23_001B1908 * 0.07f,
                          D_L23_001B190C * D_L23_001B1908, *(float *)(pl + 0x14));
    }
}
