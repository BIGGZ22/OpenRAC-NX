/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The port's sound engine (port/audio's SoundPlayer) as the sound library's replacement
 * (lib/snd.c) and each game's command-batch flush call it: banks, sounds, volumes. The
 * window's audio output pulls openrac_sound_render. Handles are the engine's; 0 is none.
 * Thread-safe: the output renders on its own thread. */
#ifndef OPENRAC_SOUND_ENGINE_H
#define OPENRAC_SOUND_ENGINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 989snd's IOP commands that the port answers from a batch (the numbers the EE library sends). */
enum {
    OPENRAC_SND_PLAY_VOL_PAN_PMPB = 0x11,
    OPENRAC_SND_SET_SOUND_PARAMS = 0x21,
};

/* A bank file in host memory, or read from the disc image at sector + offset. */
uint32_t openrac_sound_load_bank(const void* bytes, size_t size);
uint32_t openrac_sound_load_bank_from_disc(uint32_t sector, uint32_t offset);
void openrac_sound_unload_bank(uint32_t bank);

uint32_t openrac_sound_play(uint32_t bank, int32_t sound, int32_t volume, int32_t pan,
                            int32_t pitch_mod, int32_t pitch_bend);
/* snd_SetSoundParams: bit 0 volume, bits 1-2 pan, bit 3 pitch modifier, bit 4 pitch bend.
 * Returns the handle while the sound plays, else 0. */
uint32_t openrac_sound_set_params(uint32_t sound, int32_t mask, int32_t volume, int32_t pan,
                                  int32_t pitch_mod, int32_t pitch_bend);
int openrac_sound_playing(uint32_t sound);
void openrac_sound_stop(uint32_t sound);
void openrac_sound_stop_all(void);
void openrac_sound_set_master_volume(int32_t group, int32_t volume);
void openrac_sound_pause_groups(uint32_t mask);
void openrac_sound_continue_groups(uint32_t mask);

/* One 989snd command from the EE's batch (u16 command, u16 size, then `size` bytes); returns the
 * IOP's reply word. Commands the port does not take yet answer 0. */
uint32_t openrac_sound_command(uint32_t command, const void* data, uint32_t size);

/* Interleaved stereo, 48 kHz. */
void openrac_sound_render(int16_t* interleaved, size_t frames);

#ifdef __cplusplus
}
#endif

#endif
