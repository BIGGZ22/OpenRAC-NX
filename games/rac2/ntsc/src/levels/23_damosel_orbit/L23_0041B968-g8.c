/* func_L23_0041B968 is a unit of its own: it reads $gp (hence -g8), and it is built with
   -mno-split-addresses, which keeps the threshold load as one `lwc1 $f2,D_L23_001B13CC` for
   the assembler to expand through $at, as retail has it. Measured on this body:
   - in the shared -g8 unit no declaration gives that form: plain and sda make the load
     $gp-relative, and nosda splits it (`lui $v0` + `lwc1`, scheduled a word early);
   - the index and the running best are set after the early return, so the compared -1 and the
     stored -1 stay two registers and 0.0f is loaded after the test;
   - the slots are read by index: walking a pointer swaps the counter and pointer registers. */

extern char *D_L23_001B1484 __attribute__((sda));

extern float D_L23_001B13CC __attribute__((nosda));

/* Stores the index of the live slot with the greatest measure above the threshold (or -1). */
void func_L23_0041B968(void) {
    char *state = *(char **)(D_L23_001B1484 + 0x68);
    int index;
    float best;
    int i;

    if (*(int *)(state + 0x60C) == -1)
        return;
    index = -1;
    best = 0.0f;
    for (i = 0; i < 5; i++) {
        char *m = ((char **)(state + 0x358))[i];

        if (m != 0) {
            float v = *(float *)(*(char **)(m + 0x68) + 0x20);

            if (D_L23_001B13CC < v && best < v) {
                best = v;
                index = i;
            }
        }
    }
    *(int *)(state + 0x60C) = index;
}
