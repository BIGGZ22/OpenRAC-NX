#include "common.h"
#include "structs.h"
#include <stdarg.h>

     /* DeleteSema */
extern int r1_func_00118CB0(int);

typedef signed int s32_1B4C8;

extern s32_1B4C8 r1_func_00118CB0();

extern int r1_D_0012FDA0;

extern char r1_D_00157E80[];

extern s32 r1_func_00118C90(s32);

extern s32 r1_func_00118CB0(s32);

extern int r1_func_00118C70(void *);

extern s32_1B4C8 r1_func_00118C70();

extern int r1_D_0012FD9C;

extern void func_0011DD48(void);

extern int r1_D_0012FD9C NOT_SDA;

/* rac1-decomp src/core/00119D88.c: func_0011B7F8 */
/* The out-of-range guard comes first and returns 0: 2.9-ee then branches
   to the in-range block on the true side of the sltiu, as retail does
   (bnez). The in-range-first if/else that matched under 2.95.3 is laid
   out the other way round by 2.9-ee. */
void *func_0011D950(unsigned int arg0) {
    void *ret;
    func_0011D868();
    r1_func_00118CB0(r1_D_0012FDA0);
    if (arg0 >= 0x20) {
        r1_func_00118C90(r1_D_0012FDA0);
        return 0;
    }
    ret = &r1_D_00157E80[arg0 * 0x10];
    r1_func_00118C90(r1_D_0012FDA0);
    return ret;
}

INCLUDE_ASM("asm/core", func_0011D9C0);

/* rac1-decomp src/core/00119D88.c: func_0011BBF0 */
void func_0011DD48(void) {
    int buf[8];
    if (r1_D_0012FD9C == -1) {
        buf[5] = 0;
        buf[2] = 1;
        buf[1] = 1;
        r1_D_0012FD9C = r1_func_00118C70(buf);
    }
}
