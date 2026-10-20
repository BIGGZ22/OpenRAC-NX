/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native counterpart to Going Commando's ELF start-up at 0x00131AE8 (crt0).
 *
 * The retail start-up clears .bss, calls the kernel's InitMainThread and
 * SetupHeap system calls ($gp 0x001AEFF0, stack below 0x01FFC000, heap end
 * 0x01FF8000), then _InitSys (0x0011F828), FlushCache(0) and main(argc,
 * argv) with the argument block at 0x00146B40, and exit with main's result.
 * The port owns the thread, the stack and the memory, and the loader has
 * zeroed .bss, so only the user-mode steps are kept here.
 */
#include <stdint.h>
#include <string.h>

#include "openrac/guest.h"

void func_0011F828(void);
void func_0011AEA0(int cache);
int func_00131D98(int argc, int argv);

/* InitSystemCallTableAddress (libkernl's initsys, called from _InitSys): it patches
 * two kernel routines through SetSyscall, then scans kernel memory with
 * FindAddress until two searches meet, to record where the kernel's table of
 * system calls is. The port has no kernel memory (FindAddress finds nothing),
 * so the scan would never end; there is nothing for it to find. */
void func_0011F718(void) {
}

/* The Deci2 console set-up of _InitSys (libkernl, rac1-decomp's func_001195A0):
 * it creates a semaphore and starts a thread that serves the debugger's TTY
 * protocol through Deci2Call. There is no debugger station and threads do not
 * run in the port; the retail routine returns -1 when the thread cannot be
 * made, and _InitSys ignores the result. */
int func_0011B800(void) {
    return -1;
}

int func_00131AE8(void) {
    uint32_t argc;

    func_0011F828(); /* _InitSys */
    func_0011AEA0(0); /* FlushCache(0) */
    memcpy(&argc, G(0x00146B40u), sizeof argc);
    return func_00131D98((int)argc, 0x00146B44);
}
