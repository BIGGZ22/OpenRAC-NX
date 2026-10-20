/* func_L20_004424A0 (176 bytes, level 20): near miss, 44 of 44 words, 31 words differ.
 *
 * What matches: the frame (0x30 - one word too big, see below), the two float products
 * (D_L20_001B212C * 4.84813745e-06f and D_L20_001B2130 * 0.000290888245f) materialised
 * through $f14/$f15, the call to func_00341B10 with the two zeros in $f12/$f13, the
 * func_00330C60 call, the two float stores, the 16-byte zero store and the func_00302198
 * call with the doubling `add.s $f12,$f12,$f12` (this cc1 turns x * 2.0f into x + x on its
 * own).
 *
 * What differs: (1) this cc1 hoists the lwc1 of D_L20_001B2130 above the frame setup, the
 * retail keeps both loads after it; (2) the value D_L20_001B2138 * 0.000277777785f is live
 * across the func_00330C60 call here, so this cc1 puts it in $f20 and saves $f20 (frame
 * 0x30, `swc1 $f20,0x20($sp)`), while the retail keeps it in the caller-saved $f0 and
 * never saves an FPR (frame 0x20); (3) the register names of the two products follow from
 * (2).  The function has to live in the -g8 sibling: it reads four constants through $gp
 * (0x3138..0x3148 -> 0x1B2128..0x1B2138).
 *
 * Fourth wave (20_yeedil, 2026-10-20, worker w20-01) measured the second residue more
 * closely, and it is a compiler-model difference, not a source shape: the retail keeps the
 * product `D_L20_001B2138 * 0.000277777785f` in the CALLER-SAVED $f0 across the
 * `jal f_00330C60` and stores it to state[0xE0] after that call, while this cc1 keeps the
 * value in the callee-saved $f20 (and saves it, so the frame is 0x30 instead of 0x20).
 * The callee really does clobber $f0 (f_00330C60 = func_002A89A8 writes $f0 twice), so the
 * retail's second store keeps whatever the callee left in $f0 - the retail body is what it
 * is, and this cc1 will not reproduce it from C: any source that has the value live across
 * the call puts it in $f20 here, and a source that re-reads the value from state[0x7C]
 * after the call (so that it is not live across it) emits a reload the retail does not
 * have.  The first residue (this cc1 hoists the `lwc1` of D_L20_001B2130 above the frame
 * setup) was not re-measured; the note above stands.
 */
typedef int TI128 __attribute__((mode(TI)));

extern float D_L20_001B212C __attribute__((sda));
extern float D_L20_001B2130 __attribute__((sda));
extern float D_L20_001B2138 __attribute__((sda));
extern float D_L20_001B2128 __attribute__((sda));

extern void func_00341B10(float, float, float, float);
extern void func_00330C60(char *, int, float);
extern void func_00302198(float);

void func_L20_004424A0(char *arg0) {
    char *s1 = arg0;
    char *s0 = *(char **)(arg0 + 0x68);
    float f0;
    TI128 *p;

    func_00341B10(0.0f, 0.0f, D_L20_001B212C * 4.84813745e-06f,
                  D_L20_001B2130 * 0.000290888245f);
    f0 = D_L20_001B2138 * 0.000277777785f;
    func_00330C60(s1 + 0x10, 0, 0.5f);
    *(float *)(s0 + 0x7C) = f0;
    *(float *)(s0 + 0xE0) = f0;
    p = (TI128 *)(s0 + 0xF0);
    *p = 0;
    func_00302198(*(float *)(s0 + 0x7C) * 2.0f * D_L20_001B2128);
    *(float *)(s0 + 0xF8) = f0;
}
