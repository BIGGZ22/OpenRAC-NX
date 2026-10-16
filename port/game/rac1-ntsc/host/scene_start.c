/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank (NTSC-U): host C this version needs alone, in its own names (not the shared
 * C's: port/game/rac1/host). */
#include "game_protos.h"
#include "openrac/game_host.h"

/*
 * The in-game cutscene's start (PAL func_L00_00299B68, port/game/rac1-pal/hand/level_scene_start.c),
 * written from this version's instructions (FUN_L00_00298840, 728 bytes). A scene trigger's request
 * ends here with the scene's index: it clears the scene's state (D_L00_0016C860 and five 0x40-byte
 * blocks), sets the game mode D_L00_0015F5C4 to 2, hides the HUD, flags the mobys of classes 0x4A and
 * 0xCB, fades from black, queues the scene's music track (+0x1C of the music stream state at
 * D_001516D0) and waits a frame at a time for the stream (+0x5A == 3), then runs the scene's first
 * tick and waits out its counter (+0x48, set to -3). Unlike PAL's, NTSC-U's neither tests the game
 * mode first nor saves and clears the HUD byte, and its frame counter is at +0x48 (PAL +0x46).
 *
 * Host C because the NTSC decomp has no assembly stub for it to be replaced by hand C (its level
 * code comes in whole); a stub here kept every in-game scene from starting (the level's opening
 * scene on Veldin among them).
 */
void game_FUN_L00_00298840(int index) {
    const gaddr hero = 0x0013F350u;
    const gaddr scene = OPENRAC_LDATA(0, 0x0016C860u);
    if (index < 0) {
        return;
    }
    if (GREF(int, hero + 0x22A8) == 0) {
        game_FUN_L00_00211250();
        return;
    }
    game_music_pause(0);
    game_FUN_002168a8(0);
    GREF(int, OPENRAC_LDATA(0, 0x0015F5D8u)) = 1;
    func_001F97E8(scene, 0, 0x1C0);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C440u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C340u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C380u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C400u), 0, 0x40);
    func_001F97E8(OPENRAC_LDATA(0, 0x0017C3C0u), 0, 0x40);
    const gaddr header = OPENRAC_LDATA(0, 0x00173E00u);
    const int scratch = GREF(int, OPENRAC_LDATA(0, 0x001611CCu)) - 0x3C000;
    GREF(int, scene + 0x30) = index;
    GREF(int, scene + 0x5C) = GREF(int, header + 8) + scratch;
    GREF(int, scene + 0x58) = GREF(int, header + 4) + scratch;
    GREF(float, OPENRAC_LDATA(0, 0x0015F59Cu)) = 1.0f;
    GREF(int, OPENRAC_LDATA(0, 0x0015F5A0u)) = 0;
    GREF(int, scene + 0x34) = 0;
    GREF(int, scene + 0x3C) = 0;
    GREF(int, OPENRAC_LDATA(0, 0x001611CCu)) = scratch;
    game_FUN_L00_002039a0();
    GREF(float, OPENRAC_LDATA(0, 0x0015F3FCu)) = 1.0f;
    GREF(int, OPENRAC_LDATA(0, 0x0015F5C4u)) = 2;
    GREF(int, OPENRAC_LDATA(0, 0x0015F400u)) = 0;
    game_FUN_L00_002223f8(0x64, 2);
    GREF(uint8_t, hero + 0x20A5) = 1;
    game_FUN_L00_00232fe8();
    const gaddr trigger = GREF(gaddr, OPENRAC_LDATA(0, 0x00179108u));
    if (trigger != 0) {
        GREF(uint16_t, trigger + 0x34) |= 1;
    }
    const gaddr other = GREF(gaddr, OPENRAC_LDATA(0, 0x001B9DDCu));
    if (other != 0) {
        GREF(uint16_t, other + 0x34) |= 1;
    }
    game_FUN_L00_002451b8(0);
    game_FUN_001f4a58(game_FUN_001f96f8(6));
    game_FUN_002168a8(1);
    game_FUN_00204790(0);
    for (gaddr p = GREF(gaddr, OPENRAC_LDATA(0, 0x0015FFDCu)); GREF(uint8_t, p + 0x20) != 0xFF; p += 0x100) {
        const int16_t cls = GREF(int16_t, p + 0xA6);
        if (!(GREF(uint8_t, p + 0x20) & 0x80) && (cls == 0x4A || cls == 0xCB)) {
            GREF(uint16_t, p + 0x34) |= 0x80;
        }
    }
    const gaddr music = 0x001516D0u;
    GREF(int, music + 0x1C) = GREF(int, scene + 0x30);
    while (GREF(int16_t, music + 0x5A) != 3) {
        game_FUN_0022ca50();
        game_sceGsSyncV(0);
    }
    GREF(int16_t, scene + 0x48) = -3;
    game_FUN_00215b10();
    game_FUN_L00_002451b8(1);
    for (int i = 0; GREF(int16_t, scene + 0x48) < i; --i) {
        game_sceGsSyncV(0);
    }
}
