/* 0 of 1 place match - last line: 02_maktar_nebula: instruction at +0x0 differs
   Switches on the byte at arg0+0x20 with the 0x68 sub-object read up front (retail sinks that
   load into the first `beqz` slot): case 0 writes 0xFF to +0x30, and when the byte at
   0x001A7AFA is clear sets the halfword at 0x0018C31C, then calls func_002A7750(arg0, 1, -1);
   case 1 calls func_L02_003DBFD8, then, when func_002ACBC8(D_00189EA0, *(int *)state) is true,
   writes 5 to *(short *)(D_00189EA0 + 0x19A).
   The switch chain itself matches (both `beqz mode` and `beq mode,1` jump forward over the two
   out-of-line bodies, and the operand order is the `unsigned char` one). What differs:
   this cc1 copies the incoming argument to $a1 before the tests (`daddu $a1,$a0,$zero` plus a
   matching `daddu $a0,$a1,$zero` before the call) because it materialises the 0x001A7AFA
   address in $a0, where retail keeps arg0 in $a0 throughout and materialises both the 0x001A7AFA
   and the 0x0018C31C addresses in $v1. That costs two instructions and 160 bytes against 152.
   Measured and identical: `if (!D_001A7AFA)`, the flag through an array element
   `D_001A7AFA[0]`, the scalar form, `0 == D_001A7AFA`, a named local for arg0, and the whole
   body as an if/else chain (that one does reach 152 bytes but this cc1 then emits
   `bnez $v1` to the else arm where retail branches `beqz $v1` to case 0, so the block order
   costs more words than the two it saves).
   What unblocks it: a lever that keeps a parameter in $a0 while a second address is
   materialised, so the scratch goes to $v1. */

#include "include_asm.h"

extern unsigned char D_001A7AFA;
extern char D_00189EA0[];
extern unsigned short D_0018C31C[];
extern int func_002ACBC8(void *, int);
extern void func_L02_003DBFD8(void);

void func_L02_003DBF40(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    switch (*(unsigned char *)(arg0 + 0x20)) {
    case 0:
        *(unsigned char *)(arg0 + 0x30) = 0xFF;
        if (D_001A7AFA == 0)
            D_0018C31C[0] = 1;
        func_002A7750(arg0, 1, -1);
        break;
    case 1:
        func_L02_003DBFD8();
        if (func_002ACBC8(D_00189EA0, *(int *)state))
            *(unsigned short *)(D_00189EA0 + 0x19A) = 5;
        break;
    }
}
