/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Returns -1 when the input is zero, 0 otherwise. Retail reads the input from $v1 at entry rather than from an argument register, so the parameter here is a normal C argument and the calling convention is the open question; the value logic is the same as the assembly. */
int func_L00_0027895C(int a) {
    return a ? 0 : -1;
}
