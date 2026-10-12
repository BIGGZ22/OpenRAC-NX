#include "common.h"
#include "structs.h"

extern int func_0012FE78(void *, int, int);

extern int func_0012FEC0(void *, int, int);

extern int func_0012FD60(void *, int, int);

/* rac1-decomp src/core/0012AC80.c: func_0012C058 */
/* Hands the call on by the inner object's +0x174 mode: func_0012FD60 for
   mode 3, func_0012FEC0 otherwise, with the caller's arguments.

   Returns its callees' values: retail keeps the frame and calls both, and
   2.9-ee tail-calls each arm (bare `j`, SIZE 40/68) when it is void. */
int func_0012FE78(void *arg0, int arg1, int arg2) {
    Obj40 *inner = ((Wrapper *)arg0)->obj;
    if (inner->unk174 != 3) {
        return func_0012FEC0(arg0, arg1, arg2);
    } else {
        return func_0012FD60(arg0, arg1, arg2);
    }
}
