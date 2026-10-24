#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern int r1_func_00118C70(void *);

   /* CreateSema */
extern void r1_func_00118C80(int);

     /* DeleteSema */
extern int r1_func_00118CB0(int);

typedef signed int s32_1B4C8;

extern s32_1B4C8 r1_func_00118C70();

extern s32_1B4C8 r1_func_00118CB0();

extern char r1_D_00157E80[];

extern int r1_D_0012FD94;

extern s32 r1_func_00118CB0(s32);

extern s32 r1_func_0011B4C8(void *, s32, s32, void *, s32, void *, s32, void *, void *);

typedef struct {
    int handle;
    int active;
} CloseIob;

typedef struct {
    int sema;
    int result;
    int size;
    int handle;
    int slot;
} CloseState;

typedef struct {
    int count;
    int maxCount;
    int initialCount;
    int waitThreads;
    int attr;
    int option;
} CloseSema;

extern CloseState r1_D_00156980;

extern char r1_D_001575C0[];

extern char r1_D_00158080[];

extern int r1_func_0011B4C8();

extern int r1_func_0011BC40_arg(int) __asm__("func_0011DD98");

/* rac1-decomp src/core/00119D88.c: func_0011C208 */
/* sceClose: close a SIF file handle, adapted from Lombyte (GPL-3.0) for PAL. */
int func_0011E360(unsigned int fd) {
    CloseSema sema;
    int result;
    int sid;
    int rpcResult;
    CloseIob *iob;
    CloseState *state = &r1_D_00156980;

    iob = func_0011D950(fd);
    r1_func_0011BC40_arg(1);
    if (r1_D_0012FD94 == 0) {
        func_0011DDC8();
        return -1;
    }
    if (iob == 0 || iob->active == 0) {
        func_0011DDC8();
        return -9;
    }
    state->handle = iob->handle;
    sema.maxCount = 1;
    state->slot = ((char *)iob - r1_D_00157E80) >> 4;
    sema.initialCount = 0;
    sema.option = 0;
    sid = r1_func_00118C70(&sema);
    state->sema = sid;
    state->result = (int)&result;
    state->size = 4;
    rpcResult = r1_func_0011B4C8(r1_D_00158080, 1, 0, state, 0x14,
                                r1_D_001575C0, 4, 0, 0);
    if (rpcResult < 0) {
        r1_func_00118C80(sid);
        func_0011DDC8();
        return -11;
    }
    iob->active = 0;
    rpcResult = *(int *)((unsigned int)r1_D_001575C0 | 0x20000000);
    func_0011DDC8();
    if (rpcResult == 0) {
        r1_func_00118C80(sid);
        return -11;
    }
    r1_func_00118CB0(sid);
    r1_func_00118C80(sid);
    if (result < 0) {
        return result;
    }
    return 0;
}
