/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Returns -1 in $v0. Retail uses a trapping `addi` (overflow trap cannot fire for a constant); plain C returns -1 via addiu, same result. */
int func_L00_001F3980(void) {
    return -1;
}
