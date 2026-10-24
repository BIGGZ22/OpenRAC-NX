/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank (NTSC-U, SCUS-97199): the names and addresses of the shared host C
 * (port/game/rac1/host, rac1.h) in this version: games/rac1's name of each function the shared C
 * calls or defines, and the NTSC decomp's name of the same function; games/rac1's address of each
 * datum, and this version's. Paired by port/tools/versionmap (same instructions, or the same
 * instruction forming the address); the part below the mark is written and checked by
 * port/tools/versionmap/hostnames.py.
 */
#ifndef OPENRAC_RAC1_VERSION_H
#define OPENRAC_RAC1_VERSION_H

/* What this version leaves out of the shared C (rac1.h, RAC1_OMIT), and why. */
/* Level 0's 0x2EC6B8: the NTSC decomp's catalogue places its func_001E9448 there, which returns 0. */
#define RAC1_OMIT_func_L00_002EDB68 1
/* FUN_L00_0025b6b8: the NTSC decomp's C takes the two vectors by address already. */
#define RAC1_OMIT_func_L00_0025C710 1
/* The boot loop draws its picture with draw_bootImage (FUN_002012b8), which boot_picture.c wraps. */
#define RAC1_OMIT_func_001F7680 1
/* FUN_0023a2c0, the pause menu mobys' update: the NTSC decomp has C for it. */
#define RAC1_OMIT_game_D_0023B578 1
/* Variants (variants.c) the NTSC decomp's catalogue names apart, each with C of its own. */
#define RAC1_OMIT_func_L00_00232A00 1 /* FUN_L00_002321a8 */
#define RAC1_OMIT_func_L00_0020A870 1 /* FUN_L00_0020a1f8, FUN_L00_0020a210, FUN_L00_0020a228 */
#define RAC1_OMIT_func_L01_002A2B68 1 /* FUN_L01_002a1968 */
#define RAC1_OMIT_func_L00_002E9A88 1 /* FUN_L00_002e85d8 */
#define RAC1_OMIT_func_L05_0031AAA8 1 /* FUN_L05_00319598, FUN_L16_002e4be0 */
#define RAC1_OMIT_func_L12_002C08A0 1 /* FUN_L12_002bf6d0 */
#define RAC1_OMIT_func_L13_0030AD88 1 /* FUN_L13_003098d0 */
#define RAC1_OMIT_func_L00_00261448 1 /* FUN_L00_002603d0 */
/* PlayMovieEntryIfIdle's second copy in levels 2, 8 and 16 (variants.c): PAL's own, not mapped for
 * this disc yet. */
#define RAC1_OMIT_func_L02_00299F28 1
/* Entries the version map cannot pair, checked by hand. */
/* sceVu0Normalize, an entry inside the VU0 library object, which the map does not cut out. */
#define func_001252C0 game_FUN_00125180
/* sceVu0ScaleVector and sceVu0UnitMatrix: the US game has C for the matrix (sceVu0UnitMatrix) and
 * translates the scale (0x1251E8) from its assembly, so rac1/host's versions are PAL's only. */
#define RAC1_OMIT_func_00125328 1
#define RAC1_OMIT_func_00125358 1

/* The items the US game's Options list is given (Language, Sound: its enter puts them there each
 * time); PAL's list, 0x1D4810 here (Language, Video, Sound), is still in the data, unused. */
#define RAC1_ADDR_0x001D4B90u 0x001D4840u

/* Where this version's decompilation declares a function otherwise: the type in its prototypes. */
/* ParseBin's result, the loaded level's entry. */
#define RAC1_TYPE_func_0012DA38_result long long
/* DrawTexturedQuad's colour and TEX0. */
#define RAC1_TYPE_func_001F5800_u64 long long
/* The text lookup's result, a string's address. */
#define RAC1_TYPE_func_001FE540_result int
/* The sprite's TEX0 (FUN_00200468 here, PAL's func_00200CA0). */
#define RAC1_TYPE_func_00200CA0_tex0 unsigned long long
/* The hero's position (PAL gHeroPos D_0013F4D0: the hero record D_0013F450, here 0x0013F350,
 * plus 0x80), for crash reports (boot.c). */
#define RAC1_HERO_POS 0x0013F3D0u
/* The moby update loop (crash_state.c): PAL func_00213C78 is FUN_00212e28 here (its hand C,
 * hand/functional/FUN_00212e28.c, checked against the US assembly), with the list's head
 * (FUN_0020d868, kept in D_0015FF24) and the steps before and after each update (FUN_0020d580,
 * FUN_0020def8). */
#define func_00213C78 game_FUN_00212e28
#define func_0020E6B8 game_FUN_0020d868
#define func_0020E3D0 game_FUN_0020d580
#define func_0020ED48 game_FUN_0020def8
#define RAC1_ADDR_0x00160024u 0x0015FF24u
/* A scene's actors (scene_actors.c): PAL's UpdateWorldObjectAnimation func_00204FC0 is FUN_00204790
 * here (the NTSC decomp's src/world/animation/update_world_object_animation.c), CreateMoby
 * func_0020D348 is FUN_0020c4f8 and FlushCache keeps its name; the version map pairs each as `same`
 * and the data below as `exact`. The US routine takes no parameter, and keeps the model record's
 * word at +8 at +0x48 of the transfer state (PAL +0x46). */
#define func_00204FC0 game_FUN_00204790
#define func_0020D348 game_FUN_0020c4f8
#define func_00118D80 game_FlushCache
#define RAC1_TYPE_func_00204FC0_params void
#define RAC1_TRANSFER_PAD08 0x48
#define RAC1_ADDR_0x0018CC20u 0x0018CB20u
#define RAC1_ADDR_0x00160588u 0x00160488u
#define RAC1_ADDR_0x0013E130u 0x0013E030u
/* The level loader (moby_pool.c): PAL's LoadLevelGameplay func_L00_002422D8 is FUN_L00_00241940 here
 * (the NTSC decomp's src/overlays/shared/world/loaders/load_level_gameplay.c, ported from PAL's;
 * libraries.tsv pairs the two), a hook point under the wrap; its load state, PAL D_L00_00173F00,
 * is D_L00_00173E00 here (the same C reads the header from +4 of each). */
#define func_L00_002422D8 game_FUN_L00_00241940
#define func_L00_002422D8__hooked game_FUN_L00_00241940__hooked
#define RAC1_ADDR_L00_0x00173F00u 0x00173E00u
/* The run list's builder (moby_pool.c) is func_0020E6B8 above (FUN_0020d868); its group member lists
 * and target pointer table, PAL D_001C7AC0 and D_001C7C80 (the US build's own C of FUN_0020d868
 * forms these two). */
#define RAC1_ADDR_0x001C7AC0u 0x001C7740u
#define RAC1_ADDR_0x001C7C80u 0x001C7900u
/* The game's options the port remembers (options.c): PAL memcard_RestoreGame func_00209CE8 is
 * FUN_00209298 here and initialize_gameplay_sound_system func_0022DBE8 is FUN_0022c8d0 (the NTSC
 * decomp's memcard_restore_game.c and initialize_gameplay_sound_system.c); the language, PAL
 * D_0015EE88, and the playback mode, music and sound volumes, PAL D_0015EEE8..EEF0, are its
 * game_language and playback_mode, music_volume, sound_volume (include/rnc/globals.h,
 * gameplay/state/options.h). */
#define func_00209CE8 game_FUN_00209298
#define func_00209CE8__game game_FUN_00209298__game
#define func_0022DBE8 game_FUN_0022c8d0
#define func_0022DBE8__game game_FUN_0022c8d0__game
#define RAC1_TYPE_func_00209CE8_save gaddr
#define RAC1_ADDR_0x0015EE88u 0x0015ED88u
#define RAC1_ADDR_0x0015EEE8u 0x0015EDE8u

/* ---- generated below ---- */

/* Functions: games/rac1's name, this version's. */
#define func_00116078 game_printf
#define func_00116248 game_sprintf
#define func_0012DA38 game_FUN_0012d8f8
#define func_0012DA38__game game_FUN_0012d8f8__game
#define func_0012DDC0 game_FUN_0012dc80
#define func_001EFE10 game_FUN_001efa68
#define func_001F4630 game_FUN_001f4280
#define func_001F4630__game game_FUN_001f4280__game
#define func_001F4868 game_FUN_001f44b8
#define func_001F4A00 game_FUN_001f4650
#define func_001F4A00__game game_FUN_001f4650__game
#define func_001F4A78 game_FUN_001f46c8
#define func_001F4A78__game game_FUN_001f46c8__game
#define func_001F4AF0 game_FUN_001f4740
#define func_001F4AF0__game game_FUN_001f4740__game
#define func_001F4BB8 game_FUN_001f4808
#define func_001F4BB8__game game_FUN_001f4808__game
#define func_001F5800 game_FUN_001f5450
#define func_001F5800__game game_FUN_001f5450__game
#define func_001F6668 game_FUN_001f62b0
#define func_001F6668__game game_FUN_001f62b0__game
#define func_001F7070 game_FUN_001f7090
#define func_001F7560 game_FUN_001f7580
#define func_001F75D0 game_FUN_001f75f0
#define func_001F7C60 game_FUN_001f7a98
#define func_001F7DD8 func_001F7C10
#define func_001F7E98 func_001F7CD0
#define func_001F7EF8 game_FUN_001f7d30
#define func_001F84AC game_FUN_001f82e4
#define func_001F852C game_FUN_001f8364
#define func_001F856C game_FUN_001f83a4
#define func_001F8B6C game_FUN_001f89a4
#define func_001F91B8 game_FUN_001f8ff0
#define func_001F9B88 func_001F99C0
#define func_001F9F90 game_FUN_001f9dc8
#define func_001F9FA8 game_FUN_001f9de0
#define func_001FA748 game_FUN_001fa580
#define func_001FA790 game_FUN_001fa5c8
#define func_001FA898 game_FUN_001fa6d0
#define func_001FE540 game_FUN_001fdd10
#define func_001FE540__game game_FUN_001fdd10__game
#define func_001FFFB8 game_FUN_001ff780
#define func_001FFFB8__game game_FUN_001ff780__game
#define func_00200248 game_FUN_001ffa10
#define func_00200468 game_FUN_001ffc30
#define func_00200468__game game_FUN_001ffc30__game
#define func_00200650 game_FUN_001ffe18
#define func_00200650__game game_FUN_001ffe18__game
#define func_002008B8 game_FUN_00200080
#define func_002008B8__game game_FUN_00200080__game
#define func_00200A90 game_FUN_00200258
#define func_00200A90__game game_FUN_00200258__game
#define func_00200CA0 game_FUN_00200468
#define func_00200CA0__game game_FUN_00200468__game
#define func_00200E38 game_FUN_00200600
#define func_00200E38__game game_FUN_00200600__game
#define func_00201190 game_FUN_00200958
#define func_00201190__game game_FUN_00200958__game
#define func_00201AF0 game_FUN_002012b8
#define func_00201AF0__game game_FUN_002012b8__game
#define func_00204340 game_FUN_00203b08
#define func_00209DC0 game_FUN_00209370
#define func_00209DC0__game game_FUN_00209370__game
#define func_0020C268 func_0020B418
#define func_0020C468 game_FUN_0020b618
#define func_0020DD48 game_FUN_0020cef8
#define func_0020FC38 game_FUN_0020ede8
#define func_00211808 game_FUN_002109b8
#define func_00212258 game_FUN_00211408
#define func_00212508 game_FUN_002116b8
#define func_00212578 game_FUN_00211728
#define func_00212658 game_FUN_00211808
#define func_002167C0 game_FUN_00215970
#define func_00218B10 game_FUN_00217c18
#define func_00228A58 game_FUN_00227740
#define func_00228D20 game_FUN_00227a08
#define func_00229098 game_FUN_00227d80
#define func_002291E8 game_FUN_00227ed0
#define func_00229838 game_FUN_00228520
#define func_002298B0 game_FUN_00228598
#define func_00229D48 game_FUN_00228a30
#define func_00229F00 game_FUN_00228be8
#define func_0022B648 game_FUN_0022a330
#define func_0022B8F8 game_FUN_0022a5e0
#define func_0022C9A8 game_FUN_0022b690
#define func_0022C9A8__game game_FUN_0022b690__game
#define func_0022CA00 game_FUN_0022b6e8
#define func_0022CC40 game_FUN_0022b928
#define func_0022CEB8 game_FUN_0022bba0
#define func_0022D2AC game_FUN_0022bf94
#define func_0022D3F8 game_FUN_0022c0e0
#define func_0022D520 game_FUN_0022c208
#define func_0022D7E0 game_FUN_0022c4c8
#define func_0022ED80 game_FUN_0022da68
#define func_0022F4A0 game_InitializeGlobalStateEntry
#define func_00230A90 game_FUN_0022f778
#define func_00230A90__game game_FUN_0022f778__game
#define func_00234620 game_FUN_00233308
#define func_002347F0 game_FUN_002334d8
#define func_00235118 func_00233E00
#define func_002352C8 game_FUN_00233fb0
#define func_00235EF0 game_FUN_00234bd8
#define func_00236060 game_FUN_00234d48
#define func_002362B0 game_FUN_00234f98
#define func_00236A98 game_FUN_00235780
#define func_00236F00 game_FUN_00235be8
#define func_002383D8 game_FUN_002370c0
#define func_00238688 game_FUN_00237370
#define func_0023B670 game_FUN_0023a3b8
#define func_L00_001EB6A8 game_FUN_L00_001eb328
#define func_L00_001EBD58 func_L00_001EB9D8
#define func_L00_001EFFF0 func_L00_001EFC70
#define func_L00_001F10E0 game_FUN_L00_001f0d60
#define func_L00_001F1D20 game_FUN_L00_001f19a0
#define func_L00_001F2BE8 game_FUN_L00_001f2868
#define func_L00_001F34F0 game_FUN_L00_001f3170
#define func_L00_001FD1D8 func_L00_001FD228
#define func_L00_001FE688 func_L00_001FE6D8
#define func_L00_001FEE50 func_L00_001FEEA0
#define func_L00_001FEE98 func_L00_001FEEE8
#define func_L00_001FEEC0 func_L00_001FEF10
#define func_L00_001FEED0 func_L00_001FEF20
#define func_L00_001FEEF8 func_L00_001FEF48
#define func_L00_001FEF08 func_L00_001FEF58
#define func_L00_00200290 game_FUN_001fa728
#define func_L00_00200290__game game_FUN_001fa728__game
#define func_L00_00201300 func_L00_00201350
#define func_L00_0024F054 func_L00_0024E01C
#define func_L00_00252D20 func_L00_00251CE8
#define func_L00_00254218 game_FUN_0020fa90
#define func_L00_0025D3C0 func_L00_0025C368
#define func_L00_00267080 func_L00_00266238
#define func_L00_002864E0 game_FUN_L00_00285208
#define func_L00_00286598 func_L00_002852C0
#define func_L00_0028D958 func_L00_0028C680
#define func_L00_002963D8 func_L00_00295100
#define func_L00_002A2020 func_L00_002A0D90
#define func_L00_002A2080 func_L00_002A0DF0
#define func_L00_002A20E0 func_L00_002A0E50
#define func_L00_002A2148 func_L00_002A0EB8
#define func_L00_002A8A20 game_FUN_L00_002a7780
#define func_L00_002D8898 game_FUN_L00_002d73e8
#define func_L00_002D9EA0 game_FUN_L00_002d89f0
#define func_L00_002D9EA0__hooked game_FUN_L00_002d89f0__hooked
#define func_L02_0020A690 func_L02_0020A390
#define func_L02_00250720 game_FUN_L02_0024f748
#define func_L02_00250720__game game_FUN_L02_0024f748__game
#define func_L03_00292AC0 game_FUN_L03_00291918
#define func_L03_002DDD58 func_L03_002DC990
#define func_L07_002F9418 func_L07_002F8038
#define func_L07_0030E800 func_L07_0030D420
#define func_L08_002DB448 func_L08_002DA0D0
#define func_L14_0020CAE0 func_L14_0020C760
#define func_L14_003047D8 func_L14_00303350
#define func_L18_00282F48 game_FUN_L18_00281fa8
#define func_L18_002FC168 func_L18_002FAD08

/* Addresses: games/rac1's, this version's (RAC1_LADDR: in level NN's program). */
#define RAC1_ADDR(a) RAC1_ADDR_##a
#define RAC1_LADDR(n, a) RAC1_ADDR_L##n##_##a
#define RAC1_ADDR_0x0010E4C0u 0x0010E4C0u
#define RAC1_ADDR_0x0013CA40u 0x0013C940u
#define RAC1_ADDR_0x0013D010u 0x0013CF10u
#define RAC1_ADDR_0x0013D5C8u 0x0013D4C0u
#define RAC1_ADDR_0x0013E150u 0x0013E050u
#define RAC1_ADDR_0x0013E15Au 0x0013E05Au
#define RAC1_ADDR_0x0013E604u 0x0013E504u
#define RAC1_ADDR_0x0013E610u 0x0013E510u
#define RAC1_ADDR_0x0013E614u 0x0013E514u
#define RAC1_ADDR_0x0013F450u 0x0013F350u
#define RAC1_ADDR_0x001414D0u 0x001413D0u
#define RAC1_ADDR_0x001517D0u 0x001516D0u
#define RAC1_ADDR_0x00151C60u 0x00151B60u
#define RAC1_ADDR_0x0015EDA0u 0x0015ECA0u
#define RAC1_ADDR_0x0015EDA8u 0x0015ECA8u
#define RAC1_ADDR_0x0015EDB0u 0x0015ECB0u
#define RAC1_ADDR_0x0015EDC0u 0x0015ECC0u
#define RAC1_ADDR_0x0015EE60u 0x0015ED60u /* gp-0x7EA0, as PAL's; not checked against the US code */
#define RAC1_ADDR_0x0015EE64u 0x0015ED64u /* gp-0x7E9C, as PAL's; not checked against the US code */
#define RAC1_ADDR_0x0015EE68u 0x0015ED68u /* gp-0x7E98, as PAL's; not checked against the US code */
#define RAC1_ADDR_0x0015EE6Cu 0x0015ED6Cu /* gp-0x7E94, as PAL's; not checked against the US code */
#define RAC1_ADDR_0x0015EE70u 0x0015ED70u /* gp-0x7E90, as PAL's; not checked against the US code */
#define RAC1_ADDR_0x0015EE80u 0x0015ED80u
#define RAC1_ADDR_0x0015EE84u 0x0015ED84u
#define RAC1_ADDR_0x0015EE98u 0x0015ED98u
#define RAC1_ADDR_0x0015EF4Au 0x0015EE4Au
#define RAC1_ADDR_0x0015F000u 0x0015EF00u
#define RAC1_ADDR_0x0015F558u 0x0015F458u
#define RAC1_ADDR_0x0015F56Cu 0x0015F46Cu
#define RAC1_ADDR_0x0015F570u 0x0015F470u
#define RAC1_ADDR_0x0015F690u 0x0015F590u
#define RAC1_ADDR_0x0015F6E8u 0x0015F604u
#define RAC1_ADDR_0x0015F6FCu 0x0015F618u
#define RAC1_ADDR_0x00160018u 0x0015FF18u
#define RAC1_ADDR_0x00160030u 0x0015FF30u /* hand/functional/FUN_001e9b10.c reads it there */
#define RAC1_ADDR_0x0016044Cu 0x0016034Cu
#define RAC1_ADDR_0x001604CCu 0x001603CCu
#define RAC1_ADDR_0x0016055Cu 0x0016045Cu
#define RAC1_ADDR_0x00160FE0u 0x00160EE0u
#define RAC1_ADDR_0x00161000u 0x00160F00u
#define RAC1_ADDR_0x00166D00u 0x00166C00u
#define RAC1_ADDR_0x00187040u 0x00186F40u
#define RAC1_ADDR_0x00187180u 0x00187080u
#define RAC1_ADDR_0x00187390u 0x00187290u
#define RAC1_ADDR_0x00187394u 0x00187294u
#define RAC1_ADDR_0x0018A3D0u 0x0018A2D0u
#define RAC1_ADDR_0x0018CE00u 0x0018CD00u
#define RAC1_ADDR_0x0018D140u 0x0018D040u
#define RAC1_ADDR_0x0018E040u 0x0018DF40u
#define RAC1_ADDR_0x0018E140u 0x0018E040u
#define RAC1_ADDR_0x0018E240u 0x0018E140u
#define RAC1_ADDR_0x0018E340u 0x0018E240u
#define RAC1_ADDR_0x00193400u 0x00193300u
#define RAC1_ADDR_0x001940C0u 0x00193FC0u /* the NTSC decomp's occl_visibility */
#define RAC1_ADDR_0x00194200u 0x00194100u
#define RAC1_ADDR_0x001984C0u 0x001983C0u
#define RAC1_ADDR_0x00199C60u 0x00199B60u
#define RAC1_ADDR_0x0019A4E8u 0x0019A3E8u
#define RAC1_ADDR_0x0019BEC0u 0x0019BDC0u
#define RAC1_ADDR_0x001AAF40u 0x001AABC0u
#define RAC1_ADDR_0x001B7A60u 0x001B76E0u
#define RAC1_ADDR_0x001CE720u 0x001CE3A0u
#define RAC1_ADDR_0x001D4B9Cu 0x001D481Cu
#define RAC1_ADDR_0x001D4BE8u 0x001D4868u
#define RAC1_ADDR_0x001D4CC0u 0x001D4940u
#define RAC1_ADDR_0x001D4D48u 0x001D49C8u
#define RAC1_ADDR_0x001D4DE8u 0x001D4A68u
#define RAC1_ADDR_0x001D4E38u 0x001D4AB8u
#define RAC1_ADDR_0x001D4F18u 0x001D4B98u
#define RAC1_ADDR_0x001D4F40u 0x001D4BC0u
#define RAC1_ADDR_0x001D5F70u 0x001D5BF0u
#define RAC1_ADDR_0x001D82C0u 0x001D7F30u
#define RAC1_ADDR_0x001D83C0u 0x001D8030u
#define RAC1_ADDR_0x001D8440u 0x001D80B0u
#define RAC1_ADDR_0x001D9040u 0x001D8CB0u
#define RAC1_ADDR_0x001D9640u 0x001D92B0u
#define RAC1_ADDR_0x001D9A70u 0x001D96E0u
#define RAC1_ADDR_0x001DF3D0u 0x001DF050u
#define RAC1_ADDR_0x001DF770u 0x001DF3F0u
#define RAC1_ADDR_0x001E8F80u 0x001E8C00u
#define RAC1_ADDR_L00_0x0015F6A8u 0x0015F5C4u
#define RAC1_ADDR_L00_0x0015F6B0u 0x0015F5CCu
#define RAC1_ADDR_L00_0x0015F6F8u 0x0015F618u
#define RAC1_ADDR_L00_0x00160098u 0x0015FFD8u
#define RAC1_ADDR_L00_0x0016022Cu 0x0016016Cu
#define RAC1_ADDR_L00_0x00160234u 0x00160174u
#define RAC1_ADDR_L00_0x00161488u 0x001613C0u
#define RAC1_ADDR_L00_0x00173F40u 0x00173E40u
#define RAC1_ADDR_L00_0x00178000u 0x00177F00u
#define RAC1_ADDR_L00_0x00178200u 0x00178100u
#define RAC1_ADDR_L00_0x0019BB60u 0x0019B7E0u
#define RAC1_ADDR_L00_0x001B1C00u 0x001B1880u
#define RAC1_ADDR_L00_0x001BB610u 0x001BB280u
#define RAC1_ADDR_L00_0x001C2840u 0x001C24C0u
#define RAC1_ADDR_L00_0x001C2870u 0x001C24F0u
#define RAC1_ADDR_L00_0x001C3280u 0x001C2F00u
#define RAC1_ADDR_L00_0x001D95E8u 0x001D92E8u
#define RAC1_ADDR_L03_0x001DC460u 0x001DC1A0u
#define RAC1_ADDR_L07_0x001D36C0u 0x001D3400u
#define RAC1_ADDR_L07_0x00208820u 0x00208560u
#define RAC1_ADDR_L08_0x001D7320u 0x001D7060u
#define RAC1_ADDR_L14_0x001E9FA0u 0x001E9C60u
#define RAC1_ADDR_L18_0x001DFED0u 0x001DFC10u

#endif /* OPENRAC_RAC1_VERSION_H */
