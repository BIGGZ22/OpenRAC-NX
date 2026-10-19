/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The sound library (989snd), replaced at its API: the cleanest cut, since
 * everything below it ran on the IOP (docs/port/RAC1_PAL_SURVEY.md, section
 * 8). Banks and sounds go to the port's sound engine (openrac/sound_engine.h);
 * the sounds a game sends in its command batch are taken by that game's flush.
 * VAG streams (music, speech) are read from the disc and played by the same
 * engine; movie sound is the movie player's. Disc reads made through the library's
 * stream-safe calls go to the disc replacement. */
#include "openrac/game_host.h"
#include "openrac/game_lib.h"
#include "openrac/guest.h"
#include "openrac/sound_engine.h"

#include <stdio.h>
#include <stdlib.h>

/* OPENRAC_SOUND_LOG (as the command flush, rac1/host/hand.c): the stops and the still-playing
 * questions, which go to the engine directly rather than through the batch, on stderr too. */
static int sound_log(void) {
    static int on = -1;
    if (on < 0) {
        on = getenv("OPENRAC_SOUND_LOG") != NULL;
    }
    return on;
}

int openrac_lib_snd_StartSoundSystem(void) {
    return 0;
} /* snd_StartSoundSystem */

/* The disc read of snd_StreamSafeCdRead and the callback the game gave for its end
 * (snd_StreamSafeCdCallback). The read itself is done at once; the callback is
 * called with 1 (done) at the next command flush, as the IOP's answer reaches the
 * EE after the call has returned (the game marks the read as running only once
 * it has). */
static gaddr cd_callback;
static int cd_done_pending;

int openrac_lib_snd_FlushSoundCommands(void) {
    if (cd_done_pending && cd_callback != 0) {
        cd_done_pending = 0;
        GFN(void (*)(int), cd_callback)(1);
    }
    return 0;
} /* snd_FlushSoundCommands */

unsigned int openrac_lib_snd_BankLoadByLoc(int sector, int offset) { /* snd_BankLoadByLoc */
    return openrac_sound_load_bank_from_disc((uint32_t)sector, (uint32_t)offset);
}

/* snd_BankLoadFromEE_CB(bank, callback, argument): a bank in EE memory, loaded at once; the
 * callback is told, as the IOP's answer would: callback(handle, argument). */
void openrac_lib_snd_BankLoadFromEE_CB(int bank, int callback, long long argument) {
    const unsigned int handle =
        bank != 0 ? openrac_sound_load_bank(G((gaddr)bank), 0x02000000u - ((gaddr)bank & 0x01FFFFFFu)) : 0;
    if (callback != 0) {
        GFN(void (*)(int, long long), (gaddr)callback)((int)handle, argument);
    }
} /* snd_BankLoadFromEE_CB */

void openrac_lib_snd_ResolveBankXREFS(void) {} /* snd_ResolveBankXREFS */

void openrac_lib_snd_UnloadBank(int bank) {
    openrac_sound_unload_bank((uint32_t)bank);
} /* snd_UnloadBank */

void openrac_lib_snd_SetMasterVolume(int which, int volume) {
    openrac_sound_set_master_volume(which, volume);
} /* snd_SetMasterVolume */

void openrac_lib_snd_SetPlaybackMode(int mode) {
    (void)mode;
} /* snd_SetPlaybackMode */

void openrac_lib_snd_StopAllSounds(void) {
    openrac_sound_stop_all();
} /* snd_StopAllSounds */

void openrac_lib_snd_PauseAllSoundsInGroup(int group) {
    openrac_sound_pause_groups((uint32_t)group);
} /* snd_PauseAllSoundsInGroup */

void openrac_lib_snd_ContinueAllSoundsInGroup(int group) {
    openrac_sound_continue_groups((uint32_t)group);
} /* snd_ContinueAllSoundsInGroup */

/* snd_SoundIsStillPlaying_CB(handle, callback, argument): callback(handle) while it plays,
 * callback(0) once it has ended (a stream when stopped). */
void openrac_lib_snd_SoundIsStillPlaying_CB(int a, int b, int c) {
    if (b != 0) {
        const int live = openrac_stream_live((uint32_t)a) || openrac_sound_playing((uint32_t)a);
        if (sound_log()) {
            fprintf(stderr, "[debug] sound still playing %#x -> %d\n", (unsigned)a, live);
        }
        GFN(void (*)(int, long long), (gaddr)b)(live ? a : 0, (long long)c);
    }
} /* snd_SoundIsStillPlaying_CB */

void openrac_lib_snd_reset_state_and_flush_commands(void) {
} /* snd_reset_state_and_flush_commands */

int openrac_lib_snd_InitVAGStreamingEx(int a, int b, int c, int d) {
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 0;
} /* snd_InitVAGStreamingEx */

void openrac_lib_snd_StopAllStreams(void) {
    openrac_stream_stop(0);
} /* snd_StopAllStreams */

/* snd_StopSound(handle): a sound or a stream ends. */
void openrac_lib_snd_StopSound(int handle) {
    if (sound_log()) {
        fprintf(stderr, "[debug] sound stop %#x (%s)\n", (unsigned)handle,
                handle == 0 ? "none" : openrac_sound_playing((uint32_t)handle) ? "playing" : "not playing");
    }
    if (handle != 0) {
        if (openrac_stream_live((uint32_t)handle)) {
            openrac_stream_stop((uint32_t)handle);
        } else {
            openrac_sound_stop((uint32_t)handle);
        }
    }
} /* snd_StopSound */

/* snd_PlayVAGStreamByLocEx_CB(sector, 0, 0, 0, volume, pan, group, queue on, flags, callback,
 * argument), as RAC1's music and speech call it: a stream starts (or is queued behind the live
 * stream `queue on`); the callback gets its handle, as the IOP's answer would. Flags: 1 paused,
 * 4 loop. */
void openrac_lib_snd_PlayVAGStreamByLocEx_CB(
    int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, long long k
) {
    (void)b;
    (void)c;
    (void)d;
    (void)f;
    const uint32_t handle = openrac_stream_play((uint32_t)a, e, g, (uint32_t)h, (uint32_t)i);
    if (j != 0) {
        GFN(void (*)(int, long long), (gaddr)j)((int)handle, k);
    }
} /* snd_PlayVAGStreamByLocEx_CB */

void openrac_lib_snd_PauseVAGStream(int stream) {
    openrac_stream_pause((uint32_t)stream, 1);
} /* snd_PauseVAGStream */

void openrac_lib_snd_ContinueVAGStream(int stream) {
    openrac_stream_pause((uint32_t)stream, 0);
} /* snd_ContinueVAGStream */

/* snd_GetVAGStreamTimeRemaining_CB(handle, callback, argument): milliseconds left in the part
 * playing (the game only compares them with each other). */
void openrac_lib_snd_GetVAGStreamTimeRemaining_CB(int a, int b, int c) {
    if (b != 0) {
        GFN(void (*)(int, long long), (gaddr)b)(openrac_stream_remaining_ms((uint32_t)a), (long long)c);
    }
} /* snd_GetVAGStreamTimeRemaining_CB */

/* snd_IsVAGStreamBuffered_CB(handle, callback, argument): once its first part is decoded. */
void openrac_lib_snd_IsVAGStreamBuffered_CB(int a, int b, int c) {
    if (b != 0) {
        GFN(void (*)(int, long long), (gaddr)b)(openrac_stream_buffered((uint32_t)a), (long long)c);
    }
} /* snd_IsVAGStreamBuffered_CB */

void openrac_lib_snd_StreamSafeCheckCDIdle(int a) {
    (void)a;
} /* snd_StreamSafeCheckCDIdle */

int openrac_lib_snd_StreamSafeCdRead(int sector, int count, int buffer) { /* snd_StreamSafeCdRead */
    const int ok = openrac_lib_sceCdRead((unsigned int)sector, (unsigned int)count, (gaddr)buffer, 0);
    cd_done_pending = ok != 0;
    return ok;
}

int openrac_lib_snd_StreamSafeCdSync(int mode) {
    return openrac_lib_sceCdSync(mode);
} /* snd_StreamSafeCdSync */

int openrac_lib_snd_StreamSafeCdBreak(void) {
    return openrac_lib_sceCdBreak();
} /* snd_StreamSafeCdBreak */

int openrac_lib_snd_StreamSafeCdGetError(void) {
    return openrac_lib_sceCdGetError();
} /* snd_StreamSafeCdGetError */

int openrac_lib_snd_StreamSafeCdCallback(int callback) {
    const gaddr old = cd_callback;
    cd_callback = (gaddr)callback;
    return (int)old;
} /* snd_StreamSafeCdCallback */

void openrac_lib_snd_PreAllocReverbWorkArea(int a, int b) {
    (void)a;
    (void)b;
} /* snd_PreAllocReverbWorkArea */

void openrac_lib_snd_AutoReverb(int a, int b, int c, int d) {
    (void)a;
    (void)b;
    (void)c;
    (void)d;
} /* snd_AutoReverb */

void openrac_lib_snd_ResetMovieSound(void) {} /* snd_ResetMovieSound */

void openrac_lib_snd_StartMovieSound(int a, int b, int c, int d, int e) {
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
} /* snd_StartMovieSound */

int openrac_lib_snd_UpdateMovieADPCM(int a, int b) {
    (void)a;
    (void)b;
    return 0;
} /* snd_UpdateMovieADPCM */

int openrac_lib_snd_GetMovieNAX(void) {
    return 0;
} /* snd_GetMovieNAX */

int openrac_lib_snd_GetDopplerPitchMod(int a) {
    (void)a;
    return 0;
} /* snd_GetDopplerPitchMod */
