/* func_L23_00425848 (408 bytes, 1 place: level 23) -- NEAR MISS, not integrated.
   state: 388 bytes against retail's 408 (97 instructions against 102). Checked in the -G0 unit
   (L23_0041E418.c itself: the body has no `$gp` access).
   Everything after the quad region is right: renaming $s0 to $s1 and $s1 to $s2 in this output makes
   the instructions from the `lbu` of the state byte to the last call identical to retail's,
   relocated fields aside (the state switch with its two arms, the 2.0 / 1021.0 range tests on the
   three coordinates, the func_0029F338 arm, the saturating 16-bit counter, the func_00282948 tail);
   only the epilogue's register restores differ. Float constants are the ones the assembler
   encodes to retail's words (1021.0f is 0x447F4000).
   What differs is the three quad accesses and what follows from them:
   - retail keeps `pos = obj + 0x10` in $s0 (`addiu $s0,$s1,0x10` at +0x18, before the load of the
     payload) as the base of three quad accesses and as the first call argument, so it saves
     $s0, $s1, $s2 and $ra; this cc1 folds the +0x10 into every displacement (`sq $v0,0x10($s0)`),
     so the pointer has no register, only obj and the payload are saved, and the call argument is an
     `addiu $a0,$s0,0x10` where retail has `daddu $a0,$s0,$zero`: retail's `addiu $s0` is 1 missing
     word, the third saved register (`sd` and `ld`) 2 more;
   - retail materialises the address of the third local (`addiu $v1,$sp,0x20; ...; sq $v0,0($v1)`),
     this cc1 stores `sq $v0,0x20($sp)`: 1 word;
   - retail reloads the quad after storing it (`sq $v0,0($s0); lq $v0,0($s0); sq $v0,0($sp)`), this
     cc1 forwards the stored register to the second store: 1 word.
   Retail's quad accesses therefore behave like opaque block moves: pointer registers, zero
   displacements, no forwarding. In this cc1 the first cse pass (-ds dump of a reduced copy) rewrites
   a bare register address into the equivalent reg+const, apparently because reg+const has the
   lower address cost, and forwards the stored value to the following load.
   Measured without effect, each on a reduced copy of the quad region: a pointer variable, a
   `&((struct *)obj)->field` address, an integer sum, an array element address for the locals, a
   pointer that is also passed to two calls (it then keeps a register, but the quad access through it
   is still folded), the pointer to the third local assigned before or after the call, and
   -fforce-addr, -fforce-mem, -fno-cse-follow-jumps, -fno-cse-skip-blocks, -fno-gcse, -fno-regmove,
   -fcaller-saves, -O1. No matched function of the project has this shape (a scan of the retail
   bytes of the matched C for a materialised pointer feeding lq/sq finds only symbol addresses).
   With the rest correct, a compiler-side change is the way: the three quad accesses, not this body,
   are the whole gap. The three quad copies into locals are dead stores (nothing reads loc[0..2]);
   retail keeps them.
*/

typedef unsigned char u8;

typedef unsigned short u16;

typedef int s32;

typedef int TI128 __attribute__((mode(TI)));

extern void func_002AFFD8(void *, void *, s32);

extern s32 func_002A77E0(s32);

extern void func_L23_00425568(u8 *);

extern void func_L00_002E6EF8(void *, void *);

extern void func_L23_004259E0(u8 *);

extern void func_0029F338(u8 *);

extern s32 func_00282948(void *);

/* keeps the object's position vector in step with its owner's, runs the state it is in (0: arm
   the object, 1: refresh and re-register), and destroys it when it leaves the 2.0..1021.0 box or
   when its payload has no timer; otherwise it counts one more frame */
void func_L23_00425848(u8 *obj) {
    TI128 loc[3];
    u8 *pl = *(u8 **)(obj + 0x68);
    TI128 *pos = (TI128 *)(obj + 0x10);

    *pos = **(TI128 **)(pl + 0x24);
    loc[0] = *pos;
    func_002AFFD8(pos, &loc[1], 1);
    loc[2] = *pos;
    switch (obj[0x20]) {
    case 0:
        obj[0x20] = 1;
        *(u16 *)(obj + 0x34) |= 0x41;
        *(u16 *)(obj + 0x96) = func_002A77E0(0xFFFF);
        break;
    case 1:
        func_L23_00425568(obj);
        func_L00_002E6EF8((void *)func_L23_004259E0, obj);
        break;
    }
    if (*(float *)(obj + 0x10) < 2.0f || *(float *)(obj + 0x10) > 1021.0f
        || *(float *)(obj + 0x14) < 2.0f || *(float *)(obj + 0x14) > 1021.0f
        || *(float *)(obj + 0x18) < 2.0f || *(float *)(obj + 0x18) > 1021.0f
        || *(s32 *)(pl + 0x2C) == 0) {
        func_0029F338(obj);
    } else {
        u16 *cp = (u16 *)(obj + 0x96);
        *cp = *cp + 1;
        if (*cp > 0xFFFE) {
            *cp = 0;
        }
        if (*(s32 *)(pl + 0x2C) != -1) {
            func_00282948(pl + 0x2C);
        }
    }
}
