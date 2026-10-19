typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef void VOID;
#define NULL ((void *)0)

/* sceClose - RAC5 boot 0x01ED59A8, adapted from RAC2 core
 * src/core/0011E360.c:func_0011E360 (itself from rac1-decomp
 * src/core/00119D88.c:func_0011C208, Lombyte MIT).
 * Data and call addresses are RAC5's, read from the retail disassembly. */

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
    const char *name;
} CloseSema;

extern CloseState D_01FE5940;
extern u8 D_01FE6E40[];
extern u8 D_01FE6580[];
extern u8 D_01FE7040[];
extern s32 D_01EFEFDC;
extern const char S_01F27878[];

extern CloseIob *F_iob_lookup(u32 fd);
extern s32 F_lock(s32 n);
extern void F_unlock(void);
extern s32 CreateSema(CloseSema *sema);
extern s32 DeleteSema(s32 sid);
extern s32 WaitSema(s32 sid);
extern s32 F_call_rpc(u8 *send, s32 f1, s32 f2, CloseState *state, s32 n,
                      u8 *recv, s32 rsize, s32 z1, s32 z2, s32 z3);

int sceClose(unsigned int fd) {
    CloseSema sema;
    int result;
    int sid;
    int rpcResult;
    CloseIob *iob;
    CloseState *state = &D_01FE5940;

    iob = F_iob_lookup(fd);
    F_lock(1);
    /* RAC5 retail returns -9 here too (RAC2's body returned -1): the two
     * failure branches share one epilogue at 0x01ED5A00. */
    if (D_01EFEFDC == 0) {
        F_unlock();
        return -9;
    }
    if (iob == 0 || iob->active == 0) {
        F_unlock();
        return -9;
    }
    state->handle = iob->handle;
    state->slot = ((char *)iob - (char *)D_01FE6E40) >> 4;
    sema.maxCount = 1;
    sema.name = S_01F27878;
    sema.initialCount = 0;
    sid = CreateSema(&sema);
    state->sema = sid;
    state->result = (int)&result;
    state->size = 4;
    rpcResult = F_call_rpc(D_01FE7040, 1, 0, state, 0x14,
                           D_01FE6580, 4, 0, 0, 0);
    if (rpcResult < 0) {
        DeleteSema(sid);
        F_unlock();
        return -11;
    }
    iob->active = 0;
    rpcResult = *(int *)((unsigned int)D_01FE6580 | 0x20000000);
    F_unlock();
    if (rpcResult == 0) {
        DeleteSema(sid);
        return -11;
    }
    WaitSema(sid);
    DeleteSema(sid);
    if (result < 0) {
        return result;
    }
    return 0;
}
