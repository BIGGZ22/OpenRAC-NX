#include "common.h"
#include "callees.h"
#include "structs.h"

/*
 * core_text object 0x119328-0x119868. Boundaries are retail's linker fill
 * (0xCDCDCDCD) between objects; see docs/DECOMP_PROGRESS.md.
 *
 * Sony's EE kernel library (libkernl), three members of libkernl.a back to
 * back: intr.o (DisableIntc/EnableIntc/DisableDmac/EnableDmac), thread.o
 * (topThread, InitThread = func_001195A0, iWakeupThread) and deci2.o
 * (sceDeci2Open ... kputs). Built with Sony's 2.9-ee (Makefile.sn,
 * EE29_CORE), like the prebuilt archive it matches.
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

ASM_FUNC("asm/handwritten/core_text", func_00119328);

ASM_FUNC("asm/handwritten/core_text", func_00119390);

ASM_FUNC("asm/handwritten/core_text", func_001193F8);

ASM_FUNC("asm/handwritten/core_text", func_00119460);

typedef struct {
    unsigned char type;
    unsigned char id;
} TopThreadEntry;

typedef struct {
    int counter;
    unsigned int pad;
    TopThreadEntry entries[0x200];
} TopThreadContext;

extern int D_00154600;
extern char D_001527E8[];
extern int func_00118CB0(int);
extern int func_00118C10(int);
extern int func_00118BC0(int);
extern int func_00118C40(int);
extern int func_0011A690(const char *, ...);

/* topThread: kernel service dispatcher. Adapted from Lombyte (MIT) for PAL. */
void func_001194C8(void *arg0) {
    TopThreadContext *ctx = (TopThreadContext *)arg0;
    int i;
    unsigned char type;

    for (;;) {
        func_00118CB0(D_00154600);
        i = ctx->counter & 0x1FF;
        ctx->counter = i + 1;
        type = ctx->entries[i].type;
        switch (type) {
        case 0:
            func_00118C10(ctx->entries[i].id);
            break;
        case 1:
            func_00118BC0(ctx->entries[i].id);
            break;
        case 2:
            func_00118C40(ctx->entries[i].id);
            break;
        default:
            func_0011A690(D_001527E8);
            break;
        }
    }
}

/*
 * InitThread: start the kernel-side service thread once. Bail with -1 if
 * the thread id D_0012FCF8 is already > 0; CreateSema (func_00118C70,
 * maxCount 0xFF, initCount 0) into D_00154600; CreateThread
 * (func_00118B50) with entry topThread (func_001194C8), stack D_00154200,
 * stackSize 0x400, gpReg &_gp (D_00166D00), initPriority 0, its id into
 * D_0012FCF8 (DeleteSema and -1 on failure); clear D_00154608[0..1],
 * StartThread(tid, D_00154608), ChangeThreadPriority(GetThreadId(), 1);
 * return D_0012FCF8 (re-read). Exact under 2.9-ee.
 *
 * Levers: Sony's real kernel.h structs -- ThreadParam is 12 ints with
 * `status` first (entry at +4, stack +8, stackSize +0xC, gpReg +0x10,
 * initPriority +0x14), SemaParam 6 ints -- which put the thread at sp+0
 * and the semaphore at sp+0x30 in a 0x80 frame (the old decode had a
 * 9-int struct with entry first); and the CreateSema result stored to
 * the global and the global tested, which gives retail's test of $v0
 * with the store in the bltz slot. Under 2.95.3 the CreateThread failure
 * block is laid out after the success path (48/212, same size).
 */
extern int func_00118C70(void *);
extern int func_00118B50(void *);
extern int func_00118B70(int, void *);
extern int func_00118BA0(int, int);
extern void func_001194C8(void *);
extern int D_0012FCF8;
extern char D_00154200[];
extern char D_00166D00[];
extern int D_00154608[];

typedef struct {
    int status;
    void (*entry)(void *);
    void *stack;
    int stackSize;
    void *gpReg;
    int initPriority;
    int currentPriority;
    unsigned int attr;
    unsigned int option;
    int waitType;
    int waitId;
    int wakeupCount;
} ThreadParam_1195A0;

typedef struct {
    int currentCount;
    int maxCount;
    int initCount;
    int numWaitThreads;
    unsigned int attr;
    unsigned int option;
} SemaParam_1195A0;

int func_001195A0(void) {
    ThreadParam_1195A0 tp;
    SemaParam_1195A0 sp;
    int tid;

    if (D_0012FCF8 > 0) {
        return -1;
    }
    sp.maxCount = 0xFF;
    sp.initCount = 0;
    D_00154600 = func_00118C70(&sp);
    if (D_00154600 < 0) {
        return -1;
    }
    tp.entry = func_001194C8;
    tp.stack = D_00154200;
    tp.stackSize = 0x400;
    tp.gpReg = D_00166D00;
    tp.initPriority = 0;
    tid = func_00118B50(&tp);
    D_0012FCF8 = tid;
    if (tid < 0) {
        func_00118C80(D_00154600);
        return -1;
    }
    D_00154608[0] = 0;
    D_00154608[1] = 0;
    func_00118B70(tid, D_00154608);
    func_00118BA0(func_00118BE0(), 1);
    return D_0012FCF8;
}

ASM_FUNC("asm/handwritten/core_text", func_00119678);

LINKER_REMNANT("asm/remnants/core_text", func_00119710);

extern int D_00154A10;
/* Deci2Call (syscall 0x7C) returns the result; the file's later
   `extern void func_00118E90(int, void *)` drops it, hence the alias. */
extern int func_00118E90_ret(int, void *) __asm__("func_00118E90");

/* libkernel sceDeci2Open(protocol, opt, handler): Deci2Call(1, {protocol,
   opt, handler, uncached &D_00154A10}). Returning the call's value is
   what gives retail's schedule (tag complete before the stores, the &buf
   move ahead of the $ra save); the void spelling is 30/68. */
/* sceDeci2Open: returns Deci2Call's int result (func_00118E90 is
   Deci2Call, declared void in this file, so it is reached through an
   alias). */
int func_00119718(unsigned short arg0, void *arg1, void *arg2) {
    unsigned int buf[4];

    buf[0] = arg0;
    buf[1] = (unsigned int)arg1;
    buf[2] = (unsigned int)arg2;
    buf[3] = (unsigned int)&D_00154A10 | 0x20000000;
    return func_00118E90_ret(1, buf);
}

LINKER_REMNANT("asm/remnants/core_text", func_00119760);

extern void func_00118E90(int arg0, void *arg1);

void func_00119768(int arg0, signed char arg1) {
    int buf[4];
    buf[0] = arg0;
    buf[1] = arg1;
    func_00118E90(0x3, buf);
}


void func_00119798(int arg0) {
    int local = arg0;
    func_00118E90(0x4, &local);
}

void func_001197C0(int arg0, int arg1, unsigned short arg2) {
    int buf[4];
    buf[0] = arg0;
    buf[1] = arg1;
    buf[2] = arg2;
    func_00118E90(-0x5, buf);
}

void func_001197F8(int arg0, int arg1, unsigned short arg2) {
    int buf[4];
    buf[0] = arg0;
    buf[1] = arg1;
    buf[2] = arg2;
    func_00118E90(-0x6, buf);
}

LINKER_REMNANT("asm/remnants/core_text", func_00119830);

void func_00119840(int arg0) {
    int local = arg0;
    func_00118E90(0x10, &local);
}

INCLUDE_ASM("asm/nonmatchings/core_text", func_00119864);
