/* 0 of 1 places match - last line: 20_yeedil: 260 bytes, expected 264
   Near miss from src/levels/20_yeedil/L20_0043ECB0.c: 260 bytes of candidate against
   264 bytes of retail.  Every instruction of the retail is present, in the retail's order,
   with the retail's register allocation; the four bytes that are missing are the `por`
   the retail materialises the 16-byte zero in (`por $v0,$zero,$zero ; sq $v0,0x20($sp)`
   where this cc1 gives `sq $zero,0x20($sp)`).

   The shape is the retail's: the frame is 0xA0 with $s0-$s5 and $ra saved, the four
   arguments of the vector helpers, the state pointer at +0x68 of the argument, the
   difference of the two floats of the state, the zeroed 16-byte scratch with 0.5f in
   its third float, the seven helper calls and the guard on the call to func_00282948.

   Measured (2026-10-20, worker 03):
   * The scratch is ONE array of six 16-byte vectors, not six separate locals: the
     compiler allocates the locals in the order of their first use, and the retail's
     addresses (0x00, 0x10, 0x20, 0x30) with its 0xA0 frame (six vectors plus the
     saved registers) come out of `TI128 v[6]`.
   * The seventh call is func_00282CC0(arg0 + 0x10, arg0 + 0x10, scratch), not
     (arg0 + 0x10, state, scratch): the retail's `daddu $a1,$a0,$zero` in the delay
     slot of that call copies the pointer that is already in $a0.
   * The zero store is the zero-store wall (wall 4 of the wave-2 notes): the retail
     materialises the 16-byte zero in a register and this cc1 folds it into the
     store.  The fold (`rac2_fold_zero_ti_store`) applies only when the store is
     the register's last use AND the materialisation is within twelve instructions
     above it in the same block, so the por survives when the zero feeds two stores
     or when a call or a label stands between the materialisation and the store.
     Measured here and NOT enough: a second zero store to a dead local (the dead
     store is removed before the pass runs), a volatile pointee, a union with a
     float array, a TI128 local initialised to zero and copied, the zero as the
     value of a variable materialised before the first call (the compiler keeps it
     in a callee-saved register and the frame grows) - all of them fold.
*/

#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

typedef union {
    TI128 q;
    float f[4];
} Vec4;

extern void func_00283098(char *, char *, char *);
extern void func_00283958(char *, char *);
extern void func_00283738(char *, char *, char *);
extern void func_00282CF0(char *, char *, char *);

void func_L20_00443908(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    Vec4 v[6];

    func_00282CC0(arg0 + 0x10, arg0 + 0x10, state);
    *(float *)(state + 8) = *(float *)(state + 8) - *(float *)(state + 0x20);
    v[2].q = 0;
    v[2].f[2] = 0.5f;
    func_00283098(&v[0], &v[2], arg0 + 0xC0);
    func_00283958(state + 0x10, &v[3]);
    func_00283738(arg0 + 0xC0, &v[3], arg0 + 0xC0);
    func_00283098(&v[1], &v[2], arg0 + 0xC0);
    func_00282CF0(&v[2], &v[0], &v[1]);
    func_00282CC0(arg0 + 0x10, arg0 + 0x10, &v[2]);
    if (func_00282948(state + 0x24) != 0) {
        func_0029F338(arg0);
    }
}
