#include "types.h"
#include "asm.h"

#include "rnc/audio/music/music_stream_state.h"

extern s32 snd_stream_safe_cd_break() __asm__("func_0012EEA8");
void request_audio_stream_break(void) __asm__("FUN_002166e8");

void request_audio_stream_break(void) {
    if (music_stream_state.read_state != 0) {
        snd_stream_safe_cd_break();
        music_stream_state.break_requested = 1;
    }
}
