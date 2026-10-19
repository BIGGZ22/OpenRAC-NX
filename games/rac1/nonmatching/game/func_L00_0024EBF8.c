/* fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* Spins while bit 0x100 of the word at the third argument (register $3 in retail: a flag word the caller points to) is set; returns when clear. */
void func_L00_0024EBF8(void *a0, void *a1, volatile unsigned int *flag) {
    /* fix: volatile, so the word is re-read each iteration as retail does (0024EBF8 lw, j back at 0024EC10) */
    while ((*flag & 0x100) != 0) {
    }
}
