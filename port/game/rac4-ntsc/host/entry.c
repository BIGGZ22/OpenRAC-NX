/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native counterpart to the retail ELF startup at 0x001574E8.
 *
 * The port owns the EE thread and memory, so the retail kernel syscalls and
 * thread hand-off are not run. Keep the user-mode initialization that precedes
 * the retail main loop, then pass it the same argc/argv block as the ELF.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "openrac/guest.h"

int func_0012D690(void);
void func_00126E20(int cache);
int func_0011EAD0(int callback);
int func_00157C58(int count, int entries);

static void trace_boot(const char *step) {
    fprintf(stderr, "BOOT: %s\n", step);
    fflush(stderr);
}

/* func_0012D690 is part of the retail entry path. These two steps inspect
 * or program PS2-only kernel state; the port supplies its system calls and
 * drives timing from the host frame loop instead. Preserve the other work
 * in the initializer. */
int func_0012D580(void) {
    trace_boot("skip retail kernel syscall-table patch");
    return 0;
}

int func_0012DD90(void) {
    trace_boot("skip retail EE interrupt-controller setup");
    return 0;
}

int func_0012DC30(int which) {
    (void)which;
    trace_boot("skip retail EE interrupt-controller setup");
    return 0;
}

int func_001275F0(void) {
    /* The retail helper returns whether EE interrupts are disabled (CP0
     * Status bit 0). The host frame loop has no EE interrupt mask. */
    return 0;
}

int func_001574E8(void) {
    uint32_t argc;

    trace_boot("func_0012D690");
    func_0012D690();
    trace_boot("func_00126E20");
    func_00126E20(0);
    trace_boot("func_0011EAD0");
    func_0011EAD0(0x00131960);

    memcpy(&argc, G(0x001CD300u), sizeof argc);
    return func_00157C58((int)argc, 0x001CD304);
}
