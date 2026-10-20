/* from src/core/00130288.c: an IPU/decoder setup - func_12FA98(*(a0+0x858), &mode) with
   mode 2 then 3, two func_12C3B0(s) calls, a zero store to 0x10002000, func_11F5E0 /
   func_11F628 around it, then 0x1000B410 = a2 & 0x0FFFFFFF, 0x1000B420 = 4,
   0x1000B400 = 0x101, and func_12C380(s, a1) with the last func_12FA98 call.
   The mode stores are merged into the call delay slots (a shape measured elsewhere as
   hard to force), and the two `lui $at` hardware stores sit between register-based ones.

   WAVE 4 (this attempt): the body below is NEW - no C had ever been written for this
   function. It compiles to 228 bytes against retail's 224 and matches retail instruction
   for instruction apart from ONE delay slot, plus it has already fixed the frame:

     * the local is a 32-byte payload (`int buf[8]`), not an `int`: with `int mode` the
       frame comes out 0x50 with the saves at 0x10/0x20/0x30/0x40, where retail is 0x60
       with the saves at 0x20/0x30/0x40/0x50. Only buf[0] is ever written (the same
       0x0($sp) store retail has); the size is what the frame reports.
     * the residual: retail puts the 0x10002000 (IPU_CMD) clear in the delay slot of the
       SECOND func_12C3B0 call -
         retail : lui $v0,0x1000 ; daddu $a0,$s1,0 ; ori $v0,$v0,0x2000 ; jal ; sw $zero,0($v0)
         here   : lui $v0,0x1000 ; daddu $a0,$s1,0 ; ori $v0,$v0,0x2000 ; sw $zero,0($v0) ; jal ; nop
       This is the store-before-call (backward) fill this cc1 refuses. Measured and
       rejected: moving the store after both calls in the source (the RTL then puts the
       address setup after the jal and the store still lands three instructions later),
       dropping `volatile` on that store (it folds to the absolute `sw $zero,0x2000($at)`
       form and drops to 220 bytes), an unprototyped `func_0012C3B0` declaration, and
       the `sdk -O2 -g2 -gstabs` MPEG profile on a dedicated file (all still 228).
       The forward direction of the same fill does work elsewhere (func_0012CDB0's first
       IPU_CTRL read-modify-write), so the difference is the direction, not the store. */
#include "common.h"
#include "structs.h"

extern void func_0012C3B0();

extern int func_0011F5E0(void);

extern void func_0011F628(void);

extern void func_0012C380(void *, int);

extern void func_0012FA98(void *, void *);

/* rac1-decomp src/core/0012AC80.c: func_0012CBF0 */
void func_001307B0(void *arg0, int arg1, unsigned int arg2) {
    char *s = (char *)arg0;
    int buf[8];
    int intr;

    buf[0] = 2;
    func_0012FA98(*(void **)(s + 0x858), buf);
    func_0012C3B0(s);
    *(volatile unsigned int *)0x10002000 = 0;
    func_0012C3B0(s);
    intr = func_0011F5E0();
    *(volatile unsigned int *)0x1000B410 = arg2 & 0x0FFFFFFF;
    *(volatile unsigned int *)0x1000B420 = 4;
    *(volatile unsigned int *)0x1000B400 = 0x101;
    if (intr != 0) {
        func_0011F628();
    }
    func_0012C380(s, arg1);
    func_0012C3B0(s);
    buf[0] = 3;
    func_0012FA98(*(void **)(s + 0x858), buf);
}
