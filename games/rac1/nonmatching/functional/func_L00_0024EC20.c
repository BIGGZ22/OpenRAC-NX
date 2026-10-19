/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Handwritten assembly: teq $0,$0,0 is an unconditional trap, so the function never reaches its jr $ra; the trap is reproduced with __builtin_trap(). The second teq and the nops after it are never reached. Confidence is medium because the trap's exact handler on the console is not known here. */
void func_L00_0024EC20(void) {
    __builtin_trap();
}
