/* functional: high. Same reads, writes, calls and arguments as the assembly (p6.c form): reports via func_0012C468 when unk120 is set, else calls func_00129E30 with n-1 twice when unk174 == 3 or func_00129F40 otherwise, then clears unk120. Only the register choice for the second n-1 differs. */
#include "common.h"
#include "structs.h"

extern void func_0012C468(void *, void *);
extern void func_00129E30(void *, int, int, int);
extern void func_00129F40();
extern char D_00153BB8[];

void func_0012C278(void *arg0) {
    Obj40 *s = (Obj40 *)arg0;
    int n = s->unk118;

    if (s->unk120 != 0) {
        func_0012C468(s, D_00153BB8);
    } else {
        if (s->unk174 == 3) {
            func_00129E30(s, (int)s->slots[0].unk04, n - 1, n - 1);
        } else {
            func_00129F40(s, (int)s->slots[1].unk04, (int)s->slots[2].unk04);
        }
    }
    s->unk120 = 0;
}
