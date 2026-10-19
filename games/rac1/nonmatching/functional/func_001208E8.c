/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Interrupt-level wrapper around func_00118CA0 (signal semaphore, one int argument): passes the third argument (retail moves $6 into $4 in the jal delay slot), returns the callee's result, then re-enables interrupts (sync; ei) and restores the frame. The sync and ei have no host effect; the second argument ($5) is passed through untouched in retail and the callee does not take it, so it is not used. Confidence is medium because the caller is not in C and only the register use shows the argument. */
extern int func_00118CA0(int);

int func_001208E8(int arg0, int arg1, int arg2) {
    (void)arg0;
    (void)arg1;
    return func_00118CA0(arg2);
}
