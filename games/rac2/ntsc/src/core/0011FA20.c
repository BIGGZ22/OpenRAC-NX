#include "common.h"
#include "structs.h"

/*
 * Exit(status): re-init the TLB (func_0011FA18), then the _Exit syscall
 * (r1_func_00118A60) with the status, as a bare tail jump. Exact under
 * 2.9-ee, which emits "call, then tail call" itself; under 2.95.3 it was
 * 4 bytes over (2.95.3 has no sibling calls). The status lives in $16
 * across the first call.
 */
extern void r1_func_00118A60(int);

/* rac1-decomp src/core/0011DD68.c: func_0011DDA0 */
void func_0011FA20(int status) {
    func_0011FA18();
    r1_func_00118A60(status);
}

INCLUDE_ASM("asm/core", func_0011FA48);
