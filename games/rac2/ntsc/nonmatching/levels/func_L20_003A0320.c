/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   No C was written for this body in this wave; what is recorded here is
   the structure that was mapped out of the retail and the walls it runs
   into, so the next attempt starts from the structure and not from the
   disassembly.

   func_L20_003A0320 is 680 bytes and dispatches on the byte at +0x20 of
   its argument:

     - the retail's dispatch is a chain of compares, not a jump table:
       `beq $v1,$s2` for the value 1, then `slti $v0,$v1,0x2` and a
       `beql` for the values 0 and 2, then `beq $v1,$v0` for the value 3
       and `bnel` for everything else.  In C that is a `switch` on the
       byte, whose arms the compiler lays out in the order of the case
       labels - which is how the retail's order can be read back;
     - the arms call f_00331BB8, f_00339620, f_00334408 and others, and
       they pass a block of five words on the stack (`addiu $a2,$sp,0x20`
       and the stores at +0x00, +0x08, +0x10, +0x18), so the C needs a
       local struct or array whose address is passed;
     - the float constants are spelled out as literals in the retail
       (0x3F666666, 0x3FA66666, 0x3F800000, 0x3F000000, 0x41100000,
       0x41700000, 0x40C00000, 0x40400000).  0x3F666666 is 0.9f and
       0x3FA66666 is 1.3f as the compiler rounds them; both are the
       nearest float to the decimal, so they come out of the C as they
       are written;
     - the prologue interleaves the body with the saves (the `daddu
       $s0,$a0` sits between `sd $s2` and `sd $s1`), the
       prologue-interleaving wall, and the early exits use the
       branch-likely form (see the note on func_L20_0039D680).

   The body reaches D_L20_001B29E0 and its neighbours through $gp (the
   file is the -G8 unit for that reason).
*/
#include "include_asm.h"

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c: 680 bytes; the candidate
   has the retail's frame (0x60) and the retail's block order, but the
   registers of the prologue and of the case bodies differ.

   The finding of this attempt is the dispatch: the retail does NOT use a
   jump table, it uses a comparison chain

       lbu   $v1,0x20($s0)
       beq   $v1,$s2,.L003A038C        ; case 1
       slti  $v0,$v1,0x2
       beql  $v0,$zero,.L003A036C      ; { 2, 3 }
       beqz  $v1,.L003A0384            ; case 0
       beq   $v1,$v0,.L003A0408        ; case 2
       beq   $v1,$v0,.L003A0568        ; case 3

   and a C `switch` with four cases and call bodies produces exactly that
   chain - including the branch-likely `beql` - as long as the compiler
   does not build a jump table.  Measured: a switch with three or four
   cases gives the chain, five or more gives a jump table in .rdata
   (which the build discards).  So the wall of this function is not the
   dispatch.

   What is still different:
   - the prologue's registers: the retail keeps the object in $s0, the
     state in $s1 and the constant 1 in $s2 (a callee-saved register,
     for one comparison), this candidate keeps the object in $s1, the
     state in $s0 and the constant in a temporary;
   - the state is loaded in the delay slot of the switch's first branch
     in the retail, which means it is read before the switch: the
     candidate has to read it before the switch as well, or the case
     bodies read it again;
   - the rest of the case bodies (the float arguments of func_00334408
     and func_0033B2F8, the constants) has not been matched yet.

   The body belongs in the -G8 unit: it reads D_L20_001B2F9C and
   D_L20_001B2ECC through $gp. */
extern char *D_L20_001B2F9C_1 __attribute__((sda));

extern float D_L20_001B2ECC __attribute__((sda));

extern char D_001A8F20[];

extern int func_00331BB8(char *, int, int);

extern void func_0032F5B0(char *, int, int);

extern void func_00339620(char *, char *, f32, f32, f32);

extern void func_00334408(f32, f32, f32, f32, f32, f32, f32, char *, char *, char *,
                          int, char *, int, int, int, int, int, int, int);

extern void func_0033B2F8(f32, f32, f32, f32, char *, int, char *, int);

extern void func_0033B1E8(char *, int, f32);

extern void func_00375480(int, int, char *);

extern int func_0033E788(char *);

extern int func_00301ED0(char *);

extern void func_003253A8(char *);

void func_L20_003A0320(char *arg0) {
    char *s0 = arg0;
    char *s1;
    f32 buf[4];
    f32 f20;
    f32 f21;
    int v0;

    s1 = *(char **)(s0 + 0x68);

    switch (*(u8 *)(s0 + 0x20)) {
    case 0:
        func_0032F5B0(s0, 1, -1);
        break;
    case 1:
        v0 = func_00331BB8(s0, 0x40100000, 0);
        if (v0 == 0) {
            v0 = 0xFF;
        } else if (*(int *)s1 != 0) {
            v0 = 0xFF;
        } else {
            func_0032F5B0(s0, 2, -1);
            break;
        }
        *(u8 *)(s0 + 0xA8) = v0;
        v0 = *(int *)(s1 + 4);
        if (v0 <= 0) {
            break;
        }
        v0 = (int)((char *)D_L20_001B2F9C_1 + (v0 << 8));
        if (v0 == 0) {
            break;
        }
        if (*(u8 *)(v0 + 0x20) == 0xFE || *(u8 *)(v0 + 0x20) == 0xFD ||
            *(u8 *)(v0 + 0x20) == 3) {
            func_0032F5B0(s0, 2, -1);
        }
        break;
    case 2:
        f20 = 0.0f;
        f21 = 3.0f;
        func_00339620(s0, (char *)buf, f20, f20, f21);
        if (*(u8 *)(s0 + 0x31) != 0 && D_L20_001B2ECC < 0.9f) {
            func_00334408(f21, f20, 1.0f, 0.5f, 9.0f, *(f32 *)(s0 + 0x2C) * 1.3f, 15.0f,
                          s0, D_001A8F20, (char *)buf, 10, s0, 3, -1, 16, 0, -1, 0, 0);
            func_0033B2F8(f21, 6.0f, f21, 6.0f, s0, 1, D_001A8F20, 0xFF);
        } else {
            func_0033B2F8(3.0f, 6.0f, 3.0f, 6.0f, s0, 1, D_001A8F20, 0xFF);
        }
        func_0033B1E8(s0, 1, 0.0f);
        func_00375480(0, 0, s0);
        func_0032F5B0(s0, 3, -1);
        break;
    case 3:
        if (func_0033E788(s0) != 0) {
            *(u16 *)(s0 + 0x34) |= 1;
            *(u32 *)(s0 + 0x98) = 0;
            *(u8 *)(s0 + 0x31) = 0;
            *(u32 *)(s1 + 8) = 0x78;
        }
        if (func_00301ED0(s1 + 8) != 0) {
            func_003253A8(s0);
        }
        break;
    }
}
