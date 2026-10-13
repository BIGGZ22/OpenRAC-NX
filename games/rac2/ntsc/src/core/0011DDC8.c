#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_D_0012FD9C;

extern int r1_D_0012FD9C NOT_SDA;

extern s32 r1_func_00118C90(s32);

extern int r1_func_001151B4();

extern char r1_D_0012FCEC[];

extern char r1_D_001580A8[];

extern int r1_D_0012FDA8;

extern s32 func_0011E010(void);

extern void r1_func_001153FC(void *, int, int);

extern int r1_D_0012FD94;

/* rac1-decomp src/core/00119D88.c: func_0011BC70 */
/* A tail jump with the argument load in its delay slot (`lui / j /
   lw`), as 2.9-ee, which builds this SDK object, emits it. */
void func_0011DDC8(void) {
    r1_func_00118C90(r1_D_0012FD9C);
}

ASM_FUNC("asm/core", func_0011DDD8);

INCLUDE_ASM("asm/core", func_0011DE08);

/* rac1-decomp src/core/00119D88.c: func_0011BEB8 */
/* Three-way compare-and-confirm: the slot r1_D_001580A8 has to agree with
   the scratch buffer r1_D_0012FCEC, then with the handle r1_D_0012FDA8, and
   finally the two of those with each other. Only the last comparison's
   result is reported, as a 0/1.

   The two globals are bound to locals, buffer first, purely to fix the
   order the compiler materialises their %hi halves in: used directly,
   the slot's lui comes first and the function is 4/140. Declaration
   order of the locals decides it. */
int func_0011E010(void) {
    char *buf = r1_D_0012FCEC;
    char *slot = r1_D_001580A8;
    int r = 0;

    if (r1_func_001151B4(slot, buf, 4) != 0 &&
        r1_func_001151B4(slot, r1_D_0012FDA8, 4) != 0) {
        r = r1_func_001151B4(buf, r1_D_0012FDA8, 4) != 0;
    }
    return r;
}

/* rac1-decomp src/core/00119D88.c: func_0011BF48 */
int func_0011E0A0(void) {
    r1_D_0012FD94 = 0;
    r1_func_001153FC(r1_D_001580A8, 0, 0x4);
    return 0;
}
