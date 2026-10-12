#include "common.h"
#include "structs.h"

extern void func_0011F640(void);

extern void func_0011F718(void);

extern void r1_func_0011DE38(void);

extern void r1_func_001195A0(void);

extern void r1_func_0011DCB8(void);

extern void r1_func_0011D3C8(void);

/* rac1-decomp src/core/0011D960.c: func_0011DBA8 */
/* _InitSys (libkernl.a:initsys.o): the kernel's one-shot startup sequence,
   run once from the crt0 supplement. Chains six sub-system initialisers;
   the last (InitTLBFunctions, r1_func_0011D3C8) is a tail call since this is
   void and it is the final statement. */
void func_0011F828(void) {
    func_0011F640();
    func_0011F718();
    r1_func_0011DE38();
    r1_func_001195A0();
    r1_func_0011DCB8();
    r1_func_0011D3C8();
}

INCLUDE_ASM("asm/core", func_0011F864);
