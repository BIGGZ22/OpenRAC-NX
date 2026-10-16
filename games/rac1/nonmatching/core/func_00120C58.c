/* Interrupt handler registered by func_00120CA0 (entered at +8, which skips the
   frame adjust). If a handler is set (D_00159844) and the flag D_001313E4 is
   clear, calls it with D_00159848 as its argument. */
extern int D_00159844;
extern int D_00159848;
extern int D_001313E4;

void func_00120C58(void) {
    if (D_00159844 != 0 && D_001313E4 == 0) {
        ((void (*)(int))D_00159844)(D_00159848);
    }
}
