#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

/* 128-bit copies reach lq/sq only through TImode. */
typedef int TI128 __attribute__((mode(TI)));

/* 0x1AEFF0 + 0x2E18 : the three floats the retail reads through $gp here. */
extern float D_L20_001B1E08 __attribute__((sda));

extern float D_L20_001B1E0C __attribute__((sda));

extern float D_L20_001B1E10 __attribute__((sda));

extern void func_00282CF0(void *, void *, void *);

extern void func_002A7750(char *, int, int);

extern void func_002AD6F0(char *, char *, char *, char *);

extern void func_L01_0031AB20(char *, char *, int, float, float, float);

/* Near miss from src/levels/20_yeedil/L20_0039C448.c: 256 bytes; 244-byte candidate; 0/1.
    This body belongs in the -G8 unit (src/levels/20_yeedil/L20_0039C448-g8.c): the retail
    reads the three floats of the `case 1` branch through $gp (`lwc1 $f12,0x2E18($gp)`,
    `lwc1 $f13,0x2E1C($gp)`, `lwc1 $f14,0x2E20($gp)`), which is 0x001B1E08, 0x001B1E0C and
    0x001B1E10 in level 20 - the three floats are 1.0, 1.0 and 6.0.  With the `sda`
    attribute in a -G8 file this cc1 emits exactly those three $gp loads.
    The frame (0x60), the three 16-byte stack buffers, the switch, both calls with their
    argument registers and the float constants are the retail's.
    Two things are left:
      - the 128-bit copies: the retail materialises the address of the load
        (`addiu $v1,$s1,0x10; lq $v0,0($v1)`) and of the store
        (`addiu $a0,$sp,0x10; sq $v0,0($a0)`) while this cc1 folds each constant offset
        into the `lq`/`sq`.  That is the wall of the other TI copies in this level (about
        thirty spellings measured, all of them fold).
      - the register allocation: the retail keeps the object in $s1, the state in $s2,
        the second buffer in $s3 and the third in $s0, this cc1 keeps the object in $s0,
        the state in $s1, the third buffer in $s2 and the second in $s3, and it saves the
        registers in a different order (`sd $s0,0x30($sp); sd $s2,0x40($sp)` against the
        retail's `sd $s1,0x38($sp); sd $s0,0x30($sp)`).
    The candidate below is the closest measured: 244 bytes against the retail's 256. */

/* Wave 3 (worker 07), measured on this body and not in the earlier note:
   - the retail's three materialised addresses are `addiu $v1,$s1,0x10` (the
     source of the first copy), `addiu $a0,$sp,0x10` (the destination of the
     second) and `addiu $v1,$s1,0xF0` (the source of the second); the
     destination of the first copy is `sq $v0,0($sp)`, folded.
   - a TImode access only keeps the address in a register when the address
     expression is in DI mode: with a `char *` or a `u32` base this cc1 folds
     the offset into the `lq`/`sq` even when the expression is used twice (the
     address of the global and of the stack buffer are both used twice in the
     retail and in the candidate), and with a `u64` base it materialises the
     address but computes it with `daddu` (64 bits) where the retail has
     `addiu` (32 bits).  There is no spelling that materialises a 32-bit
     address.
   - the volatile pointer object (escape B) does give the zero-displacement
     `sq`, at the price of a stack store and a reload (`sw ; lq ; lw ; sq`): two
     instructions over the retail's `addiu ; sq`, and the wrong instructions.
   - the register allocation of the earlier candidate stands: the retail keeps
     the object in $s1, the state in $s2 and the buffers in $s3 and $s0, this
     cc1 in $s0, $s1, $s2 and $s3. */
void func_L20_004395B0(char *arg0) {
    char *s1 = arg0;
    char *s2 = *(char **)(s1 + 0x68);
    TI128 buf[3];
    char *s0 = (char *)&buf[2];
    char *s3 = (char *)&buf[1];

    buf[0] = *(TI128 *)(s1 + 0x10);
    buf[1] = *(TI128 *)(s1 + 0xF0);

    switch (*(u8 *)(s1 + 0x20)) {
    case 0:
        *(u16 *)(s1 + 0x34) |= 0x1000;
        *(f32 *)(s2 + 0x84) = 7.0f;
        *(int *)(s2 + 0xDC) |= 1;
        *(f32 *)(s2 + 0x88) = *(f32 *)(s2 + 0xE4);
        func_002A7750(s1, 1, -1);
        break;
    case 1:
        func_L01_0031AB20(s1, s2 + 0x50, *(int *)(s2 + 0xE0),
                          D_L20_001B1E08, D_L20_001B1E0C, D_L20_001B1E10);
        break;
    }
    func_00282CF0(s0, s1 + 0x10, (char *)buf);
    func_002AD6F0(s2 + 0xA0, s0, s3, s1 + 0xF0);
}
