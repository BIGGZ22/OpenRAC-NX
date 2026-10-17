/* functional: high. Interrupt-side callback trampoline: if the word at offset 0 of the block is non-null, calls that function with the word at offset 4 as its argument (the argument register a0 is overwritten, so the first parameter is unused); the return address is saved and restored on the stack, which the host does not need. The retail sync/ei pair re-enables interrupts on exit and is a no-op on the host. */

typedef int s32_bc80;

void func_0011BC80(int unused, s32_bc80 *cb) {
    if (cb[0] != 0) {
        ((void (*)(s32_bc80))cb[0])(cb[1]);
    }
}
