/* functional: medium. fcheck: unchecked. Fatal-error stop: retail executes an unconditional trap (teq $0, $0), which raises the EE's trap exception and never returns to the caller in a shipped game; callers use it where a state should be impossible (an unknown moby state, a broken list). The port stops the same way with the compiler's trap. Written from the asm (4 instructions). */
void func_001F9978(void) {
    __builtin_trap();
}
