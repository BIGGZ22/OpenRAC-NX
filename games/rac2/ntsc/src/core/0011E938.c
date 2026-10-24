#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_func_0011B2F8();

extern int r1_func_0011B4C8();

extern void *memcpy(void *, const void *, unsigned int);

extern int r1_D_0012FDB0;

extern char r1_D_00158500[];

extern char r1_D_00158300[];

extern char r1_D_00158528[];

extern int r1_func_001151B4();

extern char r1_D_0012FCEC[];

extern char r1_D_001580A8[];

extern int r1_D_0012FDA8;

extern int r1_func_0011CDE0();

struct SifLoadCmd {
    int unk0;
    int unk4;
};

#define SIFCMD ((struct SifLoadCmd *) r1_D_00158300)

extern int func_0011EB00(int arg0, int arg1, int arg2, void *arg3);

/* rac1-decomp src/core/0011CCE0.c: func_0011CCE0 */
/* _lf_bind (libkernl.a eeloadfile.o): unless already bound (r1_D_0012FDB0
 * >= 0), bind the IOP loadfile RPC server (r1_func_0011B2F8, 0x80000006),
 * busy-waiting 0x100000 counts between tries until its server pointer
 * (+0x24) is set; a failed bind returns -1. Then mark it bound, call
 * function 0xFF (r1_func_0011B4C8) for the 4-byte reply at r1_D_00158300
 * (-0x10001 if the call fails) and copy it to r1_D_00158528.
 *
 * The success path has its own `return 0`: its $v0 = 0 is scheduled
 * ahead of the inlined memcpy, which moves the copy's registers up by
 * one as in retail. The already-bound test jumps to the shared one. */
int func_0011E938(void) {
    int i;

    if (r1_D_0012FDB0 >= 0)
        goto done;

    for (;;) {
        char *slot = r1_D_00158500;

        if (r1_func_0011B2F8(slot, 0x80000006, 0) < 0)
            return -1;

        if (*(int *)(slot + 0x24) != 0) {
            r1_D_0012FDB0 = 0;
            if (r1_func_0011B4C8(slot, 0xFF, 0, 0, 0, r1_D_00158300, 4, 0, 0) < 0)
                return 0xFFFEFFFF;
            memcpy(r1_D_00158528, r1_D_00158300, 4);
            return 0;
        }

        for (i = 0x100000; i != -1; i--)
            ;
    }

done:
    return 0;
}
extern int func_0011EA38(void);
extern char D_00134684[];
extern char D_00140528[];
extern char *D_0013474C;
extern int func_00115340(void *, void *, int);

int func_0011EA38(void) {
    char *reference = D_00134684;
    char *state = D_00140528;
    int result = 0;
    if (func_00115340(state, reference, 4) != 0) {
        if (func_00115340(state, D_0013474C, 4) != 0)
            result = func_00115340(reference, D_0013474C, 4) != 0;
    }
    return result;
}


extern int D_00134748;
extern char D_00140528[];
extern void func_00115484(void *, int, int);

int func_0011EAC8(void) {
    D_00134748 = -1;
    func_00115484(D_00140528, 0, 4);
    return 0;
}

/* rac1-decomp src/core/0011CCE0.c: func_0011CE70 */
/* _sceSifLoadModuleBuffer. Adapted from Lombyte (GPL-3.0) for PAL. */
int func_0011EB00(int arg0, int arg1, int arg2, void *out) {
    int *arg3 = out;
    int var_2_18;
    int temp;

    var_2_18 = 0xFFFF0000;
    if (func_0011E938() < 0) {
        return var_2_18;
    }
    if (r1_func_0011CDE0() != 0) {
        return 0xFFFEFFFC;
    }
    SIFCMD->unk0 = arg0;
    if (arg2 != 0) {
        if (arg1 >= 0xFD) {
            memcpy(r1_D_00158300 + 0x104, (void *) arg2, 0xFC);
            SIFCMD->unk4 = 0xFC;
        } else {
            memcpy(r1_D_00158300 + 0x104, (void *) arg2, arg1);
            SIFCMD->unk4 = arg1;
        }
    } else {
        SIFCMD->unk4 = 0;
    }
    if (r1_func_0011B4C8(r1_D_00158500, 6, 0, r1_D_00158300, 0x200, r1_D_00158300, 8, 0, 0) < 0) {
        return 0xFFFEFFFF;
    }
    temp = SIFCMD->unk0;
    *arg3 = SIFCMD->unk4;
    return temp;
}
