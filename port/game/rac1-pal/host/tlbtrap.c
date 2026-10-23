/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * tlbtrap (func_0011D700, 0x260 bytes of libkernl's tlbtrap.o, hand-written assembly): the handler
 * of the TLB and address-error exceptions, entered by the processor with COP0's EXL set. Nothing
 * installs it: no code in any program forms 0x0011D700 (no jal, j, branch or address of it),
 * nothing writes the user handler D_001300E8 it calls, and _InitSys's table D_00130138 hands the
 * kernel only the object's second half (SetSyscall(0x54, 0x0011D840)), which only the trap path
 * itself reaches. The port takes no exceptions and has no COP0 or register file, so no recording of
 * it can exist. On the console:
 *
 *   - every general register but k0/k1 is stored a quadword each at D_001595C0 + 0x10 * n
 *     ($1..$25, $28..$31), then HI, HI1, LO, LO1 and SA a doubleword each from D_001597C0;
 *   - Status, Cause, EPC and BadVAddr are read into a0..a3, the save area into t0; EPC is kept at
 *     D_001597E8, EPC becomes D_0011D800 and Status loses IE; eret leaves the exception level there;
 *   - D_0011D800 calls the user handler *D_001300E8 (a0..a3, t0) with the save area as its stack
 *     top, then makes system call -0x54, which the kernel answers at 0x0011D840: Status loses ERL
 *     and KSU, EPC comes back from D_001597E8 (where the user handler may have changed it), HI, LO,
 *     SA and the registers are loaded back, Status gets IE, EXL and user mode (0x13), and eret goes
 *     back to the instruction that trapped (or where the handler pointed EPC);
 *   - the eight words at 0x0011D940 after the eret and its nops (a jump through a table at $2 on a
 *     stack at 0x1000, then break) are never executed.
 *
 * In the port's terms (libraries.tsv binds it; PAL only, the addresses are this program's): there
 * are no registers of an interrupted context to keep (the port's code keeps its own state across
 * this call) and nothing to return to but the caller. COP0's four registers are what the port's
 * kernel layer answers for them (iGetCop0 func_00118D90: 0 for each); EPC is kept at D_001597E8 as
 * the console keeps it, and the user handler, when one is set, gets them and the save area as the
 * console passes them. Interrupts stay as they are (the port takes none).
 */
#include "rac1.h"

/* D_001595C0: the register save area, the user handler's stack top. */
#define TLBTRAP_SAVE 0x001595C0u
/* D_001597E8: EPC, which the return path loads back. */
#define TLBTRAP_SAVED_EPC 0x001597E8u
/* D_001300E8: the user's trap handler (nothing writes it). */
#define TLBTRAP_USER_HANDLER 0x001300E8u

void func_0011D700(void) {
    const unsigned int status = func_00118D90(12); /* mfc0 $4, Status */
    const unsigned int cause = func_00118D90(13);  /* mfc0 $5, Cause */
    const unsigned int epc = func_00118D90(14);    /* mfc0 $6, EPC */
    const unsigned int bad = func_00118D90(8);     /* mfc0 $7, BadVAddr */
    gaddr handler;

    GREF(uint32_t, TLBTRAP_SAVED_EPC) = epc;
    handler = GREF(gaddr, TLBTRAP_USER_HANDLER);
    if (handler != 0) {
        /* The console calls it whatever the word holds (address 0 traps again); the port does not
         * jump to 0. */
        GFN(void (*)(unsigned int, unsigned int, unsigned int, unsigned int, gaddr),
            handler)(status, cause, epc, bad, TLBTRAP_SAVE);
    }
}
