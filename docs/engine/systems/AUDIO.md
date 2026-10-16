# Audio: the sound layer, music and reverb

How RAC1 plays sound: a game layer of 30 logical sound slots updated every
frame (distance, pan, Doppler, occlusion), batched commands to 989snd on the
IOP, a music state machine of start and loop streams with stingers, sound
instances in the level, and reverb zones. ReRAC's reading of the NTSC-U code
(`SCUS_971.99`, mostly the level 01 program); the data formats are in
[SOUND_BANKS.md](../formats/SOUND_BANKS.md). For OpenRAC's port the RPC layer
is not needed: what matters is what the game asks 989snd to do
([docs/port/DESIGN.md](../../port/DESIGN.md) replaces 989snd at its API).

**Games.** RAC1. RAC2's boot carries 989snd identifiers, and five RAC1
`audio` functions are byte-identical in RAC2
([README.md, section 6](../README.md#6-other-subsystems)); the game layer
is not measured in the sequels.

## 1. Functions (NTSC-U and PAL)

| Role | NTSC-U | PAL |
|---|---|---|
| Queue a command to the IOP | boot 0x12E6E0 | `snd_SendIOPCommandNoWait` `func_0012E820` |
| Flush the commands, each frame | boot 0x12DC80 | `snd_FlushSoundCommands` `func_0012DDC0` |
| Load a bank | boot 0x12E088 | `snd_BankLoadFromEE_CB` `func_0012E1C8` |
| Doppler pitch | boot 0x12F1A0 | `snd_GetDopplerPitchMod` `func_0012F2E0` |
| Allocate a slot | boot 0x22D7F0 | `SoundSlotAlloc` `func_0022EB08` |
| Play a class sound of a moby | boot 0x22DA68 (level 01: 0x2A1618) | `PlayClassSound` `func_0022ED80` |
| Play by class number | level 01 0x2A16C0 | `PlayClassSoundByClass` `func_L00_0028EF68` |
| Play a level sound at a moby | level 01 0x2A1770 | `func_0022EE28` |
| Footsteps | level 01 0x2A1898 | `PlayFootstepSound` `func_L00_0028F140` |
| Per frame | level 01 0x2A0638 | `sound_update` `func_0022DD68` |
| Distance volume | level 01 0x2A02E0 | `func_0022DA10` |
| Pan | level 01 0x2A0418 | `SoundComputePan` `func_0022DB48` |
| Master volumes | level 01 0x2A04B8 | `SoundMasterVolumeInit` `func_0022DBE8` |
| Stop all | level 01 0x2A1B08 | `sound_StopAllSounds` `func_0022EFE8` |
| Moby loop sound refresh | boot 0x20C940 | `update_moby_voice` `func_0020D790` |
| Sound instances | level 01 0x2A19A8 | `SoundInstanceUpdate` `func_L00_0028F230` |
| Music: reset, start, body, transition, update | level 01 0x2796F8, 0x279FA8, 0x27A080, 0x27A168, 0x27A688 | `reset_music` `func_002161E0`, `music_StartTrack` `func_00216A90`, `music_StartTrackBody` `func_00216B68`, `music_Transition` `func_00216C50`, `music_Update` `func_00217130` |
| Music request | level 01 0x27A248 | `MusicRequestTrack` `func_L00_002664B0` |
| Nearest environment sample point | level 01 0x264E98 | `EnvNearestSamplePoint` `func_L00_002510F0` |

## 2. Commands to the IOP

Batched and flushed once per frame. Ids: 0x09 master volume (group,
volume), 0x10 auto reverb, 0x11 play (bank, sound, volume, pan, pitch mod,
pitch bend, callback), 0x15 stop, 0x19 still-playing query, 0x21 set
parameters (mask: 1 volume, 2/4 pan, 8 pitch mod, 0x10 pitch bend), 0x2A
init VAG streaming, 0x2C play a VAG stream by disc location, 0x4E group voice
range, 0x50 reverb (core, type, depth, delay, feedback).

## 3. Sound slots

30 slots of 0x70 bytes (NTSC-U 0x13E5C0): handle, state (0 free, 7 start
pending, 1 playing, 4 release requested, 6 stopping), play flags, definition,
bank id, class-sound index, volume scale (0x400 = 1), pitch bend, owner moby,
owner sound instance, position, local offset, and a 36-entry occlusion ring.

- **Allocation**: first free of slots 0–25; 0–29 for Ratchet and two other
  privileged owners. Refused when the loop flag differs from the definition's
  or the starting volume is below 0x20. Pitch bend = lo + rand % (hi − lo).
- **Play flags**: 0x01 2-D (no pan, no Doppler), 0x04 loop, 0x08 do not follow
  the owner, 0x10 fixed volume, 0x20 no Doppler, 0x40 follow the owner at a
  rotated local offset. No owner and no position: 2-D and fixed, at the
  listener.

## 4. Per frame (`sound_update`)

In order: reverb commands, listener, master volumes, each slot's state, each
slot's occlusion, send, `music_Update`, flush. **The listener is the camera**,
not the hero.

- **Listener velocity**: the average of the last up to three camera
  movements, ignoring a jump of 60 units per frame or more (teleport).
- **Owner tracking**: position = the owner's position with z + 1; velocity =
  change since last frame. A deleted owner is dropped and its loops stop.
- **Distance volume**: near → volume at near, far → volume at far, linear (or
  squared with definition bit 0) between; × the slot's scale >> 10. Below
  0x20, loops stop and one-shots go on.
- **Pan**: the source in camera space, angle of its (x, y), faded to the
  centre within 1 unit.
- **Doppler**: dot of the unit vector from source to camera with the
  velocity difference, `(trunc(dot × 300) × 0x5F4) / 0x2E5`; approaching
  raises the pitch.
- **Underwater**: pitch −0x5F4 (about an octave) unless definition bit 3;
  volume halved for sources above the water level unless bit 2; music × 3/5.
- **Occlusion** (unless definition bit 1): 36 segment queries from jittered
  points around the camera to 0.75 of the way to the source; a new sound
  fills all 36 at once, a running one retests one entry every 2nd (loops) or
  4th frame (one-shots). With n blocked: n ≥ 36 silent, n > 18 volume × (36 −
  n) / 18. The queries are `CollLine_Fix` ([COLLISION_QUERIES.md](COLLISION_QUERIES.md)).
- **Master groups**: 0 sfx × 8/10, 1 music, 2 sfx × 8/10, 3 and 4 sfx × 7/10, 5
  sfx; in a cutscene music is 0 and groups 0 and 3 are halved. Groups 1, 2, 4
  use SPU voices 0x18–0x2F. Bank sounds use groups 0 and 4; streams 1 (music)
  and 2 (dialogue).

989snd then scales per tone and pans through its 181-entry constant-power
table; its grain machine ticks at 240 Hz; SPU2 has 48 voices. Its voice
stealing policy is not reversed.

## 5. What triggers sounds

| Path | Data |
|---|---|
| Class code calls `PlayClassSound` | the class's sound definitions |
| Hero code: voices, loops, item sounds | Ratchet's class definitions, gadget definitions |
| **Animation triggers**: the per-tick advance plays a trigger whose time was crossed | sequence trigger words (low half class sound, high half time in 1/16 ticks) |
| **Sequence loop sounds**: engines, jetpacks, hums | sequence header +0x11 → moby +0x7C, slot in +0x7D |
| Hit sounds | the victim through its hit-reaction sequence's triggers; the attacker its own |
| **Footsteps**: Ratchet's walk and run at fixed key times, and landings | level definitions by the ground's footstep class |
| Level sounds at a moby | definitions 0 (help box) and 1 (skill point) |
| Sound instances | gameplay section 0x0C |
| Music and speech | the level's music table and scene records |

Ratchet's walk and run sequences carry no triggers: footsteps come only from
the footstep path.

## 6. Sound instances

Gameplay section 0x0C, updated through a `{class, function}` table with pvars
`{definition, min, max, timer, slot}`; the retrigger time is
`round((min + rand % (max − min)) × scale) × 60` ticks, 0/0 replaying as soon as
the last play ends.

| Class | Behaviour | PAL |
|---|---|---|
| 0 | sphere: plays within range, released beyond range + 1 | `SndInstSphereUpdate` `func_L00_002EEE00` |
| 1 | box with volume by depth | `SndInstBoxVolumeUpdate` `func_L01_0031AD00` |
| 2 | box one-shot | `SndInstBoxOneShotUpdate` `func_L00_002EEF88` |
| 3 | reverb box (section 8) | `SndInstReverbBoxUpdate` `func_L01_0031B2F0` |
| 5 | underwater loop at the camera | `SndInstUnderwaterLoopUpdate` `func_L01_0031B450` |
| 6 | music box (section 7) | `SndInstMusicBoxUpdate` `func_L01_0031B500` |

## 7. Music

- Tracks are the level header's 15 VAG sectors and come in pairs, Start and
  Loop. Starting track k streams k once and queues k + 1 behind it, looping,
  so the hand-over is seamless.
- Three players: main, transition (stingers), dialogue. States run from idle
  through requested, started, buffered, playing, stopping and ended, plus
  "queue the body".
- A change is requested and acted on when a timer runs out (420 ticks): the
  stinger plays on the transition player; the main track fades linearly over
  the stinger's first quarter, the next track is pre-seeked paused, and it
  continues when the stinger has a quarter left.
- At level start, track 0 plays at volume 0x400; the environment sample point
  nearest the hero sets the track when the hero is placed. Music boxes test
  the hero's position against the box and request a track on leaving
  (by side).
- Dialogue: the scene record's speech VAG for the current language
  (0x13A664 + id × 0x250 + language × 4, NTSC-U), group 2.
- A movie stops all sounds and the music; leaving it restarts the level
  track.

## 8. Reverb

- **Reverb boxes** (sound instance class 3) test the hero's position: inside,
  the depth rises with x through the box; leaving through +x keeps the full
  depth, through −x turns reverb off. They come in pairs at cave mouths.
- **Environment sample points** (within 8 units, nearest) set type, depth,
  delay and feedback when the hero is placed (level start, vendor exit,
  ship, camera triggers).
- The request is sent at the head of `sound_update` as `SetReverbEx(core 2,
  …)` or an auto-reverb glide; movies and level unloads switch it off; the
  checkpoint saves and restores it.
- Types on the disc (98 boxes, 15 points): room 3, studio A 19, studio B 49,
  studio C 18, pipe 9. Level-bank tones are routed to reverb; global tones and
  streams are dry. libsd's exact reverb is not reversed.

## Open

- 989snd's voice allocation and stealing, stream time units, SPU2 reverb.
- Play flag 0x08's full meaning.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/plan/audio.md`; PAL names from
`games/rac1/config/overlays/us_map.tsv`.
