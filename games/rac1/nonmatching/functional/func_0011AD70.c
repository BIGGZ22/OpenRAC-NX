/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Data-cache writeback of [addr, addr + size) in 64-byte lines (cache op 0x18, nothing when size <= 0); the host has no separate cache, memory is already coherent, and retail returns nothing, so it is empty. */
void func_0011AD70(void *addr, int size) {
    (void)addr;
    (void)size;
}
