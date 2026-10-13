#include "include_asm.h"

typedef unsigned int u32;
typedef float f32;

typedef struct {
    u32 field00;
    u32 field04;
    f32 field08;
    f32 field0C;
} NativeCallbackTimer;

extern NativeCallbackTimer D_L04_001B98E0;
extern f32 D_L04_001B1C88 __attribute__((sda));

/* updates the registered countdown state and clears its original completion flags */
void func_L03_0043ACD8(void) {
    NativeCallbackTimer *state = &D_L04_001B98E0;
    u32 mode = state->field04;
    state->field08 = 10000.0f;
    if (mode == 0) {
        state->field0C -= D_L04_001B1C88;
        if (state->field0C < 0.0f) state->field0C = 0.0f;
    } else {
        state->field04 = 0;
    }
    state->field00 = 0;
}
