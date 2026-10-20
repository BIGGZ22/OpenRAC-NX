/* from src/core/001261F0.c: the catalogue row is 164 bytes but the function is 160.
   Its first word is the linker's 0xCDCDCDCD fill between two objects, and the
   function itself is the 160 bytes from 0x00126DC0. The C below reproduces those
   160 bytes exactly: compiled in the unit of src/core/001261F0.c (the SDK compiler,
   -O2 -G2 -ffunction-sections) and compared with the retail bytes at 0x00126DC0, it
   matches instruction for instruction and call target for call target.
   tools/check.py on the row as the catalogue has it prints "160 bytes, expected 164"
   and --diff lines the candidate up four bytes behind retail, the fill word first.
   Unblocked by splitting the row in tools/levels.py (start +4, size 160), not by more
   C.

   The body: the state at D_00137E00 (returned by func_00125960) keeps an id at +8 and
   a handle at +0xC. With arg 0 it calls the two kernel wrappers (func_0011B588,
   func_0011A920) and clears both words. With a non-zero arg it calls them only when
   the id was already set, writes the arg as the id, publishes the handle that
   func_0011A900(2, arg, -1) returns and calls func_0011B5F0; it returns the id the
   state had. The two clearing stores are written handle first, id second: the
   compiler sinks the first into the jump's delay slot, which is where retail has it.
   The guard is written on the id read before the calls, because retail tests that
   value (in $s2) and returns it. */
#include "common.h"

/* The boot's "core" state at D_00137E00, as far as this reads it. */
typedef struct {
    char pad_00[8];
    int id;     /* 0x08 */
    int handle; /* 0x0C */
} CoreState;

extern void *func_00125960(void);

extern void func_0011B588(int);

extern int func_0011A900(int, int, int);

extern void func_0011A920(int, int);

extern void func_0011B5F0(int);

int func_00126DBC(int arg) {
    CoreState *state = (CoreState *)func_00125960();
    int old = state->id;

    if (arg == 0) {
        func_0011B588(2);
        func_0011A920(2, state->handle);
        state->handle = 0;
        state->id = 0;
    } else {
        if (old != 0) {
            func_0011B588(2);
            func_0011A920(2, state->handle);
        }
        state->id = arg;
        state->handle = func_0011A900(2, arg, -1);
        func_0011B5F0(2);
    }
    return old;
}
