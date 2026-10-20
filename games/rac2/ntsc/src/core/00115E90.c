#include "common.h"

/* The unit saves $s0..$s4 and $ra in TImode slots (`sq`), so it is built with
   the -mastra-quad-saves profile. D_00133E74 is the runtime record (the
   allocator's `struct _reent *`); it is re-read after the calls, since a
   callee may store to it, which is what retail does. */
extern void *D_00133E74;

/* The exit thunk (src/core/00131780.c): a tail call to exit that drops the
   status it is given. Retail calls it with the status in $a0, and it never
   returns, so the function has no epilogue. */
extern void func_00131CA8(int arg0) __attribute__((noreturn));

/* One node of the exit-procedure list: the next node, how many handlers the
   node holds, and the handlers behind it. They are called back to front. */
struct ExitProc {
    struct ExitProc *next;
    int count;
    void (*funcs[1])(void);
};

/* The runtime record: the hook at 0x3C, called with the record itself when it
   is not null, and the exit-procedure list at 0x148. */
struct ExitReent {
    char unk00[0x3C];
    void (*unk3C)(struct ExitReent *arg0);
    char unk40[0x148 - 0x40];
    struct ExitProc *exitprocs;
};

void func_00115E90(int status) {
    struct ExitReent *reent;
    struct ExitProc *proc;
    int count;

    proc = ((struct ExitReent *)D_00133E74)->exitprocs;
    while (proc != 0) {
        count = proc->count;
        while (--count >= 0) {
            (*proc->funcs[count])();
        }
        proc = proc->next;
    }

    reent = (struct ExitReent *)D_00133E74;
    if (reent->unk3C != 0) {
        reent->unk3C(reent);
    }

    func_00131CA8(status);
}
