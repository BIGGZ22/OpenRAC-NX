/* fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* coll_sound_class: footstep sound class (bits 5-6) of the last collision hit (CollOutput+0x1c); 0 when negative or class 3. */
extern int D_L00_00173F40[];
int func_L00_001F3988(void) {
    int t = D_L00_00173F40[7];
    if (t < 0) {
        return 0;
    }
    if ((t & 0x60) == 0x60) {
        return 0;
    }
    return (t & 0x60) >> 5;
}
