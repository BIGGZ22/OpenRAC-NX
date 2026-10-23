# assets/world

What a level holds besides its geometry: library `openrac_assets_world`,
namespace `openrac::assets`. Every reader takes the `Game` and refuses the
games whose layouts are not known yet (`known_games.h`: RAC1 only today).
Built on [ReRAC](https://github.com/re-rac/rerac)'s research and `rc-formats`
(ISC License, Copyright (c) 2026 ReRAC contributors); each file names what it
draws on.

| File | |
|---|---|
| `gameplay.h` | the gameplay file: its 37 sections, level settings (background, fog, the ship), moby instances, class lists, pvar blocks with the loader's moby links and shared data, environment transitions, point lights. The volumes and paths are [disc/volumes.h](../disc/volumes.h)'s |
| `collision.h`, `collision_query.h` | the level's collision mesh, and the queries the game runs on it (rays, spheres, capsules), in the PS2's arithmetic ([common/ps2_float.h](../../common/ps2_float.h)) or the host's |
| `occlusion.h` | precomputed visibility: the grid of 4-unit cells and their masks, the octant override, the gameplay mappings and how each terrain fragment, tie and moby resolves to its bit |
| `cameras.h` | the level's camera records and their parameter blocks |
| `font.h` | the three bitmap fonts' glyph tables, found in the level program by the call sites that print text |
| `hud.h` | the HUD's icons, frames, palettes and textures across its five banks |
| `pif.h`, `spaceships.h` | PIF pictures; the player's ship files |

Images decode through [../image.h](../image.h), shared with the geometry.

Not here, by design: game behaviour, which the decompilation supplies in the
port (which mobys the loader creates, which ship a level shows, how water
and the sea animate). The level programs' own tables for those (the water
strips and ripples, the sea) are read by the decompiled level code itself.

## Tests

`tests/assets_world/`: the collision queries, and invented data for the
gameplay file, cameras, occlusion, fonts, PIF pictures, ship files and the
HUD.
