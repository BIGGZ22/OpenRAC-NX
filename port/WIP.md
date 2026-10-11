# Work in progress: ReRAC converted to C++

Paused on 2026-10-09. These parts of the port are being converted from
[ReRAC](https://github.com/re-rac/rerac) (ISC License, Copyright (c) 2026
ReRAC contributors) into OpenRAC's C++, re-organised for all four games. They
were written by six parallel agents that were stopped part-way: **nothing
here is reviewed, built or tested yet**, and the build leaves it out unless
`-DOPENRAC_PORT_WIP=ON`. [WIP_BRIEF.md](WIP_BRIEF.md) is the brief every
agent worked to (rules, layout, legal constraints).

| Part | Directory | Converted from | Where it stopped |
|---|---|---|---|
| Disc | `assets/disc/`, `tests/disc/` | rc-formats iso9660, toc, disc, wad, level, overlays, volumes, sha1, strings, scenes, saves | **done**: reviewed, tested, built by default |
| Extractor | `extract/`, `tests/extract/` | rc-extract (identify, extract, prepare) | **done**: `openrac-extractor`, built by default; the launcher still runs tools/extractor.py |
| Geometry | `assets/geometry/`, `tests/geometry/` | rc-formats texture, vif, tfrag, tie, shrub, sky, moby, moby_anim, moby_light, moby_collision, moby_shadow, gadget, lighting | **done**: reviewed, tested, built by default |
| World data | `assets/world/`, `tests/assets_world/` | rc-formats collision, occlusion, cameras, gameplay, font, hud, pif, spaceships | **done**: reviewed, tested, built by default; water and sea left to the decompiled level code |
| Sound, movies, audio | `assets/sound/`, `media/`, `audio/`, their tests | rc-formats sound_bank, vag, pss; rc-video (MPEG-2); the voices and reverb; a 989snd player after OpenGOAL's | **done**: reviewed, tested, built by default; VAG stream players and the snd.c hook-up still to do |
| World renderers | `renderer/world/`, `renderer/shaders/world/` | rc-engine renderers and its 23 WGSL shaders, to GLSL 4.1 | **tfrag and ties done**: tested (`world_lod`, `world_gl_smoke`), built by default; shrubs, mobys, sky, particles, water and HUD have draw data but no renderer yet |
| Engine docs | `../docs/engine/formats/`, `../docs/engine/systems/` | ReRAC docs/formats and docs/plan | part-way; docs/engine/README.md not updated |

To continue: finish each part against WIP_BRIEF.md, build it with
`-DOPENRAC_PORT_WIP=ON`, review it (legal: no game bytes, no personal names),
test it, then add it to `OPENRAC_PORT_READY` in `cmake/Assets.cmake`. After that: bridge assets/geometry to
the renderers' draw data, drive the viewer from the disc, wire
game/common/lib/snd.c to openrac_audio, switch the launcher to the C++
extractor, credit ReRAC in THIRD_PARTY_NOTICES.md for each finished part.
