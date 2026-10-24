#ifndef MUSIC_H
#define MUSIC_H

/*
 * The music streamer's channel record, laid out as the NTSC decomp's
 * audio/music/music_stream_state.h has it.
 */

#include "common.h"

struct MusicStreamChannel {
    s32 handle; /* 0x00: stream handle from the start callback; -1 while starting */
    s16 track; /* 0x04 */
    s16 volume; /* 0x06 */
    s16 flags; /* 0x08 */
    s16 state; /* 0x0A: set 1 on start, 1 -> 2 in the start callback */
    s16 fade_flags; /* 0x0C: -0x8000 on music_pause, 4 on music_unpause */
    s16 unkE; /* 0x0E: cleared by music_pause */
    s16 crossfade_enabled; /* 0x10: remaining-time callback starts the crossfade */
    u8 pad_12[0x2];
    s32 poll_interval; /* 0x14 */
    s32 remaining_time; /* 0x18: set by music_remaining_time_callback */
};

#endif /* MUSIC_H */
