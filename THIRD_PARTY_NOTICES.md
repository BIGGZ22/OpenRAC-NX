# Third-party notices

Third-party code and data used by OpenRAC's shared components. Each game
directory lists its own in its `THIRD_PARTY_NOTICES.md` and source comments:

- [games/rac1/pal/THIRD_PARTY_NOTICES.md](games/rac1/pal/THIRD_PARTY_NOTICES.md):
  functions adapted from Lombyte, ReRAC's notes in `config/overlays/rerac_notes.tsv`.
- [games/rac1/ntsc/THIRD_PARTY_NOTICES.md](games/rac1/ntsc/THIRD_PARTY_NOTICES.md):
  libgcc and soft-float, newlib, David Gay's dtoa, code ported from rac1-decomp,
  the patched EE-GCC.
- rac2 and rac3 credit their sources in their docs
  ([games/rac2/ntsc/docs/SECOND-C-LOT.md](games/rac2/ntsc/docs/SECOND-C-LOT.md),
  [games/rac2/ntsc/docs/COMPILER-NOTES.md](games/rac2/ntsc/docs/COMPILER-NOTES.md),
  [games/rac3/ntsc/docs/wiki/Cross-Repository-Resources.md](games/rac3/ntsc/docs/wiki/Cross-Repository-Resources.md)).

`port/audio/sound_player.h` follows the behaviour of OpenGOAL's 989snd
(`game/sound/989snd`; ISC License, Copyright (c) 2020-2026 OpenGOAL Team),
as do the grain type names in `port/assets/sound/sound_bank.h`.

[docs/port/OPENGOAL_NOTES.md](docs/port/OPENGOAL_NOTES.md) describes how
[OpenGOAL](https://github.com/open-goal/jak-project) (ISC, Copyright (c)
2020-2026 OpenGOAL Team) works, and the launcher's disc set-up and
`tools/extractor.py` follow how OpenGOAL's launcher and extractor behave
(their steps, folder layout and error numbers); no code from OpenGOAL is in
OpenRAC. The launcher lists its own notices in
[launcher/THIRD_PARTY_NOTICES.md](launcher/THIRD_PARTY_NOTICES.md).

## ReRAC

[ReRAC](https://github.com/re-rac/rerac), a native PC port of the US build,
documents the game's formats and systems. The following adapt its format code
and notes:

- `editor/mobys.py`: the moby instance record and what the level loader does
  with each field, from `crates/rc-formats/src/gameplay.rs` and
  `docs/plan/moby_render_notes.md`; described in [ASSETS.md](games/rac1/pal/docs/ASSETS.md) ("Mobys").
- `editor/moby_class.py`: the moby class header and model format (packets,
  vertex cache, skinning slots, normals, untextured faces), from
  `docs/formats/moby_rac1.md` §1-§2 and `crates/rc-formats/src/moby.rs`;
  described in [ASSETS.md](games/rac1/pal/docs/ASSETS.md) ("Moby classes").
- `editor/moby_anim.py`: moby skeletons and animation sequences, from
  `docs/formats/moby_rac1.md` §3-§4, `docs/plan/moby_animation.md` and
  `crates/rc-formats/src/moby_anim.rs`.
- `editor/collision.py`: the collision block (cell tree, packed vertices,
  faces and surface bytes), from `docs/formats/collision_rac1.md`,
  `docs/plan/collision_queries.md` and `crates/rc-formats/src/collision.rs`;
  described in [ASSETS.md](games/rac1/pal/docs/ASSETS.md) ("Collision").
- `editor/lighting.py`: the terrain and tie lighting passes (`LightTfrags`,
  `LightTies`) and the PS2 float arithmetic they run on, from
  `crates/rc-formats/src/tfrag_light.rs` and `tie_light.rs`; their checks from
  ReRAC's unit tests. The tie light slots in `editor/ties.py` after
  `crates/rc-formats/src/tie.rs`.
- `port/assets/disc/`: the disc readers (ISO 9660, table of contents, WAD
  compression, level data, overlays, scenes, saves, messages, volumes, the
  front end), converted to C++ from `crates/rc-formats` and its specs in
  `docs/formats`; each file names its source.
- `port/assets/geometry/`: the readers of what the renderer draws (textures,
  terrain, ties, shrubs, the sky, moby classes and animation, collision and
  shadow blocks, gadgets) and the lighting passes, converted to C++ from
  `crates/rc-formats` and its specs; their tests from ReRAC's unit tests.
- `port/assets/world/`: the gameplay file, collision and its queries, occlusion,
  cameras, fonts, the HUD and PIF pictures, built on `crates/rc-formats` and
  ReRAC's specs; `port/assets/ps2_float.h` and `image.h`, shared by the parts.
- `port/assets/sound/`, `port/media/`, `port/audio/`: sound banks, VAG, PSS and
  ADPCM from `crates/rc-formats`; the MPEG-2 decoder from `crates/rc-video`;
  the voices, envelope and reverb from `crates/rc-game/src/audio`.
- `port/extract/`: `openrac-extractor`'s JSON-lines output, crash-safe writes,
  free-space check and decompile step, after `crates/rc-extract`.

ISC License

Copyright (c) 2026 ReRAC contributors

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

## Lombyte

[Lombyte](https://github.com/lombyte-project/Lombyte), the US decompilation, now
`games/rac1/ntsc`:

- `editor/moby_classes.tsv`: the moby class names the editor shows, from its
  `config/overlays/us/names/level-NN.json` ("moby-class-record" evidence),
  which joins each level's class dispatch table with the class names of
  [Wrench](https://github.com/chaoticgd/wrench)'s moby class unpack.

MIT License

Copyright (c) 2026 Mateusz Kłysz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Wrench and Replanetizer

Consulted for what formats and fields mean; no code from them is included
([editor/README.md](editor/README.md#credits)):
[Wrench](https://github.com/chaoticgd/wrench) (GPL-3.0-or-later) and
[Replanetizer](https://github.com/RatchetModding/Replanetizer).

## The native port's libraries

The port ([port/](port/README.md)) builds with these libraries. CMake fetches
each at a pinned version ([port/cmake/Dependencies.cmake](port/cmake/Dependencies.cmake));
none is copied into OpenRAC.

| Library | Version | License | Used by |
|---|---|---|---|
| [SDL](https://github.com/libsdl-org/SDL) | 3.2.24 | zlib | the platform layer: window, OpenGL context, input, audio |
| [cgltf](https://github.com/jkuhlmann/cgltf) | 1.15 | MIT | the level viewer: reading the editor's glTF meshes |
| [stb_image](https://github.com/nothings/stb) | commit 2c980bb | MIT or public domain | the level viewer: reading textures (PNG) |
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.91.9b | MIT | the games' debug overlay ([docs/port/DEBUGGING.md](docs/port/DEBUGGING.md)) |

The renderer's conventions (colour 0x80 as 1.0, reversed depth, the double
draw for alpha-test fail modes, textures identified by the game's base
pointer) follow OpenGOAL's renderer as
[docs/port/OPENGOAL_NOTES.md](docs/port/OPENGOAL_NOTES.md) describes it; no
code from OpenGOAL or from any emulator is in the port.

