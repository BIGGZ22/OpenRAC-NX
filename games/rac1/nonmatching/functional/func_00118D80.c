/* functional: high. fcheck: unchecked (FlushCache is one syscall and the host has no caches: the port's C does nothing on purpose; OpenRAC tools/fcheck_gate.py, 2026-10-18). FlushCache(op), syscall 0x64 (asm/handwritten/core_text/func_00118D80.s: a0 = op, li $3, 0x64, syscall, jr $ra). Retail's only effect is on the EE caches: op 0 writes back the data cache, op 2 invalidates the instruction cache. The host has no PS2 caches, so the port does nothing. Callers pass 0 or 2 and never read a result, so the return is void. Declarations in matched src/ that say s32 f() or void f() with no argument are harmless for the same reason. */
void func_00118D80(int op) {
    (void)op;
}
