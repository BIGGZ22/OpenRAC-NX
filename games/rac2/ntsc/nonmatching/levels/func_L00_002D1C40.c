/* 0 of 27 places match - last line: 00_aranos_tutorial: 344 bytes, expected 352

   The body is in place: the two live objects the resident state points at, the
   `func_002D1DA0` call with the two output words, the two `func_002A7470` calls of the
   first path, the `func_002D1B70` fallback, the flag block with its `bnel` and `movz`
   forms and the two "one" actions of the second path.

   What differs is the address of the resident state.  Retail keeps the high half of
   0x00189E20 in a callee-saved register and re-materialises the low half at the two
   late accesses:
       lui $v0, 0x19
       addiu  $a1, $v0, -0x61E0    ; the state, in $a1
       daddu  $s4, $v0, $zero     ; the high half, kept across the call
       ...
       addiu  $v1, $s4, -0x61E0
       lw     $v0, 0x229C($v1)
   so retail splits the address; this cc1 keeps the whole pointer instead - either in a
   callee-saved register ($s0, when the state is a local variable) or re-materialising
   the high half at every use (`lui $v0, %hi(state)` + `lw $v0, 0x229C($v0)`).  That
   is two instructions short of retail in the prologue (the fifth saved register and
   the copy of the high half) and one instruction too many at each of the two late
   accesses.

   Measured: the state as a local variable, the state as the symbol at every access,
   a second name for the same address for the late accesses only, a separate local
   for the late accesses, and the address written as the literal 0x00189E20 all keep
   the pointer whole; none of them splits it.  The rest of the body matches. */
#include "common.h"

extern char D_00189E20_36[];

extern char D_00189E20_37[];

extern int func_L00_002D1DA0(int, int *, int *);

extern int func_L00_002D1B70(int, int *);

extern int func_L00_002ADEA0(int);

extern void func_002A7470(void *, int, int, int);

void func_L00_002D1C40(int a0, int a1, float f12)
{
    char *base = D_00189E20_36;
    char *p1;
    char *p2;
    int out0;
    int out1;
    int out2;
    int flag;
    int s0;
    float f20 = f12;

    if (*(int *)(base + 0x1334) != 2)
        return;
    p1 = *(char **)(base + 0x1310);
    if (p1 == 0)
        return;
    p2 = *(char **)(base + 0x1314);
    if (p2 == 0)
        return;

    if (func_L00_002D1DA0(a0, &out0, &out1) != 0 && a0 != 0) {
        if ((unsigned char)p1[0x43] != out0) {
            s0 = (int)f20;
            func_002A7470(p1, out0, a1, s0);
            func_002A7470(p2, out1, a1, s0);
        }
        return;
    }

    flag = 1;
    {
        char *g = D_00189E20_37;

        if (*(int *)(g + 0x229C) == 0) {
            if (func_L00_002D1B70((unsigned char)p1[0x43], &out2) != 0 && out2 == 0)
                flag = 0;
        }
        if (flag == 0)
            return;

        s0 = 7;
        if (*(int *)(g + 0x22A0) == 8) {
            if (func_L00_002ADEA0(3) == 2)
                s0 = 0x13;
        }
    }
    if ((unsigned char)p1[0x43] != 1)
        func_002A7470(p1, 1, 0, s0);
    if ((unsigned char)p2[0x43] != 1)
        func_002A7470(p2, 1, 0, s0);
}
