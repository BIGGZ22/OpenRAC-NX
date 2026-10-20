/* SPDX-License-Identifier: GPL-3.0-or-later
 * The retail movie entry accepts a disc sector, byte count, an EE buffer
 * allocator, audio channel, and console display mode. The native player
 * owns its buffers and window; the surrounding game code retains its movie
 * state transitions. Its result is 1 when skipped and 0 at normal completion.
 */
#include <stdint.h>
#include "openrac/game_host.h"

int func_004EEEB0(int sector, int bytes, int allocator, int channel, int display_mode) {
    (void)allocator;
    (void)display_mode;
    return openrac_game_play_movie((uint32_t)sector, (uint32_t)bytes, channel, 1);
}
