#include "include_asm.h"
extern void func_0034E4B8(int);
extern void func_00338A60(int);

void func_0034E478(char *a0) {
    extern void func_0034CEA0(void *);
    extern void func_0029CB58(void);
    func_0034CEA0(a0 + 0x7A0);
    func_0034E4B8((int)a0);
    func_00338A60((int)(a0 + 0x1FBC));
    func_0029CB58();
}
