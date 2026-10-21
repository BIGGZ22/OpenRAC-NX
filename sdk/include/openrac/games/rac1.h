/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank's hook points (docs/port/MODS.md, section 4): for each, its name, its signature
 * as openrac_api.hook takes it, and the type of a replacement. Each disc's table of them is the
 * `hook` rows of port/game/rac1-<version>/libraries.tsv; every hook point here is on both discs
 * (PAL and NTSC-U) unless it says otherwise. The comments say what the game's function does; the
 * addresses are the PAL disc's (games/rac1's names).
 *
 * Parameters are as the port translates the game: u32 is a game address or an unsigned word, i32 an
 * int, f32 a float.
 */
#ifndef OPENRAC_GAMES_RAC1_H
#define OPENRAC_GAMES_RAC1_H

#include "openrac/openrac_mod.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pad_ProcessReport (func_002181F0): one pad report into the pad's state (PadState, at pad), each
 * poll, before the tick reads it. */
#define OPENRAC_RAC1_HOOK_Pad_ProcessReport "Pad_ProcessReport"
#define OPENRAC_RAC1_SIG_Pad_ProcessReport "void(u32,u32,i32)"
typedef void (*openrac_rac1_Pad_ProcessReport)(openrac_gaddr pad, openrac_gaddr report, int32_t size);

/* Hero_UpdateMotionInput (func_L00_00211D28): the stick read into the hero, early in his update
 * each tick. */
#define OPENRAC_RAC1_HOOK_Hero_UpdateMotionInput "Hero_UpdateMotionInput"
#define OPENRAC_RAC1_SIG_Hero_UpdateMotionInput "void()"
typedef void (*openrac_rac1_Hero_UpdateMotionInput)(void);

/* Hero_TurnToHeading (func_L00_002124E8): eases the hero's facing (+0x98) toward his move yaw
 * (+0x180). */
#define OPENRAC_RAC1_HOOK_Hero_TurnToHeading "Hero_TurnToHeading"
#define OPENRAC_RAC1_SIG_Hero_TurnToHeading "void(i32,f32,f32,f32)"
typedef void (*openrac_rac1_Hero_TurnToHeading)(int32_t mode, float accel, float damping, float limit);

/* Hero_SetPlanarVelocity (func_L00_00212790): the hero's velocity at his ground speed along an
 * angle; an angle above pi means his facing. */
#define OPENRAC_RAC1_HOOK_Hero_SetPlanarVelocity "Hero_SetPlanarVelocity"
#define OPENRAC_RAC1_SIG_Hero_SetPlanarVelocity "void(f32)"
typedef void (*openrac_rac1_Hero_SetPlanarVelocity)(float angle);

/* FollowCamera_ApplyOverrides (func_L00_002E74B0): the follow camera's per-tick requests from the
 * hero's state; the camera moby's update calls it before it places the eye. */
#define OPENRAC_RAC1_HOOK_FollowCamera_ApplyOverrides "FollowCamera_ApplyOverrides"
#define OPENRAC_RAC1_SIG_FollowCamera_ApplyOverrides "void(u32)"
typedef void (*openrac_rac1_FollowCamera_ApplyOverrides)(openrac_gaddr camera);

/* Hero_AdvanceAnimation (func_L00_00232EF0): Ratchet's animation moved on by his animation speed,
 * once a tick after his state's update. */
#define OPENRAC_RAC1_HOOK_Hero_AdvanceAnimation "Hero_AdvanceAnimation"
#define OPENRAC_RAC1_SIG_Hero_AdvanceAnimation "void()"
typedef void (*openrac_rac1_Hero_AdvanceAnimation)(void);

/* Item_Give (func_L00_002618D8, rac1.item.give): gives the player item id (GiveItem): its owned
 * and obtained bytes, its banner, its starting ammo, its vendor stock mark and the quick select;
 * equip non-zero also puts it in hand. Every item the game gives goes through it but the
 * Persuader's. */
#define OPENRAC_RAC1_HOOK_Item_Give "Item_Give"
#define OPENRAC_RAC1_SIG_Item_Give "void(i32,i32)"
typedef void (*openrac_rac1_Item_Give)(int32_t id, int32_t equip);

/* Planet_Unlock (func_L00_00261848, rac1.planet.unlock): unlocks a planet's coordinates
 * (UnlockPlanet): its flag, the galaxy list, and its banner unless it is the planet the player
 * is on. */
#define OPENRAC_RAC1_HOOK_Planet_Unlock "Planet_Unlock"
#define OPENRAC_RAC1_SIG_Planet_Unlock "void(i32)"
typedef void (*openrac_rac1_Planet_Unlock)(int32_t planet);

/* Vendor_InitLevelStock (func_L00_0023DB30, rac1.vendor.init_level_stock): the vendor's stock at
 * each level start (VendorStockLevelInit): the Bomb Glove while the player has none, the level's
 * new weapon. */
#define OPENRAC_RAC1_HOOK_Vendor_InitLevelStock "Vendor_InitLevelStock"
#define OPENRAC_RAC1_SIG_Vendor_InitLevelStock "void()"
typedef void (*openrac_rac1_Vendor_InitLevelStock)(void);

/* Vendor_BuildItemList (func_L00_0029B770, rac1.vendor.build_item_list): the vendor menu's list
 * from the stock and the items owned (VendorBuildItemList), each time it opens and after each
 * purchase; skip_unflagged leaves out what is for sale. */
#define OPENRAC_RAC1_HOOK_Vendor_BuildItemList "Vendor_BuildItemList"
#define OPENRAC_RAC1_SIG_Vendor_BuildItemList "void(i32)"
typedef void (*openrac_rac1_Vendor_BuildItemList)(int32_t skip_unflagged);

/* GoldBolts_CountCollected (func_002160E0, rac1.gold_bolt.count_collected): the gold bolts
 * collected, at most 40 (count_nonzero_entries_up_to_40): the pause screen's count; the unspent
 * ones are this less 4 per gold weapon. */
#define OPENRAC_RAC1_HOOK_GoldBolts_CountCollected "GoldBolts_CountCollected"
#define OPENRAC_RAC1_SIG_GoldBolts_CountCollected "i32()"
typedef int32_t (*openrac_rac1_GoldBolts_CountCollected)(void);

/* Take a hook point by its short name: OPENRAC_RAC1_HOOK(api, mod, Hero_TurnToHeading, my_turn,
 * &s_turn_original). */
#define OPENRAC_RAC1_HOOK(api, mod, point, replacement, original)                                  \
    (api)->hook((mod), OPENRAC_RAC1_HOOK_##point, OPENRAC_RAC1_SIG_##point, (void*)(replacement),  \
                (void**)(original))

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_GAMES_RAC1_H */
