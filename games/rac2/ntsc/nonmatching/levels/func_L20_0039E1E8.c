/* Near miss from src/levels/20_yeedil/L20_0039C448.c: 352 bytes; 336-byte candidate; 0/1.

    The whole body is there - the frame (0x40), the two branches, the
    float constants, the call sequence and the epilogue - and only three
    things differ:

      - the register allocation.  The retail keeps the argument in $s1
        (`daddu $s1,$a0,$zero`), the state pointer in $s2
        (`lw $s2,0x68($s1)`), the copy of the object pointer in $s3 and
        the address of the global in $s4; this cc1 gives the argument
        $s2 and the state pointer $s1, and it uses only four callee-saved
        registers where the retail uses five;
      - the retail keeps a COPY of the object pointer in $s3
        (`daddu $s3,$s0,$zero`, in both branches) where this cc1 drops
        the copy and uses $s0 itself;
      - the comparison of the float branch is `c.lt.s $f21,$f0` in the
        retail and `c.olt.s $f21,$f0` here (the same test, the other
        encoding); `if (!(a > b))` gives `c.ole.s` with the operands
        swapped, and the plain `if (a < b)` gives `c.olt.s`.

    The globals: D_00189E20 is the boot's table (the retail keeps its
    address in $s4 and reads the field at +0x2290 through it), and
    D_L20_001B9FD8 is a level float (read absolutely, `lwc1
    %lo(D_1B9FD8)($v0)`).

    The float constants: 8.0f, 12.0f, -45.0f, 45.0f and 0.017453292f
    (pi/180, 0x3C8EFA36). */

#include "common.h"

extern f32 func_002A7878(f32, f32);
extern f32 func_00283B98(f32, f32);
extern f32 func_00283180(f32);
extern f32 func_00283198(f32);
extern void func_00282CC0(char *, char *, char *);
extern void func_00282CF0(void *, void *, void *);
extern f32 func_00282DF0(f32 *);
extern void func_00282F70(char *, char *, f32);
extern f32 func_00282E80(void *, void *);
extern char D_00189E20[];
extern float D_L20_001B9FD8;

void func_L20_0039E1E8(char *arg0) {
    char *s1 = arg0;
    char *s2 = *(char **)(arg0 + 0x68);
    char *tbl = D_00189E20;
    char *s0;
    char *s3;
    f32 f20;
    f32 f21;

    f21 = func_002A7878(8.0f, 12.0f);
    if (*(char **)(s2 + 0x200) == *(int *)(tbl + 0x2290)) {
        s0 = s2 + 0x260;
        s3 = s0;
        s1 = s1 + 0x10;
        f20 = func_002A7878(-45.0f, 45.0f);
        f20 = func_00283B98(f20 * 0.017453292f, D_L20_001B9FD8);
        *(f32 *)(s2 + 0x260) = func_00283180(f20) * f21;
        *(f32 *)(s2 + 0x264) = func_00283198(f20) * f21;
        *(int *)(s2 + 0x268) = 0;
        func_00282CC0(s0, s0, tbl + 0x80);
    } else {
        s0 = s2 + 0x260;
        s1 = s1 + 0x10;
        s3 = s0;
        func_00282CF0(s0, s1, *(char **)(s2 + 0x200) + 0x10);
        if (f21 < func_00282DF0((f32 *)s0)) {
            func_00282F70(s3, s3, f21);
        }
        func_00282CC0(s3, s3, *(char **)(s2 + 0x200) + 0x10);
    }
    *(f32 *)(s2 + 0x11C) = func_00282E80(s1, s3);
}
