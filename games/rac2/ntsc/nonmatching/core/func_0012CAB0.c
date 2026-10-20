#include "common.h"

extern void func_0012C9C8(s32);
extern s32 func_0012C878(s32, s32);
extern void func_0012CBC0(s32);
extern void func_0012D060(s32);
extern void func_00130300(s32);
extern void func_0012FA98(s32, void *);

struct Request {
    s32 word;
    s32 pad;
    long low;
    long high;
};

/* Drive one state object through the loader's transfer states until it
   stops: 0x1B3 and 0x1B8 step and keep going, 0x100 gives up and runs the
   final pass, 0x1B7 reports the failure. */
s32 func_0012CAB0(s32 arg0) {
    struct Request args;
    long high;
    long low;
    s32 state;

    for (;;) {
        func_0012C9C8(arg0);
        state = func_0012C878(arg0, 0x20);
        switch (state) {
        case 0x1B3:
            func_00130300(arg0);
            break;
        case 0x1B8:
            func_0012D060(arg0);
            break;
        case 0x100:
            func_0012CBC0(arg0);
            goto done;
        case 0x1B7:
            return 0;
        }
    }
done:
    args.word = 5;
    args.low = -1;
    args.high = -1;
    func_0012FA98(*(s32 *) (arg0 + 0x858), &args);
    high = args.high;
    low = args.low;
    *(long *) (arg0 + 0x830) = high;
    *(long *) (arg0 + 0x828) = low;
    return *(s32 *) (arg0 + 0x150);
}
/* Near miss: same size, one register pair swapped.
   The whole loop, the switch tree (0x1B3 first, then sltiu 0x1B4, then 0x100
   against 0x1B7/0x1B8), the final request structure and the 0x828/0x830
   read-back all match. `boot: instruction at +0xc differs` is the only
   complaint: retail gives the constant 5 $s6 and 0x1B3 $s5, this candidate the
   other way, and every later reference moves with them. Hoisting
   `args.word = 5` above the loop instead makes the frame 0x90 and leaves 5
   with no callee-saved home at all. */
