/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's movie player entry, PlayFmvMovie (0x0034EF58, matched C in
 * the decompilation): (first sector, size, an EE buffer pool, the player's
 * state block, a mode, the volume). It starts the IPU decoder and its threads
 * (InitFmvPlaybackEngine), feeds the stream from the disc (sceCdRead at the
 * sector in the stream object's +4, from the first argument) and tears it all
 * down again. Threads, the IPU and the DMA it waits on are not in the port:
 * the shared native player plays the same stream in the window and owns its
 * buffers, and the game keeps its own movie transitions around the call. Its
 * result is the native player's: 1 when skipped, 0 at the end, as Deadlocked's
 * port answers its own (rac4-ntsc/host/movie.c).
 */
#include <stdint.h>

#include "openrac/game_host.h"

int func_0034EF58(int first, int second, int pool, int state, int mode, int volume) {
    (void)pool;
    (void)state;
    (void)mode;
    (void)volume;
    return openrac_game_play_movie((uint32_t)first, (uint32_t)second, 0, 1);
}
