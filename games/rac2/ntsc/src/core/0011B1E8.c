#include "common.h"
#include "structs.h"

extern int func_0011B1E8();

extern int r1_D_0012FCF0 NOT_SDA;

/* r1_func_00119CC8 takes no argument (it sets $a0 itself before its first
   call); the file's `extern int r1_func_00119CC8(int)` below is wrong. */
extern int r1_func_00119CC8_v(void) __asm__("r1_func_00119CC8");

extern int r1_func_00119AA8(int, int);

extern int r1_func_00119CC8(int);

/* rac1-decomp src/core/00118A40.c: func_00119008 */
/* Deci2 tty write for fd 1/2: open the tty channel on first use
   (r1_func_00119CC8), then write. Calling the opener with no argument is
   what leaves retail's `jal; nop`: the `a0 = arg1` in the bnez slot is
   the write's argument, stolen from the branch target. */
/* r1_func_00119CC8 takes no argument; passing one puts a redundant move in
   the jal slot and swaps $s0/$s1. */
int func_0011B1E8(int fd, int buf, int len) {
    if (fd == 1 || fd == 2) {
        if (r1_D_0012FCF0 == 0) {
            if (r1_func_00119CC8_v() == 0) {
                return -1;
            }
            r1_D_0012FCF0 = 1;
        }
        return r1_func_00119AA8(buf, len);
    }
    return -1;
}

INCLUDE_ASM("asm/core", func_0011B268);

INCLUDE_ASM("asm/core", func_0011B270);

INCLUDE_ASM("asm/core", func_0011B320);

INCLUDE_ASM("asm/core", func_0011B328);

INCLUDE_ASM("asm/core", func_0011B3D0);

INCLUDE_ASM("asm/core", func_0011B450);

INCLUDE_ASM("asm/core", func_0011B458);

INCLUDE_ASM("asm/core", func_0011B500);

INCLUDE_ASM("asm/core", func_0011B580);
