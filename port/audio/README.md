# audio

The sound engine the games' sound library drives: library `openrac_audio`,
namespace `openrac::audio`. All four games use the same library, 989snd,
whose work ran on the console's I/O processor and sound processor; on the
PC this engine does that work, and `port/game/common/lib/snd.c` (the
library replaced at its API) calls it. OpenGOAL did the same for Jak and
Daxter's build of the library, and this engine follows its behaviour
(https://github.com/open-goal/jak-project, `game/sound/989snd`; ISC License,
Copyright (c) 2020-2026 OpenGOAL Team), with
[ReRAC](https://github.com/re-rac/rerac)'s notes on RAC1's banks and voices
(ISC License, Copyright (c) 2026 ReRAC contributors). It is not an
emulation of the program: nothing runs the library's code or models the
processors' memory or registers.

| File | |
|---|---|
| `sound_player.h` | banks loaded, sounds played: each sound's grain script run at 240 Hz (tones, loops, registers, random choices, pitch bends, LFOs), tones allocated onto voices by priority, instance limits, master volume groups, pausing; the volume and pan curves computed, not copied |
| `spu.h` | the voices: ADPCM decoded at the voice's address, Gaussian interpolation at its pitch, the envelope, left and right volume; 48 voices mixed at 48 kHz, streams queued part after part |
| `envelope.h` | the ADSR envelope |
| `pitch.h` | notes to pitch words (the note table computed), pitch bend |
| `reverb.h` | a native stereo reverb with the character of each of the library's effect types (measured results, not the library's tables) |

The games drive it through `port/game/common/lib/sound_engine.cpp`: `snd.c`
loads banks and sets volumes, each game's command flush passes the sounds the
game batched (play, set parameters), and the game window's audio output pulls
`SoundPlayer::render`. Music and speech (VAG streams) are played there too:
each track read from the disc and decoded on its own thread, a loop queued
behind its intro, mixed after the sounds. Not yet: streams through the
voices' own streaming (`spu.h`), reverb on streams, movie sound through this
engine.

## Tests

`tests/audio/`: banks built by the test: volume and pan, a tone keyed on a
voice and released, register and skipping grains, instance limits, pausing,
group volumes, unloading.
