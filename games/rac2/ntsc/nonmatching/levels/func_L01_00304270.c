/* Does not match: the attempt compiles to 36 bytes, retail is 52 (13 instructions). Retail loads its pointer from $v1, a register that is not an argument, and tests with a2; the self-jump (j func_304270) on a 0x100 test suggests a caller-set $v1 or a hand-written spin. Missing: the caller context that sets $v1 (no C parameter reaches $v1). */
void func_L01_00304270(volatile unsigned int *p) {
    while (*p & 0x100)
        ;
}
