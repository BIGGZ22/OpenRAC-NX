/* Near miss from src/levels/20_yeedil/L20_002B7A48-g8.c: the body is 556 bytes, the
   retail's size, but six words (24 bytes) differ, so the build refuses it at its
   place.  Kept as a starting point; not linked, not counted.

   This function belongs in the -G8 unit (src/levels/20_yeedil/L20_002B7A48-g8.c):
   it reaches three globals through $gp.

   WHAT WAVE 3 FOUND (the wave 2 attempt had twelve wrong words):

   1. THE `nosda` ATTRIBUTE CHANGES THE CODE.  In a -G8 unit a plain `extern`
      is read ABSOLUTELY by default; adding `__attribute__((nosda))` makes cc1
      materialise the address in a register of its own and load through it
      ("lui $v0,%hi(S)" + "lw $v1,%lo(S)($v0)"), where the retail has the one
      register form ("lui $v1,%hi(S)" + "lw $v1,%lo(S)($v1)").  Dropping `nosda`
      from D_001A79F0 and from the absolute name of D_001A7390 fixed nine of the
      twelve wrong words at once.  So: in a -G8 file, use a plain `extern` for an
      absolute access, and `__attribute__((sda))` for the $gp one; `nosda` is not
      only unnecessary, it is harmful.

   2. The twin of this body in level 18 (func_L18_002C41F0,
      src/levels/shared/L18_002C4000-g8.c) is the SAME game code and it is
      MATCHED.  Its C is the template: the four tests of the first block are
      nested `if` statements, not one `&&` chain, and that is what gives the
      retail's branch-likely shapes.

   The globals the retail reaches through $gp are
     lw $v0, 0xF10($gp)     -> 0x001AF000, the level word D_L20_001AF000
     lw $v1, -0x7C60($gp)  -> 0x001A7390, the boot word D_001A7390
     lwc1 $f0, -0x72F8($gp) -> 0x001A7CF8, the boot float D_001A7CF8
   and D_001A7390 is read a second time, absolutely ("lui $v1,0x1A; lw $v1,0x7390($v1)"),
   so it needs the double-name pattern: the sda name is the $gp form, a second name
   marked nosda the absolute one.  The other three symbols the retail reads
   absolutely (D_001A79F0, D_0013965E and D_002683EC) need nosda as well: with the
   sda form cc1 reaches them through $gp, and their offsets do not fit the $gp
   window (0x001AEFF0 - 0x0013965E is 0x75992, past the 0x8000 the small data area
   allows).

   What the source has to say, so that cc1 gives the retail's code:

     - the two "state byte == 0" tests are early returns: that is what gives the
       retail's "beql" with the "ld $s0,0($sp)" in the delay slot;
     - the pointer is taken after the three first calls (the retail's "lui" comes
       after the three "jal"s), and the state byte is read as a direct expression
       ("*(u8 *)(s0 + 0x22B4) == 1"), not through a variable: with a variable the
       byte lands in $a0 and the constant 1 in $v1, the retail has the byte in $v1
       and the constant in $a0;
     - the address of D_00189E20 is taken three times, once for the pointer and
       once in each of the two later blocks.  The second and the third are in
       blocks of their own, so cc1 does not fold them into the first one, and it
       gives them the "sign-extended hi" form ("daddu $s1,$v0,$zero" then
       "addiu $s0,$s1,%lo(D)") that the retail has;
     - D_0018C958 has to be reached through a pointer variable, or cc1 folds the
       +0x7900 into the %lo of the load instead of keeping the base in a register
       as the retail does;
     - the test on D_001A79F0 runs when the difference to the word at +0x7900 is
       0xB5 or more (">= 0xB5", not "< 0xB5"), and the comparison of that word with
       D_001A7390 is unsigned ("sltu", not "slt").

   WHAT IS LEFT (six words):

     - 0x2B9544 and 0x2B9548: the retail's branch is the branch-likely "bnel"
       with the "lui $s0,%hi(D_189EA0)" in its delay slot (the delay slot of a
       branch-likely is annulled when the branch is not taken, so the $s0 of the
       block survives); this build gives a plain "bne" whose delay slot holds the
       "lwc1 $f0,-0x72F8($gp)" that opens the block.  The retail's delay slot
       instruction is a COPY of the block's last instruction ("lui $s0,%hi(D_189EA0)"
       at 0x2B9598), so the retail's compiler turned the branch into a branch-likely
       and duplicated the block's last instruction; this compiler fills the delay
       slot with the block's FIRST instruction instead.  Measured and NOT the
       answer: -fno-schedule-insns (the delay slot does not change), the if/else
       spelling of the state==1 block, a "goto" over it, an explicit assignment to
       the block's tail inside the block, and the "!= 1" spelling of the test.
     - 0x2B95AC, 0x2B95B0 and 0x2B95B4: the last test (the one after
       "base = D_00189EA0 - 0x80") materialises the constant 4 BEFORE it loads the
       state byte ("addiu $v1,$zero,0x4" + "lbu $v0,0x22B4($s0)"), this build loads
       the byte first ("lbu $v1,0x22B4($s0)" + "addiu $v0,$zero,0x4") and puts the
       byte in $v1 and the constant in $v0.  The same source in level 18 (where
       the pointer is not shifted by -0x80) gives the level 18 retail's order, so
       the two levels' sources differ here.  Measured and NOT the answer: the
       constant on the left of the comparison, the byte through a variable, the byte
       through a u8 pointer, an (s32) cast of the byte, the && chain and the nested
       ifs, a switch on the state byte, and writing the tail as
       "base = (char *)D_00189EA0; base = base - 0x80;" so that the pointer is a
       runtime value and not a folded constant.

   Measured with tools/check.py --diff: 556 bytes, the retail's size, six words
   differ. */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef signed long long s64;
typedef float f32;

extern s32 D_L20_001AF000 __attribute__((sda));
extern s32 D_001A7390 __attribute__((sda));
extern s32 D_001A7390_1 __attribute__((nosda));
extern float D_001A7CF8 __attribute__((sda));
extern s32 D_001A79F0;
extern u8 D_0013965E __attribute__((nosda));

extern char D_00189E20[];
extern char D_00189EA0[];
extern char D_0018C958[];
extern float D_002683EC __attribute__((nosda));

extern void func_L00_002D24E8(void);
extern void func_L18_002D0C00(void);
extern void func_L00_002ACA08(void);
extern void func_L00_002ACF88(char *, char *, char *);
extern void func_L00_002B8930(void);
extern s32 func_00282978(s32);
extern void func_L20_002D7730(s32, s32);
extern s32 func_00288E18(s32, s32);
extern void func_L20_002DEBE0(void);
extern void func_L00_002AE0C8(void);
extern void func_L00_002B9CA8(void);
extern void func_L00_002A5368(void);
extern void func_L00_002A9600(void);
extern void func_L00_002A7958(void);
extern void func_L00_002A7748(void);
extern void func_L00_002A5840(float *);
extern void func_L00_002A5CB0(void);
extern void func_L00_002A6008(void);
extern void func_002E5CF8(void *, void *, void *);
extern void func_L00_003080D0(s64);
extern void func_L18_00428100(s32);
extern void func_L00_002A9460(void);
extern void func_002A677C(s32);

void func_L20_002B93E0(void)
{
    char *base;
    char *blk;
    char *hero;
    unsigned int now;
    unsigned int then;

    func_L00_002D24E8();
    func_L18_002D0C00();
    func_L00_002ACA08();

    base = D_00189E20;
    if (*(u8 *)(base + 0x22B4) == 1) {
        func_L00_002ACF88(base + 0x80, base + 0x90, base + 0x550);
        func_L00_002B8930();
    }
    if (*(u8 *)(base + 0x22B4) == 4) {
        if (D_L20_001AF000 == 0) {
            if (*(s16 *)(base + 0x1E0) == 0) {
                if (*(int *)(base + 0x2294) != 0x6F) {
                    if (func_00282978(*(int *)(*(int *)(base + 0x2290) + 0x68) + 0x3BC) != 0) {
                        *(int *)(base + 0x24CC) = 0;
                        func_L20_002D7730(0x91, 1);
                    }
                    if (D_001A79F0 == 0x14) {
                        if (D_0013965E == 0) {
                            blk = D_0018C958;
                            if (D_001A7390_1 - *(unsigned int *)(blk + 0x7900) >= 0xB5) {
                                if (func_00288E18(0xA15, 0x6C) != 0) {
                                    now = D_001A7390;
                                    then = *(unsigned int *)(blk + 0x7900);
                                    if (then < now) {
                                        *(unsigned int *)(blk + 0x7900) = now;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    base = D_00189E20;
    func_L20_002DEBE0();
    func_L00_002AE0C8();

    if (*(u8 *)(base + 0x22B4) == 0)
        return;
    func_L00_002B9CA8();
    func_L00_002A5368();
    if (*(u8 *)(base + 0x22B4) == 0)
        return;
    func_L00_002A9600();
    func_L00_002A7958();
    func_L00_002A7748();
    if (*(u8 *)(base + 0x22B4) == 1) {
        D_002683EC = D_001A7CF8;
        func_L00_002A5840(&D_002683EC);
        func_L00_002A5CB0();
        func_L00_002A6008();
        if (*(u32 *)(base + 0x24E4) != 0) {
            func_002E5CF8((void *)(*(u32 *)(base + 0x24E4) + 0x10), 0,
                          (void *)(*(u32 *)(base + 0x2290)));
            *(u16 *)(base + 0x2520) = 0x39;
            *(u16 *)(base + 0x2522) = *(u16 *)(base + 0x24E4);
        }
        *(s32 *)(base + 0x24E4) = 0;
    }

    func_L00_003080D0((s64)D_00189EA0);
    base = (char *)D_00189EA0 - 0x80;

    if (*(u8 *)(base + 0x22B4) == 4 && D_L20_001AF000 == 0 && *(s16 *)(base + 0x1E0) == 0 &&
        *(int *)(base + 0x2294) != 0x6F) {
        func_L18_00428100(*(int *)(base + 0x2290));
    }
    func_L00_002A9460();
    hero = D_00189E20;
    func_002A677C(*(int *)(hero + 0x2290));
}
