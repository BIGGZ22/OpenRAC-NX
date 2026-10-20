#include "common.h"

/* The unit saves $ra in a TImode slot (`sq`), so it is built with the
   -mastra-quad-saves profile. Variadic wrapper: the compiler's own register
   save area is the va_list retail passes (sp + 0x50, not the stack arguments
   ginclude's va_start would point at), so the body reaches it the way the other varargs shims in src/ do,
   from __builtin_next_arg(arg1) minus the 0x30 of register-save area. */
extern int func_00119BC8(int arg0, int arg1, void *arg2);

void func_00115CF0(int arg0, int arg1, ...) {
    func_00119BC8(arg0, arg1, (char *)__builtin_next_arg(arg1) - 0x30);
}
