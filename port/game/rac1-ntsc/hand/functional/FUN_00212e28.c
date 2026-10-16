/* Carried from the PAL decompilation's nonmatching/game/func_00213C78.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* Moby update loop. Fetches the head of the moby list from func_0020D868 and keeps it in
   D_0015FF24. For each moby whose state byte is non-negative (signed): calls func_0020D580
   unless flags 0x40 is set, calls its update callback at 0x74 if non-null, then calls
   func_0020DEF8 unless flags 0x4 is set. Walks the list through next at 0x28. */
extern void *func_0020D868(void);
extern void func_0020D580(void *);
extern void func_0020DEF8(void *);
extern void *D_0015FF24;

void FUN_00212e28(void) {
    char *m;

    m = (char *)func_0020D868();
    D_0015FF24 = m;
    if (m != 0) {
        do {
            if ((signed char)m[0x20] >= 0) {
                if (!(*(unsigned short *)(m + 0x34) & 0x40)) {
                    func_0020D580(m);
                }
                if (*(void (**)(void *))(m + 0x74) != 0) {
                    (*(void (**)(void *))(m + 0x74))(m);
                }
                if (!(*(unsigned short *)(m + 0x34) & 0x4)) {
                    func_0020DEF8(m);
                }
            }
            m = *(char **)(m + 0x28);
        } while (m != 0);
    }
}
