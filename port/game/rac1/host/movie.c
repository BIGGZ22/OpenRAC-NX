/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * Ratchet & Clank: a movie played in a level (an infobot's, the pause menu's).
 *
 * The console's movie decoder works in the level's main RAM, so the game parks that RAM in the GS's
 * memory first (prepare_movie_playback: thirteen 128 x 128 24-bit strips from block 0x1B00, then
 * seventeen 64 x 64 32-bit ones) and reads it back after (func_L00_0029AB38), over the HUD's textures
 * it uploads again later. The port's movie player reads the disc into memory of its own, so the RAM is
 * never touched; and the renderer keeps each upload as an image of its own format, so the parked
 * strips stayed on top of the 8-bit textures in the same blocks: after a movie the Gadgetron vendor's
 * item icons drew as blank squares. Here both calls run the game's own code with the image transfers
 * switched off (sceGsExecLoadImage, sceGsExecStoreImage in common/lib/graph.c).
 */
#include "openrac/game_lib.h"
#include "rac1.h"

/* prepare_movie_playback(sector, size, language): stops the sound, queues the movie, parks the RAM. */
void func_L00_0029A940(int sector, int size, int language) {
    openrac_lib_gs_images_parked = 1;
    func_L00_0029A940__game(sector, size, language);
    openrac_lib_gs_images_parked = 0;
}

/* After the movie: reads the parked RAM back, then starts the level's music again. */
void func_L00_0029AB38(void) {
    openrac_lib_gs_images_parked = 1;
    func_L00_0029AB38__game();
    openrac_lib_gs_images_parked = 0;
}
