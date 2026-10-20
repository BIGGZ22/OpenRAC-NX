/* func_L20_00448A18 (284 bytes, level 20): NOT ATTEMPTED in this wave; the blocker is
 * measured and recorded.
 *
 * Three 128-bit copies:
 *   `addiu $a0,$s1,0x10 ; lq $v0,0x0($s2) ; sq $v0,0x0($a0)`,
 *   `addiu $v1,$s1,0xF0 ; lq $v0,0x0($s3) ; sq $v0,0x0($v1)` and
 *   `lq $v0,0x0($s4) ; sq $v0,0x0($s0)`.
 * The third is a plain register on both sides and compiles as it is; the first two have a
 * constant destination offset and this cc1 folds it into the `sq`, so each of them loses
 * the retail's `addiu` and the body is eight bytes short.
 *
 * The one escape of that wall that exists (measured in this wave, see
 * nonmatching/levels/func_L20_004499E8.c) is a volatile pointer OBJECT, which reaches the
 * retail's zero-displacement `sq` at the price of a stack store and a reload - eight bytes
 * over per destination.
 */
/* Near miss from src/levels/20_yeedil/L20_00448A18-g8.c: 284 bytes of retail,
   268 bytes of candidate.  The body reaches four globals through $gp (the limit
   at 0x1AEFF0 + 0x33A0), so it belongs in a -G8 sibling.

   The shape is the retail's: the lookup of the object of type 0x125A, the
   null guard, the four arguments copied into $s0-$s3 in the prologue, the
   state pointer loaded from +0x68 before the first call, the clamp of the
   float at +0x2C, the three 16-byte copies, the call to func_002A0F48, the
   two scaled floats of func_002A7A00, the 1/3600 of the float at +0x20 of
   the state and the two ints at +0x24.

   What differs:

   1. The three 16-byte copies.  The retail materialises the address of the
      first two of them (`addiu $a0,$s1,0x10` and `addiu $v1,$s1,0xF0`) and
      stores with a zero displacement; this cc1 folds the offset into the
      `sq` (`sq $v0,0x10($s1)`).  The third copy (into the state pointer, an
      offset of zero) matches.  Measured on this body and on twenty other
      spellings (a plain TI128 store, a volatile pointee, a volatile pointer
      object, a pointer variable, an array index, a struct, a union, a
      memcpy, a 64-bit address, an out-of-range offset): every one of them
      folds.  The only spelling that materialises the address is an offset
      outside the TImode displacement range [-32768, 32752], and that keeps
      the wrong immediate.

   2. The order of the two float loads of the two calls to func_002A7878.
      The retail loads the absolute one (1.0f and 20.0f) before the call
      and the $gp one (0.666f and 7.0f) in the delay slot; the candidate
      loads them in the source order, so the absolute one lands in the delay
      slot - and there the assembler can only use the $gp form, so the
      absolute load also comes out wrong.

   The globals of the level: 0x1B2378 (7.0f), 0x1B2380 (10.0f), 0x1B2388
   (180) and 0x1B2390 (0.666f) are reached through $gp; 0x1B237C (20.0f),
   0x1B2384 (90.0f), 0x1B238C (240) and 0x1B2394 (1.0f) are absolute.
*/

#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern void *func_0029EFD0(int);

extern void func_002B37E8(void *, void *);

extern float func_002A7878(float, float);

extern void func_002A7A00(void *, float, float);

extern void func_002A0F48(void *);

extern int func_002A7820(int, int);

extern float D_L20_001B2394;
extern float D_L20_001B2384;
extern float D_L20_001B237C;
extern int D_L20_001B238C;

extern float D_L20_001B2390 __attribute__((sda));
extern float D_L20_001B2380 __attribute__((sda));
extern int D_L20_001B2388 __attribute__((sda));
extern float D_L20_001B2378 __attribute__((sda));

char *func_L20_00448A18(char *arg0, char *arg1, char *arg2, char *arg3) {
    char *obj = (char *)func_0029EFD0(0x125A);
    char *state;
    float f;

    if (obj != 0) {
        state = *(char **)(obj + 0x68);
        func_002B37E8(obj, arg0);
        f = func_002A7878(D_L20_001B2390, D_L20_001B2394);
        *(float *)(obj + 0x2C) = *(float *)(*(char **)(obj + 0x24) + 0x24) * f;
        *(volatile TI128 *)(obj + 0x10) = *(TI128 *)arg1;
        *(volatile TI128 *)(obj + 0xF0) = *(TI128 *)arg2;
        *(volatile TI128 *)state = *(TI128 *)arg3;
        func_002A0F48(obj);
        func_002A7A00(state + 0x10, D_L20_001B2380 * 0.000290888245f,
                      D_L20_001B2384 * 0.000290888245f);
        f = func_002A7878(D_L20_001B2378, D_L20_001B237C);
        *(float *)(state + 0x20) = f * 0.00027777778f;
        *(int *)(state + 0x24) = func_002A7820(D_L20_001B2388, D_L20_001B238C);
    }
    return obj;
}
