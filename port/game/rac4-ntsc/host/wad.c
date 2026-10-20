/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native boundary for the NTSC-U WAD decoder at 0x00469C40.
 * The retail routine checks the WAD header, feeds compressed data through
 * scratchpad DMA, and returns the number of decompressed bytes.
 * The shared asset decoder processes the same stream directly in memory.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openrac/guest.h"
#include "openrac/game_host.h"

uint32_t openrac_lib_wad_decompress(const uint8_t*, uint8_t*, uint32_t);

int func_00469C40(int source, int destination) {
    const uint32_t from = (uint32_t)source & 0x1FFFFFFFu;
    const uint32_t to = (uint32_t)destination & 0x1FFFFFFFu;
    if (from > GUEST_RAM_SIZE - 16u || to >= GUEST_RAM_SIZE) {
        openrac_game_crash("Deadlocked WAD buffer lies outside game RAM");
        return 0;
    }
    const uint8_t* data = (const uint8_t*)G(from);
    uint32_t compressed;
    memcpy(&compressed, data + 3, sizeof compressed);
    if (memcmp(data, "WAD", 3) != 0 || compressed < 16u ||
        compressed > GUEST_RAM_SIZE - from) {
        openrac_game_crash("Deadlocked WAD header or compressed size is invalid");
        return 0;
    }
    const uint32_t made = openrac_lib_wad_decompress(data, G(to), GUEST_RAM_SIZE - to);
    if (made == 0) {
        openrac_game_crash("Deadlocked WAD decompression failed");
        return 0;
    }
    if (getenv("OPENRAC_TRACE_WAD")) {
        fprintf(stderr, "[debug] Deadlocked WAD at %08x: %u -> %u bytes, destination %08x\n",
                from, compressed, made, to);
    }
    return (int)made;
}
