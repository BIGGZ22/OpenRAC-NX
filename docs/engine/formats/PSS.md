# PSS movies

RAC1's full-motion video: 88 slots in the table of contents
(`mpegs`, +0x17F8, exact byte sizes; [DISC.md](DISC.md#4-the-table-of-contents-lba-1500)),
84 of them used. A PSS file is an MPEG-2 program stream whose audio is SPU
ADPCM in private stream 1. ReRAC surveyed every header of the 84 files of the
NTSC-U disc (98,629 pictures, 3,587 seconds) and decodes all of them with a
decoder of its own, checked against an independent MPEG-2 decoder. Which game
code plays which slot: [CUTSCENES.md](../systems/CUTSCENES.md#5-movies).

The movie player in the executable is to be decompiled from the retail
assembly alone; [MOVIE.md](../../../games/rac1/docs/MOVIE.md) gives the
rules. This page describes the files, not the player's code.

**Games.** RAC1 (NTSC-U surveyed; the PAL disc's copies are among them).
The sequels' movies are in their global MPEG WADs (Wrench); not measured.

## 1. Slots

Empty on NTSC-U: 2, 21, 74, 79. In-level movie n is slot 2 + n (NTSC) or 21 + n
(PAL); planet transitions 40–50 (NTSC) and 52–62 (PAL); extras 70–74 and
75–79; title attract 80–83 and 84–87. The roles of 0, 1 and 64–69 are not
known.

## 2. Program stream

| Unit | Content |
|---|---|
| `00 00 01 BA` pack header | MPEG-2 form; packs fill 2 KB sectors |
| `00 00 01 BB` system header | first pack only |
| `00 00 01 E0` video PES | MPEG-2 PES header (PTS, or PTS and DTS on pictures) |
| `00 00 01 BD` private stream 1 | PES header, then a 4-byte sub-stream header `FF A1 00 cc` (cc = audio channel), then audio |
| `00 00 01 BE` padding | |
| `00 00 01 B9` end | zero padding to the sector end |

**Audio channels are languages.** The game passes its language to the
player, which keeps the matching channel. Files carry channel 0 only (slots 0,
1, 70–87) or channels 0, 2, 3, 4, 5 (English, French, German, Spanish,
Italian; 3–69). Where a language is missing the game would get no audio;
ReRAC falls back to channel 0.

## 3. Audio

Each channel's bytes, concatenated in file order, start with a 0x28-byte
header:

| Offset | Content | On the disc |
|---|---|---|
| 0x00 | `SShd` | |
| 0x04 | header size | 0x18 |
| 0x08 | type | 0x10, SPU ADPCM |
| 0x0C | sample rate | 48,000; 44,100 in slots 0, 1, 45, 57 |
| 0x10 | channels | 2 |
| 0x14 | interleave | 0x20: two 16-byte ADPCM frames per channel block |
| 0x18, 0x1C | loop start, end | −1, −1 |
| 0x20 | `SSbd` | |
| 0x24 | body size | exactly the bytes that follow |

The body alternates left and right blocks of `interleave` bytes. Each channel
is one continuous SPU ADPCM stream (28 samples per 16-byte frame, the format
in [SOUND_BANKS.md](SOUND_BANKS.md)), decoded from zero history. Audio and
video durations agree within 0.3 s.

## 4. Video

| Field | Values |
|---|---|
| Size | 512 × 416 in every file, PAL copies included |
| Profile, level | Main Profile at Main Level, 4:2:0 |
| Frame rate | 30 fps (NTSC in-level, transitions, attract), 25 fps (PAL copies), 29.97 fps (extras 70–73) |
| Aspect code | 1 or 2; the display ignores it |
| Pictures | frame pictures only; I, P, B; every stream starts with a closed GOP |
| Intra DC precision | 8, 9 or 10 bits |
| Progressive | 82 files: progressive sequence, frame DCT and prediction, zig-zag |
| Interlaced | slots 73 and 78: per-macroblock frame or field prediction and DCT, alternate scan, top field second; no dual prime |

So a decoder needs exactly: Annex B VLC tables, intra DC prediction, both
dequantisations with saturation and mismatch control, frame and field motion
compensation in frame pictures, half-sample interpolation, skipped
macroblocks, display reordering, and an IEEE 1180 IDCT. Field pictures, dual
prime, 4:2:2 and concealment vectors never occur.

**Colour**: the console's IPU converts limited-range BT.601 YCbCr to
full-range RGB with each chroma sample covering 2 × 2 pixels; ReRAC uses the
standard BT.601 matrix in fixed point (the IPU's own constants are not
reproduced).

## Open

- The roles of slots 0, 1 and 64–69.
- RAC2–RAC4: not measured.

Sources: ReRAC (https://github.com/re-rac/rerac, ISC, Copyright (c) 2026
ReRAC contributors), `docs/formats/pss.md`,
`docs/plan/cutscenes_transitions.md` section 5.
