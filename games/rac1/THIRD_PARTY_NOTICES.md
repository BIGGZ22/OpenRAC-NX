# Third-party notices

## Lombyte

The following functions adapt source from
[Lombyte](https://github.com/mateuszklysz/Lombyte) for the PAL executable:

- `src/overlays/l04_eudora/vuchain_00293490.c`: `func_L04_00293578` (`src/overlays/l04/runtime/dma/002922d0.c`, `FUN_L04_002923b8`)
- `src/overlays/l06_blarg/vendor_002B5990.c`: `func_L06_002FD3E0` (`src/overlays/l06/gameplay/entities/002b4770.c`, `FUN_L06_002fbfb0`)
- `src/overlays/l06_blarg/vendor_002FE5D0.c`: `func_L06_003045A0` (`src/overlays/l06/gameplay/entities/002fd1a0.c`, `FUN_L06_00303160`)
- `src/overlays/l07_umbris/vendor_00313D28.c`: `func_L07_00318CE0` (`src/overlays/l07/gameplay/entities/00312948.c`, `FUN_L07_00317910`)
- `src/overlays/l08_batalia/vendor_002B9438.c`: `func_L08_002D68D0` (`src/overlays/l08/gameplay/entities/002b8228.c`, `FUN_L08_002d55c8`)
- `src/overlays/l08_batalia/vendor_002B9438.c`: `func_L08_002DDD20` (`src/overlays/l08/gameplay/entities/002b8228.c`, `FUN_L08_002dc9a8`)
- `src/overlays/l08_batalia/vendor_002EAF48.c`: `func_L08_002F5980` (`src/overlays/l08/gameplay/entities/002e9b70.c`, `FUN_L08_002f44c0`)
- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_00304750` (`src/overlays/l12/gameplay/vendor/003028c8.c`, `FUN_L12_00303370`)
- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_003061E0` (`src/overlays/l12/gameplay/vendor/003028c8.c`, `FUN_L12_00304e00`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002A8AA8` (`src/overlays/l15/gameplay/entities/0029aff0.c`, `FUN_L15_002a7880`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002D96B0` (`src/overlays/l18/gameplay/entities/002a7220.c`, `FUN_L18_002d82c0`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FB318` (`src/overlays/l18/gameplay/vendor/002f88e8.c`, `FUN_L18_002f9eb8`)
- `src/overlays/shared/mobyfunc_002649C8.c`: `func_L01_002649D8` (`src/overlays/shared/gameplay/state/00263a28.c`, `FUN_L01_00263a28`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025A8E8` (`src/overlays/shared/gameplay/entities/00259710.c`, `FUN_L00_002598b0`)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_002629E0` (`src/overlays/shared/gameplay/state/00261968.c`, `FUN_L00_00261968`)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_002630A8` (`src/overlays/shared/gameplay/state/00261968.c`, `FUN_L00_00262030`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002DFA70` (`src/overlays/shared/gameplay/entities/002b17d8.c`, `FUN_L14_002de670`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002E1110` (`src/overlays/shared/gameplay/entities/002b17d8.c`, `FUN_L14_002dfd10`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002BA380` (`src/overlays/shared/gameplay/entities/002b8c08.c`, `FUN_L01_002b91c8`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002EF078` (`src/overlays/shared/gameplay/entities/002b8c08.c`, `FUN_L01_002edca0`)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C39D0` (`src/overlays/shared/gameplay/entities/002bffa8.c`, `FUN_L00_002c26c8`)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002CD110` (`src/overlays/shared/gameplay/entities/002c8440.c`, `FUN_L00_002cbcc0`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D7298` (`src/overlays/shared/gameplay/entities/002cfcb8.c`, `FUN_L00_002d5de8`)
- `src/overlays/l05_rilgar/vendor_002D28D0.c`: `func_L05_003054B0` (path collision and nearest wall distance; adapts this project's matched `func_L00_00262DF0`, based on Lombyte's `src/overlays/shared/gameplay/state/00261968.c`, `FUN_L00_00261d78`, MIT; the distance tracking and return follow the PAL assembly)
- `src/overlays/shared/mobyfunc_0024FD50.c`: `func_L00_00250478` (animation frame packet builder; `overlays/shared/gameplay/state/0024eec0.c`, `FUN_L00_0024f440`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E72E8` (follow camera approach; `overlays/shared/gameplay/vendor/002e5e38.c`, `follow_camera_approach` / `FUN_L00_002e5e38`)
- `src/core/00119328.c`: `func_001194C8` (`topThread`)
- `src/game/stream.c`: `func_00217AE8` (menu entry transitions and message formatting; `src/ui/menus/fun_00216c48.c`)
- `src/core/00119D88.c`: `func_0011C208` (`sceClose`)
- `src/core/00119868.c`: `func_00119CC8` (`sceTtyInit`)
- `src/game/hud.c`: `func_00201190` (HUD sprite with explicit UV corners)
- `src/game/vendor.c`: `func_00239A00` (vendor item carousel)
- `src/game/lights.c`: `func_002027C0` (detach point light)
- `src/core/0011D0D0.c`: `func_0011D248` (`sceSifRebootIop`)
- `src/game/menu.c`: `func_00208338` (read sector, track size)
- `src/game/loaders.c`: `func_00203038` (unpack point records)
- `src/core/00112380.c`: `func_001123A8` (`_calloc_r`)
- `src/core/00119D88.c`: `func_0011B2F8` (`sceSifBindRpc`)
- `src/core/00119D88.c`: `func_0011AE20` (`sceSifInitRpc`)
- `src/core/00121750.c`: `func_00121B78` (`sceGsResetGraph`)
- `src/core/00123168.c`: `func_001233E8` (`sceDmaPutEnv`)
- `src/core/00125630.c`: `func_001273A0` (`_waitBdecOut`)
- `src/core/00125630.c`: `func_001286E8` (`_ipuVdec`)
- `src/core/00125630.c`: `func_001299E8` (`_cpr8`)
- `src/core/0012AC80.c`: `func_0012C0A0` (`_decodeOrSkipField`)
- `src/core/00113B70.c`: `func_00113B70` (`_free_r`)
- `src/game/memcard.c`: `func_0020BBC8` (memcard_PrepData)
- `src/game/memcard.c`: `func_00209E68` (memory-card state update; `src/storage/memory_card/memcard_update_state.c`, with PAL directory validation and request handling reconstructed from retail assembly)
- `src/game/map.c`: `func_00205E70` (map overlay renderer; `src/assembly/textbin/ui/map/draw_map_overlay.c`, with PAL control flow, addressing and mixed argument ordering reconstructed from retail assembly)
- `src/core/0011CCE0.c`: `func_0011CE70` (`_sceSifLoadModuleBuffer`)
- `src/game/loaders.c`: `func_00205220` (ParseSpaceSceneChunk)
- `src/game/draw.c`: `func_001F5368` (screen stripe fill)
- `src/core/00114060.c`: `func_00114060` (`__sfvwrite`)
- `src/game/memcard.c`: `func_0020BD70` (memcard_RestoreData)
- `src/core/0012AC80.c`: `func_0012C990` (`_setDefaultQM`)
- `src/core/00125630.c`: `func_00128F90` (`_pictureCodingExtension`)
- `src/core/989snd.c`: `func_0012DDC0` (snd_FlushSoundCommands)
- `src/game/loaders.c`: `func_00204FC0` (update_world_object_animation)
- `src/core/989snd.c`: `func_0012E688` (snd_SendIOPCommandAndWait)
- `src/core/00125630.c`: `func_00128860` (`_peepBit`)
- `src/core/0011D0D0.c`: `func_0011D0D0` (`sceSifResetIop`)
- `src/core/0012AC80.c`: `func_0012BF40` (`_decodeOrSkipFrame`)
- `src/core/001236F0.c`: `func_00123D48` (sceMcWrite)
- `src/core/boot.c`: `func_0012DA38` (parse_bin)
- `src/game/pause.c`: `func_002224A8` (draws an options menu: each item's label at left, its current value's name at right, selected one highlighted)
- `src/game/framebuf.c`: `func_001FB908` (clears the screen through a GIF packet appended to D_00161000: a fixed header then n = w / 32 pairs of sprite corner registers stepping 0x200 per column across a w x h area centred on 0x8000)
- `src/game/space.c`: `func_002308C8` (spawns six particles around a moby: random velocity and a position taken from the flare corner table, transformed by the moby's matrices)
- `src/game/space.c`: `func_00230A90` (resident gameplay and cinematic state update; `src/gameplay/update_resident_gameplay_state.c`)
- `src/core/0012CC90.c`: `func_0012D068` (sceIpuInit)
- `src/game/loaders.c`: `func_002032D0` (load_hud_banks)
- `src/game/pause.c`: `func_0021D4C0` (pause slot-select tick)
- `src/game/draw.c`: `func_001F2608` (builds the view matrices in the camera block from the rotation set up in D_00187390 or the three rotation angles, then copies the result to D_0018D080)
- `src/game/lights.c`: `func_002023E0` (create_point_light)
- `src/game/initonce.c`: `func_00201E88` (init_once)
- `src/game/draw.c`: `func_001F2BC8` (build_occlusion_visibility)
- `src/game/pause.c`: `func_0021E4B0` (draws a pause-menu list (rows of text with optional subtext, cursor highlight, scrolling))
- `src/core/00125630.c`: `func_00127D40` (_decMB0)
- `src/game/vendor.c`: `func_00239CF8` (fun_002389e0)
- `src/core/0012AC80.c`: `func_0012B2C0` (_PES_packet)
- `src/game/camera.c`: `func_001ED080` (camera update toward a target orientation: blends yaw/pitch/distance and rebuilds the camera basis)
- `src/game/space.c`: `func_00233308` (do_space_transition)
- `src/game/help.c`: `func_001FE6C0` (update_help_state)
- `src/overlays/shared/stream_002670E0.c`: `func_L00_002676A0` (stores b in the d field of the slot func_L00_00267618 finds, if any)
- `src/overlays/shared/tieproc_00299108.c`: `func_L00_0029A868` (plays the movie of table entry i (set chosen by a flag))
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00276F18` (FUN_L00_00276078)
- `src/overlays/shared/hud_00235960.c`: `func_L00_00238478` (FUN_L00_00237ae8)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_00212D70` (builds the two camera-relative vectors a and b from offsets fa and fb, rotating about the up axis when mode is set)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002DD2D0` (frees a moby's state slot and returns 1 when it is in state 4)
- `src/overlays/shared/vendor_002BA7C8.c`: `func_L00_002BFF88` (probe below a point and return the ground height offset, or zero when nothing is hit)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002DCFD0` (place a moby at the given position under its owner, reset its owner's state to 4 and start the matching animation)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E5630` (computes two adjusted points from a moby's lookup entry and its direction vector; returns 1 on success)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020DF90` (finds the listed object nearest a point)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026A7F8` (spawn a particle of type 4 with position, velocity, color, lifetime and flag)
- `src/overlays/shared/vendor_002AB910.c`: `func_L00_002ADD78` (updates a decaying moby: spins down its position fields and fades it out)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C6608` (spawns a moby of a class and initialises its data block)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00264570` (spawns impact sparks at a point on the ground near the camera)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025B040` (records a shadow-like entry for the moby at its ground height, scaled by arg)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_0030D248` (spawn a debris moby on src, scaled, moving along dir with random spin and lifetime)
- `src/overlays/shared/hud_00235960.c`: `func_L00_0023B750` (shows or hides the weapon HUD element depending on the current weapon)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025AAC0` (records a timed effect entry for a moby unless a fresher one of its own is still live)
- `src/overlays/shared/vendor_002BA7C8.c`: `func_L00_002C0BB8` (updates a homing state block o from target m: turn rate, heading, and a speed clamped to seven times a global)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FB440` (spawns a beam moby (0x2D2) from owner between two points, colored, with random rotation and two glow sprites)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025BA50` (sends a message to every moby in a list except one, each carrying the angle from pos to it and a velocity along that angle)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002C96D0` (places a moby at a spot: copies the vector in, transforms it, and if the move is blocked pushes it back out along the stored normal)
- `src/overlays/shared/space_0028FB78.c`: `func_L00_00290030` (start a ship-travel transition to the given destination, setting up the flight path and timing)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002B6E90` (steers the held object's pose: eases a field toward 0.45, spins it, and rebuilds its matrix and position from the parent)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025CCF0` (spring a wrapped angle *P (rate *V) toward target X; returns the remaining angle difference)
- `src/overlays/shared/vendor_002AB910.c`: `func_L00_002AE110` (spawns a type-0x79 moby with random spin axes and speeds, owned by the given object, at pos)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026D3F8` (particle type 18 update: bounces its alpha ramp and fades, killing when spent)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_002767B0` (spawns a type 78 particle: position, colour, lifetime and mode, with a trailing record of speeds, id and target)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002F7C78` (spawns a beam moby from an owner, position, direction and colour vectors)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FAFC8` (oscillating spinner: swings its angle between -1 and 1 and turns the moby to match)
- `src/overlays/shared/update_0029B6A0.c`: `func_L00_0029BED8` (leaves the vendor screen: releases two handles, resets state, repositions the hero and restores the game state)
- `src/overlays/shared/vendor_002AB910.c`: `func_L00_002B0B98` (spawns a particle moby of the given type owned by owner, with random spin and size)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00272158` (spawns a type 0x2C particle with position, velocity, colour and scale parameters)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_002722F0` (per-frame update of a type 0x2C particle: fade colour, move and rotate its velocity)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026FA28` (part type 31 update: eases two trailing points toward its parent's position and dies when they converge)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_002732D8` (part type 54 update: advances a particle, fades its color near the end of life and spawns a light effect at its position)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002D9438` (update a trailing moby: advance its timer, delete it when done or its owner died, else follow the owner)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_00300F00` (smoke emitter: keeps its sound alive and spawns two puffs of particles per frame)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00273F80` (spawns an effect record at a position inside the level bounds;)
- `src/overlays/shared/pause_00277208.c`: `func_L00_00286128` (seeds the pause-menu snapshot: copies the header, marks valid entries in the bitmasks, fills the state record)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028A3E0` (steps each shrub sparkle entry: type 0 spins and scales a direction, type 1 randomises a colour)
- `src/overlays/shared/camera_001EB508.c`: `func_L00_001EDA28` (moves the camera position a toward a target in steps, stopping each step on collision)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00264BE8` (bursts two rings of sparks (and then three more) from two points with the given velocity)
- `src/overlays/shared/help_00203E98.c`: `func_L00_00209EC0` (every lim calls of the counter, sprays a dust particle at the player's position with a random velocity and resets the counter)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E1C38` (scrolls the four hue values of a moby's effect and spawns a coloured streak for each)
- `src/overlays/shared/help_00203E98.c`: `func_L00_00209CB0` (emits a pair of sparks from the player's position each time the counter passes its limit)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_00271F40` (updates a fading colour particle: interpolates its packed colour and alpha over its life and drifts it upward)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D6978` (walks a moby toward a target point: turn to face it, ease its speed, step it along its heading; returns the distance (or 37.0 when a flag is set))
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E4F38` (path-following moby update: start a run along its path in state 0, then step along the path each frame and pick an animation when the flag is set)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026DB50` (per-frame update of a particle that drifts by its velocity and fades; sparks off a child particle at random)
- `src/overlays/shared/vendor_002BA7C8.c`: `func_L00_002BFB98` (draws a textured quad twice (first tinted, then white) from a moby's orientation)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C8680` (aims a point near p toward q (at most range away), then fills out with a heading vector of length 8.5 * D_0015EE6C and a clamped height)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00263950` (eases a moby's per-axis offsets toward zero and applies them and its scale to the held object, resetting when settled)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00263680` (pushes the source object's transforms into an 8-entry ring and replays them into the trailing objects, ending them when all faded)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028A198` (fills the sky particle list: entries from a up are dim specks, the first a are coloured sparks with random direction and speed)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002FA998` (elevator update: waits, then moves the lift between its two stops and carries whatever rides it)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D8180` (draws four drifting, colour-cycling sprites around a moby, stepping each one's phase and fade timer)
- `src/overlays/shared/help_00221A98.c`: `func_L00_00222668` (picks the nearest eligible moby of class 0x323 near the player and stores it as the target)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002E78F0` (spawns three bursts of sparks around a moby with random velocity, colours and size)
- `src/overlays/shared/vendor_0029FD68.c`: `func_L00_002A0F60` (draws a screen-space rectangle from five joints of a moby: builds a box from its joint vectors, projects it and draws the rectangle)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C53A8` (steers a moby toward the player's follow distance, easing its height)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025ED30` (nearest point on the segment a-b to p (clamped), returned with the distance result)
- `src/overlays/shared/vendor_002A5138.c`: `func_L00_002AB2A8` (initialises a spawned moby: random spin vector, nearby-count cull of the oldest same-class moby, then a bounce off the floor probe)
- `src/overlays/shared/help_002297E0.c`: `func_L00_00229E30` (tries the crouch-jump style moves in priority order and starts the first whose conditions hold)
- `src/overlays/shared/vendor_002BA7C8.c`: `func_L00_002BC3B8` (builds and draws two layered glow quads around a moby)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020E100` (picks the hero's current animation id and speed from its state and starts or updates the animation slot)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_00300140` (sliding door update: opens when the player is near, closes when far, and offsets the door along its heading by the open amount)
- `src/overlays/shared/help_00214D60.c`: `func_L00_00216880` (builds the camera rotation matrix m that looks along v, with pitch smoothing)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_002742E8` (updates a fading, pulsing particle: animates its phase, colour and scale, then orients it)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00274D80` (particle type 65 update: fall under gravity, bounce off the world, fade the colour and die when spent)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_00211A38` (updates the hero camera's follow-speed targets by mode, then eases them)
- `src/overlays/shared/vendor_002AB910.c`: `func_L00_002B08A0` (per-frame update of a falling spark moby: emits a burst particle, moves, fades and dies when out of bounds)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002B5998` (finds the best-aimed target moby in the global list for a given yaw and pitch, refining them)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002CE390` (spawns a type-0x1C9 projectile moby at a position, aimed at an optional target; returns it, or 0 if it was launched at once)
- `src/overlays/shared/help_00203E98.c`: `func_L00_0020A540` (per-frame spark effect: emits a burst of particles around a saved position while a global timer runs down)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_00211F80` (turns the stick vector into a target heading and move direction for the player (mode 1 snaps axes, 2 and 3 snap to 4 or 8 directions, 4 uses the player's own heading))
- `src/overlays/l00_veldin1/initonce_0023E300.c`: `func_L00_0023EC00` (per-frame Veldin hook: queues callbacks and spawns particles by game state)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002EA068` (steers the hero heading and pitch towards a stick input)
- `src/overlays/shared/help_00214D60.c`: `func_L00_002162B8` (sets the camera lean/offset terms from the pad input for the glide, spin and hover states)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C7128` (draws a streak of quads along the vector from the moby's joint to its target, swelling in the middle)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_002594C8` (moves pos toward to while keeping it over the ground and out of walls; returns 0 if it was snapped back)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026E5A0` (per-frame update of a fading, bouncing spark particle: moves it, bounces off geometry, dims its colour and kills it when spent)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_002129D0` (hero ground probe: measures the support under the feet, and slides the velocity off a steep floor)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00273A60` (part type 58 update: fade in, hold spawning sparks, then fall and fade out)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002FA030` (emitter update: ticks timers, occasionally bursts a smoke puff and spawns two drifting smoke particles per frame)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028A878` (initialises and animates the star field of the sky)
- `src/overlays/l00_veldin1/vendor_002DB278.c`: `func_L00_002E2F58` (spawns a spark: steps the moby's position and emits one fast particle and three slow tails)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002FA458` (collapsing platform: sparkles while idle, then drops when the player stands near, and records it as collected)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FB898` (per-frame update of a falling drop: integrates its velocity, tests it against the world and spawns its impact effect)
- `src/overlays/shared/help_002297E0.c`: `func_L00_00229B78` (menu/hint update: picks an action for the player from the game block's timers and flags)
- `src/overlays/shared/tieproc_00299108.c`: `func_L00_0029A8D0` (looks up table entry i, from one of two sets chosen by a flag, and passes it on)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C6F48` (per-frame step of a counter moby: advances, plays sound and finishes)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002CD3B8` (spawns a debris moby with randomized spin and velocity)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00264EA8` (spawns a burst of effects on a moby: a fixed set at its position, then randomly offset ones around it)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020DD48` (scores how far a moby is from the camera/hero reference point and flags it when out of range)
- `src/overlays/shared/vendor_002AB910.c`: `func_L00_002B0F58` (spawns an effect moby tied to an owner and a target and aims its starting state)
- `src/overlays/shared/vendor_002EB0D8.c`: `func_L00_002EB3A0` (clears a moby's state block and calls its reset helper)
- `src/overlays/shared/vendor_002EB0D8.c`: `func_L00_002ECAF8` (steps a follow-camera object toward a target moby, easing its angle and position)
- `src/overlays/l00_veldin1/vendor_002DB278.c`: `func_L00_002E2038` (Veldin vendor update: walks a path, turns toward the player, attacks and self-destructs)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FED78` (breakable pot: waits for its hit flag, then shatters into four sparks)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FCC80` (hinged bridge: swings with a sound while triggered and poses the bridge from its pivot)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025A868` (tests whether a moby's linked object has a non-zero field at +8)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E99E4` (stores a float into a moby field at +0xB4)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002DCD40` (calls the callback a moby's class defines at +0x10, unless the moby is deleted)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_00261848` (marks a planet as discovered and appends it to the galaxy list)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025BBA0` (splits a vector into an angle and two scales)
- `src/overlays/shared/vendor_002EB0D8.c`: `func_L00_002EC860` (sets a moby's damping and scale constants, then re-runs its setup)
- `src/overlays/shared/help_00203E98.c`: `func_L00_0020A8B8` (tests a segment against the world and returns the hit distance)

The next 114 were carried over by machine (OpenRAC's `tools/port.py`, 2026-10-04): the PAL code is
the same instructions as the US code Lombyte matched, the C is Lombyte's with every symbol renamed
to its PAL address, and each passed this project's own check. In parentheses, Lombyte's name.

- `src/overlays/l05_rilgar/help_0023A638.c`: `func_L05_00240968` (`FUN_L05_002402a8`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_00212E70` (`FUN_L00_002127b8`)
- `src/overlays/l02_aridia/help_0021BC90.c`: `func_L02_00221CE0` (`FUN_L02_002216a0`)
- `src/overlays/l16_kalebo3/help_00209D98.c`: `func_L16_002105C8` (`FUN_L16_0020ffb0`)
- `src/overlays/l06_blarg/help_00223630.c`: `func_L06_00229CE8` (`FUN_L06_00229650`)
- `src/overlays/l12_hoven/help_0022E428.c`: `func_L12_00234498` (`FUN_L12_00233e80`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_002113A0` (`FUN_L00_00210ce8`)
- `src/overlays/shared/vendor_002D3DF8.c`: `func_L08_002F6CF0` (`FUN_L08_002f5830`)
- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002DBD70` (`FUN_L02_002da938`)
- `src/overlays/shared/vendor_002EB0D8.c`: `func_L00_002EB3E8` (`FUN_L00_002e9f38`)
- `src/overlays/l18_veldin2/help_00214138.c`: `func_L18_002190C8` (`FUN_L18_00218a90`)
- `src/overlays/l15_quartu/help_00204AC0.c`: `func_L15_00209448` (`FUN_L15_00208e10`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D83D8` (`FUN_L00_002d6f28`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E4838` (`FUN_L00_002e3388`)
- `src/overlays/shared/help_00232560.c`: `func_L00_00233410` (`FUN_L00_00232b90`)
- `src/overlays/shared/help_00222E18.c`: `func_L08_00222E18` (`FUN_L08_00222800`)
- `src/overlays/shared/vendor_002AB910.c`: `func_L00_002B1290` (`FUN_L00_002affa8`)
- `src/overlays/shared/effects_001EE2E0.c`: `func_L00_001EE698` (`FUN_L00_001ee318`)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002BE2C8` (`FUN_L01_002bd100`)
- `src/overlays/shared/mobyproc_00251A78.c`: `func_L00_002584A8` (`FUN_L00_00257470`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002F3F40` (`FUN_L01_002f2b68`)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002D99C0` (`FUN_L00_002d8510`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E1850` (`FUN_L00_002e03a0`)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002DB810` (`FUN_L00_002da360`)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FB158` (`FUN_L01_002f9d80`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_0030E9C8` (`FUN_L01_0030d5f0`)
- `src/overlays/shared/help_001FFED0.c`: `func_L15_00209A88` (`FUN_L15_00209450`)
- `src/overlays/shared/help_00203E98.c`: `func_L00_0020A0B8` (`FUN_L00_00209a40`)
- `src/overlays/shared/help_00232560.c`: `func_L00_00235088` (`FUN_L00_00234808`)
- `src/overlays/shared/vendor_00299AF0.c`: `func_L10_00299AF0` (`FUN_L10_00298940`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E85E8` (`FUN_L00_002e7138`)
- `src/overlays/shared/vendor_002D3DF8.c`: `func_L08_002D5410` (`FUN_L08_002d4108`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020EB60` (`FUN_L00_0020e4c8`)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002CF170` (`FUN_L00_002cdcc0`)
- `src/overlays/shared/vendor_002EB0D8.c`: `func_L00_002EDC98` (`FUN_L00_002ec7e8`)
- `src/overlays/shared/partupd_00280428.c`: `func_L01_0028C3A8` (`FUN_L01_0028b5e0`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002D9970` (`FUN_L06_002d85a0`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002DAA70` (`FUN_L14_002d96e0`)
- `src/overlays/shared/vendor_002EB0D8.c`: `func_L00_002EEE00` (`FUN_L00_002ed950`)
- `src/overlays/shared/help_00203E98.c`: `func_L00_00207EC0` (`FUN_L00_002078a8`)
- `src/overlays/shared/vendor_002A5138.c`: `func_L00_002AB170` (`FUN_L00_002a9ed0`)
- `src/overlays/shared/help_0021D6B8.c`: `func_L06_00228390` (`FUN_L06_00227cf8`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E33D8` (`FUN_L00_002e1f28`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_00211908` (`FUN_L00_00211250`)
- `src/overlays/shared/loaders_00240398.c`: `func_L00_00245FE0` (`FUN_L00_00245610`)
- `src/overlays/shared/tieproc_00299108.c`: `func_L00_00299148` (`FUN_L00_00297e70`)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00273478` (`FUN_L00_002725d8`)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002E0888` (`FUN_L00_002df3d8`)
- `src/overlays/shared/stream_002670E0.c`: `func_L00_002676E8` (`FUN_L00_002668a0`)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00272BC0` (`FUN_L00_00271d20`)
- `src/overlays/shared/vendor_00292AC0.c`: `func_L03_002E9C38` (`FUN_L03_002e8870`)
- `src/overlays/shared/lights_0023FA70.c`: `func_L00_0023FAB8` (`FUN_L00_0023f120`)
- `src/overlays/shared/vendor_0031AD00.c`: `func_L01_0031B450` (`FUN_L01_0031a078`)
- `src/overlays/shared/vendor_002FF000.c`: `func_L06_0030D338` (`FUN_L06_0030bef8`)
- `src/overlays/shared/vendor_002BD3D0.c`: `func_L12_00309A50` (`FUN_L12_00308670`)
- `src/overlays/shared/missionfunc_0024EEF0.c`: `func_L00_0024F7E0` (`FUN_L00_0024e7a8`)
- `src/overlays/shared/vendor_002CF2C0.c`: `func_L05_0031E670` (`FUN_L05_0031d160`)
- `src/overlays/shared/vendor_002CF2C0.c`: `func_L05_0031AA20` (`FUN_L05_00319510`)
- `src/overlays/shared/vendor_002C99E0.c`: `func_L11_002CC950` (`FUN_L11_002cb790`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_0030AC10` (`FUN_L01_00309838`)
- `src/overlays/shared/mobyfunc_0024FD50.c`: `func_L00_00250418` (`FUN_L00_0024f3e0`)
- `src/overlays/shared/help_00203E98.c`: `func_L00_002056D8` (`FUN_L00_002050c0`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002B90A8` (`FUN_L01_002b7ef0`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002EBBF8` (`FUN_L06_002ea7c8`)
- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002FBB10` (`FUN_L02_002fa6d8`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025AD38` (`FUN_L10_0024c7f0`)
- `src/overlays/shared/vendor_00299AF0.c`: `func_L10_002F6E10` (`FUN_L10_002f5a50`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002B9440` (`FUN_L01_002b8288`)
- `src/overlays/shared/help_00214D60.c`: `func_L00_00216F58` (`FUN_L00_002167d0`)
- `src/overlays/shared/help_00214D60.c`: `func_L00_00217BD8` (`FUN_L00_00217450`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_002333D8` (`FUN_L01_00232dc0`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_00231B98` (`FUN_L01_00231580`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_002320F8` (`FUN_L01_00231ae0`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_002490F8` (`FUN_L01_00248900`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_00270DB0` (`FUN_L00_0026ff10`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_002717E8` (`FUN_L00_00270948`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026F510` (`FUN_L00_0026e670`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026D700` (`FUN_L00_0026c860`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026F248` (`FUN_L00_0026e3a8`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_002709A0` (`FUN_L00_0026fb00`)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002B4F40` (`FUN_L00_002b3c50`)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002B8208` (`FUN_L00_002b6f10`)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002B9528` (`FUN_L00_002b8230`)
- `src/overlays/shared/draw_001F3A78.c`: `func_L00_001F3A78` (`FUN_L00_001f36f8`)
- `src/game/pause.c`: `func_0021E950` (`draw_menu_item_grid`)
- `src/game/actuator.c`: `func_001E9088` (`FUN_L00_001eaa08`)
- `src/game/transition.c`: `func_001EB7C0` (`transition_default_draw`)
- `src/game/pause.c`: `func_00226410` (`update_ammo_preview_transform`)
- `src/game/draw.c`: `func_001F69F0` (`process_bgm_display_text_event`)
- `src/game/pause.c`: `func_002250B8` (`draw_menu_flashing_panel`)
- `src/game/hud.c`: `func_00201AF0` (`draw_boot_image`)
- `src/game/menu.c`: `func_00207408` (`passes_projected_region_callback_0`)
- `src/game/partupd.c`: `func_00219780` (`FUN_00218888`)
- `src/game/mobyutil.c`: `func_00214F78` (`normalize_vector_triplet`)
- `src/game/hud.c`: `func_00201A38` (`draw_framed_text`)
- `src/game/pause.c`: `func_00227A70` (`clear_preview_resource_bindings`)
- `src/game/mobyfunc.c`: `func_0020E340` (`PackRenderCommandFields`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025F368` (`FUN_L00_0025e310`)
- `src/overlays/shared/help_00214D60.c`: `func_L00_002178A0` (`FUN_L00_00217118`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002F4CF8` (`FUN_L06_002f38c8`)
- `src/core/00112468.c`: `func_001126D8` (`_dtoa_r`; David M. Gay's dtoa, see below)
- `src/core/00116070.c`: `func_00116168` (`double_is_not_nan`; fdlibm, see below)
- `src/core/00116070.c`: `func_001161B0` (`ClassifyDoubleNaN`; fdlibm, see below)
- `src/core/00119D88.c`: `func_0011BF80` (`sceOpen`)
- `src/core/00119D88.c`: `func_0011C388` (`sceLseek`)
- `src/core/00119D88.c`: `func_0011C5C0` (`sceRead`)
- `src/core/00119D88.c`: `func_0011C820` (`sceWrite`)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C4748` (`FUN_L00_002c3440`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_00259B88` (`FUN_L00_00258b50`)
- `src/overlays/shared/vendor_002D9438.c`: `func_L00_002E0958` (`FUN_L00_002df4a8`)
- `src/overlays/shared/vendor_00299AF0.c`: `func_L10_002F6E38` (`FUN_L10_002f5a78`)
- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002A5238` (`FUN_L02_002a4058`)
- `src/core/00114518.c`: `func_00114920` (`_malloc_r`; newlib's allocator, see below)
- `src/core/00116FA0.c`: `func_00117118` (`_vfprintf_r`; adapted from the local newlib-derived US candidate, see below)
- `src/core/00119D88.c`: `func_0011AA90` (`_sceSifSendCmd`; its two small structs' members are named by offset here)

Four more began as machine ports that came out a few bytes off and were finished by hand by the GPT agent
(2026-10-05):

- `src/overlays/shared/vendor_002BA7C8.c`: `func_L00_002C0358` (`FUN_L00_002bf050`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028AF90` (`FUN_L00_00289cb8`)
- `src/overlays/shared/hud_00235960.c`: `func_L00_00239510` (`FUN_L00_00238b80`)
- `src/overlays/shared/help_00203E98.c`: `func_L00_002091D8` (`FUN_L00_00208b60`)

Fourteen more were carried over by machine on 2026-10-05, from what Lombyte matched in its pull request 94
(the same tool, the same rule: identical instructions, Lombyte's C with every symbol renamed to its PAL
address, and each passed this project's own check):

- `src/overlays/l02_aridia/help_0021BC90.c`: `func_L02_002205B0` (`FUN_L02_0021ff70`)
- `src/overlays/l02_aridia/help_0021BC90.c`: `func_L02_00220A28` (`FUN_L02_002203e8`)
- `src/overlays/l04_eudora/vendor_002CB800.c`: `func_L04_002E30E0` (`FUN_L04_002e1d00`)
- `src/overlays/l05_rilgar/vendor_0030EB68.c`: `func_L05_0031A8B8` (`FUN_L05_003193a8`)
- `src/overlays/l06_blarg/help_00223630.c`: `func_L06_00228510` (`FUN_L06_00227e78`)
- `src/overlays/l06_blarg/vendor_002FE5D0.c`: `func_L06_0030B248` (`FUN_L06_00309e08`)
- `src/overlays/l12_hoven/help_0022E428.c`: `func_L12_00233130` (`FUN_L12_00232b18`)
- `src/overlays/l14_oltanis/help_0021E3A8.c`: `func_L14_00223140` (`FUN_L14_00222aa8`)
- `src/overlays/l14_oltanis/help_0021E3A8.c`: `func_L14_002235D0` (`FUN_L14_00222f38`)
- `src/overlays/l16_kalebo3/vendor_002E7C70.c`: `func_L16_002EA2F8` (`FUN_L16_002e8e80`)
- `src/overlays/l16_kalebo3/vendor_002E7C70.c`: `func_L16_002EAD58` (`FUN_L16_002e98e0`)
- `src/overlays/l16_kalebo3/vendor_002E7C70.c`: `func_L16_002EB5A0` (`FUN_L16_002ea128`)
- `src/overlays/l17_fleet/help_00202740.c`: `func_L17_0020E320` (`FUN_L17_0020dbe8`)
- `src/overlays/shared/vendor_002A5138.c`: `func_L00_002A6058` (`FUN_L00_002a4dc8`)

Fifteen more were adapted by queue workers on 2026-10-05, each from Lombyte's matched C for the function's US
counterpart where the PAL code is not the same instructions (so the machine port above does not apply): the
control flow is Lombyte's, the symbols and the differing parts are PAL's, and each passed this project's own
check. In parentheses, Lombyte's name.

- `src/overlays/l01_novalis/help_002343F8.c`: `func_L01_002351A8` (`FUN_L01_00234b40`)
- `src/overlays/l02_aridia/help_0021BC90.c`: `func_L02_0021BC90` (`FUN_L02_0021b698`)
- `src/overlays/l06_blarg/help_00223630.c`: `func_L06_0022BB20` (`FUN_L06_0022b438`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_00304750` (`FUN_L09_003033a0`)
- `src/overlays/l11_pokitaru/vendor_00312BD8.c`: `func_L11_003153D0` (`FUN_L11_00313f60`)
- `src/overlays/l12_hoven/help_0022E428.c`: `func_L12_0022E428` (`FUN_L12_0022de30`)
- `src/overlays/shared/help_00214D60.c`: `func_L00_00216B40` (`FUN_L00_002163f0`)
- `src/overlays/shared/help_0021A2E0.c`: `func_L02_00223AE0` (`FUN_L02_00223450`)
- `src/overlays/shared/help_002284A8.c`: `func_L12_00236270` (`FUN_L12_00235c08`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025B4D0` (`FUN_L00_0025a478`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026B890` (`FUN_L00_0026a9f0`)
- `src/overlays/shared/tieproc_00299108.c`: `func_L00_00299250` (`FUN_L00_00297f78`)
- `src/overlays/shared/vendor_002A5138.c`: `func_L00_002A6A38` (`FUN_L00_002a57a8`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002BB068` (`FUN_L01_002b9eb0`)
- `src/overlays/shared/vuchain_002A21A8.c`: `func_L00_002A2680` (`FUN_L00_002a13f0`)

One more was adapted the same way on 2026-10-06:

- `src/overlays/shared/pause_00277208.c`: `func_L00_00277A88` (`FUN_L00_00276bd0`)
- `src/overlays/l00_veldin1/vendor_002DB278.c`: `func_L00_002E2B28` (`FUN_L00_002e1678`)

Nine more were carried over by machine on 2026-10-06, from what Lombyte matched in its pull requests 98 to 108
(the same tool, the same rule: identical instructions, Lombyte's C with every symbol renamed to its PAL
address, each passing this project's own check). In parentheses, Lombyte's name.

- `src/overlays/shared/effects_001EE2E0.c`: `func_L00_001EE530` (`FUN_L00_001ee1b0`)
- `src/overlays/shared/help_0021A2E0.c`: `func_L02_002211C0` (`FUN_L02_00220b80`)
- `src/overlays/shared/help_0021D6B8.c`: `func_L06_0021D6B8` (`FUN_L06_0021d0a0`)
- `src/overlays/shared/help_0021D6B8.c`: `func_L06_0022A868` (`FUN_L06_0022a1d0`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_002328A8` (`FUN_L01_00232290`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025E210` (`FUN_L00_0025d1b8`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_00260878` (`FUN_L00_0025f800`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_002608F0` (`FUN_L00_0025f878`)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002B9A90` (`FUN_L00_002b8798`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E1790` (`FUN_L00_002e02e0`)
- `src/overlays/l13_gemlik/vendor_002C2638.c`: `func_L13_002C2638` (`FUN_L13_002c13b0`)

Twelve executable functions from the same pull requests followed on 2026-10-07, proven by the full build.
Ten went in as the tool wrote them. `func_001EC2B8` needed its file to change around it: a later
declaration retyped to its parameter, and two typedefs it shares with `func_001ED818` kept once.
`func_001EE858` was adapted: it writes the fog preset as the struct `src/game/draw.c` already reads.

- `src/core/00119868.c`: `func_00119AA8` (`sceTtyWrite`)
- `src/core/0012AC80.c`: `func_0012AD10` (`sceMpegDemuxPssRing`)
- `src/game/camera.c`: `func_001EC2B8` (`switch_active_camera_record`)
- `src/game/camera.c`: `func_001ED818` (`FUN_001ed470`)
- `src/game/drawquad.c`: `func_001F9478` (`append_billboard_batch`)
- `src/game/effects.c`: `func_001EE858` (`update_camera_environment_from_regions`)
- `src/game/pause.c`: `func_0021C1B0` (`render_localized_ui_entry_list`)
- `src/game/pause.c`: `func_0021F6A0` (`update_item_preview_transform`)
- `src/game/pause.c`: `func_002255F8` (`load_hand_gadget`)
- `src/game/pause.c`: `func_00228860` (`submit_graphics_setup_command_stream`)
- `src/game/skyfunc.c`: `func_0022C188` (`update_sky_effects`)
- `src/game/vendor.c`: `func_0023A5E0` (`render_vendor_capture_texture_overlays_pass`)

One more on 2026-10-07, written from the adapted `func_L12_00236270` above plus one state arm:

- `src/overlays/shared/help_00214D60.c`: `func_L00_00214D60` (`FUN_L00_00214658`)

Ten more were carried over by machine on 2026-10-07, from Lombyte's pull requests 109 to 114 and from its
unmerged branches `decomp/twins-wave` (the first three below) and `overlay/shared-veldin-5` (the same tool,
the same rule: identical instructions, Lombyte's C with every symbol renamed to its PAL address, each
passing this project's own check). In parentheses, Lombyte's name.

- `src/overlays/shared/help_001FFED0.c`: `func_L15_0020ADD8` (`FUN_L15_0020a7a0`)
- `src/overlays/l02_aridia/help_0022BE40.c`: `func_L02_002368E8` (`FUN_L02_002360c8`)
- `src/overlays/shared/help_0021D6B8.c`: `func_L06_002431A0` (`FUN_L06_00242950`)
- `src/overlays/l02_aridia/vendor_002E21F8.c`: `func_L02_002E33F0` (`FUN_L02_002e1fb8`)
- `src/overlays/l02_aridia/vendor_002E21F8.c`: `func_L02_002ED660` (`FUN_L02_002ec228`)
- `src/overlays/l05_rilgar/vendor_002D28D0.c`: `func_L05_003052A8` (`FUN_L05_00303e50`)
- `src/overlays/l05_rilgar/vendor_002D28D0.c`: `func_L05_0030D3F0` (`FUN_L05_0030bf98`)
- `src/overlays/l08_batalia/vendor_002EAF48.c`: `func_L08_002F2838` (`FUN_L08_002f1378`)
- `src/overlays/l14_oltanis/vendor_002FF358.c`: `func_L14_00306B08` (`FUN_L14_00305680`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002F0FD0` (`FUN_L01_002efbf8`)

Fifteen more were carried over by machine on 2026-10-05 once `include/common.h` had `qzero()`, the 128-bit
zero store Lombyte identified as inline assembly in the original (its `include/qzero.h`; the helper here is
written after it). Same tool, same rule; the two executable functions are proven by the full build:

- `src/game/pause.c`: `func_00219C08` (`FUN_00218d10`)
- `src/game/sound.c`: `func_0022EFE8` (`sound_stop_all_sounds`)
- `src/overlays/l00_veldin1/shrubproc_0028A608.c`: `func_L00_0028A608` (`FUN_L00_00289330`)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002F7558` (`FUN_L01_002f6180`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_00260D30` (`FUN_L00_0025fcb8`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026D270` (`FUN_L00_0026c3d0`)
- `src/overlays/shared/partupd_0026A130.c`: `func_L00_0026ED30` (`FUN_L00_0026de90`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028AEB0` (`FUN_L00_00289bd8`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028B4E0` (`FUN_L00_0028a208`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028B5C8` (`FUN_L00_0028a2f0`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028B758` (`FUN_L00_0028a480`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028BC70` (`FUN_L00_0028a998`)
- `src/overlays/shared/shrubproc_0028A198.c`: `func_L00_0028C358` (`FUN_L00_0028b080`)
- `src/overlays/shared/space_0028FB78.c`: `func_L00_002902A0` (`FUN_L00_0028efc8`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002E4920` (`FUN_L01_002e35a8`)

Seventy-one more were carried over by machine on 2026-10-09, from what Lombyte matched in its pull requests
109 to 132 (the same tool, the same rule: identical instructions, Lombyte's C with every symbol renamed to its
PAL address, each passing this project's own check). In parentheses, Lombyte's name.

- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002F44F8` (`FUN_L01_002f3120`)
- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002F5AE8` (`FUN_L01_002f4710`)
- `src/overlays/l02_aridia/hud_0023D600.c`: `func_L02_0023D6E0` (`FUN_L02_0023cda8`)
- `src/overlays/l02_aridia/vendor_002A59D8.c`: `func_L02_002E0D80` (`FUN_L02_002df948`)
- `src/overlays/l02_aridia/vendor_002E21F8.c`: `func_L02_002E3660` (`FUN_L02_002e2228`)
- `src/overlays/l03_kerwan/help_00205E28.c`: `func_L03_00205E28` (`FUN_L03_00205830`)
- `src/overlays/l03_kerwan/mobyutil_00250B88.c`: `func_L03_00250B88` (`FUN_L03_0024fba8`)
- `src/overlays/l03_kerwan/vendor_00293720.c`: `func_L03_0029ED60` (`FUN_L03_0029dba8`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002CD270` (`FUN_L03_002cbea8`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002CDC50` (`FUN_L03_002cc888`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002E1598` (`FUN_L03_002e01d0`)
- `src/overlays/l04_eudora/vendor_0029FCF0.c`: `func_L04_002C5560` (`FUN_L04_002c41e0`)
- `src/overlays/l04_eudora/vendor_0029FCF0.c`: `func_L04_002C7F38` (`FUN_L04_002c6bb8`)
- `src/overlays/l04_eudora/vendor_002CB800.c`: `func_L04_002E6780` (`FUN_L04_002e53a0`)
- `src/overlays/l05_rilgar/vendor_002D28D0.c`: `func_L05_00304F60` (`FUN_L05_00303b08`)
- `src/overlays/l06_blarg/vendor_002B5990.c`: `func_L06_002FD0B8` (`FUN_L06_002fbc88`)
- `src/overlays/l06_blarg/vendor_002FE5D0.c`: `func_L06_00301848` (`FUN_L06_003003f0`)
- `src/overlays/l06_blarg/vendor_002FE5D0.c`: `func_L06_00301B78` (`FUN_L06_00300720`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_003102C8` (`FUN_L07_0030eee8`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_003106E8` (`FUN_L07_0030f308`)
- `src/overlays/l08_batalia/vendor_002B9438.c`: `func_L08_002DB768` (`FUN_L08_002da3f0`)
- `src/overlays/l08_batalia/vendor_002E0258.c`: `func_L08_002E3860` (`FUN_L08_002e24e8`)
- `src/overlays/l08_batalia/vendor_002EAF48.c`: `func_L08_002EB980` (`FUN_L08_002ea4c0`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_002F0D00` (`FUN_L09_002ef9b0`)
- `src/overlays/l09_gaspar/vendor_0030B5E8.c`: `func_L09_0030B648` (`FUN_L09_0030a298`)
- `src/overlays/l11_pokitaru/vendor_002CC828.c`: `func_L11_002F3888` (`FUN_L11_002f2518`)
- `src/overlays/l11_pokitaru/vendor_00312BD8.c`: `func_L11_003175D0` (`FUN_L11_00316160`)
- `src/overlays/l12_hoven/mobyutil_00272D90.c`: `func_L12_00272D90` (`FUN_L12_00271dd8`)
- `src/overlays/l12_hoven/vendor_002C0310.c`: `func_L12_002E4F18` (`FUN_L12_002e3c08`)
- `src/overlays/l12_hoven/vendor_002C0310.c`: `func_L12_002EC2E8` (`FUN_L12_002eaf68`)
- `src/overlays/l13_gemlik/vendor_002C2638.c`: `func_L13_002E9B30` (`FUN_L13_002e86f8`)
- `src/overlays/l13_gemlik/vendor_002C2638.c`: `func_L13_002EA840` (`FUN_L13_002e9408`)
- `src/overlays/l13_gemlik/vendor_002EBD00.c`: `func_L13_0030B340` (`FUN_L13_00309e88`)
- `src/overlays/l13_gemlik/vendor_0030CAE0.c`: `func_L13_0030D318` (`FUN_L13_0030be60`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002AFA18` (`FUN_L14_002ae7d8`)
- `src/overlays/l14_oltanis/vendor_002E0538.c`: `func_L14_002ECC70` (`FUN_L14_002eb810`)
- `src/overlays/l14_oltanis/vendor_002FF358.c`: `func_L14_00306890` (`FUN_L14_00305408`)
- `src/overlays/l15_quartu/mobyutil_00248E58.c`: `func_L15_00248E58` (`FUN_L15_00247e90`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002C65D8` (`FUN_L15_002c52b8`)
- `src/overlays/l15_quartu/vendor_002EDB50.c`: `func_L15_002EEF70` (`FUN_L15_002edb20`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002D76D8` (`FUN_L18_002d62e8`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FDB28` (`FUN_L18_002fc668`)
- `src/overlays/shared/camera_001FC308.c`: `func_L02_001FC308` (`FUN_L02_001fc008`)
- `src/overlays/shared/help_001FFED0.c`: `func_L15_00216568` (`FUN_L15_00215e58`)
- `src/overlays/shared/help_0021D6B8.c`: `func_L06_00235038` (`FUN_L06_002348d0`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_0023CE00` (`FUN_L01_0023c710`)
- `src/overlays/shared/loaders_00240398.c`: `func_L00_00245B88` (`FUN_L00_002451b8`)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_002633D8` (`FUN_L00_00262360`)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_00274908` (`FUN_L00_00273a68`)
- `src/overlays/shared/partupd_00295880.c`: `func_L09_00295880` (`FUN_L09_00294ae0`)
- `src/overlays/shared/vendor_00292AC0.c`: `func_L03_002DD6D8` (`FUN_L03_002dc310`)
- `src/overlays/shared/vendor_00299AF0.c`: `func_L10_002CD6A8` (`FUN_L10_002cc2e8`)
- `src/overlays/shared/vendor_002A1B58.c`: `func_L16_002E4408` (`FUN_L16_002e2fa0`)
- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002D9D48` (`FUN_L02_002d8910`)
- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002DC2C0` (`FUN_L02_002dae88`)
- `src/overlays/shared/vendor_002B0068.c`: `func_L04_002B0290` (`FUN_L04_002af058`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002B4128` (`FUN_L14_002b2ed8`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_00300130` (`FUN_L14_002feca8`)
- `src/overlays/shared/vendor_002B8FC0.c`: `func_L13_002B8FC0` (`FUN_L13_002b7d50`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002E2E38` (`FUN_L01_002e1ac0`)
- `src/overlays/shared/vendor_002BA7C8.c`: `func_L00_002BEF58` (`FUN_L00_002bdc50`)
- `src/overlays/shared/vendor_002BD3D0.c`: `func_L12_00309AE8` (`FUN_L12_00308708`)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C4E00` (`FUN_L00_002c3af8`)
- `src/overlays/shared/vendor_002C6B30.c`: `func_L09_0030C7F0` (`FUN_L09_0030b3b0`)
- `src/overlays/shared/vendor_002C99E0.c`: `func_L11_00311F98` (`FUN_L11_00310ad0`)
- `src/overlays/shared/vendor_002CF2C0.c`: `func_L05_002F6588` (`FUN_L05_002f5200`)
- `src/overlays/shared/vendor_002D7C00.c`: `func_L15_002E5770` (`FUN_L15_002e4380`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002EB360` (`FUN_L06_002e9f30`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_002F9AF0` (`FUN_L01_002f8718`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_00316270` (`FUN_L01_00314e98`)
- `src/overlays/shared/vendor_002FF000.c`: `func_L06_00300AB0` (`FUN_L06_002ff680`)

Nine more were adapted by workers on 2026-10-09 from the candidates the same tool carried over, which came out
one to twenty-one bytes off with this project's compiler: the logic is Lombyte's, and a declaration, a renamed
symbol or one statement form was changed until this project's own check passed. In parentheses, Lombyte's name.

- `src/overlays/l05_rilgar/vendor_0030EB68.c`: `func_L05_00316B98` (`FUN_L05_003156d0`)
- `src/overlays/shared/help_00214D60.c`: `func_L00_00216648` (`FUN_L00_00215ef8`)
- `src/overlays/shared/hud_00235960.c`: `func_L00_00238530` (`FUN_L00_00237ba0`)
- `src/overlays/shared/hud_00235960.c`: `func_L00_0023A788` (`FUN_L00_00239df8`)
- `src/overlays/shared/hud_00263490.c`: `func_L05_00263490` (`FUN_L05_00262ae8`)
- `src/overlays/shared/tieproc_00299108.c`: `func_L00_0029A300` (`FUN_L00_00298f90`)
- `src/overlays/shared/vendor_00298BB8.c`: `func_L15_002A47B8` (`FUN_L15_002a35d8`)
- `src/overlays/shared/vendor_002A5138.c`: `func_L00_002A86D8` (`FUN_L00_002a7438`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E87C8` (`FUN_L00_002e7318`)

Six more of the same pull requests' functions were carried over by machine later on 2026-10-09, once the
files they go into were free:

- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002D6738` (`FUN_L18_002d5348`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020CDF0` (`FUN_L00_0020c758`)
- `src/overlays/shared/partupd_00280428.c`: `func_L01_00286530` (`FUN_L01_00285768`)
- `src/overlays/shared/vendor_00298BB8.c`: `func_L15_0029BE10` (`FUN_L15_0029ac30`)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002D06C8` (`FUN_L00_002cf218`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E80A8` (`FUN_L00_002e6bf8`)

Fifty-four more were adapted by workers the same day from the candidates the tool carried over, which did not
pass this project's check as written. The logic is Lombyte's; what changed is declarations (the function
defined under an alias where its file declares it otherwise, type names suffixed, a `$gp` global given its
real type, a table declared unsized), a renamed symbol, or a displacement that differs between US and PAL.
In parentheses, Lombyte's name.

- `src/overlays/l01_novalis/vendor_002BA898.c`: `func_L01_002BA898` (`FUN_L01_002b96e0`)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_003105E0` (`FUN_L01_0030f208`)
- `src/overlays/l02_aridia/vendor_002A59D8.c`: `func_L02_002D99F0` (`FUN_L02_002d85b8`)
- `src/overlays/l02_aridia/vendor_002E21F8.c`: `func_L02_002EB480` (`FUN_L02_002ea048`)
- `src/overlays/l03_kerwan/vendor_00293720.c`: `func_L03_002BC038` (`FUN_L03_002bad40`)
- `src/overlays/l03_kerwan/vendor_00293720.c`: `func_L03_002CA970` (`FUN_L03_002c95a8`)
- `src/overlays/l03_kerwan/vendor_00293720.c`: `func_L03_002CB068` (`FUN_L03_002c9ca0`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002CB280` (`FUN_L03_002c9eb8`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002CBBD0` (`FUN_L03_002ca808`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002D4560` (`FUN_L03_002d3198`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002DC648` (`FUN_L03_002db280`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002ECBA8` (`FUN_L03_002eb7e0`)
- `src/overlays/l05_rilgar/vendor_0030EB68.c`: `func_L05_00316378` (`FUN_L05_00314eb0`)
- `src/overlays/l06_blarg/vendor_002B5990.c`: `func_L06_002F8430` (`FUN_L06_002f7000`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_0030CFA8` (`FUN_L07_0030bbc8`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_0030FB00` (`FUN_L07_0030e720`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_0030FC38` (`FUN_L07_0030e858`)
- `src/overlays/l07_umbris/vendor_00313D28.c`: `func_L07_00315108` (`FUN_L07_00313d30`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_00304EE0` (`FUN_L09_00303b30`)
- `src/overlays/l10_orxon/vendor_00296BD8.c`: `func_L10_002DE630` (`FUN_L10_002dd270`)
- `src/overlays/l11_pokitaru/vendor_002CC828.c`: `func_L11_002D3970` (`FUN_L11_002d27b0`)
- `src/overlays/l11_pokitaru/vendor_002CC828.c`: `func_L11_00311318` (`FUN_L11_0030fe50`)
- `src/overlays/l11_pokitaru/vendor_00312BD8.c`: `func_L11_0031BBC8` (`FUN_L11_0031a758`)
- `src/overlays/l12_hoven/vendor_002C0310.c`: `func_L12_002E4838` (`FUN_L12_002e3528`)
- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_00309590` (`FUN_L12_003081b0`)
- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_0030D3D0` (`FUN_L12_0030bff0`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002B6E80` (`FUN_L14_002b5c30`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002B6F68` (`FUN_L14_002b5d18`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002B7028` (`FUN_L14_002b5dd8`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002D7250` (`FUN_L14_002d5f40`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002D7880` (`FUN_L14_002d6570`)
- `src/overlays/l14_oltanis/vendor_002E0538.c`: `func_L14_002EC7E8` (`FUN_L14_002eb388`)
- `src/overlays/l14_oltanis/vendor_002E0538.c`: `func_L14_002F0A30` (`FUN_L14_002ef5a8`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002A4D88` (`FUN_L15_002a3ba8`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002ED318` (`FUN_L15_002ebf28`)
- `src/overlays/l15_quartu/vendor_002EDB50.c`: `func_L15_002F9D38` (`FUN_L15_002f88e8`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002D7310` (`FUN_L18_002d5f20`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002EC0C8` (`FUN_L18_002eacd8`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FB080` (`FUN_L18_002f9c20`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FDD20` (`FUN_L18_002fc860`)
- `src/overlays/shared/hud_00235960.c`: `func_L00_0023D750` (`FUN_L00_0023cdb8`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025A208` (`FUN_L00_002591d0`)
- `src/overlays/shared/vendor_002A5138.c`: `func_L00_002A7E10` (`FUN_L00_002a6b70`)
- `src/overlays/shared/vendor_002B33E8.c`: `func_L00_002BA608` (`FUN_L00_002b9310`)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C2A80` (`FUN_L00_002c1778`)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002CF6A0` (`FUN_L00_002ce1f0`)
- `src/overlays/shared/vendor_002CF2C0.c`: `func_L05_0032ABF0` (`FUN_L05_003296e0`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D6CE0` (`FUN_L00_002d5830`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D90A0` (`FUN_L00_002d7bf0`)
- `src/overlays/shared/vendor_002D7C00.c`: `func_L15_002D8DD8` (`FUN_L15_002d79e8`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002F4720` (`FUN_L06_002f32f0`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002F4A70` (`FUN_L06_002f3640`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E9E60` (`FUN_L00_002e89b0`)
- `src/overlays/shared/vendor_002FF000.c`: `func_L06_002FF000` (`FUN_L06_002fdbd0`)

`func_L00_0025B478` in `src/overlays/shared/mobyutil_00258BC8.c` takes one form from Lombyte's match of the
same function: the byte read through a small inline accessor.

Nine functions of the executable were carried over by machine on 2026-10-09, from the same pull requests of
Lombyte and by the same rule (identical instructions, Lombyte's C with every symbol renamed to its PAL
address, each passing this project's check and the full build):

- `src/game/draw.c`: `func_001F5E60` (`append_rotated_sprite_quad`)
- `src/game/music.c`: `func_00217130` (`music_update`)
- `src/game/pause.c`: `func_0021F238` (`update_item_preview_binding`)
- `src/game/pause.c`: `func_0021FB28` (`draw_items_menu`)
- `src/game/pause.c`: `func_00222070` (`draw_cheats_menu`)
- `src/game/pause.c`: `func_002260A8` (`update_menu_preview_pose_and_attachments`)
- `src/game/pause.c`: `func_002282D0` (`FUN_00226fb8`)
- `src/game/space.c`: `func_00233AB8` (`render_environment_mapped_object`)
- `src/game/vendor.c`: `func_0023B210` (`sample_surface_height_map`)

Eighteen more were adapted by workers on 2026-10-09 from Lombyte's matched C for the same functions, where the
candidate the tool carried over did not pass this project's check as written. The logic is Lombyte's; what
changed is declarations (private aliases for globals and callees, the function defined under an alias where
its file declares it otherwise) and single statement forms. The twelve executable functions are proven by the
full build. In parentheses, Lombyte's name.

- `src/core/989snd.c`: `func_0012E820` (`snd_send_iop_command_no_wait`)
- `src/game/draw.c`: `func_001F3D78` (`draw_debug_profiler`)
- `src/game/draw.c`: `func_001F6668` (`font_print`)
- `src/game/hud.c`: `func_00200248` (`get_frame_texture`)
- `src/game/loaders.c`: `func_00203B70` (`prepare_resident_class_render_data`)
- `src/game/pause.c`: `func_00228690` (`append_fullscreen_clear_strips`)
- `src/game/skyfunc.c`: `func_0022C5A0` (`draw_sky_shells`)
- `src/game/skyfunc.c`: `func_0022CA00` (`sky_draw_shell_textured`)
- `src/game/skyfunc.c`: `func_0022CC40` (`sky_draw_shell_gouraud`)
- `src/game/space.c`: `func_002305A0` (`render_level_frame`)
- `src/game/space.c`: `func_00232EF0` (`play_level_loading_slides`)
- `src/game/update.c`: `func_002391E8` (`FUN_00237ed0`)
- `src/overlays/l03_kerwan/vendor_00293720.c`: `func_L03_0029E498` (`FUN_L03_0029d2e0`)
- `src/overlays/l04_eudora/vendor_002CB800.c`: `func_L04_002E2DF0` (`FUN_L04_002e1a10`)
- `src/overlays/l10_orxon/vendor_00296BD8.c`: `func_L10_00299818` (`FUN_L10_00298668`)
- `src/overlays/shared/hud_00263490.c`: `func_L05_002670A0` (`FUN_L05_00266710`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_00309078` (`FUN_L01_00307ca0`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_00317408` (`FUN_L01_00316030`)

Two more were adapted the same day, each replacing a near miss of this project's own with Lombyte's logic for
the same function:

- `src/overlays/l02_aridia/vendor_002A59D8.c`: `func_L02_002D70C0` (`FUN_L02_002d5c88`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_0030CA88` (`FUN_L07_0030b6a8`)

Twenty-two more of the functions the tool carried over from Lombyte's pull requests 109 to 132 landed later on
2026-10-09. They are Lombyte's C with every symbol renamed, unchanged in their statements; what kept them out
was their destination files, which by then defined the same type names and declared the same globals another
way. Every type, callee and global of each function now has a name private to it. Three of them needed the
build tools first: `func_L00_00269BE8` and `func_L00_002761C0` the assembler model's rule for `mfc1`, and
`func_L00_002C9820` the level check's handling of a shared `%lo`. In parentheses, Lombyte's name.

- `src/overlays/l02_aridia/vendor_002E21F8.c`: `func_L02_002ED358` (`FUN_L02_002ebf20`)
- `src/overlays/l02_aridia/vendor_002E21F8.c`: `func_L02_002F9E50` (`FUN_L02_002f8a18`)
- `src/overlays/l03_kerwan/vendor_00293720.c`: `func_L03_00293EB8` (`FUN_L03_00292d10`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002D5220` (`FUN_L03_002d3e58`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002D5790` (`FUN_L03_002d43c8`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002DC3E8` (`FUN_L03_002db020`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002DC560` (`FUN_L03_002db198`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002DE088` (`FUN_L03_002dccc0`)
- `src/overlays/l07_umbris/vendor_002CE470.c`: `func_L07_0030C320` (`FUN_L07_0030af40`)
- `src/overlays/l08_batalia/vendor_002B9438.c`: `func_L08_002DB468` (`FUN_L08_002da0f0`)
- `src/overlays/l13_gemlik/vendor_002C2638.c`: `func_L13_002E9D58` (`FUN_L13_002e8920`)
- `src/overlays/l13_gemlik/vendor_002C2638.c`: `func_L13_002EB978` (`FUN_L13_002ea540`)
- `src/overlays/l13_gemlik/vendor_002EBD00.c`: `func_L13_00306D40` (`FUN_L13_003058a8`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002B0068` (`FUN_L14_002aee28`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002D74F8` (`FUN_L18_002d6108`)
- `src/overlays/shared/partproc_002697A0.c`: `func_L00_00269BE8` (`FUN_L00_00268d48`)
- `src/overlays/shared/partupd_00272158.c`: `func_L00_002761C0` (`FUN_L00_00275320`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002E4580` (`FUN_L01_002e3208`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002F4290` (`FUN_L01_002f2eb8`)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002C9820` (`FUN_L00_002c8440`)

Eighteen more of the same kind followed, the last of the candidates carried over from those pull requests:

- `src/overlays/l04_eudora/vendor_0029FCF0.c`: `func_L04_0029FCF0` (`FUN_L04_0029eb20`)
- `src/overlays/l04_eudora/vendor_0029FCF0.c`: `func_L04_002C7BD8` (`FUN_L04_002c6858`)
- `src/overlays/l05_rilgar/vendor_0030EB68.c`: `func_L05_003165B8` (`FUN_L05_003150f0`)
- `src/overlays/l08_batalia/vendor_002E0258.c`: `func_L08_002EA0A8` (`FUN_L08_002e8cd0`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_002EBD30` (`FUN_L09_002eaa50`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_002F0780` (`FUN_L09_002ef430`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_00305378` (`FUN_L09_00303fc8`)
- `src/overlays/shared/help_00221A98.c`: `func_L00_00221D50` (`FUN_L00_002215c8`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_002293D0` (`FUN_L01_00228e38`)
- `src/overlays/shared/help_002274A8.c`: `func_L01_0022DE30` (`FUN_L01_0022d838`)
- `src/overlays/shared/vendor_002B0068.c`: `func_L04_002B0068` (`FUN_L04_002aee30`)
- `src/overlays/shared/vendor_002C6B30.c`: `func_L09_00308F58` (`FUN_L09_00307ba8`)
- `src/overlays/shared/vendor_002C6B30.c`: `func_L09_00309118` (`FUN_L09_00307d68`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D19E8` (`FUN_L00_002d0538`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_002F7700` (`FUN_L01_002f6328`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_002F9908` (`FUN_L01_002f8530`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_003015F8` (`FUN_L01_00300220`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_0030EC58` (`FUN_L01_0030d880`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002EBA10` (`FUN_L06_002ea5e0`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002F4908` (`FUN_L06_002f34d8`)

Twenty more level functions were adapted by a worker on 2026-10-09 from Lombyte's matched C for the same
functions, each replacing a near miss of this project's own. Lombyte's statements stand unchanged; the
declarations are this project's (private aliases, small data with its real type, the hero, pad and voice
blocks under the symbols this project uses), and two are defined under an alias. In parentheses, Lombyte's name.

- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FB588` (`FUN_L01_002fa1b0`)
- `src/overlays/l05_rilgar/vendor_0030EB68.c`: `func_L05_0030F9D0` (`FUN_L05_0030e508`)
- `src/overlays/l05_rilgar/vendor_0030EB68.c`: `func_L05_00319F78` (`FUN_L05_00318a68`)
- `src/overlays/l07_umbris/vendor_00313D28.c`: `func_L07_00319080` (`FUN_L07_00317cb0`)
- `src/overlays/l07_umbris/vendor_00313D28.c`: `func_L07_0031AAB0` (`FUN_L07_003196e0`)
- `src/overlays/l07_umbris/vendor_00313D28.c`: `func_L07_0031B318` (`FUN_L07_00319f48`)
- `src/overlays/l08_batalia/vendor_002EAF48.c`: `func_L08_002EAF48` (`FUN_L08_002e9b70`)
- `src/overlays/l08_batalia/vendor_002EAF48.c`: `func_L08_002F8560` (`FUN_L08_002f70a0`)
- `src/overlays/l10_orxon/vendor_00296BD8.c`: `func_L10_002CAD18` (`FUN_L10_002c9958`)
- `src/overlays/l13_gemlik/vendor_002C2638.c`: `func_L13_002E9910` (`FUN_L13_002e84d8`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002B5750` (`FUN_L14_002b4500`)
- `src/overlays/l14_oltanis/vendor_002E0538.c`: `func_L14_002FF040` (`FUN_L14_002fdbb8`)
- `src/overlays/l18_veldin2/vendor_002F2AE0.c`: `func_L18_002F7F00` (`FUN_L18_002f6aa0`)
- `src/overlays/shared/vendor_0029FD68.c`: `func_L00_002A1540` (`FUN_L00_002a02b0`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002B3850` (`FUN_L14_002b2600`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_003039C0` (`FUN_L14_00302538`)
- `src/overlays/shared/vendor_002C12B0.c`: `func_L00_002C8DB8` (`FUN_L00_002c7a58`)
- `src/overlays/shared/vendor_002C6B30.c`: `func_L09_002C6B30` (`FUN_L09_002c5990`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D4398` (`FUN_L00_002d2ee8`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002EB5C8` (`FUN_L06_002ea198`)

Nineteen more functions of the executable were adapted by a worker on 2026-10-09 from Lombyte's matched C
for their US counterparts. Three are the same code and three the same instructions with other constants;
in the other thirteen the PAL version differs (the menu and memory card structures have more fields, the
loading screen and the pad code have extra steps), and those parts were written from the PAL assembly.
Lombyte's `volatile` declarations, one `do {} while (0)` and one `#define` are replaced by declarations and
statement order. Proven by the full build. In parentheses, Lombyte's name.

- `src/game/bmain.c`: `func_001E99D8` (`startlevel`)
- `src/game/loaders.c`: `func_00204C60` (`service_level_archive_load`)
- `src/game/missionfunc.c`: `func_0020CDE0` (`update_map_icons`)
- `src/game/pad.c`: `func_002181F0` (`process_pad_input`)
- `src/game/pause.c`: `func_00219E90` (`FUN_00218f98`)
- `src/game/pause.c`: `func_0021A1A0` (`FUN_002192a8`)
- `src/game/pause.c`: `func_0021BB90` (`update_menu_entry_actions`)
- `src/game/pause.c`: `func_0021CE60` (`draw_map_screen`)
- `src/game/pause.c`: `func_0021DB30` (`sound_options_menu`)
- `src/game/pause.c`: `func_0021DE08` (`draw_sound_menu`)
- `src/game/pause.c`: `func_002209A0` (`FUN_0021f990`)
- `src/game/pause.c`: `func_00220DF0` (`update_menu_resource_stream`)
- `src/game/pause.c`: `func_00223B40` (`draw_localized_three_option_menu`)
- `src/game/pause.c`: `func_00224C30` (`draw_save_slot_list`)
- `src/game/pause.c`: `func_00227B00` (`load_preview_resource_bindings`)
- `src/game/pause.c`: `func_00227DB0` (`FUN_00226b08`)
- `src/game/sound.c`: `func_0022EB08` (`allocate_voice_slot`)
- `src/game/transition.c`: `func_001EABE8` (`transition_load_wad`)
- `src/game/transition.c`: `func_001EB458` (`update_gameplay_frame`)

Six more were carried over by machine on 2026-10-09, from Lombyte's pull requests 133 to 135 (the same
tool, the same rule: identical instructions, Lombyte's C with every symbol renamed to its PAL address, each
passing this project's own check; `func_L04_002C2F00` is defined under an alias because its file declares
it otherwise). In parentheses, Lombyte's name.

- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FFF30` (`FUN_L01_002feb58`)
- `src/overlays/l04_eudora/vendor_0029FCF0.c`: `func_L04_002C2F00` (`FUN_L04_002c1b80`)
- `src/overlays/l04_eudora/vuchain_00293490.c`: `func_L04_00296FA8` (`FUN_L04_00295de8`)
- `src/overlays/shared/help_00203E98.c`: `func_L00_0020A320` (`FUN_L00_00209ca8`)
- `src/overlays/shared/help_00214D60.c`: `func_L00_00217DE0` (`FUN_L00_00217658`)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00264140` (`FUN_L00_002630c8`)

One more was carried over by machine on 2026-10-10, from Lombyte's pull request 139 (the same tool, the
same rule). In parentheses, Lombyte's name.

- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_00307730` (`FUN_L12_00306350`)

One more is this project's own port of a function it has from Lombyte: `func_L00_001EB890` in
`src/overlays/shared/camera_001EB508.c` is the level programs' copy of `func_001EC2B8` (Lombyte's
`switch_active_camera_record`), with the same C under the level's symbols.

`include/moby.h`, the shared definition of the moby (the game's 0x100-byte object), its class header and
the vector type, is adapted from Lombyte's `include/rnc/gameplay/entities/moby.h` at 88c78925, whose layout
and field names come from the matched code it cites.

Seven more were adapted by hand on 2026-10-13 from Lombyte's matched C for functions this project's
catalogue splits into pieces, which is why no tool carried them over: the C is Lombyte's with every symbol
renamed to its PAL address, each has its row in `config/overlays/joined.tsv`, and each passed this
project's own check. In parentheses, Lombyte's name.

- `src/overlays/shared/camera_001EB508.c`: `func_L00_001EB578` (`FUN_L00_001eb1f8`)
- `src/overlays/shared/help_00237B00.c`: `func_L05_002559B8` (`FUN_L05_002551d0`)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_00264BB0` (`FUN_L00_00263b38`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E9838` (`FUN_L00_002e8388`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002DDEF8` (`FUN_L03_002dcb30`)
- `src/overlays/l06_blarg/vendor_002FE5D0.c`: `func_L06_00305C38` (`FUN_L06_003047f8`)
- `src/overlays/l14_oltanis/vendor_002FF358.c`: `func_L14_00307660` (`FUN_L14_003061d8`)

Fifteen more were adapted by hand on 2026-10-14 from Lombyte's matched C. The C is Lombyte's with every
symbol renamed to its PAL address, the level data at its PAL place and two dialogue ids at their PAL
values, and each passed this project's own check. In parentheses, Lombyte's name.

- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002EAF90` (`FUN_L06_002e9b60`)
- `src/overlays/shared/vendor_002FF000.c`: `func_L06_0030D3E8` (`FUN_L06_0030bfa8`)
- `src/overlays/shared/mobyutil_00261B00.c`: `func_L00_002629E0` (`FUN_L00_00261968`)
- `src/overlays/l09_gaspar/vendor_002C2B08.c`: `func_L09_002F1198` (`FUN_L09_002efe48`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_00303810` (`FUN_L01_00302438`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020D3A0` (`FUN_L00_0020cd08`)
- `src/overlays/shared/vendor_002C6B30.c`: `func_L09_0030BB28` (`FUN_L09_0030a778`)
- `src/overlays/l11_pokitaru/vendor_002CC828.c`: `func_L11_00310058` (`FUN_L11_0030eb90`)
- `src/overlays/l14_oltanis/vendor_002ACCC0.c`: `func_L14_002D8500` (`FUN_L14_002d71f0`)
- `src/overlays/l14_oltanis/vendor_002FF358.c`: `func_L14_00306BE0` (`FUN_L14_00305758`)
- `src/overlays/l14_oltanis/vendor_002E0538.c`: `func_L14_002FD578` (`FUN_L14_002fc0f0`)
- `src/overlays/shared/vendor_002B90A8.c`: `func_L01_002F0B48` (`FUN_L01_002ef770`)
- `src/overlays/shared/vendor_002D9548.c`: `func_L06_002F3D40` (`FUN_L06_002f2910`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FABE0` (`FUN_L18_002f9780`)
- `src/overlays/l13_gemlik/vendor_002EBD00.c`: `func_L13_002EBD00` (`FUN_L13_002ea8c8`)

Six more were adapted by hand on 2026-10-14 from Lombyte's matched C. The C is Lombyte's with every symbol
renamed to its PAL address and the level data at its PAL place, rewritten where Lombyte's form only steered the
compiler, and each passed this project's own check. In parentheses, Lombyte's name.

- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002DDAE8` (`FUN_L02_002dc6b0`)
- `src/overlays/shared/hud_00235960.c`: `func_L00_0023AC68` (`FUN_L00_0023a2d0`)
- `src/overlays/shared/help_00237B00.c`: `func_L05_0024C558` (`FUN_L05_0024bdc0`)
- `src/overlays/l11_pokitaru/vendor_00312BD8.c`: `func_L11_00319108` (`FUN_L11_00317c98`)
- `src/overlays/l10_orxon/vendor_00296BD8.c`: `func_L10_002DB688` (`FUN_L10_002da2c8`)
- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FABE8` (`FUN_L01_002f9810`)

Three more were adapted by hand on 2026-10-15 in the same way. In parentheses, Lombyte's name.

- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002DCE10` (`FUN_L18_002dba20`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002DD8A8` (`FUN_L18_002dc4b8`)
- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_002EE180` (`FUN_L12_002ece00`)

`include/hero.h` (the hero and the records inside it) and `include/vector.h` (the 16-byte vector) are
adapted from Lombyte's `include/rnc/gameplay/hero.h` and `include/rnc/math/vector.h` at 42adb529, while
Lombyte was MIT licensed. `include/pad.h` (the controller state) is adapted from its
`include/rnc/input/pad_state.h` at the same commit.

Data taken from Lombyte:

- `editor/moby_classes.tsv` (OpenRAC's top level): the moby class names the level editor
  shows, from its `config/overlays/us/names/level-NN.json` ("moby-class-record"
  evidence), which joins each level's class dispatch table with the class
  names of [Wrench](https://github.com/chaoticgd/wrench)'s moby class unpack.

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

## newlib, fdlibm and David M. Gay's dtoa

The game's C library is newlib. Five functions here are its open sources,
as Lombyte reconstructed them for the US build and adapted above:

- `src/core/00112468.c`: `func_001126D8` (`_dtoa_r`), David M. Gay's dtoa:

```
The author of this software is David M. Gay.

Copyright (c) 1991 by AT&T.

Permission to use, copy, modify, and distribute this software for any
purpose without fee is hereby granted, provided that this entire notice
is included in all copies of any software which is or includes a copy
or modification of this software and in all copies of the supporting
documentation for such software.

THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
```

- `src/core/00114518.c`: `func_00114920` (`_malloc_r`), newlib's allocator, which
  is Doug Lea's malloc (dlmalloc 2.6.5), released by its author to the public
  domain.

- `src/core/00116FA0.c`: `func_00117118` (`_vfprintf_r`), adapted from
  Lombyte's newlib 1999 `vfprintf.c` reconstruction in
  `src/assembly/runtime/newlib/vfprintf_r.c`. Its macros are expanded into
  ordinary C; its symbols and stdio types use this checkout's PAL definitions.
  The Berkeley notice from Lombyte's `licenses/COPYING.NEWLIB.txt` follows:

```
Copyright (c) 1981-2000 The Regents of the University of California.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the University nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
```

- `src/core/00116070.c`: `func_00116168` and `func_001161B0`, fdlibm's finite
  and NaN tests:

```
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.

Developed at SunPro, a Sun Microsystems, Inc. business.
Permission to use, copy, modify, and distribute this
software is freely granted, provided that this notice
is preserved.
```

## ReRAC

[ReRAC](https://github.com/re-rac/rerac), a native PC port of the US build,
documents the game's formats and systems. The following adapt its format
code and notes, or quote it:

- `editor/mobys.py` (OpenRAC's top level): the moby instance record and what the level
  loader does with each field, from `crates/rc-formats/src/gameplay.rs` and
  `docs/plan/moby_render_notes.md`; described in `docs/ASSETS.md` ("Mobys").
- `editor/moby_class.py`: the moby class header and model format
  (packets, vertex cache, skinning slots, normals, untextured faces), from
  `docs/formats/moby_rac1.md` §1-§2 and `crates/rc-formats/src/moby.rs`;
  described in `docs/ASSETS.md` ("Moby classes").
- `editor/moby_anim.py`: moby skeletons and animation sequences,
  from `docs/formats/moby_rac1.md` §3-§4, `docs/plan/moby_animation.md` and
  `crates/rc-formats/src/moby_anim.rs`.
- `editor/collision.py`: the collision block (cell tree, packed
  vertices, faces and surface bytes), from `docs/formats/collision_rac1.md`,
  `docs/plan/collision_queries.md` and `crates/rc-formats/src/collision.rs`;
  described in `docs/ASSETS.md` ("Collision").
- `config/overlays/rerac_notes.tsv`: names and notes quoted from its
  `tools/ghidra/names/doc_names.csv` (the commit is in the file's header),
  shown in worker packets.

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

## Wrench

[Wrench](https://github.com/chaoticgd/wrench) by chaoticgd and contributors
provides reverse-engineered asset decoders, level modding tools, and C++ type
definitions for moby instance private variables (`pvars`). The following adapt
its source code and reverse-engineering findings:

- `include/moby_pvars.h`: moby instance private variable (pvar) layouts, NPC
  state machine structs (`npcVars`, `npcStep`, `npcstring`), and IK / procedural
  structures (`Manipulator`, `Tweaker`), adapted from
  `data/overlay/src/game_common/` and `data/overlay/src/game_rac/` for C
  compilation with SN ProDG.
- `docs/PVARS.md`: documentation of moby pvar layouts and subsystem
  architecture synthesized from Wrench's overlay definitions.
- `tools/extract/`: asset extraction logic (disc layout, table of contents
  groups, level data header sections, core block bounds, WAD compression,
  terrain, ties, shrubs, sky, gameplay block boundaries), as described in
  `docs/ASSETS.md`.
- `tools/extract/moby_classes.tsv`: moby class names and categories joined with
  Wrench's `data/underlay/rac_moby_classes.asset`.

GNU General Public License v3.0 or later (GPL-3.0-or-later)

Copyright (C) 2019-2023 chaoticgd and contributors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Forty-nine more were ported on 2026-10-14 from Lombyte's matched C and written as plain C (no `__asm__` symbol
aliases, no inline-asm helpers): every symbol renamed to its PAL address, and each passed this project's own
check. In parentheses, Lombyte's name.

- `src/overlays/l01_novalis/vendor_002FABE8.c`: `func_L01_002FABE8` (`FUN_L01_002f9810`)
- `src/overlays/l03_kerwan/vendor_002CB280.c`: `func_L03_002DD158` (`FUN_L03_002dbd90`)
- `src/overlays/l04_eudora/mobyutil_0024D4A8.c`: `func_L04_0024D4A8` (`FUN_L04_0024c4f0`)
- `src/overlays/l04_eudora/vuchain_00293490.c`: `func_L04_00293578` (`FUN_L04_002923b8`)
- `src/overlays/l06_blarg/vendor_002B5990.c`: `func_L06_002EB2D0` (`FUN_L06_002e9ea0`)
- `src/overlays/l06_blarg/vendor_002FE5D0.c`: `func_L06_003045A0` (`FUN_L06_00303160`)
- `src/overlays/l08_batalia/vendor_002EAF48.c`: `func_L08_00308A00` (`FUN_L08_00307540`)
- `src/overlays/l10_orxon/vendor_00296BD8.c`: `func_L10_002DB688` (`FUN_L10_002da2c8`)
- `src/overlays/l11_pokitaru/vendor_002CC828.c`: `func_L11_0030F4F8` (`FUN_L11_0030e088`)
- `src/overlays/l11_pokitaru/vendor_002CC828.c`: `func_L11_00310BF0` (`FUN_L11_0030f728`)
- `src/overlays/l11_pokitaru/vendor_00312BD8.c`: `func_L11_00317500` (`FUN_L11_00316090`)
- `src/overlays/l12_hoven/vendor_002EDAA0.c`: `func_L12_00304038` (`FUN_L12_00302c58`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_0029C1D0` (`FUN_L15_0029aff0`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002AB4A8` (`FUN_L15_002aa280`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002C62A8` (`FUN_L15_002c4f88`)
- `src/overlays/l15_quartu/vendor_0029C1D0.c`: `func_L15_002ED3A0` (`FUN_L15_002ebfb0`)
- `src/overlays/l18_veldin2/vendor_002A8400.c`: `func_L18_002F1780` (`FUN_L18_002f0390`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FABE0` (`FUN_L18_002f9780`)
- `src/overlays/l18_veldin2/vendor_002F9D48.c`: `func_L18_002FDCA0` (`FUN_L18_002fc7e0`)
- `src/overlays/shared/help_00203E98.c`: `func_L00_00209940` (`FUN_L00_002092c8`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020D3A0` (`FUN_L00_0020cd08`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_0020DC68` (`FUN_L00_0020d5d0`)
- `src/overlays/shared/help_0020CDF0.c`: `func_L00_002147C0` (`FUN_L00_00214108`)
- `src/overlays/shared/help_00232560.c`: `func_L00_002346C0` (`FUN_L00_00233e40`)
- `src/overlays/shared/hud_00235960.c`: `func_L00_0023AC68` (`FUN_L00_0023a2d0`)
- `src/overlays/shared/mobyutil_00258BC8.c`: `func_L00_0025F450` (`FUN_L00_0025e3f8`)
- `src/overlays/shared/mobyutil_0026E8E0.c`: `func_L01_0026EFB8` (`FUN_L01_0026e008`)
- `src/overlays/shared/mobyutil_0026E8E0.c`: `func_L01_00278FA8` (`FUN_L01_00277fb8`)
- `src/overlays/shared/mobyutil_0028AA68.c`: `func_L05_0028AA68` (`FUN_L05_002899f8`)
- `src/overlays/shared/pause_00277208.c`: `func_L00_002788E0` (`handle_menu_screen_buttons`)
- `src/overlays/shared/pause_00277208.c`: `func_L00_00286498` (`FUN_L00_002851c0`)
- `src/overlays/shared/stream_002670E0.c`: `func_L00_00267290` (`FUN_L00_00266448`)
- `src/overlays/shared/stream_002670E0.c`: `func_L00_00267618` (`FUN_L00_002667d0`)
- `src/overlays/shared/stream_002670E0.c`: `func_L00_00267BA8` (`FUN_L00_00266d60`)
- `src/overlays/shared/vendor_00292AC0.c`: `func_L03_002DDDF8` (`FUN_L03_002dca30`)
- `src/overlays/shared/vendor_002A5218.c`: `func_L02_002DDAE8` (`FUN_L02_002dc6b0`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002B4BF0` (`FUN_L14_002b39a0`)
- `src/overlays/shared/vendor_002B2A28.c`: `func_L14_002DFA70` (`FUN_L14_002de670`)
- `src/overlays/shared/vendor_002B8FC0.c`: `func_L13_002E3010` (`FUN_L13_002e1cc8`)
- `src/overlays/shared/vendor_002C96D0.c`: `func_L00_002CD110` (`FUN_L00_002cbcc0`)
- `src/overlays/shared/vendor_002CF2C0.c`: `func_L05_002F62E8` (`FUN_L05_002f4f60`)
- `src/overlays/shared/vendor_002CF2C0.c`: `func_L05_0031AAA8` (`FUN_L16_002e4be0`)
- `src/overlays/shared/vendor_002D1168.c`: `func_L00_002D7298` (`FUN_L00_002d5de8`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E74B0` (`FUN_L00_002e6000`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E91D0` (`FUN_L00_002e7d20`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002E9870` (`FUN_L00_002e83c0`)
- `src/overlays/shared/vendor_002E1660.c`: `func_L00_002EA4C0` (`FUN_L00_002e9010`)
- `src/overlays/shared/vendor_002F7700.c`: `func_L01_00303810` (`FUN_L01_00302438`)
- `src/overlays/shared/vendor_002FF000.c`: `func_L06_00309170` (`FUN_L06_00307d30`)
