#include "include_asm.h"

/* Initialises the call state at 0x001A63A8: the mode word at +0x3C, the four flag bytes
   at +0x40..+0x43, the busy counters and the step words, then the two channel helpers.
   The candidate has retail's exact size and every instruction but one: retail keeps the
   final store (the -1 at +0x24) in the delay slot of the call to func_002B66F0, this cc1
   schedules it before the last flag store.  Making every store volatile keeps the
   retail order exactly but then the delay slot holds a nop (a volatile store is not moved
   into a delay slot), which is one word too many. */
extern void func_0026FEB8(void *);
extern char D_001BA198[];

void func_002B68C8(void)
{
    volatile u8 *state;
    short mode;

    func_0026FEB8(D_001BA198);
    state = D_001A63A8;
    mode = -1;
    *(volatile short *)(state + 0x3C) = mode;
    state[0x40] = 0x20;
    state[0x41] = 0;
    state[0x42] = 0;
    state[0x43] = 0;
    *(volatile u32 *)(state + 0x44) = 0;
    *(volatile short *)(state + 0x4E) = 0;
    *(volatile u32 *)(state + 0x68) = 0;
    *(volatile short *)(state + 0x72) = 0;
    *(volatile u32 *)(state + 0x8C) = 0;
    *(volatile short *)(state + 0x96) = 0;
    *(int *)((char *)state + 0x24) = mode;
    func_002B66F0();
    func_002B6938();
}
