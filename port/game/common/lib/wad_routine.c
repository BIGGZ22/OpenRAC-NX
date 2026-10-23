/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A game's WAD decompressor routine (wad_routine.h): the checks the retail
 * routine makes on its two game addresses and the header, then the shared
 * asset decoder (wad.cpp) on the stream where it lies in game memory. */
#include "game/common/lib/wad_routine.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "openrac/host_env.h"

/* The EE's segments map the same RAM; the low 29 bits are its physical
 * address (documented). */
static const uint32_t kPhysicalMask = 0x1FFFFFFFu;

/* A WAD stream's 16-byte header: "WAD", then the compressed size, header
 * included, as a little-endian word at byte 3 (documented: assets/disc/wad.h,
 * after ReRAC). */
static const char kMagic[] = "WAD";
static const size_t kSizeOffset = 3;
static const uint32_t kHeaderBytes = 16u;

/* Decodes the WAD stream at `source` into at most `capacity` bytes at
 * `destination` and returns how many it wrote; 0 for a malformed stream or
 * one whose output does not fit (wad.cpp). */
uint32_t openrac_lib_wad_decompress(const uint8_t* source, uint8_t* destination, uint32_t capacity);

/* Ends the program with the crash message "<game> WAD <problem>". */
static _Noreturn void crash(const char* game, const char* problem) {
    /* Room for the game's name and the longest problem after it. */
    char message[160];

    snprintf(message, sizeof message, "%s WAD %s", game, problem);
    openrac_game_crash(message);
}

int openrac_wad_routine(int source, int destination, const char* game) {
    const uint32_t from = (uint32_t)source & kPhysicalMask;
    const uint32_t to = (uint32_t)destination & kPhysicalMask;

    /* The whole header is read below, and the output has to start inside RAM. */
    if (from > GUEST_RAM_SIZE - kHeaderBytes || to >= GUEST_RAM_SIZE) {
        crash(game, "buffer lies outside game RAM");
    }

    const uint8_t* data = (const uint8_t*)G(from);
    uint32_t compressed;
    memcpy(&compressed, data + kSizeOffset, sizeof compressed);

    /* The retail routine traps on a bad header; a size past RAM's end cannot be read. */
    if (memcmp(data, kMagic, sizeof kMagic - 1) != 0 || compressed < kHeaderBytes
        || compressed > GUEST_RAM_SIZE - from) {
        crash(game, "header or compressed size is invalid");
    }

    const uint32_t made = openrac_lib_wad_decompress(data, G(to), GUEST_RAM_SIZE - to);

    /* The stream's packets did not decode, or would not fit in RAM after `to`. */
    if (made == 0) {
        crash(game, "decompression failed");
    }

    /* OPENRAC_TRACE_WAD (docs/port/DEBUGGING.md). */
    if (openrac_env_string("OPENRAC_TRACE_WAD")) {
        fprintf(
            stderr,
            "[debug] %s WAD at %08x: %u -> %u bytes, destination %08x\n",
            game,
            from,
            compressed,
            made,
            to
        );
    }

    return (int)made;
}
