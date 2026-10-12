#include "common.h"
#include "structs.h"

extern int r1_D_00130BD0[];

extern char r1_D_00130428[];

extern int func_0011F8D0(void);

extern void r1_func_0011DBE8(int, int);

extern void r1_func_0011DBF8(int, void *, int);

extern int r1_func_0011DC40(int);

extern void r1_func_00118D80(int);

typedef struct {
    int num;
    int fn;
} SysEnt;

/* rac1-decomp src/core/0011DBE8.c: func_0011DCB8 */
/* When func_0011F8D0 says so, installs the kernel entries of the
   {num, fn} table at r1_D_00130BD0 (the third through r1_func_0011DC40) and
   copies 0x7A8 bytes of r1_D_00130428 to 0x80074000. Indexing the table in
   the loop keeps its base live, so the argument loads keep source
   order. */
void func_0011F938(void) {
    SysEnt *t;
    unsigned int i;

    if (func_0011F8D0() == 0) {
        return;
    }
    t = (SysEnt *)r1_D_00130BD0;
    r1_func_0011DBE8(t[0].num, t[0].fn);
    r1_func_0011DBF8((int)0x80074000, r1_D_00130428, 0x7A8);
    r1_func_00118D80(0);
    r1_func_00118D80(2);
    r1_func_0011DBE8(t[1].num, t[1].fn);
    for (i = 2; i < 3; i++) {
        r1_func_0011DBE8(t[i].num, r1_func_0011DC40(t[i].num));
    }
}

INCLUDE_ASM("asm/core", func_0011F9E4);
