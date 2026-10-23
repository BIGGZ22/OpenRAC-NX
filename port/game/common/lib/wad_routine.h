/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * A game's WAD decompressor routine, for the games whose host C replaces it
 * (rac2-ntsc/host/wad.c, rac4-ntsc/host/wad.c): each binds the routine's
 * address to a two-line function that calls this one with its name.
 *
 * The retail routine checks the "WAD" header at its first argument, streams
 * the compressed data through the scratchpad by DMA and writes the output at
 * its second argument; it returns the number of bytes written and traps on a
 * bad header. The port decodes the same stream directly in game memory with
 * the shared asset decoder (wad.cpp). */
#ifndef OPENRAC_WAD_ROUTINE_H
#define OPENRAC_WAD_ROUTINE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Decompresses the WAD stream at game address `source` into game memory at
 * `destination` and returns the number of bytes written. `game` is the name
 * the crash and trace messages begin with ("Going Commando WAD ..."). A
 * stream that lies outside game RAM, has a bad header or does not decode ends
 * the program through openrac_game_crash. With OPENRAC_TRACE_WAD set, prints
 * each decompression on stderr. */
int openrac_wad_routine(int source, int destination, const char* game);

#ifdef __cplusplus
}
#endif

#endif
