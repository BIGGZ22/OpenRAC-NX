#include "common.h"

/* The unit saves $ra in a TImode slot (`sq`), so it is built with the
   -mastra-quad-saves profile. Variadic: one named argument, so the register
   save area starts at the $a1 slot, 0x38 below the stack arguments. The
   object is stamped into the context it points at, and both the context and
   the va_list go to func_00118CC8. */
extern void *D_00133E74;

extern int func_00118CC8(void *arg0, int arg1, void *arg2);

void func_00115D48(int arg0, ...) {
    void *obj = D_00133E74;
    void *ctx = *(void **)((char *)obj + 8);

    *(void **)((char *)ctx + 0x54) = obj;
    func_00118CC8(*(void **)((char *)obj + 8), arg0,
                  (char *)__builtin_next_arg(arg0) - 0x38);
}
