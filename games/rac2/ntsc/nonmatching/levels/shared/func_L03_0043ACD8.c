/* from src/levels/shared/L03_003C5B28.c: C that compiles but does not link at its places yet */
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
