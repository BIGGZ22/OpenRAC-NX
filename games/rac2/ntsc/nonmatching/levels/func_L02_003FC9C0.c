#include "include_asm.h"

typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef float f32;

/* func_L02_003FC9C0 -- 188 bytes, level 2.  The body is read end to end: in state 8
 * the sub-object's 0xC0 slot is spawned through func_L00_002D70A0(5); otherwise the
 * function is closed through func_L00_002D7140 and the slot cleared.  When the 0xD0
 * source object exists, its id at +0xAA picks one of two float triples (0x9F8, 0xA44),
 * otherwise a third triple is used, and the three floats are written to the 0xC0
 * object.
 *
 * retail:
 *   0x003FC9C0  addiu sp,sp,-0x10 ; v1 = 8 ; sd s0,0x0(sp) ; sd ra,0x8(sp)
 *   0x003FC9D0  lbu v0,0x20(a0) ; bne v0,v1,+0xA0 ; lw s0,0x68(a0) [delay]
 *   0x003FC9DC  lw a0,0xC0(s0) ; bnel a0,zero,+0x38 ; lw v0,0xD0(s0) [delay]
 *   0x003FC9E8  jal func_L00_002D70A0(5) ; sw v0,0xC0(s0) [delay] ; b +0xAC
 *   0x003FC9F8  beqz v0,+0x80 ; lwc1 f0,0x2244(gp) [delay]
 *   0x003FCA00  lh v1,0xAA(v0) ; li v0,0x9F8 ; bne v1,v0,+0x64 ; li v0,0xA44 [delay]
 *   0x003FCA10  f0 = 0x2250(gp) ; f1 = 0x2254(gp) ; swc1 f0,0x0(a0) ; b ; f2 = 0x2258(gp)
 *   0x003FCA24  bnel v1,v0,epilogue ; ld s0,0x0(sp) [delay]
 *   0x003FCA2C  f0 = 0x225C(gp) ; f1 = 0x2260(gp) ; swc1 f0,0x0(a0) ; b ; f2 = 0x2264(gp)
 *   0x003FCA40  f1 = 0x2248(gp) ; swc1 f0,0x0(a0) ; f2 = 0x224C(gp)
 *   0x003FCA4C  lw v0,0xC0(s0) ; swc1 f1,0x4(v0) ; lw v1,0xC0(s0) ; b ; swc1 f2,0x8(v1)
 *   0x003FCA60  jal func_L00_002D7140(a0 = 0xC0(s0)) ; sw zero,0xC0(s0)
 *
 * measured: the nine $gp floats are 0x001B1234..0x001B1254 (gp = 0x001AEFF0), which is
 * what the `sda` declarations below name.  The three stores re-read the 0xC0 field
 * (retail reloads it into v0 and v1), so the source must re-read it too.  Candidate
 * comes out 208 bytes against 188 with the arms laid out the other way (this cc1 emits
 * `bnel` where retail has a `beqz` to the else arm) - block layout, not the source.
 * $gp recipe (lead, 2026-10-16, re-measured here): both `extern f32 X __attribute__((sda));` and
 * `extern f32 X __asm__("X_asm") __attribute__((section(".sdata")));` leave the reference BARE in
 * the .s (`l.s $f0,D_L02_001B1234`), i.e. GPREL16, which is retail's form.  The `lui $at`+absolute
 * pair seen in this candidate is the ASSEMBLER expanding the reference at that use site, not the
 * declaration spelling; the alias form is worth retrying here first for the next worker.
 */

extern void *func_L00_002D70A0(s32);
extern void func_L00_002D7140(void *);
extern f32 D_L02_001B1234 __attribute__((sda));
extern f32 D_L02_001B1238 __attribute__((sda));
extern f32 D_L02_001B123C __attribute__((sda));
extern f32 D_L02_001B1240 __attribute__((sda));
extern f32 D_L02_001B1244 __attribute__((sda));
extern f32 D_L02_001B1248 __attribute__((sda));
extern f32 D_L02_001B124C __attribute__((sda));
extern f32 D_L02_001B1250 __attribute__((sda));
extern f32 D_L02_001B1254 __attribute__((sda));

void func_L02_003FC9C0(u8 *a0) {
    u8 *s0 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x20) == 8) {
        u8 *obj = *(u8 **)(s0 + 0xC0);

        if (obj == 0) {
            *(u8 **)(s0 + 0xC0) = func_L00_002D70A0(5);
        } else {
            u8 *src = *(u8 **)(s0 + 0xD0);
            f32 f0, f1, f2;

            if (src == 0) {
                f0 = D_L02_001B1234;
                f1 = D_L02_001B1238;
                f2 = D_L02_001B123C;
            } else if (*(s16 *)(src + 0xAA) == 0x9F8) {
                f0 = D_L02_001B1240;
                f1 = D_L02_001B1244;
                f2 = D_L02_001B1248;
            } else if (*(s16 *)(src + 0xAA) == 0xA44) {
                f0 = D_L02_001B124C;
                f1 = D_L02_001B1250;
                f2 = D_L02_001B1254;
            } else {
                return;
            }
            *(f32 *)(obj + 0) = f0;
            *(f32 *)(*(u8 **)(s0 + 0xC0) + 4) = f1;
            *(f32 *)(*(u8 **)(s0 + 0xC0) + 8) = f2;
        }
    } else {
        func_L00_002D7140(*(u8 **)(s0 + 0xC0));
        *(u8 **)(s0 + 0xC0) = 0;
    }
}
