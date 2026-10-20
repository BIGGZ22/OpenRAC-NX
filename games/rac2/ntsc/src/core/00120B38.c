#include "common.h"

/* The value is taken through a named s64 local: writing `(int)*(s64 *)p`
   directly folds the load into a 32-bit `lw`, while the local keeps retail's
   `ld` plus the truncdisi2 pair (dsll32/dsra32). */
extern char *func_001240C8(int, int);
extern void *func_00120A30(int, void *, int);

char *func_00120B38(int arg0, char *arg1, int arg2) {
    char *s0 = func_001240C8(arg0, arg2);
    int a0;
    s64 value;

    if (s0 == 0)
        return 0;
    a0 = *(u16 *)(s0 + 0x298);
    if (*(s8 *)(arg1 + a0 + 0x29C) != 0) {
        value = *(s64 *)func_00120A30(a0, arg1, 0);
        *(int *)s0 = value;
    } else {
        *(int *)s0 = *(int *)arg1;
    }
    *(int *)s0 += *(int *)(s0 + 8);
    return s0;
}
