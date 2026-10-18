#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x1208E8-0x121750. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's libcdvd, cdvd000.o (the RPC core: sceCdDelayThread,
 * sceCdCallback, the N/S command pre-checks, sceCdSync/SyncS, sceCdInit,
 * sceCdDiskReady, sceCdMmode ...). Built with Sony's 2.9-ee (Makefile.sn,
 * EE29_CORE), like the prebuilt libcdvd.a it matches. The module sees
 * most of its globals as volatile (func_001209D8).
 */

/* Declarations in scope here before the split. */
extern long func_00116F68(int arg0, int arg1, int arg2);
extern int D_0015ED10;
extern void *D_0012F86C NOT_SDA;
extern int func_001162B8(void *arg0, void *arg1, void *arg2);
extern int func_00116320(void *arg0, void *arg1, void *arg2);
extern long func_001163A0(void *arg0, void *arg1, void *arg2);
extern void func_00116408(void *arg0);
extern void func_00113968(void);
extern void func_00114438(void *, void *);
extern char D_00152470[];
extern int func_00119088();
extern long func_00116108_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00116108");
extern long func_001188C8_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_001188C8");
extern long func_00114518_wide(int *errOut, void *a, void *b, void *c)
    __asm__("func_00114518");
extern int func_00112468(int *errOut, int arg1);
extern int func_00114060(int, void *);
extern void func_00113AE0(void *);
extern void func_00117118(void *, void *, int, int);
extern int func_00119008();
extern int D_0012FCF0 NOT_SDA;
extern void func_00118E90(int arg0, void *arg1);
extern void *D_00154A40 NOT_SDA;
extern int D_00155080[];
extern void func_001193F8(int);
extern void func_00118AD0(int, int);
extern int D_00154F54;
extern int D_0012FD04;
extern int D_00154F64 NOT_SDA;
extern int D_00154F6C NOT_SDA;
extern void func_0011AA90(int, int, int, int, int, int, int);
extern void func_0011AA00(void);
extern int D_0012FD08 NOT_SDA;
extern int func_00118C70(void *);
extern int D_0012FDA0;
extern int D_0012FDA4;
extern char D_00157E80[];
extern int D_0012FD9C;
extern void func_0011BBF0(void);
extern int D_0012FD9C NOT_SDA;
extern char D_0012FCEC[];
extern char D_001580A8[];
extern int D_0012FDA8;
extern void func_001153FC(void *, int, int);
extern int D_0012FD94;
extern int D_0012FDAC;
extern char D_00158140[];
extern int D_00158180;
extern int D_001581C0;
extern char D_00158528[];
extern int D_0012FDB4;
extern int func_0011CE70(int arg0, int arg1, int arg2, void *arg3);
extern void func_00118EC0(void);
extern void func_0011D4E0(void);
extern void func_00118EB0(void);
extern int D_00130420;
extern int D_00130424;
extern int D_00130BD0[];
extern char D_00130428[];
extern int func_0011DC50(void);
extern void func_00118D80(int);
extern void func_0011AA38(int arg0, int arg1, int arg2);
extern void func_0011B710(void);
extern int func_0011CBC8(int arg0);

ASM_FUNC("asm/handwritten/core_text", func_001208E8);


/* sceCdDelayThread: sleep on an alarm. Same stack-descriptor idiom as
   func_0011BBF0 (a SemaParam: initCount [2] = 0, maxCount [1] = 1,
   option [5] = 0). The second argument to func_00118B20 (SetAlarm) is the
   address of func_001208E4's *second* instruction -- retail builds it as
   one %hi/%lo pair on `func_001208E4 + 4`, so it is a code address the
   source names by symbol, not a separate label. `unsigned short arg0` is
   what puts `andi $17,$4,0xFFFF` in the prologue.

   Exact under 2.9-ee with the stores written initCount, maxCount, option
   (as func_0011B710). Under 2.95.3 it was 24/104: that build front-loads
   `sd $31` where retail interleaves the descriptor stores between the
   `$16` and `$31` saves. */
void func_00120910(unsigned short arg0) {
    int buf[8];
    int h;
    buf[2] = 0;
    buf[1] = 1;
    buf[5] = 0;
    h = func_00118C70(buf);
    func_00118B20(arg0, (char *)func_001208E4 + 4, h);
    func_00118CB0(h);
    func_00118C80(h);
}

extern int func_00120F30(int);
extern int func_0011D960(void);
extern void func_0011D9A8(void);
extern void *D_00159840;

void *func_00120978(void *arg0) {
    void *old;
    int r;
    if (func_00120F30(1) != 0) {
        return 0;
    }
    r = func_0011D960();
    old = D_00159840;
    D_00159840 = arg0;
    if (r != 0) {
        func_0011D9A8();
    }
    return old;
}

/*
 * _sceCd_cd_callback: the N-command completion callback. Latch *arg0 into
 * D_00131414 and D_00131418; status 0xB clears D_00131414/D_001313F0 and
 * returns; otherwise signal the D_001313E8 semaphore (func_00118CA0,
 * iSignalSema, int return) and D_001313E0 too if D_001313D4 and the
 * handler D_00159840 are set, else clear D_001313F0; finally clear
 * D_00131414.
 *
 * Every global it touches is read and written as `volatile`, through
 * `__asm__` aliases (the file's plain `extern int` declarations below
 * stay as they are). That gives retail's two reloads of D_00131414, the
 * bare `nop`s after both jal (the semaphore-id loads cannot go into the
 * slots) and the unfilled `b` slot; with fewer volatiles the size
 * changes. Exact under 2.9-ee only: 2.95.3's GCSE copies the %hi of
 * D_00131414 into $16 (`lui $2` + `move $16,$2`) where retail loads
 * `lui $16` directly (SIZE 164/160).
 */
extern volatile int D_00131414_v __asm__("D_00131414");
extern volatile int D_00131418_v __asm__("D_00131418");
extern volatile int D_001313D4_v __asm__("D_001313D4");
extern volatile int D_001313E0_v __asm__("D_001313E0");
extern volatile int D_001313E8_v __asm__("D_001313E8");
extern volatile int D_001313F0_v __asm__("D_001313F0");
extern void *volatile D_00159840_v __asm__("D_00159840");
extern int func_00118CA0(int);

void func_001209D8(int *arg0) {
    D_00131414_v = *arg0;
    D_00131418_v = D_00131414_v;
    if (D_00131414_v == 0xB) {
        D_00131414_v = 0;
        D_001313F0_v = 0;
        return;
    }
    func_00118CA0(D_001313E8_v);
    if (D_001313D4_v != 0 && D_00159840_v != 0) {
        func_00118CA0(D_001313E0_v);
    } else {
        D_001313F0_v = 0;
    }
    D_00131414_v = 0;
}

LINKER_REMNANT("asm/remnants/core_text", func_00120A78);

/* libcdvd S-command completion callback: through the uncached view of the
   receive block, copies n0 bytes of buf0 to dst0 and n1 bytes of buf1 to
   dst1, then finishes like the N-command callback (func_001209D8 on
   D_00131414). Each destination is loaded once inside its own `if`, which
   gives retail's doubled count test. */
extern int D_00131414;

typedef struct {
    int n0;
    int n1;
    char *dst0;
    char *dst1;
    char buf0[0x40];
    char buf1[0x40];
} CdRecv;

void func_00120A88(int arg0) {
    CdRecv *p = (CdRecv *)(arg0 | 0x20000000);
    int i;

    if (p->n0 > 0) {
        char *d0 = p->dst0;
        for (i = 0; i < p->n0; i++) d0[i] = p->buf0[i];
    }
    if (p->n1 > 0) {
        char *d1 = p->dst1;
        for (i = 0; i < p->n1; i++) d1[i] = p->buf1[i];
    }
    func_001209D8((int *)&D_00131414);
}

extern int D_001313E0;
extern int D_001313E8;
extern int D_001313EC;
extern int D_001313F0;

struct SemaParam_120B28 {
    s32 currentCount;
    s32 maxCount;
    s32 initCount;
    s32 numWaitThreads;
    u32 attr;
    u32 option;
};
extern volatile s32 D_001313E0_120B28[] __asm__("D_001313E0");
extern s32 D_001313E8_120B28[] __asm__("D_001313E8");
extern s32 D_001313EC_120B28[] __asm__("D_001313EC");
extern volatile s32 D_001313F0_120B28[] __asm__("D_001313F0");
extern s32 func_00118C70_120B28(struct SemaParam_120B28 *) __asm__("func_00118C70");

/* cmd_sem_init: unless both command semaphores already exist, create them
   (count 1, max 1) and the completion semaphore (count 0) through
   func_00118C70 (CreateSema), then clear D_001313F0.
   Adapted from Lombyte (MIT): src/sdk/library/cmd_sem_init.c, cmd_sem_init. */
void func_00120B28(void) {
    struct SemaParam_120B28 sema;

    if ((D_001313E8_120B28[0] != -1) && (D_001313EC_120B28[0] != -1)) {
        return;
    }
    sema.option = 0;
    sema.initCount = 1;
    sema.maxCount = 1;
    /* Both results use the same record; storing the first handle inside
       the second call's argument puts the store in that jal's delay slot,
       as retail has it (the open residual of the old near-miss). */
    D_001313EC_120B28[0] = func_00118C70_120B28(
        (D_001313E8_120B28[0] = func_00118C70_120B28(&sema)) ? &sema : &sema);
    sema.initCount = 0;
    D_001313E0_120B28[0] = func_00118C70_120B28(&sema);
    D_001313F0_120B28[0] = 0;
}

extern int D_001313D4;
/* DeleteSema returns int; declared void, the lui after each call would take
   $v0 instead of retail's $v1. */
extern int func_00118C80_i(int) __asm__("func_00118C80");
extern void func_00118C90(int);
extern void func_0011AA68(int);

/*
 * cdvd_exit (libcdvd.a cdvd000.o): if the callback thread exists
 * (D_001313D4, cb_thid), set the callback number (D_00131414,
 * sceCdCbfunc_num) to -1 and SignalSema its semaphore (D_001313E0,
 * cb_semid); DeleteSema the N-command, S-command and callback
 * semaphores; then remove SIF command handler 0x80000012 with interrupts
 * off, re-enabling them if DIntr said they were on.
 *
 * The do-while (0) around the store leaves loop notes in the block, which
 * the scheduler treats as a barrier: the store stays ahead of the
 * semaphore-id load, as in retail, and the load still fills the jal's
 * delay slot. Without it the load is hoisted above the store.
 */
void func_00120BC0(void) {
    int di;
    if (D_001313D4 != 0) {
        do {
            D_00131414_v = -1;
        } while (0);
        func_00118C90(D_001313E0);
    }
    func_00118C80_i(D_001313E8);
    func_00118C80_i(D_001313EC);
    func_00118C80_i(D_001313E0);
    di = func_0011D960();
    func_0011AA68(0x80000012);
    if (di != 0) {
        func_0011D9A8();
    }
}

LINKER_REMNANT("asm/remnants/core_text", func_00120C58);

/* Interrupt handler registered by func_00120CA0. If a handler is set
   (D_00159844) and the flag D_001313E4 is clear, calls it with D_00159848
   as its argument. The linker remnant in front of it was split off. */
extern int D_00159844;
extern int D_00159848;
extern int D_001313E4;

void func_00120C60(void) {
    if (D_00159844 != 0 && D_001313E4 == 0) {
        ((void (*)(int))D_00159844)(D_00159848);
    }
}

extern int D_001313E4;
extern int D_001313FC;

int func_00120CA0(void) {
    int r;

    D_001313E4 = 1;
    r = func_0011D960();
    func_0011AA38(0x80000012, (int)((char *)func_00120C58 + 8), 0);
    if (r != 0) {
        func_0011D9A8();
    }
    D_001313E4 = 0;
    D_001313FC = 1;
    return 1;
}

LINKER_REMNANT("asm/remnants/core_text", func_00120D18);

typedef struct {
    char pad[0x24];
    void *serve;
} SifRpcClientData;

extern void func_00120B28(void);
extern int func_00118CC0(int);
extern int func_00118BF0(int, void *);
extern void func_0011AE20(int);
extern int func_0011B2F8(void *, unsigned int, int);
extern void func_0011A6C8();
extern void func_00118C90(int);

extern int D_001313E8;
extern int D_001313DC;
extern int D_001313D0;
extern int D_001313F8;
extern int D_00159850;
extern char D_00159858[];
extern char D_00152FB0[];
extern char D_00152FD8[];
extern char D_00132590[];

/* _sceCd_scmd_prechk (libcdvd.a cdvd000.o) */
int func_00120D28(int arg0) {
    int i;
    volatile int *sem = &D_001313E8;

    func_00120B28();
    if (*sem != func_00118CC0(*sem)) {
        if (D_001313D0 > 0) {
            func_0011A6C8(D_00152FB0, arg0, D_001313DC);
        }
        return 0;
    }
    D_001313DC = arg0;
    func_00118BF0(D_00159850, D_00159858);
    if (func_00120F30(1) != 0) {
        func_00118C90(*sem);
        return 0;
    }
    func_0011AE20(0);
    if (D_001313F8 >= 0) {
        return 1;
    }
    while (1) {
        if (func_0011B2F8(D_00132590, 0x80000595, 0) < 0) {
            if (D_001313D0 > 0) {
                func_0011A6C8(D_00152FD8);
            }
            for (i = 0x100000; i != -1; i--)
                ;
        } else {
            if (((SifRpcClientData *)D_00132590)->serve != 0) {
                break;
            }
            for (i = 0x100000; i != -1; i--)
                ;
        }
    }
    D_001313F8 = 0;
    return 1;
}

extern int func_0011B4C8();
extern int func_00120D28(int);
extern void func_00118C90(int);
extern int D_00131440;

/*
 * sceCdNcmdDiskReady, sibling of func_00121930 (and of the func_0011CBC8
 * pair): open the service, run one func_0011B4C8 RPC, then release the
 * lock and hand back the reply. The reply is read back through the
 * uncached mirror of the DMA buffer (| 0x20000000), which is why the
 * address is spelled as an integer or.
 *
 * The RPC failure path comes first (release, return 0), then the success
 * path, as retail lays them out. Under 2.9-ee that is the form that keeps
 * the early guard's `return 0` and the failure arm's `return 0` apart
 * (retail keeps two separate zeroings); the success-first spelling that
 * 2.95.3 needed is cross-jumped by 2.9-ee into one tail (SIZE 144/152).
 * The two compilers merge the tails in mirror-image layouts, so the
 * layout alone does not tell them apart; the prebuilt libcdvd.a, which
 * this matches, is 2.9-ee's.
 */
int func_00120E98(void) {
    int r;

    if (func_00120D28(2) == 0) {
        return 0;
    }
    if (func_0011B4C8(D_00132590, 0xE, 0, 0, 0, &D_00131440, 4, 0, 0) < 0) {
        func_00118C90(D_001313E8);
        return 0;
    }
    r = *(int *)((unsigned int)&D_00131440 | 0x20000000);
    func_00118C90(D_001313E8);
    return r;
}

extern char D_00152FF0[];
extern int D_001313D0;

/* sceCdSyncS */
int func_00120F30(int arg0) {
    if (arg0 == 0) {
        if (D_001313D0 > 0) {
            func_0011A6C8(D_00152FF0);
        }
        while (D_001313F0 != 0 || func_0011B6B8(D_00132590) != 0) {
            func_00120910(0x3C);
        }
        return 0;
    }
    if (D_001313F0 != 0 || func_0011B6B8(D_00132590) != 0) {
        return 1;
    }
    return 0;
}

extern int func_0011B6B8(void *);
extern char D_00153000[];
extern char D_00132E08[];

/* sceCdSync */
int func_00120FD0(int arg0) {
    if (arg0 == 0) {
        if (D_001313D0 > 0) {
            func_0011A6C8(D_00153000);
        }
        while (func_0011B6B8(D_00132E08) != 0) {
            func_00120910(0x3C);
        }
        return 0;
    }
    return func_0011B6B8(D_00132E08);
}

extern int D_001313EC;
extern int D_001313D8;
extern char D_00153010[];
extern char D_00153038[];
extern int D_00131408;

/* _sceCd_ncmd_prechk (libcdvd.a cdvd000.o) */
int func_00121040(int arg0) {
    int i;
    volatile int *sem = &D_001313EC;

    func_00120B28();
    if (*sem != func_00118CC0(*sem)) {
        if (D_001313D0 > 0) {
            func_0011A6C8(D_00153010, arg0, D_001313D8);
        }
        return 0;
    }
    D_001313D8 = arg0;
    func_00118BF0(D_00159850, D_00159858);
    if (func_00120FD0(1) != 0) {
        func_00118C90(*sem);
        return 0;
    }
    func_0011AE20(0);
    if (D_00131408 >= 0) {
        return 1;
    }
    while (1) {
        if (func_0011B2F8(D_00132E08, 0x80000593, 0) < 0) {
            if (D_001313D0 > 0) {
                func_0011A6C8(D_00153038);
            }
            for (i = 0x100000; i != -1; i--)
                ;
        } else {
            if (((SifRpcClientData *)D_00132E08)->serve != 0) {
                break;
            }
            for (i = 0x100000; i != -1; i--)
                ;
        }
    }
    D_00131408 = 0;
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_001211B0);

struct sceSifRpcData {
    void *paddr; /* 0x0: packet in flight; released by _request_end */
    u32 pid; /* 0x4 */
    s32 tid; /* 0x8: semaphore signalled by _request_end when >= 0 */
    u32 mode; /* 0xC */
};
struct sceSifClientData {
    struct sceSifRpcData rpcd; /* 0x0 */
    u32 command; /* 0x10 */
    void *buff; /* 0x14 */
    void *cbuff; /* 0x18 */
    void (*func)(void *); /* 0x1C: end function, called with para when the call completes */
    void *para; /* 0x20 */
    void *serve; /* 0x24: server bound by sceSifBindRpc; 0 until bound */
};
extern s32 D_001313D0_21490[] __asm__("D_001313D0");
extern volatile s32 D_001313EC_21490[] __asm__("D_001313EC");
extern s32 D_00131404[];
extern struct sceSifClientData D_00159A90;
extern s32 D_00159AD0[];
extern u8 D_001325C0_21490[] __asm__("D_001325C0");
extern u8 D_00153080[];
extern u8 D_00153090[];
extern u8 D_001530B0[];
extern s32 func_00118CC0();
extern void func_00118C90();
extern void func_00120B28_21490() __asm__("func_00120B28");
extern s32 func_00120FD0();
extern s32 func_0011B2F8();
extern void func_0011AE20();
extern void func_0011AD70();
extern s32 func_0011B4C8();

/* libcdvd disk-ready query (the sceCdDiskReady pattern): take the S-command semaphore, bind the 0x8000059A server once, send mode and return the server's status word. Mode 8 reports -1 on failure, every other mode 6.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_00121190.c, FUN_00121190. */
s32 func_00121490(s32 mode) {
    s32 response;
    s32 spin;

    if (D_001313D0_21490[0] > 0) {
        func_0011A6C8(D_00153080);
    }
    func_00120B28_21490();
    if (D_001313EC_21490[0] != func_00118CC0(D_001313EC_21490[0])) {
        return 6;
    }
    if (func_00120FD0(1) != 0) {
        func_00118C90(D_001313EC_21490[0]);
        if (mode == 8) {
            return -1;
        }
        return 6;
    }
    func_0011AE20(0);
    if (D_00131404[0] < 0) {
        for (;;) {
            if (func_0011B2F8(&D_00159A90, 0x8000059A, 0) < 0) {
                if (D_001313D0_21490[0] > 0) {
                    func_0011A6C8(D_00153090);
                }
                for (spin = 0x100000; spin != -1; spin--) {
                }
                continue;
            }
            if (D_00159A90.serve != 0) {
                D_00131404[0] = 0;
                break;
            }
            for (spin = 0x100000; spin != -1; spin--) {
            }
        }
    }
    D_00159AD0[0] = mode;
    func_0011AD70(D_00159AD0, 4);
    if (func_0011B4C8(&D_00159A90, 0, 0, D_00159AD0, 4, D_001325C0_21490, 4, 0, 0) < 0) {
        func_00118C90(D_001313EC_21490[0]);
        if (mode == 8) {
            return -1;
        }
        return 6;
    }
    if (D_001313D0_21490[0] > 0) {
        func_0011A6C8(D_001530B0);
    }
    response = *(s32 *)((u32)D_001325C0_21490 | 0x20000000);
    func_00118C90(D_001313EC_21490[0]);
    return response;
}

extern int D_00132A00;
extern int D_001325C0;
extern int func_00121040(int);
extern void func_0011AD70(void *, int);

/* sceCdMmode */
int func_00121688(int arg0) {
    int *p = &D_00132A00;
    int r;

    if (func_00121040(0x22) == 0) {
        return 0;
    }
    D_00132A00 = arg0;
    func_0011AD70(p, 4);
    if (func_0011B4C8(D_00132E08, 0x22, 0, p, 4, &D_001325C0, 4, 0, 0) < 0) {
        func_00118C90(D_001313EC);
        return 0;
    }
    r = *(int *)((unsigned int)&D_001325C0 | 0x20000000);
    func_00118C90(D_001313EC);
    return r;
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_0012174C);
