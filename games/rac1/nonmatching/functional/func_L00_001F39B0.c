/* functional: high. Returns 0 in $v0. Retail uses a trapping `addi $2, $0, 0x0`; the trap cannot fire for a constant, so plain C returning 0 behaves the same. */
int func_L00_001F39B0(void) {
    return 0;
}
