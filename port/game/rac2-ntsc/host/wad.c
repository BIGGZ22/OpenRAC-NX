/* SPDX-License-Identifier: GPL-3.0-or-later
 * Going Commando's WAD decompressor, 0x0029D4D0 (hand-written assembly).
 *
 * The retail routine checks the "WAD" header at its first argument, streams
 * the compressed data through the scratchpad by DMA (channel 0x1000D400) and
 * writes the output at its second argument; it returns the number of bytes
 * written (its output pointer less the second argument) and traps on a bad
 * header. The shared asset decoder reads the same stream directly in memory,
 * as Deadlocked's port does (rac4-ntsc/host/wad.c).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openrac/guest.h"
#include "openrac/game_host.h"

uint32_t openrac_lib_wad_decompress(const uint8_t*, uint8_t*, uint32_t);

int func_0029D4D0(int source, int destination) {
    const uint32_t from = (uint32_t)source & 0x1FFFFFFFu;
    const uint32_t to = (uint32_t)destination & 0x1FFFFFFFu;
    if (from > GUEST_RAM_SIZE - 16u || to >= GUEST_RAM_SIZE) {
        openrac_game_crash("Going Commando WAD buffer lies outside game RAM");
        return 0;
    }
    const uint8_t* data = (const uint8_t*)G(from);
    uint32_t compressed;
    memcpy(&compressed, data + 3, sizeof compressed);
    if (memcmp(data, "WAD", 3) != 0 || compressed < 16u || compressed > GUEST_RAM_SIZE - from) {
        openrac_game_crash("Going Commando WAD header or compressed size is invalid");
        return 0;
    }
    const uint32_t made = openrac_lib_wad_decompress(data, G(to), GUEST_RAM_SIZE - to);
    if (made == 0) {
        openrac_game_crash("Going Commando WAD decompression failed");
        return 0;
    }
    if (getenv("OPENRAC_TRACE_WAD")) {
        fprintf(stderr, "[debug] Going Commando WAD at %08x: %u -> %u bytes, destination %08x\n", from,
                compressed, made, to);
    }
    return (int)made;
}
