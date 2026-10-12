#include "common.h"
#include "structs.h"

extern void func_00130288(void *, void *);

extern void func_00130240(void *);

/* rac1-decomp src/core/0012AC80.c: func_0012C468 */
void func_00130288(void *arg0, void *arg1) {
    char *a = (char *)arg0;
    void *t = *(void **)(a + 0x858);
    /* the arg0 null test is retail's, after it has already dereferenced
       arg0 -- one of the "dead-looking guards" that must be written out */
    if (t != 0 && arg0 != 0 && *(int *)(a + 0xC) != 0) {
        int buf[4];
        buf[0] = 0;
        buf[1] = (int)arg1;
        func_0012FA98(t, buf);
    } else {
        func_00130240(arg1);
    }
}

/* rac1-decomp src/core/0012AC80.c: func_0012C4C0 */
/* Store the two sizes and their 16-pixel macroblock counts. (An old note
   here recorded 4/32 with the 0x4/0x8 stores swapped; the function is
   exact under both compilers now.) */
int func_001302E0(void *arg0, int arg1, int arg2) {
    char *p = (char *)arg0;
    *(int *)(p + 0x4) = arg1;
    *(int *)(p + 0x8) = arg2;
    *(int *)(p + 0xC) = arg1 >> 4;
    *(int *)(p + 0x10) = arg2 >> 4;
    return 1;
}

INCLUDE_ASM("asm/core", func_00130300);

INCLUDE_ASM("asm/core", func_00130428);

INCLUDE_ASM("asm/core", func_001306D0);

INCLUDE_ASM("asm/core", func_001307B0);
