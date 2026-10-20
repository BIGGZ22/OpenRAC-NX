/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's file I/O entry points that the generated binding cannot
 * take: sceOpen (0x0011E0D8, libkernl's fileio client) is declared variadic
 * in the decompilation, as the library declares it; the mode is its only
 * optional argument. The shared replacement answers it.
 */
#include "openrac/guest.h"
#include "openrac/game_lib.h"

int func_0011E0D8(gaddr path, int flags, ...) {
    return openrac_lib_sceOpen(path, flags);
}
