/* Best attempt for func_L20_00449178 (444 bytes, 1 place) - NOT MARKED.
   G8: this body reaches the level's data through $gp (D_L20_001B23A0,
   D_L20_001B23A8, D_L20_001B23B0, addressed as $gp+0x33B0/$gp+0x33B8/
   $gp+0x33C0), so it belongs in the -G8 sibling of its unit
   (src/levels/20_yeedil/L20_00449178-g8.c), not in L20_00449178.c.

   last check.py line: "20_yeedil: 464 bytes, expected 404"
   (20 bytes over: the frame is retail's 0x50, but the body is longer.)

   What the retail body does, and what the attempt got wrong:

   - the dispatch tests "mode == 1", then "mode >= 2" (slti + a
     branch-likely beql), then "mode == 0"; the mode 2 block is the big
     one, the mode 0 block builds a 128-bit vector on the stack.
   - the attempt reproduces the dispatch and the block order, but cc1
     gives "mode >= 2" a plain `bnez` where retail has the branch-likely
     `beql`, and it allocates $s1 to the object and $s0 to the state
     where retail has $s0 for the object and $s1 for the state.
   - the 128-bit vector: retail writes it as `por $v0,$zero,$zero` +
     `sq $v0,0x20($sp)` (one 16-byte store of zero, the por in the
     delay slot of the branch into the block) and then `swc1 $f0,0x28($sp)`
     for the 2.0f in its upper half.  cc1 folds the zero store to
     `sd $zero` (or to two of them) and never emits the por; the docs
     say the por survives only when the zero feeds two or more stores,
     so the source must write the zero twice in a way that cc1 merges
     into one 16-byte store.
   - the argument setup: retail materialises the arguments of
     f_00353920 (func_L00_0033E220) as f12, a0, f13, a1, a2, a3, t0..t3,
     so the prototype is the interleaved `(void *, void *, int, int, int,
     int, int, int, float, float)` - the same order the already matched
     func_0032C700 call uses.  The constants are 420000.0f (0x48CD1400),
     1.0f, and the two INT constants 0x40204080 and 0x4010000 (they are
     bit patterns of floats, but they travel in integer registers).
   - the call to f_0033F540 (func_L00_0032A258) takes the addresses of
     the three level globals as pointers, so they must be declared with a
     type bigger than the -G8 threshold (a TI128) for cc1 to compute
     $gp+offset instead of loading them out of .sdata. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef int TI128 __attribute__((mode(TI)));

extern TI128 D_L20_001B23A0;

extern TI128 D_L20_001B23A8;

extern TI128 D_L20_001B23B0;

extern void func_L00_0033E220(void *, void *, int, int, int, int, int, int, float, float);

extern void func_002A7750(char *, int, int);

extern int func_002A9640(char *, int, int);

extern void func_L00_0032A258(char *, void *, void *, void *, int, int, int, int, int, int, int);

extern void func_L01_0031A278(void *, void *, float, float, float);

extern void func_0032C700(void *, int, int, int, int, int, int, int, float, float, int, int, int, int);

extern void func_L00_00326620(char *, int, int);

extern void func_0029F338(char *, char *);

void func_L20_00449178(char *arg0) {
    char *self = arg0;
    char *state = *(char **)(self + 0x68);
    int mode = *(u8 *)(self + 0x20);
    TI128 v;

    if (mode == 1) {
        goto L49230;
    }
    if (mode >= 2) {
        goto L492BC;
    }
    if (mode == 0) {
        goto L491CC;
    }
    return;
L491CC:
    *(u64 *)&v = 0;
    *(u64 *)((char *)&v + 8) = 0;
    *(float *)((char *)&v + 8) = 2.0f;
    func_L00_0033E220(&v, self, 0x40204080, -1, 1, -1, 0, -1, 420000.0f, 1.0f);
    func_002A7750(self, 1, -1);
    return;
L492BC:
    if (mode == 2) {
        goto L49270;
    }
    return;
L49230:
    if (func_002A9640(self, 0x4010000, 0) != 0) {
        if (*(u32 *)state == 0) {
            func_002A7750(self, 2, -1);
        }
    }
    *(u8 *)(self + 0xA8) = 0xFF;
    return;
L49270:
    func_L00_0032A258(self, &D_L20_001B23B0, &D_L20_001B23A8, &D_L20_001B23A0,
                      0xFFFF, 0, -1, 0, 0xFFFF, mode, 0);
    func_L01_0031A278(self, &v, 0.0f, 0.0f, 2.0f);
    func_0032C700(&v, 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 1, 0.5f, 20.0f, 1, 1, 0, 0);
    func_L00_00326620(self, 0x126A, 0);
    func_0029F338(self, self);
    return;
}
