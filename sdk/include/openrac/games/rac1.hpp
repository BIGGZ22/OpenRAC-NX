// Generated from sdk/bindings/rac1.json by sdk/tools/gen_binding.py. Do not edit.
// SPDX-License-Identifier: MIT
//
// rac1's hook points as C++ types, for the SDK's C++ layer (openrac/mod.hpp, docs/port/MODS.md):
// each one's permanent id, its name and signature as openrac_api.hook takes them, and the type of a
// function of it. mod.hook<...>() asks for a hook point by its id. The same types by id are in
// openrac::rac1::hooks (rac1.player.turn_to_heading: hooks::player::turn_to_heading).
#pragma once

#include <cstdint>

namespace openrac::rac1 {

// One pad report into the pad's state (PadState, at pad), each poll, before the tick reads it.
struct Pad_ProcessReport {
    static constexpr const char* id = "rac1.pad.process_report";
    static constexpr const char* name = "Pad_ProcessReport";
    static constexpr const char* signature = "void(u32,u32,i32)";
    using fn = void (*)(uint32_t, uint32_t, int32_t);
};

// The stick read into the hero, early in his update each tick.
struct Hero_UpdateMotionInput {
    static constexpr const char* id = "rac1.player.update_motion_input";
    static constexpr const char* name = "Hero_UpdateMotionInput";
    static constexpr const char* signature = "void()";
    using fn = void (*)();
};

// Eases the hero's facing toward his move yaw.
struct Hero_TurnToHeading {
    static constexpr const char* id = "rac1.player.turn_to_heading";
    static constexpr const char* name = "Hero_TurnToHeading";
    static constexpr const char* signature = "void(i32,f32,f32,f32)";
    using fn = void (*)(int32_t, float, float, float);
};

// The hero's velocity at his ground speed along an angle; an angle above pi means his facing.
struct Hero_SetPlanarVelocity {
    static constexpr const char* id = "rac1.player.set_planar_velocity";
    static constexpr const char* name = "Hero_SetPlanarVelocity";
    static constexpr const char* signature = "void(f32)";
    using fn = void (*)(float);
};

// The follow camera's per-tick requests from the hero's state; the camera moby's update calls it before it places the eye.
struct FollowCamera_ApplyOverrides {
    static constexpr const char* id = "rac1.camera.follow.apply_overrides";
    static constexpr const char* name = "FollowCamera_ApplyOverrides";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// Ratchet's animation moved on by his animation speed, once a tick after his state's update.
struct Hero_AdvanceAnimation {
    static constexpr const char* id = "rac1.player.advance_animation";
    static constexpr const char* name = "Hero_AdvanceAnimation";
    static constexpr const char* signature = "void()";
    using fn = void (*)();
};

// Gives the player item id (GiveItem): sets its owned and obtained bytes, shows its banner, gives its starting ammo, marks it bought in the vendor's stock and adds it to the quick select; equip non-zero also puts it in hand. Every item the game gives goes through it but the Persuader's.
struct Item_Give {
    static constexpr const char* id = "rac1.item.give";
    static constexpr const char* name = "Item_Give";
    static constexpr const char* signature = "void(i32,i32)";
    using fn = void (*)(int32_t, int32_t);
};

// Unlocks a planet's coordinates (UnlockPlanet): its flag (one byte per level), the galaxy list, and its banner unless it is the planet the player is on.
struct Planet_Unlock {
    static constexpr const char* id = "rac1.planet.unlock";
    static constexpr const char* name = "Planet_Unlock";
    static constexpr const char* signature = "void(i32)";
    using fn = void (*)(int32_t);
};

// The vendor's stock at each level start (VendorStockLevelInit): gives the Bomb Glove while the player has none, keeps the stock's known items and adds the level's new weapon.
struct Vendor_InitLevelStock {
    static constexpr const char* id = "rac1.vendor.init_level_stock";
    static constexpr const char* name = "Vendor_InitLevelStock";
    static constexpr const char* signature = "void()";
    using fn = void (*)();
};

// The vendor menu's list (VendorBuildItemList), from the stock and the items owned: each time the menu opens and after each purchase. skip_unflagged leaves out what is for sale (the menu opened from the pause screen).
struct Vendor_BuildItemList {
    static constexpr const char* id = "rac1.vendor.build_item_list";
    static constexpr const char* name = "Vendor_BuildItemList";
    static constexpr const char* signature = "void(i32)";
    using fn = void (*)(int32_t);
};

// The gold bolts collected, at most 40 (count_nonzero_entries_up_to_40): what the pause screen shows; the unspent ones are this less 4 per gold weapon.
struct GoldBolts_CountCollected {
    static constexpr const char* id = "rac1.gold_bolt.count_collected";
    static constexpr const char* name = "GoldBolts_CountCollected";
    static constexpr const char* signature = "i32()";
    using fn = int32_t (*)();
};

// Takes hit points off the hero's health (Hero_TakeDamage): at most one a call, none for 0, then the HUD's health bank. Every hit the hero takes goes through it but the instant kills (falls, crushers), which set his health to 0.
struct Hero_TakeDamage {
    static constexpr const char* id = "rac1.player.take_damage";
    static constexpr const char* name = "Hero_TakeDamage";
    static constexpr const char* signature = "void(i32)";
    using fn = void (*)(int32_t);
};

// The hit pending on moby (MobyGetHitMessage): the record of the last hit posted to it (+0x20 the attacker, +0x24 the flags, +0x2C the damage, an f32) when its flags match mask, else 0, dropping the hit unless keep. How the hero and every enemy read the hits they take, each tick, so a hit may be read more than once.
struct Moby_GetHitMessage {
    static constexpr const char* id = "rac1.combat.get_hit";
    static constexpr const char* name = "Moby_GetHitMessage";
    static constexpr const char* signature = "u32(u32,i32,i32)";
    using fn = uint32_t (*)(uint32_t, int32_t, int32_t);
};

// Takes amount of ammo from item for a shot (TakeAmmo; -1 is the item in hand): 1 when taken, 0 when there is too little. An item with no ammo limit takes none and gives 1.
struct Weapon_TakeAmmo {
    static constexpr const char* id = "rac1.weapon.take_ammo";
    static constexpr const char* name = "Weapon_TakeAmmo";
    static constexpr const char* signature = "i32(i32,i32)";
    using fn = int32_t (*)(int32_t, int32_t);
};

// Records a placed moby's death or a crate's break (SetDeathBits): its save id's killed bits (none for a save id below 0), its mission, then drops its bolts: about the s16 at +0xB4 (or what a tallied source has left of +0xB6), which it then clears.
struct Moby_RecordDeath {
    static constexpr const char* id = "rac1.moby.record_death";
    static constexpr const char* name = "Moby_RecordDeath";
    static constexpr const char* signature = "void(u32,i32,i32)";
    using fn = void (*)(uint32_t, int32_t, int32_t);
};

// Loads a level's gameplay file into the level heap (LoadLevelGameplay), once per level start: the settings, the placed mobys and their pvars, the spare moby pool, the paths and the rest; returns the heap cursor. After it, every placed moby exists and none has run.
struct Level_LoadGameplay {
    static constexpr const char* id = "rac1.level.load_gameplay";
    static constexpr const char* name = "Level_LoadGameplay";
    static constexpr const char* signature = "u32(i32)";
    using fn = uint32_t (*)(int32_t);
};

// Eases the hero's ground speed (+0x194, the speed Hero_SetPlanarVelocity moves him at) toward his target speed (+0x190) by up when below it and by down when above, each tick he moves on foot, in every level.
struct Hero_ApproachSpeed {
    static constexpr const char* id = "rac1.player.approach_speed";
    static constexpr const char* name = "Hero_ApproachSpeed";
    static constexpr const char* signature = "void(f32,f32)";
    using fn = void (*)(float, float);
};

// The update of class 1750, each tick; it gets no moby. Experimental: from the behaviour catalogue.
struct Moby_Class1750_Update {
    static constexpr const char* id = "rac1.moby.class_1750.update";
    static constexpr const char* name = "Moby_Class1750_Update";
    static constexpr const char* signature = "void()";
    using fn = void (*)();
};

// The update of class 676, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class676_Update {
    static constexpr const char* id = "rac1.moby.class_676.update";
    static constexpr const char* name = "Moby_Class676_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 482, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class482_Update {
    static constexpr const char* id = "rac1.moby.class_482.update";
    static constexpr const char* name = "Moby_Class482_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 809, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class809_Update {
    static constexpr const char* id = "rac1.moby.texture_scroll.update";
    static constexpr const char* name = "Moby_Class809_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class camera_object (1007), each tick. Experimental: from the behaviour catalogue.
struct Moby_CameraObject_Update {
    static constexpr const char* id = "rac1.moby.camera_object.update";
    static constexpr const char* name = "Moby_CameraObject_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class vendor (11), each tick. Experimental: from the behaviour catalogue.
struct Moby_Vendor_Update {
    static constexpr const char* id = "rac1.moby.light_beam.update";
    static constexpr const char* name = "Moby_Vendor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes bolt (13), bolt_hexnut (14), bolt_starnut (15), bolt_circlenut (16), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bolt_Update {
    static constexpr const char* id = "rac1.moby.bolt.update";
    static constexpr const char* name = "Moby_Bolt_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class wrench (71), each tick. Experimental: from the behaviour catalogue.
struct Moby_Wrench_Update {
    static constexpr const char* id = "rac1.moby.wrench.update";
    static constexpr const char* name = "Moby_Wrench_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (74), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine_Update {
    static constexpr const char* id = "rac1.moby.mine.update";
    static constexpr const char* name = "Moby_Mine_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 112, 1192, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class112_Update {
    static constexpr const char* id = "rac1.moby.spin_fade_out.update";
    static constexpr const char* name = "Moby_Class112_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 121, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class121_Update {
    static constexpr const char* id = "rac1.moby.hero_bomb.update";
    static constexpr const char* name = "Moby_Class121_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 122, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class122_Update {
    static constexpr const char* id = "rac1.moby.falling_spark.update";
    static constexpr const char* name = "Moby_Class122_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 149, nanotech_crate_fragment (348), nanotech_crate_fragment (349), metal_crate_fragment (354) and 7 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_NanotechCrateFragment_Update {
    static constexpr const char* id = "rac1.moby.debris_bounce.update";
    static constexpr const char* name = "Moby_NanotechCrateFragment_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gold_devastator_missile (153), each tick. Experimental: from the behaviour catalogue.
struct Moby_GoldDevastatorMissile_Update {
    static constexpr const char* id = "rac1.moby.missile.update";
    static constexpr const char* name = "Moby_GoldDevastatorMissile_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class visibomb (163), each tick. Experimental: from the behaviour catalogue.
struct Moby_Visibomb_Update {
    static constexpr const char* id = "rac1.moby.moby163.update";
    static constexpr const char* name = "Moby_Visibomb_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 166, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class166_Update {
    static constexpr const char* id = "rac1.moby.moby166.update";
    static constexpr const char* name = "Moby_Class166_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 167, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class167_Update {
    static constexpr const char* id = "rac1.moby.moby167.update";
    static constexpr const char* name = "Moby_Class167_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class visibomb_missile (172), each tick. Experimental: from the behaviour catalogue.
struct Moby_VisibombMissile_Update {
    static constexpr const char* id = "rac1.moby.visibomb_missile.update";
    static constexpr const char* name = "Moby_VisibombMissile_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taunter (175), each tick. Experimental: from the behaviour catalogue.
struct Moby_Taunter_Update {
    static constexpr const char* id = "rac1.moby.moby175.update";
    static constexpr const char* name = "Moby_Taunter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class pyrocitor (176), each tick. Experimental: from the behaviour catalogue.
struct Moby_Pyrocitor_Update {
    static constexpr const char* id = "rac1.moby.pyrocitor.update";
    static constexpr const char* name = "Moby_Pyrocitor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 179, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class179_Update {
    static constexpr const char* id = "rac1.moby.moby179.update";
    static constexpr const char* name = "Moby_Class179_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class walloper (180), each tick. Experimental: from the behaviour catalogue.
struct Moby_Walloper_Update {
    static constexpr const char* id = "rac1.moby.moby180.update";
    static constexpr const char* name = "Moby_Walloper_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class morphoray (185), each tick. Experimental: from the behaviour catalogue.
struct Moby_Morphoray_Update {
    static constexpr const char* id = "rac1.moby.morphoray.update";
    static constexpr const char* name = "Moby_Morphoray_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class doom_bot (186), each tick. Experimental: from the behaviour catalogue.
struct Moby_DoomBot_Update {
    static constexpr const char* id = "rac1.moby.doom_bot.update";
    static constexpr const char* name = "Moby_DoomBot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class trespasser (188), each tick. Experimental: from the behaviour catalogue.
struct Moby_Trespasser_Update {
    static constexpr const char* id = "rac1.moby.moby188.update";
    static constexpr const char* name = "Moby_Trespasser_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine_glove (190), each tick. Experimental: from the behaviour catalogue.
struct Moby_MineGlove_Update {
    static constexpr const char* id = "rac1.moby.mine_glove.update";
    static constexpr const char* name = "Moby_MineGlove_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb_glove (192), each tick. Experimental: from the behaviour catalogue.
struct Moby_BombGlove_Update {
    static constexpr const char* id = "rac1.moby.moby192.update";
    static constexpr const char* name = "Moby_BombGlove_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes decoy (203), gold_decoy (1900), each tick. Experimental: from the behaviour catalogue.
struct Moby_Decoy_Update {
    static constexpr const char* id = "rac1.moby.thrown_object.update";
    static constexpr const char* name = "Moby_Decoy_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes devestator_ammo (204), tesla_claw_ammo (213), pyrocitor_ammo (214), visibomb_ammo (222) and 7 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_DevestatorAmmo_Update {
    static constexpr const char* id = "rac1.moby.ammo_pickup.update";
    static constexpr const char* name = "Moby_DevestatorAmmo_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class swingshot_hook (209), each tick. Experimental: from the behaviour catalogue.
struct Moby_SwingshotHook_Update {
    static constexpr const char* id = "rac1.moby.moby209.update";
    static constexpr const char* name = "Moby_SwingshotHook_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class glove_of_doom (229), each tick. Experimental: from the behaviour catalogue.
struct Moby_GloveOfDoom_Update {
    static constexpr const char* id = "rac1.moby.carrier.update";
    static constexpr const char* name = "Moby_GloveOfDoom_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (230), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb_Update {
    static constexpr const char* id = "rac1.moby.bomb_230.update";
    static constexpr const char* name = "Moby_Bomb_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class morphoray_chicken (270), each tick. Experimental: from the behaviour catalogue.
struct Moby_MorphorayChicken_Update {
    static constexpr const char* id = "rac1.moby.morphoray_chicken.update";
    static constexpr const char* name = "Moby_MorphorayChicken_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 305, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class305_Update {
    static constexpr const char* id = "rac1.moby.moby305.update";
    static constexpr const char* name = "Moby_Class305_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 428, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class428_Update {
    static constexpr const char* id = "rac1.moby.moby428.update";
    static constexpr const char* name = "Moby_Class428_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ryno (454), each tick. Experimental: from the behaviour catalogue.
struct Moby_Ryno_Update {
    static constexpr const char* id = "rac1.moby.ryno.update";
    static constexpr const char* name = "Moby_Ryno_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (457), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket_Update {
    static constexpr const char* id = "rac1.moby.rocket_steer.update";
    static constexpr const char* name = "Moby_Rocket_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class drone (479), each tick. Experimental: from the behaviour catalogue.
struct Moby_Drone_Update {
    static constexpr const char* id = "rac1.moby.drone.update";
    static constexpr const char* name = "Moby_Drone_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes bolt_crate (500), nanotech_crate (501), metal_crate (502), oom_oom_box (505) and 1 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltCrate_Update {
    static constexpr const char* id = "rac1.moby.crate.update";
    static constexpr const char* name = "Moby_BoltCrate_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class veldin_ship (530), each tick. Experimental: from the behaviour catalogue.
struct Moby_VeldinShip_Update {
    static constexpr const char* id = "rac1.moby.veldin_ship.update";
    static constexpr const char* name = "Moby_VeldinShip_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class decoy_glove (562), each tick. Experimental: from the behaviour catalogue.
struct Moby_DecoyGlove_Update {
    static constexpr const char* id = "rac1.moby.decoy_glove.update";
    static constexpr const char* name = "Moby_DecoyGlove_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class metal_detector (585), each tick. Experimental: from the behaviour catalogue.
struct Moby_MetalDetector_Update {
    static constexpr const char* id = "rac1.moby.target_tracker.update";
    static constexpr const char* name = "Moby_MetalDetector_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class helipack (607), each tick. Experimental: from the behaviour catalogue.
struct Moby_Helipack_Update {
    static constexpr const char* id = "rac1.moby.clank_pack.update";
    static constexpr const char* name = "Moby_Helipack_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 639, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class639_Update {
    static constexpr const char* id = "rac1.moby.moby639.update";
    static constexpr const char* name = "Moby_Class639_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class horny_toad (749), each tick. Experimental: from the behaviour catalogue.
struct Moby_HornyToad_Update {
    static constexpr const char* id = "rac1.moby.horny_toad.update";
    static constexpr const char* name = "Moby_HornyToad_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class infobot (750), each tick. Experimental: from the behaviour catalogue.
struct Moby_Infobot_Update {
    static constexpr const char* id = "rac1.moby.infobot.update";
    static constexpr const char* name = "Moby_Infobot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class particle_spawner_smoke__fire (760), each tick. Experimental: from the behaviour catalogue.
struct Moby_ParticleSpawnerSmokeFire_Update {
    static constexpr const char* id = "rac1.moby.particle_field.update";
    static constexpr const char* name = "Moby_ParticleSpawnerSmokeFire_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 775, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class775_Update {
    static constexpr const char* id = "rac1.moby.grow_and_fade_out.update";
    static constexpr const char* name = "Moby_Class775_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 804, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class804_Update {
    static constexpr const char* id = "rac1.moby.trail_moby.update";
    static constexpr const char* name = "Moby_Class804_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class nanotech_ball (806), each tick. Experimental: from the behaviour catalogue.
struct Moby_NanotechBall_Update {
    static constexpr const char* id = "rac1.moby.nanotech_ball.update";
    static constexpr const char* name = "Moby_NanotechBall_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class clank_cutscene_trigger (834), each tick. Experimental: from the behaviour catalogue.
struct Moby_ClankCutsceneTrigger_Update {
    static constexpr const char* id = "rac1.moby.moby834.update";
    static constexpr const char* name = "Moby_ClankCutsceneTrigger_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class suckcannon (849), each tick. Experimental: from the behaviour catalogue.
struct Moby_Suckcannon_Update {
    static constexpr const char* id = "rac1.moby.suck_cannon.update";
    static constexpr const char* name = "Moby_Suckcannon_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class red_orb (1060), each tick. Experimental: from the behaviour catalogue.
struct Moby_RedOrb_Update {
    static constexpr const char* id = "rac1.moby.pulsing_color_moby.update";
    static constexpr const char* name = "Moby_RedOrb_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1138, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1138_Update {
    static constexpr const char* id = "rac1.moby.quad_moby.update";
    static constexpr const char* name = "Moby_Class1138_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class vendor_orb (1143), each tick. Experimental: from the behaviour catalogue.
struct Moby_VendorOrb_Update {
    static constexpr const char* id = "rac1.moby.moby1143.update";
    static constexpr const char* name = "Moby_VendorOrb_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hydrodisplacer (1251), each tick. Experimental: from the behaviour catalogue.
struct Moby_Hydrodisplacer_Update {
    static constexpr const char* id = "rac1.moby.hydrodisplacer_pad.update";
    static constexpr const char* name = "Moby_Hydrodisplacer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class pilots_helmet (1290), each tick. Experimental: from the behaviour catalogue.
struct Moby_PilotsHelmet_Update {
    static constexpr const char* id = "rac1.moby.moby1290.update";
    static constexpr const char* name = "Moby_PilotsHelmet_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1413, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1413_Update {
    static constexpr const char* id = "rac1.moby.moby1413.update";
    static constexpr const char* name = "Moby_Class1413_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class veldin_robot_enemy (1440), each tick. Experimental: from the behaviour catalogue.
struct Moby_VeldinRobotEnemy_Update {
    static constexpr const char* id = "rac1.moby.vendor.update";
    static constexpr const char* name = "Moby_VeldinRobotEnemy_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1471, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1471_Update {
    static constexpr const char* id = "rac1.moby.beam_slot_manager.update";
    static constexpr const char* name = "Moby_Class1471_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1545, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1545_Update {
    static constexpr const char* id = "rac1.moby.moby1545.update";
    static constexpr const char* name = "Moby_Class1545_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class pterodactyl (1564), each tick. Experimental: from the behaviour catalogue.
struct Moby_Pterodactyl_Update {
    static constexpr const char* id = "rac1.moby.path_follow_moby.update";
    static constexpr const char* name = "Moby_Pterodactyl_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes bombthrower_grass (492), green_leaves (724), blue_leaves (725), plant (766) and 3 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_BombthrowerGrass_Update {
    static constexpr const char* id = "rac1.moby.grass.update";
    static constexpr const char* name = "Moby_BombthrowerGrass_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1505, 1506, 1517, 1518 and 199 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Hoverboard_Update {
    static constexpr const char* id = "rac1.moby.fx_group.update";
    static constexpr const char* name = "Moby_Hoverboard_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes novalis_elevator (726), elevator (1141), each tick. Experimental: from the behaviour catalogue.
struct Moby_NovalisElevator_Update {
    static constexpr const char* id = "rac1.moby.path_platform.update";
    static constexpr const char* name = "Moby_NovalisElevator_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 27, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class27_Update {
    static constexpr const char* id = "rac1.moby.particle_emitter.update";
    static constexpr const char* name = "Moby_Class27_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 156, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class156_Update {
    static constexpr const char* id = "rac1.moby.moby156.update";
    static constexpr const char* name = "Moby_Class156_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bolt_crank (280), each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltCrank_Update {
    static constexpr const char* id = "rac1.moby.bolt_crank.update";
    static constexpr const char* name = "Moby_BoltCrank_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes gold_weapon_case (304), gold_bomb_glove (1456), gold_pyrocitor (1457), gold_blaster (1458) and 7 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_GoldWeaponCase_Update {
    static constexpr const char* id = "rac1.moby.item_offer.update";
    static constexpr const char* name = "Moby_GoldWeaponCase_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sand_mouse (437), each tick. Experimental: from the behaviour catalogue.
struct Moby_SandMouse_Update {
    static constexpr const char* id = "rac1.moby.sand_mouse.update";
    static constexpr const char* name = "Moby_SandMouse_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class birdbot (459), each tick. Experimental: from the behaviour catalogue.
struct Moby_Birdbot_Update {
    static constexpr const char* id = "rac1.moby.birdbot.update";
    static constexpr const char* name = "Moby_Birdbot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 567, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class567_Update {
    static constexpr const char* id = "rac1.moby.burning_debris.update";
    static constexpr const char* name = "Moby_Class567_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes large_amoeboid (572), medium_amoeboid (865), small_amoeboid (866), each tick. Experimental: from the behaviour catalogue.
struct Moby_LargeAmoeboid_Update {
    static constexpr const char* id = "rac1.moby.amoeboid.update";
    static constexpr const char* name = "Moby_LargeAmoeboid_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class peckbot (577), each tick. Experimental: from the behaviour catalogue.
struct Moby_Peckbot_Update {
    static constexpr const char* id = "rac1.moby.peckbot.update";
    static constexpr const char* name = "Moby_Peckbot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sandmouse_house (604), each tick. Experimental: from the behaviour catalogue.
struct Moby_SandmouseHouse_Update {
    static constexpr const char* id = "rac1.moby.moby604.update";
    static constexpr const char* name = "Moby_SandmouseHouse_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 605, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class605_Update {
    static constexpr const char* id = "rac1.moby.moby605.update";
    static constexpr const char* name = "Moby_Class605_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class water_current (613), each tick. Experimental: from the behaviour catalogue.
struct Moby_WaterCurrent_Update {
    static constexpr const char* id = "rac1.moby.path_carrier.update";
    static constexpr const char* name = "Moby_WaterCurrent_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class cave_door (641), each tick. Experimental: from the behaviour catalogue.
struct Moby_CaveDoor_Update {
    static constexpr const char* id = "rac1.moby.crank_rotator.update";
    static constexpr const char* name = "Moby_CaveDoor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class fighter_jet (660), each tick. Experimental: from the behaviour catalogue.
struct Moby_FighterJet_Update {
    static constexpr const char* id = "rac1.moby.tethered_mine.update";
    static constexpr const char* name = "Moby_FighterJet_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class door_half (665), each tick. Experimental: from the behaviour catalogue.
struct Moby_DoorHalf_Update {
    static constexpr const char* id = "rac1.moby.crank_slider.update";
    static constexpr const char* name = "Moby_DoorHalf_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class stationary_ship (666), each tick. Experimental: from the behaviour catalogue.
struct Moby_StationaryShip_Update {
    static constexpr const char* id = "rac1.moby.rider_carrier.update";
    static constexpr const char* name = "Moby_StationaryShip_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 845, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class845_Update {
    static constexpr const char* id = "rac1.moby.class_845.update";
    static constexpr const char* name = "Moby_Class845_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 678, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class678_Update {
    static constexpr const char* id = "rac1.moby.scrolling_layers2.update";
    static constexpr const char* name = "Moby_Class678_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 679, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class679_Update {
    static constexpr const char* id = "rac1.moby.grind_rail.update";
    static constexpr const char* name = "Moby_Class679_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 686, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class686_Update {
    static constexpr const char* id = "rac1.moby.flying_burst.update";
    static constexpr const char* name = "Moby_Class686_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomber_jet (688), each tick. Experimental: from the behaviour catalogue.
struct Moby_BomberJet_Update {
    static constexpr const char* id = "rac1.moby.gunship.update";
    static constexpr const char* name = "Moby_BomberJet_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class water_lily (695), each tick. Experimental: from the behaviour catalogue.
struct Moby_WaterLily_Update {
    static constexpr const char* id = "rac1.moby.hover_enemy.update";
    static constexpr const char* name = "Moby_WaterLily_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 200, 201, 205, 206 and 7 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Rock_Update {
    static constexpr const char* id = "rac1.moby.debris_moby.update";
    static constexpr const char* name = "Moby_Rock_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 700, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class700_Update {
    static constexpr const char* id = "rac1.moby.steam_vent.update";
    static constexpr const char* name = "Moby_Class700_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class falling_rock (701), each tick. Experimental: from the behaviour catalogue.
struct Moby_FallingRock_Update {
    static constexpr const char* id = "rac1.moby.collapsing_platform.update";
    static constexpr const char* name = "Moby_FallingRock_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes piston_platform (703), piston_platform_2 (715), each tick. Experimental: from the behaviour catalogue.
struct Moby_PistonPlatform_Update {
    static constexpr const char* id = "rac1.moby.lift_move.update";
    static constexpr const char* name = "Moby_PistonPlatform_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rock_formation (704), each tick. Experimental: from the behaviour catalogue.
struct Moby_RockFormation_Update {
    static constexpr const char* id = "rac1.moby.breakable_rock.update";
    static constexpr const char* name = "Moby_RockFormation_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class fan (705), each tick. Experimental: from the behaviour catalogue.
struct Moby_Fan_Update {
    static constexpr const char* id = "rac1.moby.oscillating_spinner.update";
    static constexpr const char* name = "Moby_Fan_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes bridge_piece_1 (709), bridge_piece_2 (710), bridge_piece_3 (711), each tick. Experimental: from the behaviour catalogue.
struct Moby_BridgePiece1_Update {
    static constexpr const char* id = "rac1.moby.projectile_breakable.update";
    static constexpr const char* name = "Moby_BridgePiece1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 722, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class722_Update {
    static constexpr const char* id = "rac1.moby.falling_drop.update";
    static constexpr const char* name = "Moby_Class722_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class destroyable_cave_wall (729), each tick. Experimental: from the behaviour catalogue.
struct Moby_DestroyableCaveWall_Update {
    static constexpr const char* id = "rac1.moby.breakable_wall.update";
    static constexpr const char* name = "Moby_DestroyableCaveWall_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes agnogg_ship_front (730), agnogg_ship_back (731), ship_rear (790), each tick. Experimental: from the behaviour catalogue.
struct Moby_AgnoggShipFront_Update {
    static constexpr const char* id = "rac1.moby.agnogg_ship.update";
    static constexpr const char* name = "Moby_AgnoggShipFront_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 737, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class737_Update {
    static constexpr const char* id = "rac1.moby.cutscene_trigger.update";
    static constexpr const char* name = "Moby_Class737_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bridge_half (746), each tick. Experimental: from the behaviour catalogue.
struct Moby_BridgeHalf_Update {
    static constexpr const char* id = "rac1.moby.hinged_bridge.update";
    static constexpr const char* name = "Moby_BridgeHalf_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 751, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class751_Update {
    static constexpr const char* id = "rac1.moby.water_surface_fx.update";
    static constexpr const char* name = "Moby_Class751_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class green_mushroom (754), each tick. Experimental: from the behaviour catalogue.
struct Moby_GreenMushroom_Update {
    static constexpr const char* id = "rac1.moby.breakable_pot.update";
    static constexpr const char* name = "Moby_GreenMushroom_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 761, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class761_Update {
    static constexpr const char* id = "rac1.moby.pulsing_mesh.update";
    static constexpr const char* name = "Moby_Class761_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes door_left (768), door_right (769), each tick. Experimental: from the behaviour catalogue.
struct Moby_DoorLeft_Update {
    static constexpr const char* id = "rac1.moby.sliding_door.update";
    static constexpr const char* name = "Moby_DoorLeft_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class the_plumber (774), each tick. Experimental: from the behaviour catalogue.
struct Moby_ThePlumber_Update {
    static constexpr const char* id = "rac1.moby.talking_npc.update";
    static constexpr const char* name = "Moby_ThePlumber_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class pipe (778), each tick. Experimental: from the behaviour catalogue.
struct Moby_Pipe_Update {
    static constexpr const char* id = "rac1.moby.moby778.update";
    static constexpr const char* name = "Moby_Pipe_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 779, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class779_Update {
    static constexpr const char* id = "rac1.moby.smoke_emitter.update";
    static constexpr const char* name = "Moby_Class779_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 787, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class787_Update {
    static constexpr const char* id = "rac1.moby.falling_chunk.update";
    static constexpr const char* name = "Moby_Class787_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class spawn_point (805), each tick. Experimental: from the behaviour catalogue.
struct Moby_SpawnPoint_Update {
    static constexpr const char* id = "rac1.moby.checkpoint_trigger.update";
    static constexpr const char* name = "Moby_SpawnPoint_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class beehive (815), each tick. Experimental: from the behaviour catalogue.
struct Moby_Beehive_Update {
    static constexpr const char* id = "rac1.moby.enemy_spawner.update";
    static constexpr const char* name = "Moby_Beehive_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 832, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class832_Update {
    static constexpr const char* id = "rac1.moby.rc_range_limiter.update";
    static constexpr const char* name = "Moby_Class832_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mushroom_underside (1042), each tick. Experimental: from the behaviour catalogue.
struct Moby_MushroomUnderside_Update {
    static constexpr const char* id = "rac1.moby.moby1042.update";
    static constexpr const char* name = "Moby_MushroomUnderside_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gold_bolt (1134), each tick. Experimental: from the behaviour catalogue.
struct Moby_GoldBolt_Update {
    static constexpr const char* id = "rac1.moby.gold_bolt.update";
    static constexpr const char* name = "Moby_GoldBolt_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class teleporter (1135), each tick. Experimental: from the behaviour catalogue.
struct Moby_Teleporter_Update {
    static constexpr const char* id = "rac1.moby.teleporter_pad.update";
    static constexpr const char* name = "Moby_Teleporter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1225, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1225_Update {
    static constexpr const char* id = "rac1.moby.moby1225.update";
    static constexpr const char* name = "Moby_Class1225_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1341, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1341_Update {
    static constexpr const char* id = "rac1.moby.help_hint_director.update";
    static constexpr const char* name = "Moby_Class1341_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1504, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1504_Update {
    static constexpr const char* id = "rac1.moby.wandering_light.update";
    static constexpr const char* name = "Moby_Class1504_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1510, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1510_Update {
    static constexpr const char* id = "rac1.moby.moby1510.update";
    static constexpr const char* name = "Moby_Class1510_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1546, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1546_Update {
    static constexpr const char* id = "rac1.moby.moby1546.update";
    static constexpr const char* name = "Moby_Class1546_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1633, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1633_Update {
    static constexpr const char* id = "rac1.moby.class_1633.update";
    static constexpr const char* name = "Moby_Class1633_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class tall_shroom (1813), each tick. Experimental: from the behaviour catalogue.
struct Moby_TallShroom_Update {
    static constexpr const char* id = "rac1.moby.moby1813.update";
    static constexpr const char* name = "Moby_TallShroom_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sandmouse (1818), each tick. Experimental: from the behaviour catalogue.
struct Moby_Sandmouse_Update {
    static constexpr const char* id = "rac1.moby.mouse_critter.update";
    static constexpr const char* name = "Moby_Sandmouse_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1848, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1848_Update {
    static constexpr const char* id = "rac1.moby.moby1848.update";
    static constexpr const char* name = "Moby_Class1848_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class trackdrive_left (331), each tick. Experimental: from the behaviour catalogue.
struct Moby_TrackdriveLeft_Update {
    static constexpr const char* id = "rac1.moby.trackdrive_left.update";
    static constexpr const char* name = "Moby_TrackdriveLeft_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sand_shark (580), each tick. Experimental: from the behaviour catalogue.
struct Moby_SandShark_Update {
    static constexpr const char* id = "rac1.moby.sand_shark.update";
    static constexpr const char* name = "Moby_SandShark_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class constructorbot (612), each tick. Experimental: from the behaviour catalogue.
struct Moby_Constructorbot_Update {
    static constexpr const char* id = "rac1.moby.moby612.update";
    static constexpr const char* name = "Moby_Constructorbot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class tresspasser_pad (615), each tick. Experimental: from the behaviour catalogue.
struct Moby_TresspasserPad_Update {
    static constexpr const char* id = "rac1.moby.trespasser_puzzle.update";
    static constexpr const char* name = "Moby_TresspasserPad_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class elevator (651), each tick. Experimental: from the behaviour catalogue.
struct Moby_Elevator_Update {
    static constexpr const char* id = "rac1.moby.elevator_651.update";
    static constexpr const char* name = "Moby_Elevator_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes conveyor_belt (296), escalator (652), escalator_end (653), each tick. Experimental: from the behaviour catalogue.
struct Moby_ConveyorBelt_Update {
    static constexpr const char* id = "rac1.moby.pusher.update";
    static constexpr const char* name = "Moby_ConveyorBelt_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bulldozer (656), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bulldozer_Update {
    static constexpr const char* id = "rac1.moby.moby656.update";
    static constexpr const char* name = "Moby_Bulldozer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sand_shark_nest (668), each tick. Experimental: from the behaviour catalogue.
struct Moby_SandSharkNest_Update {
    static constexpr const char* id = "rac1.moby.sand_shark_nest.update";
    static constexpr const char* name = "Moby_SandSharkNest_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class grab_crane (675), each tick. Experimental: from the behaviour catalogue.
struct Moby_GrabCrane_Update {
    static constexpr const char* id = "rac1.moby.moby675.update";
    static constexpr const char* name = "Moby_GrabCrane_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes platform (707), platform (734), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform_Update {
    static constexpr const char* id = "rac1.moby.rotate_platform.update";
    static constexpr const char* name = "Moby_Platform_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class transport_pod (713), each tick. Experimental: from the behaviour catalogue.
struct Moby_TransportPod_Update {
    static constexpr const char* id = "rac1.moby.moby713.update";
    static constexpr const char* name = "Moby_TransportPod_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class crane (732), each tick. Experimental: from the behaviour catalogue.
struct Moby_Crane_Update {
    static constexpr const char* id = "rac1.moby.moby732.update";
    static constexpr const char* name = "Moby_Crane_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket_launcher (733), each tick. Experimental: from the behaviour catalogue.
struct Moby_RocketLauncher_Update {
    static constexpr const char* id = "rac1.moby.rocket_launcher.update";
    static constexpr const char* name = "Moby_RocketLauncher_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes rocket_door1 (735), rocket_door2 (736), each tick. Experimental: from the behaviour catalogue.
struct Moby_RocketDoor1_Update {
    static constexpr const char* id = "rac1.moby.rocket_door1.update";
    static constexpr const char* name = "Moby_RocketDoor1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class tresspasser_door1 (743), each tick. Experimental: from the behaviour catalogue.
struct Moby_TresspasserDoor1_Update {
    static constexpr const char* id = "rac1.moby.moby743.update";
    static constexpr const char* name = "Moby_TresspasserDoor1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class tresspasser_door2 (744), each tick. Experimental: from the behaviour catalogue.
struct Moby_TresspasserDoor2_Update {
    static constexpr const char* id = "rac1.moby.moby744.update";
    static constexpr const char* name = "Moby_TresspasserDoor2_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sandshark_egg (755), each tick. Experimental: from the behaviour catalogue.
struct Moby_SandsharkEgg_Update {
    static constexpr const char* id = "rac1.moby.moby755.update";
    static constexpr const char* name = "Moby_SandsharkEgg_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes swingshot_versatarget (758), swingshot_swinging_versatarget (803), each tick. Experimental: from the behaviour catalogue.
struct Moby_SwingshotVersatarget_Update {
    static constexpr const char* id = "rac1.moby.hideable_flower.update";
    static constexpr const char* name = "Moby_SwingshotVersatarget_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocks (762), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocks_Update {
    static constexpr const char* id = "rac1.moby.moby762.update";
    static constexpr const char* name = "Moby_Rocks_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class skids_agent (786), each tick. Experimental: from the behaviour catalogue.
struct Moby_SkidsAgent_Update {
    static constexpr const char* id = "rac1.moby.skids_agent.update";
    static constexpr const char* name = "Moby_SkidsAgent_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class skid_mcmarx (788), each tick. Experimental: from the behaviour catalogue.
struct Moby_SkidMcmarx_Update {
    static constexpr const char* id = "rac1.moby.skid_mc_marx_aridia.update";
    static constexpr const char* name = "Moby_SkidMcmarx_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class liftcar_platform (792), each tick. Experimental: from the behaviour catalogue.
struct Moby_LiftcarPlatform_Update {
    static constexpr const char* id = "rac1.moby.driver_lift.update";
    static constexpr const char* name = "Moby_LiftcarPlatform_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ocean_aridia (854), each tick. Experimental: from the behaviour catalogue.
struct Moby_OceanAridia_Update {
    static constexpr const char* id = "rac1.moby.moby854.update";
    static constexpr const char* name = "Moby_OceanAridia_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes tresspasser (1005), hydrodisplacer (1016), each tick. Experimental: from the behaviour catalogue.
struct Moby_Tresspasser_Update {
    static constexpr const char* id = "rac1.moby.moby1005.update";
    static constexpr const char* name = "Moby_Tresspasser_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class constructorbot (1208), each tick. Experimental: from the behaviour catalogue.
struct Moby_Constructorbot1208_Update {
    static constexpr const char* id = "rac1.moby.shard_debris.update";
    static constexpr const char* name = "Moby_Constructorbot1208_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class background_ship_1 (1212), each tick. Experimental: from the behaviour catalogue.
struct Moby_BackgroundShip1_Update {
    static constexpr const char* id = "rac1.moby.background_ship.update";
    static constexpr const char* name = "Moby_BackgroundShip1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes background_ship_2 (1213), ship (1973), each tick. Experimental: from the behaviour catalogue.
struct Moby_BackgroundShip2_Update {
    static constexpr const char* id = "rac1.moby.ship_path.update";
    static constexpr const char* name = "Moby_BackgroundShip2_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1324, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1324_Update {
    static constexpr const char* id = "rac1.moby.tutorial_hint.update";
    static constexpr const char* name = "Moby_Class1324_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1479, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1479_Update {
    static constexpr const char* id = "rac1.moby.moby1479.update";
    static constexpr const char* name = "Moby_Class1479_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes white_flower (1819), lightpost (1827), pipe (1834), 1844 and 3 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_WhiteFlower_Update {
    static constexpr const char* id = "rac1.moby.break_on_hit_l02.update";
    static constexpr const char* name = "Moby_WhiteFlower_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class metropolis_train_locomotive (822), each tick. Experimental: from the behaviour catalogue.
struct Moby_MetropolisTrainLocomotive_Update {
    static constexpr const char* id = "rac1.moby.metropolis_train.update";
    static constexpr const char* name = "Moby_MetropolisTrainLocomotive_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes moving_platform_small (868), moving_platform_medium (905), moving_platform_large (928), each tick. Experimental: from the behaviour catalogue.
struct Moby_MovingPlatformSmall_Update {
    static constexpr const char* id = "rac1.moby.moving_platform.update";
    static constexpr const char* name = "Moby_MovingPlatformSmall_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class metropolis_train_carriage (1210), each tick. Experimental: from the behaviour catalogue.
struct Moby_MetropolisTrainCarriage_Update {
    static constexpr const char* id = "rac1.moby.metropolis_train_carriage.update";
    static constexpr const char* name = "Moby_MetropolisTrainCarriage_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes red_ship_1 (75), white_ship (115), red_ship_2 (116), yellow_ship (117) and 5 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_RedShip1_Update {
    static constexpr const char* id = "rac1.moby.train_car.update";
    static constexpr const char* name = "Moby_RedShip1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 235, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class235_Update {
    static constexpr const char* id = "rac1.moby.parent_follow_fade.update";
    static constexpr const char* name = "Moby_Class235_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class spawner (455), each tick. Experimental: from the behaviour catalogue.
struct Moby_Spawner_Update {
    static constexpr const char* id = "rac1.moby.child_spawner.update";
    static constexpr const char* name = "Moby_Spawner_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (545), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb545_Update {
    static constexpr const char* id = "rac1.moby.moby545.update";
    static constexpr const char* name = "Moby_Bomb545_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class robot_dog (573), each tick. Experimental: from the behaviour catalogue.
struct Moby_RobotDog_Update {
    static constexpr const char* id = "rac1.moby.moby573.update";
    static constexpr const char* name = "Moby_RobotDog_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 574, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class574_Update {
    static constexpr const char* id = "rac1.moby.moby574.update";
    static constexpr const char* name = "Moby_Class574_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine_layer (578), each tick. Experimental: from the behaviour catalogue.
struct Moby_MineLayer_Update {
    static constexpr const char* id = "rac1.moby.trooper.update";
    static constexpr const char* name = "Moby_MineLayer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (627), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine627_Update {
    static constexpr const char* id = "rac1.moby.kerwan_mine.update";
    static constexpr const char* name = "Moby_Mine627_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_helicopter (631), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargHelicopter_Update {
    static constexpr const char* id = "rac1.moby.helicopter_gunship.update";
    static constexpr const char* name = "Moby_BlargHelicopter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi (816), each tick. Experimental: from the behaviour catalogue.
struct Moby_Taxi_Update {
    static constexpr const char* id = "rac1.moby.rail_taxi.update";
    static constexpr const char* name = "Moby_Taxi_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes statue (817), seashell (1686), each tick. Experimental: from the behaviour catalogue.
struct Moby_Statue_Update {
    static constexpr const char* id = "rac1.moby.moby817.update";
    static constexpr const char* name = "Moby_Statue_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class robo_shack_sign (825), each tick. Experimental: from the behaviour catalogue.
struct Moby_RoboShackSign_Update {
    static constexpr const char* id = "rac1.moby.spinner.update";
    static constexpr const char* name = "Moby_RoboShackSign_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 833, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class833_Update {
    static constexpr const char* id = "rac1.moby.kerwan_homing_rocket.update";
    static constexpr const char* name = "Moby_Class833_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 848, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class848_Update {
    static constexpr const char* id = "rac1.moby.moby848.update";
    static constexpr const char* name = "Moby_Class848_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class helga (890), each tick. Experimental: from the behaviour catalogue.
struct Moby_Helga_Update {
    static constexpr const char* id = "rac1.moby.quest_giver.update";
    static constexpr const char* name = "Moby_Helga_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 898, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class898_Update {
    static constexpr const char* id = "rac1.moby.rail_moby.update";
    static constexpr const char* name = "Moby_Class898_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 899, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class899_Update {
    static constexpr const char* id = "rac1.moby.rail_spawner.update";
    static constexpr const char* name = "Moby_Class899_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class big_al (909), each tick. Experimental: from the behaviour catalogue.
struct Moby_BigAl_Update {
    static constexpr const char* id = "rac1.moby.guide.update";
    static constexpr const char* name = "Moby_BigAl_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class robot_qwark (914), each tick. Experimental: from the behaviour catalogue.
struct Moby_RobotQwark_Update {
    static constexpr const char* id = "rac1.moby.moby914.update";
    static constexpr const char* name = "Moby_RobotQwark_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes fred_1 (915), fred_2 (916), fred_3 (917), each tick. Experimental: from the behaviour catalogue.
struct Moby_Fred1_Update {
    static constexpr const char* id = "rac1.moby.fred_1.update";
    static constexpr const char* name = "Moby_Fred1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes qwark_umbris (447), qwark_rilgar (920), each tick. Experimental: from the behaviour catalogue.
struct Moby_QwarkUmbris_Update {
    static constexpr const char* id = "rac1.moby.qwark_watch.update";
    static constexpr const char* name = "Moby_QwarkUmbris_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 994, 1260, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class994_Update {
    static constexpr const char* id = "rac1.moby.ambient_stream_l03.update";
    static constexpr const char* name = "Moby_Class994_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes gate (353), garage_door (997), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gate_Update {
    static constexpr const char* id = "rac1.moby.rise_when_collected.update";
    static constexpr const char* name = "Moby_Gate_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi (1012), each tick. Experimental: from the behaviour catalogue.
struct Moby_Taxi1012_Update {
    static constexpr const char* id = "rac1.moby.rail_lift.update";
    static constexpr const char* name = "Moby_Taxi1012_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1221, 1222, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1221_Update {
    static constexpr const char* id = "rac1.moby.falling_moby.update";
    static constexpr const char* name = "Moby_Class1221_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1342, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1342_Update {
    static constexpr const char* id = "rac1.moby.tutorial_hint_trigger.update";
    static constexpr const char* name = "Moby_Class1342_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1548, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1548_Update {
    static constexpr const char* id = "rac1.moby.cutscene_beams.update";
    static constexpr const char* name = "Moby_Class1548_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 86, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class86_Update {
    static constexpr const char* id = "rac1.moby.swarm.update";
    static constexpr const char* name = "Moby_Class86_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 161, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class161_Update {
    static constexpr const char* id = "rac1.moby.class_161.update";
    static constexpr const char* name = "Moby_Class161_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 184, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class184_Update {
    static constexpr const char* id = "rac1.moby.moby184_shot.update";
    static constexpr const char* name = "Moby_Class184_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class big_axebot (217), each tick. Experimental: from the behaviour catalogue.
struct Moby_BigAxebot_Update {
    static constexpr const char* id = "rac1.moby.big_axebot.update";
    static constexpr const char* name = "Moby_BigAxebot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class circular_saw_enemy (340), each tick. Experimental: from the behaviour catalogue.
struct Moby_CircularSawEnemy_Update {
    static constexpr const char* id = "rac1.moby.circular_saw.update";
    static constexpr const char* name = "Moby_CircularSawEnemy_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_trooper (427), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargTrooper_Update {
    static constexpr const char* id = "rac1.moby.blarg_trooper.update";
    static constexpr const char* name = "Moby_BlargTrooper_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes eudora_bridge (432), bridge (1052), each tick. Experimental: from the behaviour catalogue.
struct Moby_EudoraBridge_Update {
    static constexpr const char* id = "rac1.moby.switch_follower.update";
    static constexpr const char* name = "Moby_EudoraBridge_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform_booster (434), each tick. Experimental: from the behaviour catalogue.
struct Moby_PlatformBooster_Update {
    static constexpr const char* id = "rac1.moby.lever_door.update";
    static constexpr const char* name = "Moby_PlatformBooster_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes snow_plow (466), grab_helicopter (480), ball_ship_1 (485), ball_ship_2 (486) and 7 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_SnowPlow_Update {
    static constexpr const char* id = "rac1.moby.path_vehicle.update";
    static constexpr const char* name = "Moby_SnowPlow_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class floating_platform (481), each tick. Experimental: from the behaviour catalogue.
struct Moby_FloatingPlatform_Update {
    static constexpr const char* id = "rac1.moby.floating_platform_481.update";
    static constexpr const char* name = "Moby_FloatingPlatform_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class transport_ship (484), each tick. Experimental: from the behaviour catalogue.
struct Moby_TransportShip_Update {
    static constexpr const char* id = "rac1.moby.delete_moby.update";
    static constexpr const char* name = "Moby_TransportShip_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class woodcut_bot (563), each tick. Experimental: from the behaviour catalogue.
struct Moby_WoodcutBot_Update {
    static constexpr const char* id = "rac1.moby.wb_moby.update";
    static constexpr const char* name = "Moby_WoodcutBot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 584, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class584_Update {
    static constexpr const char* id = "rac1.moby.moby584.update";
    static constexpr const char* name = "Moby_Class584_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 617, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class617_Update {
    static constexpr const char* id = "rac1.moby.path_keyframe_blend.update";
    static constexpr const char* name = "Moby_Class617_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class leaf (642), each tick. Experimental: from the behaviour catalogue.
struct Moby_Leaf_Update {
    static constexpr const char* id = "rac1.moby.drift_spark_moby.update";
    static constexpr const char* name = "Moby_Leaf_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes upper_door (1101), lower_door (1102), lower_door (1531), upper_door (1532), each tick. Experimental: from the behaviour catalogue.
struct Moby_UpperDoor_Update {
    static constexpr const char* id = "rac1.moby.collectable.update";
    static constexpr const char* name = "Moby_UpperDoor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class suck_cannon (1120), each tick. Experimental: from the behaviour catalogue.
struct Moby_SuckCannon_Update {
    static constexpr const char* id = "rac1.moby.warp_portal.update";
    static constexpr const char* name = "Moby_SuckCannon_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class lieutenant (1190), each tick. Experimental: from the behaviour catalogue.
struct Moby_Lieutenant_Update {
    static constexpr const char* id = "rac1.moby.unlock_planet_pad.update";
    static constexpr const char* name = "Moby_Lieutenant_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1343, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1343_Update {
    static constexpr const char* id = "rac1.moby.moby1343.update";
    static constexpr const char* name = "Moby_Class1343_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1516, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1516_Update {
    static constexpr const char* id = "rac1.moby.bouncing_debris.update";
    static constexpr const char* name = "Moby_Class1516_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1549, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1549_Update {
    static constexpr const char* id = "rac1.moby.sparks_trigger_l04.update";
    static constexpr const char* name = "Moby_Class1549_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sharkigator (35), each tick. Experimental: from the behaviour catalogue.
struct Moby_Sharkigator_Update {
    static constexpr const char* id = "rac1.moby.sharkigator_35.update";
    static constexpr const char* name = "Moby_Sharkigator_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class scenery_ship (79), each tick. Experimental: from the behaviour catalogue.
struct Moby_SceneryShip_Update {
    static constexpr const char* id = "rac1.moby.scenery_ship.update";
    static constexpr const char* name = "Moby_SceneryShip_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class amoeboid (133), each tick. Experimental: from the behaviour catalogue.
struct Moby_Amoeboid_Update {
    static constexpr const char* id = "rac1.moby.grazing_critter.update";
    static constexpr const char* name = "Moby_Amoeboid_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 258, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class258_Update {
    static constexpr const char* id = "rac1.moby.moby258.update";
    static constexpr const char* name = "Moby_Class258_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hydrodisplacer_faucet (341), each tick. Experimental: from the behaviour catalogue.
struct Moby_HydrodisplacerFaucet_Update {
    static constexpr const char* id = "rac1.moby.moby341.update";
    static constexpr const char* name = "Moby_HydrodisplacerFaucet_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hoverboard (439), each tick. Experimental: from the behaviour catalogue.
struct Moby_Hoverboard439_Update {
    static constexpr const char* id = "rac1.moby.hoverboard.update";
    static constexpr const char* name = "Moby_Hoverboard439_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class extermibot (623), each tick. Experimental: from the behaviour catalogue.
struct Moby_Extermibot_Update {
    static constexpr const char* id = "rac1.moby.extermibot.update";
    static constexpr const char* name = "Moby_Extermibot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class extermitank (625), each tick. Experimental: from the behaviour catalogue.
struct Moby_Extermitank_Update {
    static constexpr const char* id = "rac1.moby.extermitank.update";
    static constexpr const char* name = "Moby_Extermitank_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class skid_mcmarx (717), each tick. Experimental: from the behaviour catalogue.
struct Moby_SkidMcmarx717_Update {
    static constexpr const char* id = "rac1.moby.skid_mc_marx.update";
    static constexpr const char* name = "Moby_SkidMcmarx717_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class floating_platform (810), each tick. Experimental: from the behaviour catalogue.
struct Moby_FloatingPlatform810_Update {
    static constexpr const char* id = "rac1.moby.floating_platform_810.update";
    static constexpr const char* name = "Moby_FloatingPlatform810_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class box (812), each tick. Experimental: from the behaviour catalogue.
struct Moby_Box_Update {
    static constexpr const char* id = "rac1.moby.moby812.update";
    static constexpr const char* name = "Moby_Box_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class lamp (823), each tick. Experimental: from the behaviour catalogue.
struct Moby_Lamp_Update {
    static constexpr const char* id = "rac1.moby.moby823.update";
    static constexpr const char* name = "Moby_Lamp_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class floor_button (830), each tick. Experimental: from the behaviour catalogue.
struct Moby_FloorButton_Update {
    static constexpr const char* id = "rac1.moby.floor_button.update";
    static constexpr const char* name = "Moby_FloorButton_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class water (831), each tick. Experimental: from the behaviour catalogue.
struct Moby_Water_Update {
    static constexpr const char* id = "rac1.moby.water_surface.update";
    static constexpr const char* name = "Moby_Water_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class electric_gate (838), each tick. Experimental: from the behaviour catalogue.
struct Moby_ElectricGate_Update {
    static constexpr const char* id = "rac1.moby.electric_gate.update";
    static constexpr const char* name = "Moby_ElectricGate_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 841, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class841_Update {
    static constexpr const char* id = "rac1.moby.fog_zone.update";
    static constexpr const char* name = "Moby_Class841_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 843, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class843_Update {
    static constexpr const char* id = "rac1.moby.pose_slide_out.update";
    static constexpr const char* name = "Moby_Class843_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class drain_cover (844), each tick. Experimental: from the behaviour catalogue.
struct Moby_DrainCover_Update {
    static constexpr const char* id = "rac1.moby.drain_cover.update";
    static constexpr const char* name = "Moby_DrainCover_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class pipe_mounted_grating (846), each tick. Experimental: from the behaviour catalogue.
struct Moby_PipeMountedGrating_Update {
    static constexpr const char* id = "rac1.moby.pipe_mounted_grating.update";
    static constexpr const char* name = "Moby_PipeMountedGrating_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 852, 853, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class852_Update {
    static constexpr const char* id = "rac1.moby.gate_rise.update";
    static constexpr const char* name = "Moby_Class852_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 855, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class855_Update {
    static constexpr const char* id = "rac1.moby.emitter_l05.update";
    static constexpr const char* name = "Moby_Class855_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class elevator_with_button (877), each tick. Experimental: from the behaviour catalogue.
struct Moby_ElevatorWithButton_Update {
    static constexpr const char* id = "rac1.moby.lift_l05.update";
    static constexpr const char* name = "Moby_ElevatorWithButton_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ocean_rilgar (879), each tick. Experimental: from the behaviour catalogue.
struct Moby_OceanRilgar_Update {
    static constexpr const char* id = "rac1.moby.pulsing_marker.update";
    static constexpr const char* name = "Moby_OceanRilgar_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 893, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class893_Update {
    static constexpr const char* id = "rac1.moby.class_893.update";
    static constexpr const char* name = "Moby_Class893_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 895, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class895_Update {
    static constexpr const char* id = "rac1.moby.triggered_rotor.update";
    static constexpr const char* name = "Moby_Class895_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hoverboard_girl (918), each tick. Experimental: from the behaviour catalogue.
struct Moby_HoverboardGirl_Update {
    static constexpr const char* id = "rac1.moby.hoverboard_girl.update";
    static constexpr const char* name = "Moby_HoverboardGirl_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class qwarks_bouncer (919), each tick. Experimental: from the behaviour catalogue.
struct Moby_QwarksBouncer_Update {
    static constexpr const char* id = "rac1.moby.bouncer.update";
    static constexpr const char* name = "Moby_QwarksBouncer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shady_salesman (925), each tick. Experimental: from the behaviour catalogue.
struct Moby_ShadySalesman_Update {
    static constexpr const char* id = "rac1.moby.shady_salesman.update";
    static constexpr const char* name = "Moby_ShadySalesman_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes blarg_gun (344), 547, 548, 549 and 19 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargGun_Update {
    static constexpr const char* id = "rac1.moby.blarg_gun.update";
    static constexpr const char* name = "Moby_BlargGun_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 982, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class982_Update {
    static constexpr const char* id = "rac1.moby.water_plane_marker.update";
    static constexpr const char* name = "Moby_Class982_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class television (984), each tick. Experimental: from the behaviour catalogue.
struct Moby_Television_Update {
    static constexpr const char* id = "rac1.moby.bobbing_moby_l05.update";
    static constexpr const char* name = "Moby_Television_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi (998), each tick. Experimental: from the behaviour catalogue.
struct Moby_Taxi998_Update {
    static constexpr const char* id = "rac1.moby.pickup_l05.update";
    static constexpr const char* name = "Moby_Taxi998_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class qwarks_trailer (1099), each tick. Experimental: from the behaviour catalogue.
struct Moby_QwarksTrailer_Update {
    static constexpr const char* id = "rac1.moby.qwarks_trailer.update";
    static constexpr const char* name = "Moby_QwarksTrailer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hoverboard_ring (1139), each tick. Experimental: from the behaviour catalogue.
struct Moby_HoverboardRing_Update {
    static constexpr const char* id = "rac1.moby.moby1139.update";
    static constexpr const char* name = "Moby_HoverboardRing_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1347, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1347_Update {
    static constexpr const char* id = "rac1.moby.challenge.update";
    static constexpr const char* name = "Moby_Class1347_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class lamp (1511), each tick. Experimental: from the behaviour catalogue.
struct Moby_Lamp1511_Update {
    static constexpr const char* id = "rac1.moby.mine_moby.update";
    static constexpr const char* name = "Moby_Lamp1511_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1550, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1550_Update {
    static constexpr const char* id = "rac1.moby.moby1550.update";
    static constexpr const char* name = "Moby_Class1550_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hangar_door (55), each tick. Experimental: from the behaviour catalogue.
struct Moby_HangarDoor_Update {
    static constexpr const char* id = "rac1.moby.orbiting_platform.update";
    static constexpr const char* name = "Moby_HangarDoor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gadgebot_spawn_container (302), each tick. Experimental: from the behaviour catalogue.
struct Moby_GadgebotSpawnContainer_Update {
    static constexpr const char* id = "rac1.moby.moby302.update";
    static constexpr const char* name = "Moby_GadgebotSpawnContainer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gadgebot_spawn_pad (303), each tick. Experimental: from the behaviour catalogue.
struct Moby_GadgebotSpawnPad_Update {
    static constexpr const char* id = "rac1.moby.moby303.update";
    static constexpr const char* name = "Moby_GadgebotSpawnPad_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 367, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class367_Update {
    static constexpr const char* id = "rac1.moby.moby367.update";
    static constexpr const char* name = "Moby_Class367_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class frog (827), each tick. Experimental: from the behaviour catalogue.
struct Moby_Frog_Update {
    static constexpr const char* id = "rac1.moby.frog.update";
    static constexpr const char* name = "Moby_Frog_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gadgebot (857), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gadgebot_Update {
    static constexpr const char* id = "rac1.moby.gadgebot.update";
    static constexpr const char* name = "Moby_Gadgebot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class toxic_gas_vent (911), each tick. Experimental: from the behaviour catalogue.
struct Moby_ToxicGasVent_Update {
    static constexpr const char* id = "rac1.moby.swinging_hazard.update";
    static constexpr const char* name = "Moby_ToxicGasVent_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1021, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1021_Update {
    static constexpr const char* id = "rac1.moby.door_ring.update";
    static constexpr const char* name = "Moby_Class1021_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1028, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1028_Update {
    static constexpr const char* id = "rac1.moby.drawbridge.update";
    static constexpr const char* name = "Moby_Class1028_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1035, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1035_Update {
    static constexpr const char* id = "rac1.moby.class_1035.update";
    static constexpr const char* name = "Moby_Class1035_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class energy_sphere_holder (1038), each tick. Experimental: from the behaviour catalogue.
struct Moby_EnergySphereHolder_Update {
    static constexpr const char* id = "rac1.moby.collectible_spawner.update";
    static constexpr const char* name = "Moby_EnergySphereHolder_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1039, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1039_Update {
    static constexpr const char* id = "rac1.moby.trigger_lists.update";
    static constexpr const char* name = "Moby_Class1039_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class energy_sphere (1040), each tick. Experimental: from the behaviour catalogue.
struct Moby_EnergySphere_Update {
    static constexpr const char* id = "rac1.moby.moby1040.update";
    static constexpr const char* name = "Moby_EnergySphere_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class alien_snapper (1048), each tick. Experimental: from the behaviour catalogue.
struct Moby_AlienSnapper_Update {
    static constexpr const char* id = "rac1.moby.moby1048.update";
    static constexpr const char* name = "Moby_AlienSnapper_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class alien_queen (1051), each tick. Experimental: from the behaviour catalogue.
struct Moby_AlienQueen_Update {
    static constexpr const char* id = "rac1.moby.alien_queen.update";
    static constexpr const char* name = "Moby_AlienQueen_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1054, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1054_Update {
    static constexpr const char* id = "rac1.moby.moby1054.update";
    static constexpr const char* name = "Moby_Class1054_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1061, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1061_Update {
    static constexpr const char* id = "rac1.moby.callable_lift.update";
    static constexpr const char* name = "Moby_Class1061_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1062, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1062_Update {
    static constexpr const char* id = "rac1.moby.class_1062.update";
    static constexpr const char* name = "Moby_Class1062_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1066, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1066_Update {
    static constexpr const char* id = "rac1.moby.launch_trigger.update";
    static constexpr const char* name = "Moby_Class1066_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_space_commando (1068), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargSpaceCommando_Update {
    static constexpr const char* id = "rac1.moby.moby1068.update";
    static constexpr const char* name = "Moby_BlargSpaceCommando_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1083, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1083_Update {
    static constexpr const char* id = "rac1.moby.camera_takeover_trigger.update";
    static constexpr const char* name = "Moby_Class1083_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1085, 1086, 1087, 1088 and 1 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1085_Update {
    static constexpr const char* id = "rac1.moby.spinning_debris.update";
    static constexpr const char* name = "Moby_Class1085_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1091, 1092, 1093, 1094 and 5 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1091_Update {
    static constexpr const char* id = "rac1.moby.bobbing_moby.update";
    static constexpr const char* name = "Moby_Class1091_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class fred (1105), each tick. Experimental: from the behaviour catalogue.
struct Moby_Fred_Update {
    static constexpr const char* id = "rac1.moby.moby1105.update";
    static constexpr const char* name = "Moby_Fred_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1108, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1108_Update {
    static constexpr const char* id = "rac1.moby.moby1108.update";
    static constexpr const char* name = "Moby_Class1108_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shuttle (1109), each tick. Experimental: from the behaviour catalogue.
struct Moby_Shuttle_Update {
    static constexpr const char* id = "rac1.moby.shuttle.update";
    static constexpr const char* name = "Moby_Shuttle_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class red_button (1118), each tick. Experimental: from the behaviour catalogue.
struct Moby_RedButton_Update {
    static constexpr const char* id = "rac1.moby.red_button.update";
    static constexpr const char* name = "Moby_RedButton_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1123, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1123_Update {
    static constexpr const char* id = "rac1.moby.moby1123.update";
    static constexpr const char* name = "Moby_Class1123_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bot_door_lock (1302), each tick. Experimental: from the behaviour catalogue.
struct Moby_BotDoorLock_Update {
    static constexpr const char* id = "rac1.moby.moby1302.update";
    static constexpr const char* name = "Moby_BotDoorLock_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1348, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1348_Update {
    static constexpr const char* id = "rac1.moby.class_1348.update";
    static constexpr const char* name = "Moby_Class1348_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1512, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1512_Update {
    static constexpr const char* id = "rac1.moby.spark_emitter.update";
    static constexpr const char* name = "Moby_Class1512_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1551, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1551_Update {
    static constexpr const char* id = "rac1.moby.sparks_trigger_l06.update";
    static constexpr const char* name = "Moby_Class1551_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes mushroom (1650), 1655, robot_head (1663), 1677, each tick. Experimental: from the behaviour catalogue.
struct Moby_Mushroom_Update {
    static constexpr const char* id = "rac1.moby.spawn_child_and_delete.update";
    static constexpr const char* name = "Moby_Mushroom_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1660, 1664, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1660_Update {
    static constexpr const char* id = "rac1.moby.break_on_hit_blarg.update";
    static constexpr const char* name = "Moby_Class1660_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1898, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1898_Update {
    static constexpr const char* id = "rac1.moby.moby1898.update";
    static constexpr const char* name = "Moby_Class1898_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi_platform (38), each tick. Experimental: from the behaviour catalogue.
struct Moby_TaxiPlatform_Update {
    static constexpr const char* id = "rac1.moby.taxi_platform_38.update";
    static constexpr const char* name = "Moby_TaxiPlatform_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi_platform (1474), each tick. Experimental: from the behaviour catalogue.
struct Moby_TaxiPlatform1474_Update {
    static constexpr const char* id = "rac1.moby.taxi_platform_1474.update";
    static constexpr const char* name = "Moby_TaxiPlatform1474_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 436, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class436_Update {
    static constexpr const char* id = "rac1.moby.moby436.update";
    static constexpr const char* name = "Moby_Class436_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 317, 460, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class317_Update {
    static constexpr const char* id = "rac1.moby.ambient_stream_l07.update";
    static constexpr const char* name = "Moby_Class317_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class purple_ship (529), each tick. Experimental: from the behaviour catalogue.
struct Moby_PurpleShip_Update {
    static constexpr const char* id = "rac1.moby.purple_ship_529.update";
    static constexpr const char* name = "Moby_PurpleShip_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class boltcrate_possibly_for_metal_detector (871), each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltcratePossiblyForMetalDetector_Update {
    static constexpr const char* id = "rac1.moby.path_train.update";
    static constexpr const char* name = "Moby_BoltcratePossiblyForMetalDetector_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 880, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class880_Update {
    static constexpr const char* id = "rac1.moby.bolt_projectile.update";
    static constexpr const char* name = "Moby_Class880_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (882), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb882_Update {
    static constexpr const char* id = "rac1.moby.gravity_debris.update";
    static constexpr const char* name = "Moby_Bomb882_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class timed_activation_pads (886), each tick. Experimental: from the behaviour catalogue.
struct Moby_TimedActivationPads_Update {
    static constexpr const char* id = "rac1.moby.timed_pad.update";
    static constexpr const char* name = "Moby_TimedActivationPads_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes water (902), water (941), water (943), water (944) and 16 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Water902_Update {
    static constexpr const char* id = "rac1.moby.water_plate.update";
    static constexpr const char* name = "Moby_Water902_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1013, 1014, 1064, 1065, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1013_Update {
    static constexpr const char* id = "rac1.moby.target_driven_slider.update";
    static constexpr const char* name = "Moby_Class1013_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ocean_umbris (1018), each tick. Experimental: from the behaviour catalogue.
struct Moby_OceanUmbris_Update {
    static constexpr const char* id = "rac1.moby.audio_stream_trigger.update";
    static constexpr const char* name = "Moby_OceanUmbris_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bombomatic (1041), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bombomatic_Update {
    static constexpr const char* id = "rac1.moby.lobber_enemy.update";
    static constexpr const char* name = "Moby_Bombomatic_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1046, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1046_Update {
    static constexpr const char* id = "rac1.moby.shockwave.update";
    static constexpr const char* name = "Moby_Class1046_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1049, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1049_Update {
    static constexpr const char* id = "rac1.moby.lobbed_bomb.update";
    static constexpr const char* name = "Moby_Class1049_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sharkigator (1059), each tick. Experimental: from the behaviour catalogue.
struct Moby_Sharkigator1059_Update {
    static constexpr const char* id = "rac1.moby.sharkigator_1059.update";
    static constexpr const char* name = "Moby_Sharkigator1059_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 129, 130, door (182), door (183) and 5 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Door_Update {
    static constexpr const char* id = "rac1.moby.door.update";
    static constexpr const char* name = "Moby_Door_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1069, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1069_Update {
    static constexpr const char* id = "rac1.moby.moby1069.update";
    static constexpr const char* name = "Moby_Class1069_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1080, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1080_Update {
    static constexpr const char* id = "rac1.moby.falling_leaf.update";
    static constexpr const char* name = "Moby_Class1080_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class snagglebeast (1106), each tick. Experimental: from the behaviour catalogue.
struct Moby_Snagglebeast_Update {
    static constexpr const char* id = "rac1.moby.snagglebeast.update";
    static constexpr const char* name = "Moby_Snagglebeast_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (1112), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine1112_Update {
    static constexpr const char* id = "rac1.moby.swooper.update";
    static constexpr const char* name = "Moby_Mine1112_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1113, 1114, 1115, 1116, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1113_Update {
    static constexpr const char* id = "rac1.moby.breakable_prop.update";
    static constexpr const char* name = "Moby_Class1113_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class machinegun_turret (1126), each tick. Experimental: from the behaviour catalogue.
struct Moby_MachinegunTurret_Update {
    static constexpr const char* id = "rac1.moby.machinegun_turret_1126.update";
    static constexpr const char* name = "Moby_MachinegunTurret_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1128, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1128_Update {
    static constexpr const char* id = "rac1.moby.moving_moby_l07.update";
    static constexpr const char* name = "Moby_Class1128_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes bridge (104), bridge (106), 1129, each tick. Experimental: from the behaviour catalogue.
struct Moby_Bridge_Update {
    static constexpr const char* id = "rac1.moby.bridge.update";
    static constexpr const char* name = "Moby_Bridge_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1133, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1133_Update {
    static constexpr const char* id = "rac1.moby.moby1133.update";
    static constexpr const char* name = "Moby_Class1133_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1142, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1142_Update {
    static constexpr const char* id = "rac1.moby.moby1142.update";
    static constexpr const char* name = "Moby_Class1142_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1552, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1552_Update {
    static constexpr const char* id = "rac1.moby.spark_burst_spawner.update";
    static constexpr const char* name = "Moby_Class1552_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1789, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1789_Update {
    static constexpr const char* id = "rac1.moby.moby1789.update";
    static constexpr const char* name = "Moby_Class1789_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 152, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class152_Update {
    static constexpr const char* id = "rac1.moby.moby152.update";
    static constexpr const char* name = "Moby_Class152_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (248), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb248_Update {
    static constexpr const char* id = "rac1.moby.bomb_248.update";
    static constexpr const char* name = "Moby_Bomb248_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class scoutbot (252), each tick. Experimental: from the behaviour catalogue.
struct Moby_Scoutbot_Update {
    static constexpr const char* id = "rac1.moby.scoutbot.update";
    static constexpr const char* name = "Moby_Scoutbot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomber_tank (253), each tick. Experimental: from the behaviour catalogue.
struct Moby_BomberTank_Update {
    static constexpr const char* id = "rac1.moby.bomber_tank.update";
    static constexpr const char* name = "Moby_BomberTank_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 327, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class327_Update {
    static constexpr const char* id = "rac1.moby.moby327.update";
    static constexpr const char* name = "Moby_Class327_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_bombthrower (333), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargBombthrower_Update {
    static constexpr const char* id = "rac1.moby.blarg_bomb_thrower.update";
    static constexpr const char* name = "Moby_BlargBombthrower_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shuttle (424), each tick. Experimental: from the behaviour catalogue.
struct Moby_Shuttle424_Update {
    static constexpr const char* id = "rac1.moby.route_platform.update";
    static constexpr const char* name = "Moby_Shuttle424_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (425), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb425_Update {
    static constexpr const char* id = "rac1.moby.moby425.update";
    static constexpr const char* name = "Moby_Bomb425_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (435), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket435_Update {
    static constexpr const char* id = "rac1.moby.guided_rocket.update";
    static constexpr const char* name = "Moby_Rocket435_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 438, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class438_Update {
    static constexpr const char* id = "rac1.moby.moby438.update";
    static constexpr const char* name = "Moby_Class438_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 440, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class440_Update {
    static constexpr const char* id = "rac1.moby.cannon.update";
    static constexpr const char* name = "Moby_Class440_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 441, 442, 443, 445 and 6 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class441_Update {
    static constexpr const char* id = "rac1.moby.falling_debris.update";
    static constexpr const char* name = "Moby_Class441_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 444, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class444_Update {
    static constexpr const char* id = "rac1.moby.moby444.update";
    static constexpr const char* name = "Moby_Class444_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bolt_thief (452), each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltThief_Update {
    static constexpr const char* id = "rac1.moby.batalia_bolt_thief.update";
    static constexpr const char* name = "Moby_BoltThief_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (458), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket458_Update {
    static constexpr const char* id = "rac1.moby.moby458.update";
    static constexpr const char* name = "Moby_Rocket458_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shrub (462), each tick. Experimental: from the behaviour catalogue.
struct Moby_Shrub_Update {
    static constexpr const char* id = "rac1.moby.moby462.update";
    static constexpr const char* name = "Moby_Shrub_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 463, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class463_Update {
    static constexpr const char* id = "rac1.moby.swarm_leader.update";
    static constexpr const char* name = "Moby_Class463_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 467, 472, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class467_Update {
    static constexpr const char* id = "rac1.moby.group_clear.update";
    static constexpr const char* name = "Moby_Class467_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 468, 469, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class468_Update {
    static constexpr const char* id = "rac1.moby.gate_swing.update";
    static constexpr const char* name = "Moby_Class468_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 600, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class600_Update {
    static constexpr const char* id = "rac1.moby.moby600.update";
    static constexpr const char* name = "Moby_Class600_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 621, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class621_Update {
    static constexpr const char* id = "rac1.moby.moby621.update";
    static constexpr const char* name = "Moby_Class621_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 648, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class648_Update {
    static constexpr const char* id = "rac1.moby.moby648.update";
    static constexpr const char* name = "Moby_Class648_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 671, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class671_Update {
    static constexpr const char* id = "rac1.moby.flicker_light.update";
    static constexpr const char* name = "Moby_Class671_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_paratrooper (1023), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargParatrooper_Update {
    static constexpr const char* id = "rac1.moby.paratrooper.update";
    static constexpr const char* name = "Moby_BlargParatrooper_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class commando (1130), each tick. Experimental: from the behaviour catalogue.
struct Moby_Commando_Update {
    static constexpr const char* id = "rac1.moby.commando.update";
    static constexpr const char* name = "Moby_Commando_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class batalia_deserter (1144), each tick. Experimental: from the behaviour catalogue.
struct Moby_BataliaDeserter_Update {
    static constexpr const char* id = "rac1.moby.deserter.update";
    static constexpr const char* name = "Moby_BataliaDeserter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class the_plumber_batalia (1283), each tick. Experimental: from the behaviour catalogue.
struct Moby_ThePlumberBatalia_Update {
    static constexpr const char* id = "rac1.moby.moby1283.update";
    static constexpr const char* name = "Moby_ThePlumberBatalia_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1292, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1292_Update {
    static constexpr const char* id = "rac1.moby.fire_bolt.update";
    static constexpr const char* name = "Moby_Class1292_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1349, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1349_Update {
    static constexpr const char* id = "rac1.moby.moby1349.update";
    static constexpr const char* name = "Moby_Class1349_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1400, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1400_Update {
    static constexpr const char* id = "rac1.moby.moby1400.update";
    static constexpr const char* name = "Moby_Class1400_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1553, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1553_Update {
    static constexpr const char* id = "rac1.moby.moby1553.update";
    static constexpr const char* name = "Moby_Class1553_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1629, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1629_Update {
    static constexpr const char* id = "rac1.moby.moby1629.update";
    static constexpr const char* name = "Moby_Class1629_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1641, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1641_Update {
    static constexpr const char* id = "rac1.moby.moby1641.update";
    static constexpr const char* name = "Moby_Class1641_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1649, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1649_Update {
    static constexpr const char* id = "rac1.moby.moby1649.update";
    static constexpr const char* name = "Moby_Class1649_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes airship_part (1182), airship_part (1183), airship_part (1184), airship_part (1185) and 4 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_AirshipPart_Update {
    static constexpr const char* id = "rac1.moby.airship_part.update";
    static constexpr const char* name = "Moby_AirshipPart_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class seeker (52), each tick. Experimental: from the behaviour catalogue.
struct Moby_Seeker_Update {
    static constexpr const char* id = "rac1.moby.moby52.update";
    static constexpr const char* name = "Moby_Seeker_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 193, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class193_Update {
    static constexpr const char* id = "rac1.moby.ground_enemy.update";
    static constexpr const char* name = "Moby_Class193_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes asteroid (263), asteroid (264), asteroid (265), asteroid (266) and 1 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Asteroid_Update {
    static constexpr const char* id = "rac1.moby.asteroid_belt.update";
    static constexpr const char* name = "Moby_Asteroid_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes display (276), display_damaged (1298), display_damaged (1299), each tick. Experimental: from the behaviour catalogue.
struct Moby_Display_Update {
    static constexpr const char* id = "rac1.moby.breakable_stack.update";
    static constexpr const char* name = "Moby_Display_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 279, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class279_Update {
    static constexpr const char* id = "rac1.moby.moby279.update";
    static constexpr const char* name = "Moby_Class279_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 313, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class313_Update {
    static constexpr const char* id = "rac1.moby.tumbling_debris.update";
    static constexpr const char* name = "Moby_Class313_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 320, 321, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class320_Update {
    static constexpr const char* id = "rac1.moby.heavy_tumbling_debris.update";
    static constexpr const char* name = "Moby_Class320_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 417, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class417_Update {
    static constexpr const char* id = "rac1.moby.moby417.update";
    static constexpr const char* name = "Moby_Class417_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform (664), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform664_Update {
    static constexpr const char* id = "rac1.moby.moby664.update";
    static constexpr const char* name = "Moby_Platform664_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1000, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1000_Update {
    static constexpr const char* id = "rac1.moby.moby1000.update";
    static constexpr const char* name = "Moby_Class1000_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes elevator (1150), elevator (1151), each tick. Experimental: from the behaviour catalogue.
struct Moby_Elevator1150_Update {
    static constexpr const char* id = "rac1.moby.path_platform_l09.update";
    static constexpr const char* name = "Moby_Elevator1150_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class refueling_station (1172), each tick. Experimental: from the behaviour catalogue.
struct Moby_RefuelingStation_Update {
    static constexpr const char* id = "rac1.moby.pressure_plate.update";
    static constexpr const char* name = "Moby_RefuelingStation_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class airship_part (1181), each tick. Experimental: from the behaviour catalogue.
struct Moby_AirshipPart1181_Update {
    static constexpr const char* id = "rac1.moby.airship_chain_link.update";
    static constexpr const char* name = "Moby_AirshipPart1181_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class turret (1201), each tick. Experimental: from the behaviour catalogue.
struct Moby_Turret_Update {
    static constexpr const char* id = "rac1.moby.mountable_turret.update";
    static constexpr const char* name = "Moby_Turret_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1206, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1206_Update {
    static constexpr const char* id = "rac1.moby.moby1206.update";
    static constexpr const char* name = "Moby_Class1206_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1258, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1258_Update {
    static constexpr const char* id = "rac1.moby.enemy_shot.update";
    static constexpr const char* name = "Moby_Class1258_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes barrier (1285), barrier (1286), barrier (1287), barrier (1288), each tick. Experimental: from the behaviour catalogue.
struct Moby_Barrier_Update {
    static constexpr const char* id = "rac1.moby.barrier.update";
    static constexpr const char* name = "Moby_Barrier_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1293, 1320, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1293_Update {
    static constexpr const char* id = "rac1.moby.linked_riser.update";
    static constexpr const char* name = "Moby_Class1293_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shuttle (1766), each tick. Experimental: from the behaviour catalogue.
struct Moby_Shuttle1766_Update {
    static constexpr const char* id = "rac1.moby.moby1766.update";
    static constexpr const char* name = "Moby_Shuttle1766_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class spawner (1885), each tick. Experimental: from the behaviour catalogue.
struct Moby_Spawner1885_Update {
    static constexpr const char* id = "rac1.moby.moby1885.update";
    static constexpr const char* name = "Moby_Spawner1885_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class spawner_egg (1886), each tick. Experimental: from the behaviour catalogue.
struct Moby_SpawnerEgg_Update {
    static constexpr const char* id = "rac1.moby.moby1886.update";
    static constexpr const char* name = "Moby_SpawnerEgg_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform (1033), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform1033_Update {
    static constexpr const char* id = "rac1.moby.moby1033.update";
    static constexpr const char* name = "Moby_Platform1033_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform (1117), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform1117_Update {
    static constexpr const char* id = "rac1.moby.bobbing_platform.update";
    static constexpr const char* name = "Moby_Platform1117_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class magnetboots (18), each tick. Experimental: from the behaviour catalogue.
struct Moby_Magnetboots_Update {
    static constexpr const char* id = "rac1.moby.spinning_pickup.update";
    static constexpr const char* name = "Moby_Magnetboots_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 22, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class22_Update {
    static constexpr const char* id = "rac1.moby.moby22.update";
    static constexpr const char* name = "Moby_Class22_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes gadgebot_telepot_pad (351), bot_elevator (1301), each tick. Experimental: from the behaviour catalogue.
struct Moby_GadgebotTelepotPad_Update {
    static constexpr const char* id = "rac1.moby.near_hero_sparkle.update";
    static constexpr const char* name = "Moby_GadgebotTelepotPad_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 702, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class702_Update {
    static constexpr const char* id = "rac1.moby.moby702.update";
    static constexpr const char* name = "Moby_Class702_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 794, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class794_Update {
    static constexpr const char* id = "rac1.moby.moby794.update";
    static constexpr const char* name = "Moby_Class794_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (819), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket819_Update {
    static constexpr const char* id = "rac1.moby.moby819.update";
    static constexpr const char* name = "Moby_Rocket819_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 859, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class859_Update {
    static constexpr const char* id = "rac1.moby.moby859.update";
    static constexpr const char* name = "Moby_Class859_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (938), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb938_Update {
    static constexpr const char* id = "rac1.moby.moby938.update";
    static constexpr const char* name = "Moby_Bomb938_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bombthrower_grass (939), each tick. Experimental: from the behaviour catalogue.
struct Moby_BombthrowerGrass939_Update {
    static constexpr const char* id = "rac1.moby.timed_shooter.update";
    static constexpr const char* name = "Moby_BombthrowerGrass939_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 947, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class947_Update {
    static constexpr const char* id = "rac1.moby.electric_spark.update";
    static constexpr const char* name = "Moby_Class947_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes lab_door (1015), flame_door (1282), each tick. Experimental: from the behaviour catalogue.
struct Moby_LabDoor_Update {
    static constexpr const char* id = "rac1.moby.height_glide.update";
    static constexpr const char* name = "Moby_LabDoor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class button (1031), each tick. Experimental: from the behaviour catalogue.
struct Moby_Button_Update {
    static constexpr const char* id = "rac1.moby.moby1031.update";
    static constexpr const char* name = "Moby_Button_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1047, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1047_Update {
    static constexpr const char* id = "rac1.moby.moby1047.update";
    static constexpr const char* name = "Moby_Class1047_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1067, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1067_Update {
    static constexpr const char* id = "rac1.moby.sound_prop.update";
    static constexpr const char* name = "Moby_Class1067_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1090, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1090_Update {
    static constexpr const char* id = "rac1.moby.moby1090.update";
    static constexpr const char* name = "Moby_Class1090_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class block_cave (1100), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlockCave_Update {
    static constexpr const char* id = "rac1.moby.collectible_trigger.update";
    static constexpr const char* name = "Moby_BlockCave_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class screamer (1196), each tick. Experimental: from the behaviour catalogue.
struct Moby_Screamer_Update {
    static constexpr const char* id = "rac1.moby.moby1196.update";
    static constexpr const char* name = "Moby_Screamer_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1199, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1199_Update {
    static constexpr const char* id = "rac1.moby.moby1199.update";
    static constexpr const char* name = "Moby_Class1199_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class toxic_crab (1202), each tick. Experimental: from the behaviour catalogue.
struct Moby_ToxicCrab_Update {
    static constexpr const char* id = "rac1.moby.toxic_crab.update";
    static constexpr const char* name = "Moby_ToxicCrab_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_ship (1229), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargShip_Update {
    static constexpr const char* id = "rac1.moby.blarg_ship.update";
    static constexpr const char* name = "Moby_BlargShip_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1240, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1240_Update {
    static constexpr const char* id = "rac1.moby.exhaust_vent.update";
    static constexpr const char* name = "Moby_Class1240_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class nanotech_upgrade_vendor (1326), each tick. Experimental: from the behaviour catalogue.
struct Moby_NanotechUpgradeVendor_Update {
    static constexpr const char* id = "rac1.moby.nanotech_vendor.update";
    static constexpr const char* name = "Moby_NanotechUpgradeVendor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1344, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1344_Update {
    static constexpr const char* id = "rac1.moby.moby1344.update";
    static constexpr const char* name = "Moby_Class1344_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gun (1346), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gun_Update {
    static constexpr const char* id = "rac1.moby.linked_turret.update";
    static constexpr const char* name = "Moby_Gun_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1378, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1378_Update {
    static constexpr const char* id = "rac1.moby.moby1378.update";
    static constexpr const char* name = "Moby_Class1378_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gate_1 (1421), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gate1_Update {
    static constexpr const char* id = "rac1.moby.swing_door.update";
    static constexpr const char* name = "Moby_Gate1_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gate_2 (1424), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gate2_Update {
    static constexpr const char* id = "rac1.moby.moby1424.update";
    static constexpr const char* name = "Moby_Gate2_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1544, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1544_Update {
    static constexpr const char* id = "rac1.moby.effect_emitter.update";
    static constexpr const char* name = "Moby_Class1544_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1555, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1555_Update {
    static constexpr const char* id = "rac1.moby.moby1555.update";
    static constexpr const char* name = "Moby_Class1555_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1855, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1855_Update {
    static constexpr const char* id = "rac1.moby.moby1855.update";
    static constexpr const char* name = "Moby_Class1855_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1856, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1856_Update {
    static constexpr const char* id = "rac1.moby.moby1856.update";
    static constexpr const char* name = "Moby_Class1856_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 23, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class23_Update {
    static constexpr const char* id = "rac1.moby.pickup.update";
    static constexpr const char* name = "Moby_Class23_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 37, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class37_Update {
    static constexpr const char* id = "rac1.moby.moby37.update";
    static constexpr const char* name = "Moby_Class37_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 65, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class65_Update {
    static constexpr const char* id = "rac1.moby.moby65.update";
    static constexpr const char* name = "Moby_Class65_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bob (90), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bob_Update {
    static constexpr const char* id = "rac1.moby.moby90.update";
    static constexpr const char* name = "Moby_Bob_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class resort_owner (114), each tick. Experimental: from the behaviour catalogue.
struct Moby_ResortOwner_Update {
    static constexpr const char* id = "rac1.moby.moby114.update";
    static constexpr const char* name = "Moby_ResortOwner_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class fred_pokitaru (298), each tick. Experimental: from the behaviour catalogue.
struct Moby_FredPokitaru_Update {
    static constexpr const char* id = "rac1.moby.pokitaru_vendor.update";
    static constexpr const char* name = "Moby_FredPokitaru_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class teleporter (318), each tick. Experimental: from the behaviour catalogue.
struct Moby_Teleporter318_Update {
    static constexpr const char* id = "rac1.moby.moby318.update";
    static constexpr const char* name = "Moby_Teleporter318_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 361, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class361_Update {
    static constexpr const char* id = "rac1.moby.moby361.update";
    static constexpr const char* name = "Moby_Class361_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1009, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1009_Update {
    static constexpr const char* id = "rac1.moby.class_1009.update";
    static constexpr const char* name = "Moby_Class1009_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1017, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1017_Update {
    static constexpr const char* id = "rac1.moby.fireball.update";
    static constexpr const char* name = "Moby_Class1017_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (1034), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket1034_Update {
    static constexpr const char* id = "rac1.moby.moby1034.update";
    static constexpr const char* name = "Moby_Rocket1034_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class pokitaru_boat (1075), each tick. Experimental: from the behaviour catalogue.
struct Moby_PokitaruBoat_Update {
    static constexpr const char* id = "rac1.moby.pokitaru_boat.update";
    static constexpr const char* name = "Moby_PokitaruBoat_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ocean_pokitaru (1111), each tick. Experimental: from the behaviour catalogue.
struct Moby_OceanPokitaru_Update {
    static constexpr const char* id = "rac1.moby.ocean.update";
    static constexpr const char* name = "Moby_OceanPokitaru_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1156, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1156_Update {
    static constexpr const char* id = "rac1.moby.cutscene_controller.update";
    static constexpr const char* name = "Moby_Class1156_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1157, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1157_Update {
    static constexpr const char* id = "rac1.moby.moby1157.update";
    static constexpr const char* name = "Moby_Class1157_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1158, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1158_Update {
    static constexpr const char* id = "rac1.moby.moby1158.update";
    static constexpr const char* name = "Moby_Class1158_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1159, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1159_Update {
    static constexpr const char* id = "rac1.moby.moby1159.update";
    static constexpr const char* name = "Moby_Class1159_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class seesaw_platform (1178), each tick. Experimental: from the behaviour catalogue.
struct Moby_SeesawPlatform_Update {
    static constexpr const char* id = "rac1.moby.moby1178.update";
    static constexpr const char* name = "Moby_SeesawPlatform_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class thruster_pack_lock (1179), each tick. Experimental: from the behaviour catalogue.
struct Moby_ThrusterPackLock_Update {
    static constexpr const char* id = "rac1.moby.moby1179.update";
    static constexpr const char* name = "Moby_ThrusterPackLock_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1180, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1180_Update {
    static constexpr const char* id = "rac1.moby.moby1180.update";
    static constexpr const char* name = "Moby_Class1180_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes ammocrate (1218), jet_fighter_health_crate (1220), each tick. Experimental: from the behaviour catalogue.
struct Moby_Ammocrate_Update {
    static constexpr const char* id = "rac1.moby.ship_collider.update";
    static constexpr const char* name = "Moby_Ammocrate_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1219, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1219_Update {
    static constexpr const char* id = "rac1.moby.moby1219.update";
    static constexpr const char* name = "Moby_Class1219_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class psytcopus (1231), each tick. Experimental: from the behaviour catalogue.
struct Moby_Psytcopus_Update {
    static constexpr const char* id = "rac1.moby.psytcopus.update";
    static constexpr const char* name = "Moby_Psytcopus_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class jet_fighter (1242), each tick. Experimental: from the behaviour catalogue.
struct Moby_JetFighter_Update {
    static constexpr const char* id = "rac1.moby.jet_fighter.update";
    static constexpr const char* name = "Moby_JetFighter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class puffoid (1246), each tick. Experimental: from the behaviour catalogue.
struct Moby_Puffoid_Update {
    static constexpr const char* id = "rac1.moby.moby1246.update";
    static constexpr const char* name = "Moby_Puffoid_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1248, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1248_Update {
    static constexpr const char* id = "rac1.moby.gate.update";
    static constexpr const char* name = "Moby_Class1248_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class oil_tanker_front (1264), each tick. Experimental: from the behaviour catalogue.
struct Moby_OilTankerFront_Update {
    static constexpr const char* id = "rac1.moby.moby1264.update";
    static constexpr const char* name = "Moby_OilTankerFront_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1265, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1265_Update {
    static constexpr const char* id = "rac1.moby.bubbling_vent.update";
    static constexpr const char* name = "Moby_Class1265_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class energy_sphere (1297), each tick. Experimental: from the behaviour catalogue.
struct Moby_EnergySphere1297_Update {
    static constexpr const char* id = "rac1.moby.moby1297.update";
    static constexpr const char* name = "Moby_EnergySphere1297_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_ship (1319), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargShip1319_Update {
    static constexpr const char* id = "rac1.moby.flying_craft.update";
    static constexpr const char* name = "Moby_BlargShip1319_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1350, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1350_Update {
    static constexpr const char* id = "rac1.moby.moby1350.update";
    static constexpr const char* name = "Moby_Class1350_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1524, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1524_Update {
    static constexpr const char* id = "rac1.moby.bubble.update";
    static constexpr const char* name = "Moby_Class1524_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1903, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1903_Update {
    static constexpr const char* id = "rac1.moby.pokitaru_surface_near.update";
    static constexpr const char* name = "Moby_Class1903_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1919, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1919_Update {
    static constexpr const char* id = "rac1.moby.pokitaru_surface_far.update";
    static constexpr const char* name = "Moby_Class1919_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 19, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class19_Update {
    static constexpr const char* id = "rac1.moby.moby19.update";
    static constexpr const char* name = "Moby_Class19_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 238, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class238_Update {
    static constexpr const char* id = "rac1.moby.hoven_troll.update";
    static constexpr const char* name = "Moby_Class238_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform (240), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform240_Update {
    static constexpr const char* id = "rac1.moby.hoven_platform.update";
    static constexpr const char* name = "Moby_Platform240_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class miner (282), each tick. Experimental: from the behaviour catalogue.
struct Moby_Miner_Update {
    static constexpr const char* id = "rac1.moby.miner.update";
    static constexpr const char* name = "Moby_Miner_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 293, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class293_Update {
    static constexpr const char* id = "rac1.moby.moby293.update";
    static constexpr const char* name = "Moby_Class293_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_trooper (294), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargTrooper294_Update {
    static constexpr const char* id = "rac1.moby.moby294.update";
    static constexpr const char* name = "Moby_BlargTrooper294_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_helicopter (326), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargHelicopter326_Update {
    static constexpr const char* id = "rac1.moby.moby326.update";
    static constexpr const char* name = "Moby_BlargHelicopter326_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class edwina (328), each tick. Experimental: from the behaviour catalogue.
struct Moby_Edwina_Update {
    static constexpr const char* id = "rac1.moby.edwina.update";
    static constexpr const char* name = "Moby_Edwina_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class helicopter (336), each tick. Experimental: from the behaviour catalogue.
struct Moby_Helicopter_Update {
    static constexpr const char* id = "rac1.moby.moby336.update";
    static constexpr const char* name = "Moby_Helicopter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class crane_arm (339), each tick. Experimental: from the behaviour catalogue.
struct Moby_CraneArm_Update {
    static constexpr const char* id = "rac1.moby.moby339.update";
    static constexpr const char* name = "Moby_CraneArm_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class water_reservoir (384), each tick. Experimental: from the behaviour catalogue.
struct Moby_WaterReservoir_Update {
    static constexpr const char* id = "rac1.moby.water_reservoir.update";
    static constexpr const char* name = "Moby_WaterReservoir_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (409), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket409_Update {
    static constexpr const char* id = "rac1.moby.hoven_shot.update";
    static constexpr const char* name = "Moby_Rocket409_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 422, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class422_Update {
    static constexpr const char* id = "rac1.moby.hoven_challenge.update";
    static constexpr const char* name = "Moby_Class422_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1259, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1259_Update {
    static constexpr const char* id = "rac1.moby.moby1259.update";
    static constexpr const char* name = "Moby_Class1259_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class hoven_turret (1267), each tick. Experimental: from the behaviour catalogue.
struct Moby_HovenTurret_Update {
    static constexpr const char* id = "rac1.moby.moby1267.update";
    static constexpr const char* name = "Moby_HovenTurret_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (1269), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine1269_Update {
    static constexpr const char* id = "rac1.moby.hoven_homing_mine.update";
    static constexpr const char* name = "Moby_Mine1269_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class planetbuster_maximus_part (1274), each tick. Experimental: from the behaviour catalogue.
struct Moby_PlanetbusterMaximusPart_Update {
    static constexpr const char* id = "rac1.moby.planetbuster_part.update";
    static constexpr const char* name = "Moby_PlanetbusterMaximusPart_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class homing_mine_spawner (1281), each tick. Experimental: from the behaviour catalogue.
struct Moby_HomingMineSpawner_Update {
    static constexpr const char* id = "rac1.moby.door_switch.update";
    static constexpr const char* name = "Moby_HomingMineSpawner_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1345, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1345_Update {
    static constexpr const char* id = "rac1.moby.moby1345.update";
    static constexpr const char* name = "Moby_Class1345_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_commander (1371), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargCommander_Update {
    static constexpr const char* id = "rac1.moby.blarg_commander.update";
    static constexpr const char* name = "Moby_BlargCommander_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1404, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1404_Update {
    static constexpr const char* id = "rac1.moby.cutscene_volume_trigger.update";
    static constexpr const char* name = "Moby_Class1404_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1557, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1557_Update {
    static constexpr const char* id = "rac1.moby.class_1557.update";
    static constexpr const char* name = "Moby_Class1557_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1901, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1901_Update {
    static constexpr const char* id = "rac1.moby.moby1901.update";
    static constexpr const char* name = "Moby_Class1901_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 6, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class6_Update {
    static constexpr const char* id = "rac1.moby.clip_volume_trigger.update";
    static constexpr const char* name = "Moby_Class6_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes elevator (21), elevator (244), each tick. Experimental: from the behaviour catalogue.
struct Moby_Elevator21_Update {
    static constexpr const char* id = "rac1.moby.elevator_21.update";
    static constexpr const char* name = "Moby_Elevator21_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_turret (29), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargTurret_Update {
    static constexpr const char* id = "rac1.moby.blarg_turret.update";
    static constexpr const char* name = "Moby_BlargTurret_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_gunner (36), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargGunner_Update {
    static constexpr const char* id = "rac1.moby.gem_lock_turret.update";
    static constexpr const char* name = "Moby_BlargGunner_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class robosquawker (63), each tick. Experimental: from the behaviour catalogue.
struct Moby_Robosquawker_Update {
    static constexpr const char* id = "rac1.moby.robosquawker.update";
    static constexpr const char* name = "Moby_Robosquawker_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class fighter_jet (69), each tick. Experimental: from the behaviour catalogue.
struct Moby_FighterJet69_Update {
    static constexpr const char* id = "rac1.moby.fighter_jet_69.update";
    static constexpr const char* name = "Moby_FighterJet69_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (82), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket82_Update {
    static constexpr const char* id = "rac1.moby.homing_rocket82.update";
    static constexpr const char* name = "Moby_Rocket82_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (83), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine83_Update {
    static constexpr const char* id = "rac1.moby.moby83.update";
    static constexpr const char* name = "Moby_Mine83_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_space_fighter (101), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargSpaceFighter_Update {
    static constexpr const char* id = "rac1.moby.blarg_space_fighter.update";
    static constexpr const char* name = "Moby_BlargSpaceFighter_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_space_fighter (111), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargSpaceFighter111_Update {
    static constexpr const char* id = "rac1.moby.blarg_fighter111.update";
    static constexpr const char* name = "Moby_BlargSpaceFighter111_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 127, 128, 159, 169, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class127_Update {
    static constexpr const char* id = "rac1.moby.rotating_door.update";
    static constexpr const char* name = "Moby_Class127_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class forcefield_tower (170), each tick. Experimental: from the behaviour catalogue.
struct Moby_ForcefieldTower_Update {
    static constexpr const char* id = "rac1.moby.forcefield_tower.update";
    static constexpr const char* name = "Moby_ForcefieldTower_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes asteroid (212), asteroid (1412), each tick. Experimental: from the behaviour catalogue.
struct Moby_Asteroid212_Update {
    static constexpr const char* id = "rac1.moby.drifting_blob.update";
    static constexpr const char* name = "Moby_Asteroid212_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes ammocrate (224), jet_fighter_health_crate (228), each tick. Experimental: from the behaviour catalogue.
struct Moby_Ammocrate224_Update {
    static constexpr const char* id = "rac1.moby.ship_pickup.update";
    static constexpr const char* name = "Moby_Ammocrate224_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 231, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class231_Update {
    static constexpr const char* id = "rac1.moby.switch.update";
    static constexpr const char* name = "Moby_Class231_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (295), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket295_Update {
    static constexpr const char* id = "rac1.moby.rocket295.update";
    static constexpr const char* name = "Moby_Rocket295_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 352, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class352_Update {
    static constexpr const char* id = "rac1.moby.fade_in_hold_out.update";
    static constexpr const char* name = "Moby_Class352_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class qwarks_starfighter_part (388), each tick. Experimental: from the behaviour catalogue.
struct Moby_QwarksStarfighterPart_Update {
    static constexpr const char* id = "rac1.moby.qwark_starfighter_part.update";
    static constexpr const char* name = "Moby_QwarksStarfighterPart_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes qwarks_starfighter_part (389), qwarks_starfighter_part (390), qwarks_starfighter_part (391), qwarks_starfighter_part (392) and 7 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_QwarksStarfighterPart389_Update {
    static constexpr const char* id = "rac1.moby.starfighter_part.update";
    static constexpr const char* name = "Moby_QwarksStarfighterPart389_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes forcefield (404), forcefield (405), each tick. Experimental: from the behaviour catalogue.
struct Moby_Forcefield_Update {
    static constexpr const char* id = "rac1.moby.gem_lock_pickup.update";
    static constexpr const char* name = "Moby_Forcefield_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class purple_ship (533), each tick. Experimental: from the behaviour catalogue.
struct Moby_PurpleShip533_Update {
    static constexpr const char* id = "rac1.moby.purple_ship_533.update";
    static constexpr const char* name = "Moby_PurpleShip533_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 558, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class558_Update {
    static constexpr const char* id = "rac1.moby.help_volume.update";
    static constexpr const char* name = "Moby_Class558_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 667, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class667_Update {
    static constexpr const char* id = "rac1.moby.moby_state_relay.update";
    static constexpr const char* name = "Moby_Class667_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 674, 677, 680, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class674_Update {
    static constexpr const char* id = "rac1.moby.controlled_gate.update";
    static constexpr const char* name = "Moby_Class674_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1238, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1238_Update {
    static constexpr const char* id = "rac1.moby.projectile_l13.update";
    static constexpr const char* name = "Moby_Class1238_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class poison_vessel (1261), each tick. Experimental: from the behaviour catalogue.
struct Moby_PoisonVessel_Update {
    static constexpr const char* id = "rac1.moby.moby1261.update";
    static constexpr const char* name = "Moby_PoisonVessel_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_space_commando (1262), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargSpaceCommando1262_Update {
    static constexpr const char* id = "rac1.moby.blarg_space_commando.update";
    static constexpr const char* name = "Moby_BlargSpaceCommando1262_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1263, 1393, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1263_Update {
    static constexpr const char* id = "rac1.moby.bobbing_lights.update";
    static constexpr const char* name = "Moby_Class1263_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1270, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1270_Update {
    static constexpr const char* id = "rac1.moby.lift1270.update";
    static constexpr const char* name = "Moby_Class1270_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1271, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1271_Update {
    static constexpr const char* id = "rac1.moby.class_1271.update";
    static constexpr const char* name = "Moby_Class1271_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1353, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1353_Update {
    static constexpr const char* id = "rac1.moby.moby1353.update";
    static constexpr const char* name = "Moby_Class1353_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1403, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1403_Update {
    static constexpr const char* id = "rac1.moby.class_1403.update";
    static constexpr const char* name = "Moby_Class1403_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1558, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1558_Update {
    static constexpr const char* id = "rac1.moby.sparks_trigger_l13.update";
    static constexpr const char* name = "Moby_Class1558_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1577, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1577_Update {
    static constexpr const char* id = "rac1.moby.watch_mobys.update";
    static constexpr const char* name = "Moby_Class1577_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1632, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1632_Update {
    static constexpr const char* id = "rac1.moby.moby1632.update";
    static constexpr const char* name = "Moby_Class1632_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1634, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1634_Update {
    static constexpr const char* id = "rac1.moby.moby1634.update";
    static constexpr const char* name = "Moby_Class1634_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_ship (1805), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargShip1805_Update {
    static constexpr const char* id = "rac1.moby.moby1805.update";
    static constexpr const char* name = "Moby_BlargShip1805_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform (903), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform903_Update {
    static constexpr const char* id = "rac1.moby.keyframe_platform.update";
    static constexpr const char* name = "Moby_Platform903_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_heavy_interceptor (8), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargHeavyInterceptor_Update {
    static constexpr const char* id = "rac1.moby.moby8.update";
    static constexpr const char* name = "Moby_BlargHeavyInterceptor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class plasmabot (28), each tick. Experimental: from the behaviour catalogue.
struct Moby_Plasmabot_Update {
    static constexpr const char* id = "rac1.moby.plasmabot.update";
    static constexpr const char* name = "Moby_Plasmabot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class machinegun_turret (30), each tick. Experimental: from the behaviour catalogue.
struct Moby_MachinegunTurret30_Update {
    static constexpr const char* id = "rac1.moby.machinegun_turret_30.update";
    static constexpr const char* name = "Moby_MachinegunTurret30_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class minibomber (31), each tick. Experimental: from the behaviour catalogue.
struct Moby_Minibomber_Update {
    static constexpr const char* id = "rac1.moby.minibomber.update";
    static constexpr const char* name = "Moby_Minibomber_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 81, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class81_Update {
    static constexpr const char* id = "rac1.moby.moby81.update";
    static constexpr const char* name = "Moby_Class81_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class plasmabot (211), each tick. Experimental: from the behaviour catalogue.
struct Moby_Plasmabot211_Update {
    static constexpr const char* id = "rac1.moby.moby211.update";
    static constexpr const char* name = "Moby_Plasmabot211_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class plasmabot_hatch (250), each tick. Experimental: from the behaviour catalogue.
struct Moby_PlasmabotHatch_Update {
    static constexpr const char* id = "rac1.moby.moby250.update";
    static constexpr const char* name = "Moby_PlasmabotHatch_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class platform (309), each tick. Experimental: from the behaviour catalogue.
struct Moby_Platform309_Update {
    static constexpr const char* id = "rac1.moby.moby309.update";
    static constexpr const char* name = "Moby_Platform309_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 325, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class325_Update {
    static constexpr const char* id = "rac1.moby.debris_particle.update";
    static constexpr const char* name = "Moby_Class325_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 386, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class386_Update {
    static constexpr const char* id = "rac1.moby.gem_lock_sentry.update";
    static constexpr const char* name = "Moby_Class386_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 403, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class403_Update {
    static constexpr const char* id = "rac1.moby.race_debris.update";
    static constexpr const char* name = "Moby_Class403_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shuttle (557), each tick. Experimental: from the behaviour catalogue.
struct Moby_Shuttle557_Update {
    static constexpr const char* id = "rac1.moby.path_lift.update";
    static constexpr const char* name = "Moby_Shuttle557_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 610, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class610_Update {
    static constexpr const char* id = "rac1.moby.moby610.update";
    static constexpr const char* name = "Moby_Class610_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (643), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine643_Update {
    static constexpr const char* id = "rac1.moby.mine_l14.update";
    static constexpr const char* name = "Moby_Mine643_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 681, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class681_Update {
    static constexpr const char* id = "rac1.moby.sweeping_bolt.update";
    static constexpr const char* name = "Moby_Class681_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 684, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class684_Update {
    static constexpr const char* id = "rac1.moby.class_684.update";
    static constexpr const char* name = "Moby_Class684_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class elevator (685), each tick. Experimental: from the behaviour catalogue.
struct Moby_Elevator685_Update {
    static constexpr const char* id = "rac1.moby.moby685.update";
    static constexpr const char* name = "Moby_Elevator685_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi (712), each tick. Experimental: from the behaviour catalogue.
struct Moby_Taxi712_Update {
    static constexpr const char* id = "rac1.moby.taxi.update";
    static constexpr const char* name = "Moby_Taxi712_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class qwark_oltanis (851), each tick. Experimental: from the behaviour catalogue.
struct Moby_QwarkOltanis_Update {
    static constexpr const char* id = "rac1.moby.moby851.update";
    static constexpr const char* name = "Moby_QwarkOltanis_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class jet (908), each tick. Experimental: from the behaviour catalogue.
struct Moby_Jet_Update {
    static constexpr const char* id = "rac1.moby.moby908.update";
    static constexpr const char* name = "Moby_Jet_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class jet (921), each tick. Experimental: from the behaviour catalogue.
struct Moby_Jet921_Update {
    static constexpr const char* id = "rac1.moby.oltanis_jet.update";
    static constexpr const char* name = "Moby_Jet921_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (922), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket922_Update {
    static constexpr const char* id = "rac1.moby.flyer.update";
    static constexpr const char* name = "Moby_Rocket922_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bolt_thief (923), each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltThief923_Update {
    static constexpr const char* id = "rac1.moby.bolt_thief.update";
    static constexpr const char* name = "Moby_BoltThief923_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sam (924), each tick. Experimental: from the behaviour catalogue.
struct Moby_Sam_Update {
    static constexpr const char* id = "rac1.moby.moby924.update";
    static constexpr const char* name = "Moby_Sam_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (933), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine933_Update {
    static constexpr const char* id = "rac1.moby.floating_mine.update";
    static constexpr const char* name = "Moby_Mine933_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1193, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1193_Update {
    static constexpr const char* id = "rac1.moby.bouncing_spark.update";
    static constexpr const char* name = "Moby_Class1193_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1224, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1224_Update {
    static constexpr const char* id = "rac1.moby.moby1224.update";
    static constexpr const char* name = "Moby_Class1224_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1331, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1331_Update {
    static constexpr const char* id = "rac1.moby.moby1331.update";
    static constexpr const char* name = "Moby_Class1331_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1352, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1352_Update {
    static constexpr const char* id = "rac1.moby.triggered_unlock.update";
    static constexpr const char* name = "Moby_Class1352_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class morphoray (1354), each tick. Experimental: from the behaviour catalogue.
struct Moby_Morphoray1354_Update {
    static constexpr const char* id = "rac1.moby.collectible_pickup.update";
    static constexpr const char* name = "Moby_Morphoray1354_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1395, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1395_Update {
    static constexpr const char* id = "rac1.moby.moby1395.update";
    static constexpr const char* name = "Moby_Class1395_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1397, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1397_Update {
    static constexpr const char* id = "rac1.moby.moby1397.update";
    static constexpr const char* name = "Moby_Class1397_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class button (1416), each tick. Experimental: from the behaviour catalogue.
struct Moby_Button1416_Update {
    static constexpr const char* id = "rac1.moby.step_on_switch.update";
    static constexpr const char* name = "Moby_Button1416_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_heavy_interceptor (1417), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargHeavyInterceptor1417_Update {
    static constexpr const char* id = "rac1.moby.blarg_interceptor.update";
    static constexpr const char* name = "Moby_BlargHeavyInterceptor1417_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1418, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1418_Update {
    static constexpr const char* id = "rac1.moby.queue_effect_draw.update";
    static constexpr const char* name = "Moby_Class1418_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1559, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1559_Update {
    static constexpr const char* id = "rac1.moby.skill_point.update";
    static constexpr const char* name = "Moby_Class1559_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sentrybot (44), each tick. Experimental: from the behaviour catalogue.
struct Moby_Sentrybot_Update {
    static constexpr const char* id = "rac1.moby.sentrybot.update";
    static constexpr const char* name = "Moby_Sentrybot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (49), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket49_Update {
    static constexpr const char* id = "rac1.moby.projectile.update";
    static constexpr const char* name = "Moby_Rocket49_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 67, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class67_Update {
    static constexpr const char* id = "rac1.moby.moby67.update";
    static constexpr const char* name = "Moby_Class67_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 256, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class256_Update {
    static constexpr const char* id = "rac1.moby.class_256.update";
    static constexpr const char* name = "Moby_Class256_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1427, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1427_Update {
    static constexpr const char* id = "rac1.moby.scale_fade_and_delete.update";
    static constexpr const char* name = "Moby_Class1427_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1523, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1523_Update {
    static constexpr const char* id = "rac1.moby.moby1523.update";
    static constexpr const char* name = "Moby_Class1523_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (77), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine77_Update {
    static constexpr const char* id = "rac1.moby.moby77.update";
    static constexpr const char* name = "Moby_Mine77_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 78, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class78_Update {
    static constexpr const char* id = "rac1.moby.moby78.update";
    static constexpr const char* name = "Moby_Class78_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gate (92), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gate92_Update {
    static constexpr const char* id = "rac1.moby.sliding_gate.update";
    static constexpr const char* name = "Moby_Gate92_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gate (93), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gate93_Update {
    static constexpr const char* id = "rac1.moby.clip_volume_door.update";
    static constexpr const char* name = "Moby_Gate93_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class spiderbot (123), each tick. Experimental: from the behaviour catalogue.
struct Moby_Spiderbot_Update {
    static constexpr const char* id = "rac1.moby.spiderbot.update";
    static constexpr const char* name = "Moby_Spiderbot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 148, 255, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class148_Update {
    static constexpr const char* id = "rac1.moby.class_148.update";
    static constexpr const char* name = "Moby_Class148_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 154, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class154_Update {
    static constexpr const char* id = "rac1.moby.hit_reaction_effect_alt.update";
    static constexpr const char* name = "Moby_Class154_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes door (196), door (197), underwater_door (1958), each tick. Experimental: from the behaviour catalogue.
struct Moby_Door196_Update {
    static constexpr const char* id = "rac1.moby.moving_moby.update";
    static constexpr const char* name = "Moby_Door196_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class construction_arm (221), each tick. Experimental: from the behaviour catalogue.
struct Moby_ConstructionArm_Update {
    static constexpr const char* id = "rac1.moby.construction_arm.update";
    static constexpr const char* name = "Moby_ConstructionArm_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class helicopter (233), each tick. Experimental: from the behaviour catalogue.
struct Moby_Helicopter233_Update {
    static constexpr const char* id = "rac1.moby.helicopter.update";
    static constexpr const char* name = "Moby_Helicopter233_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class sentrybot_alarm (408), each tick. Experimental: from the behaviour catalogue.
struct Moby_SentrybotAlarm_Update {
    static constexpr const char* id = "rac1.moby.sentrybot_alarm.update";
    static constexpr const char* name = "Moby_SentrybotAlarm_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ultramech_unlimited (491), each tick. Experimental: from the behaviour catalogue.
struct Moby_UltramechUnlimited_Update {
    static constexpr const char* id = "rac1.moby.ultramech.update";
    static constexpr const char* name = "Moby_UltramechUnlimited_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 655, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class655_Update {
    static constexpr const char* id = "rac1.moby.class_655.update";
    static constexpr const char* name = "Moby_Class655_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 934, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class934_Update {
    static constexpr const char* id = "rac1.moby.moby934.update";
    static constexpr const char* name = "Moby_Class934_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 937, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class937_Update {
    static constexpr const char* id = "rac1.moby.class_937.update";
    static constexpr const char* name = "Moby_Class937_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class underwater_ring (1209), each tick. Experimental: from the behaviour catalogue.
struct Moby_UnderwaterRing_Update {
    static constexpr const char* id = "rac1.moby.underwater_ring.update";
    static constexpr const char* name = "Moby_UnderwaterRing_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class conveyor_belt (1250), each tick. Experimental: from the behaviour catalogue.
struct Moby_ConveyorBelt1250_Update {
    static constexpr const char* id = "rac1.moby.side_drift.update";
    static constexpr const char* name = "Moby_ConveyorBelt1250_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class poison_vessel (1257), each tick. Experimental: from the behaviour catalogue.
struct Moby_PoisonVessel1257_Update {
    static constexpr const char* id = "rac1.moby.poison_vessel.update";
    static constexpr const char* name = "Moby_PoisonVessel1257_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bolt_grabber (1388), each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltGrabber_Update {
    static constexpr const char* id = "rac1.moby.moby1388.update";
    static constexpr const char* name = "Moby_BoltGrabber_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1394, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1394_Update {
    static constexpr const char* id = "rac1.moby.moby1257_spawner.update";
    static constexpr const char* name = "Moby_Class1394_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1405, 1406, 1407, 1408 and 4 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1405_Update {
    static constexpr const char* id = "rac1.moby.ripple_strips_moby.update";
    static constexpr const char* name = "Moby_Class1405_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1419, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1419_Update {
    static constexpr const char* id = "rac1.moby.moby1419.update";
    static constexpr const char* name = "Moby_Class1419_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1425, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1425_Update {
    static constexpr const char* id = "rac1.moby.class_1425.update";
    static constexpr const char* name = "Moby_Class1425_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class codebot (1428), each tick. Experimental: from the behaviour catalogue.
struct Moby_Codebot_Update {
    static constexpr const char* id = "rac1.moby.moby1428.update";
    static constexpr const char* name = "Moby_Codebot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class codebot_slot (1430), each tick. Experimental: from the behaviour catalogue.
struct Moby_CodebotSlot_Update {
    static constexpr const char* id = "rac1.moby.codebot_slot.update";
    static constexpr const char* name = "Moby_CodebotSlot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class ultramech_scientist (1446), each tick. Experimental: from the behaviour catalogue.
struct Moby_UltramechScientist_Update {
    static constexpr const char* id = "rac1.moby.ultramech_scientist.update";
    static constexpr const char* name = "Moby_UltramechScientist_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes giant_clank_pad (1451), giant_clank_pad (1899), each tick. Experimental: from the behaviour catalogue.
struct Moby_GiantClankPad_Update {
    static constexpr const char* id = "rac1.moby.giant_clank_pad.update";
    static constexpr const char* name = "Moby_GiantClankPad_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1469, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1469_Update {
    static constexpr const char* id = "rac1.moby.moby1469.update";
    static constexpr const char* name = "Moby_Class1469_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1560, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1560_Update {
    static constexpr const char* id = "rac1.moby.boss_slot_effect.update";
    static constexpr const char* name = "Moby_Class1560_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (50), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket50_Update {
    static constexpr const char* id = "rac1.moby.homing_projectile.update";
    static constexpr const char* name = "Moby_Rocket50_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (1475), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket1475_Update {
    static constexpr const char* id = "rac1.moby.homing_spark_1475.update";
    static constexpr const char* name = "Moby_Rocket1475_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 257, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class257_Update {
    static constexpr const char* id = "rac1.moby.moby257.update";
    static constexpr const char* name = "Moby_Class257_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bomb (281), each tick. Experimental: from the behaviour catalogue.
struct Moby_Bomb281_Update {
    static constexpr const char* id = "rac1.moby.thrown_bomb.update";
    static constexpr const char* name = "Moby_Bomb281_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (470), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket470_Update {
    static constexpr const char* id = "rac1.moby.moby470.update";
    static constexpr const char* name = "Moby_Rocket470_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class conveyor_belt (471), each tick. Experimental: from the behaviour catalogue.
struct Moby_ConveyorBelt471_Update {
    static constexpr const char* id = "rac1.moby.moby471.update";
    static constexpr const char* name = "Moby_ConveyorBelt471_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class test_dummy (541), each tick. Experimental: from the behaviour catalogue.
struct Moby_TestDummy_Update {
    static constexpr const char* id = "rac1.moby.test_dummy.update";
    static constexpr const char* name = "Moby_TestDummy_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class wall_button (546), each tick. Experimental: from the behaviour catalogue.
struct Moby_WallButton_Update {
    static constexpr const char* id = "rac1.moby.moby546.update";
    static constexpr const char* name = "Moby_WallButton_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 552, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class552_Update {
    static constexpr const char* id = "rac1.moby.moby552.update";
    static constexpr const char* name = "Moby_Class552_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class test_dummy_on_hoverboard (556), each tick. Experimental: from the behaviour catalogue.
struct Moby_TestDummyOnHoverboard_Update {
    static constexpr const char* id = "rac1.moby.hoverboard_racer.update";
    static constexpr const char* name = "Moby_TestDummyOnHoverboard_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_elite_commando (638), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargEliteCommando_Update {
    static constexpr const char* id = "rac1.moby.floating_enemy.update";
    static constexpr const char* name = "Moby_BlargEliteCommando_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class door (647), each tick. Experimental: from the behaviour catalogue.
struct Moby_Door647_Update {
    static constexpr const char* id = "rac1.moby.moby647.update";
    static constexpr const char* name = "Moby_Door647_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class test_dummy_door (650), each tick. Experimental: from the behaviour catalogue.
struct Moby_TestDummyDoor_Update {
    static constexpr const char* id = "rac1.moby.moby650.update";
    static constexpr const char* name = "Moby_TestDummyDoor_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 654, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class654_Update {
    static constexpr const char* id = "rac1.moby.moby654.update";
    static constexpr const char* name = "Moby_Class654_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 800, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class800_Update {
    static constexpr const char* id = "rac1.moby.moby800.update";
    static constexpr const char* name = "Moby_Class800_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class armed_transport (1356), each tick. Experimental: from the behaviour catalogue.
struct Moby_ArmedTransport_Update {
    static constexpr const char* id = "rac1.moby.armed_transport.update";
    static constexpr const char* name = "Moby_ArmedTransport_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class help_desk_lady (1377), each tick. Experimental: from the behaviour catalogue.
struct Moby_HelpDeskLady_Update {
    static constexpr const char* id = "rac1.moby.watcher.update";
    static constexpr const char* name = "Moby_HelpDeskLady_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1387, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1387_Update {
    static constexpr const char* id = "rac1.moby.moby1387.update";
    static constexpr const char* name = "Moby_Class1387_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class bolt_thief (1401), each tick. Experimental: from the behaviour catalogue.
struct Moby_BoltThief1401_Update {
    static constexpr const char* id = "rac1.moby.crate_platform.update";
    static constexpr const char* name = "Moby_BoltThief1401_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class taxi (1410), each tick. Experimental: from the behaviour catalogue.
struct Moby_Taxi1410_Update {
    static constexpr const char* id = "rac1.moby.path_pickup.update";
    static constexpr const char* name = "Moby_Taxi1410_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class grindrail_gate (1439), each tick. Experimental: from the behaviour catalogue.
struct Moby_GrindrailGate_Update {
    static constexpr const char* id = "rac1.moby.moby1439.update";
    static constexpr const char* name = "Moby_GrindrailGate_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class grindrail_gate (1441), each tick. Experimental: from the behaviour catalogue.
struct Moby_GrindrailGate1441_Update {
    static constexpr const char* id = "rac1.moby.moby1441.update";
    static constexpr const char* name = "Moby_GrindrailGate1441_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class grindrail_button (1442), each tick. Experimental: from the behaviour catalogue.
struct Moby_GrindrailButton_Update {
    static constexpr const char* id = "rac1.moby.moby1442.update";
    static constexpr const char* name = "Moby_GrindrailButton_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes grindrail_barrier (1443), 1890, each tick. Experimental: from the behaviour catalogue.
struct Moby_GrindrailBarrier_Update {
    static constexpr const char* id = "rac1.moby.ring_effect.update";
    static constexpr const char* name = "Moby_GrindrailBarrier_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class chomper (1445), each tick. Experimental: from the behaviour catalogue.
struct Moby_Chomper_Update {
    static constexpr const char* id = "rac1.moby.wandering_helper.update";
    static constexpr const char* name = "Moby_Chomper_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gadgetron_ceo (1455), each tick. Experimental: from the behaviour catalogue.
struct Moby_GadgetronCeo_Update {
    static constexpr const char* id = "rac1.moby.challenge_npc.update";
    static constexpr const char* name = "Moby_GadgetronCeo_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1561, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1561_Update {
    static constexpr const char* id = "rac1.moby.banner_trigger.update";
    static constexpr const char* name = "Moby_Class1561_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1667, 1668, 1669, 1670 and 1 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1667_Update {
    static constexpr const char* id = "rac1.moby.path_walker.update";
    static constexpr const char* name = "Moby_Class1667_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1812, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1812_Update {
    static constexpr const char* id = "rac1.moby.spark_ring_emitter.update";
    static constexpr const char* name = "Moby_Class1812_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class elevator (1826), each tick. Experimental: from the behaviour catalogue.
struct Moby_Elevator1826_Update {
    static constexpr const char* id = "rac1.moby.elevator_platform.update";
    static constexpr const char* name = "Moby_Elevator1826_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gadgetron_sign (1891), each tick. Experimental: from the behaviour catalogue.
struct Moby_GadgetronSign_Update {
    static constexpr const char* id = "rac1.moby.spinning_object.update";
    static constexpr const char* name = "Moby_GadgetronSign_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1923, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1923_Update {
    static constexpr const char* id = "rac1.moby.target_activator.update";
    static constexpr const char* name = "Moby_Class1923_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1943, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1943_Update {
    static constexpr const char* id = "rac1.moby.env_map_a.update";
    static constexpr const char* name = "Moby_Class1943_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1947, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1947_Update {
    static constexpr const char* id = "rac1.moby.class_1947.update";
    static constexpr const char* name = "Moby_Class1947_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1948, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1948_Update {
    static constexpr const char* id = "rac1.moby.env_map_b.update";
    static constexpr const char* name = "Moby_Class1948_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 1907, 1908, 1910, 1911 and 3 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1907_Update {
    static constexpr const char* id = "rac1.moby.class_1907.update";
    static constexpr const char* name = "Moby_Class1907_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1953, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1953_Update {
    static constexpr const char* id = "rac1.moby.env_map_c.update";
    static constexpr const char* name = "Moby_Class1953_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 99, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class99_Update {
    static constexpr const char* id = "rac1.moby.moby99.update";
    static constexpr const char* name = "Moby_Class99_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class dreks_fleet_turret (347), each tick. Experimental: from the behaviour catalogue.
struct Moby_DreksFleetTurret_Update {
    static constexpr const char* id = "rac1.moby.fleet_turret.update";
    static constexpr const char* name = "Moby_DreksFleetTurret_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 669, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class669_Update {
    static constexpr const char* id = "rac1.moby.moby669.update";
    static constexpr const char* name = "Moby_Class669_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 835, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class835_Update {
    static constexpr const char* id = "rac1.moby.moby835.update";
    static constexpr const char* name = "Moby_Class835_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes door (1359), door (1360), door (1361), door (1362) and 6 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Door1359_Update {
    static constexpr const char* id = "rac1.moby.stops_platform.update";
    static constexpr const char* name = "Moby_Door1359_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1368, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1368_Update {
    static constexpr const char* id = "rac1.moby.moby1368.update";
    static constexpr const char* name = "Moby_Class1368_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class fighter_jet (1379), each tick. Experimental: from the behaviour catalogue.
struct Moby_FighterJet1379_Update {
    static constexpr const char* id = "rac1.moby.fighter_jet_1379.update";
    static constexpr const char* name = "Moby_FighterJet1379_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class magnet_elevator (1380), each tick. Experimental: from the behaviour catalogue.
struct Moby_MagnetElevator_Update {
    static constexpr const char* id = "rac1.moby.moby1380.update";
    static constexpr const char* name = "Moby_MagnetElevator_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class robot_dog (1382), each tick. Experimental: from the behaviour catalogue.
struct Moby_RobotDog1382_Update {
    static constexpr const char* id = "rac1.moby.moby1382.update";
    static constexpr const char* name = "Moby_RobotDog1382_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class shuttle (1448), each tick. Experimental: from the behaviour catalogue.
struct Moby_Shuttle1448_Update {
    static constexpr const char* id = "rac1.moby.moby1448.update";
    static constexpr const char* name = "Moby_Shuttle1448_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1470, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1470_Update {
    static constexpr const char* id = "rac1.moby.moby1470.update";
    static constexpr const char* name = "Moby_Class1470_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1562, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1562_Update {
    static constexpr const char* id = "rac1.moby.moby750_spark_trigger.update";
    static constexpr const char* name = "Moby_Class1562_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1772, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1772_Update {
    static constexpr const char* id = "rac1.moby.fleet_reflection.update";
    static constexpr const char* name = "Moby_Class1772_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class blarg_ship (1843), each tick. Experimental: from the behaviour catalogue.
struct Moby_BlargShip1843_Update {
    static constexpr const char* id = "rac1.moby.moby1843.update";
    static constexpr const char* name = "Moby_BlargShip1843_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1873, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1873_Update {
    static constexpr const char* id = "rac1.moby.moby1873.update";
    static constexpr const char* name = "Moby_Class1873_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1876, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1876_Update {
    static constexpr const char* id = "rac1.moby.moby1876.update";
    static constexpr const char* name = "Moby_Class1876_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (41), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket41_Update {
    static constexpr const char* id = "rac1.moby.moby41.update";
    static constexpr const char* name = "Moby_Rocket41_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket (564), each tick. Experimental: from the behaviour catalogue.
struct Moby_Rocket564_Update {
    static constexpr const char* id = "rac1.moby.moby564.update";
    static constexpr const char* name = "Moby_Rocket564_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class mine (568), each tick. Experimental: from the behaviour catalogue.
struct Moby_Mine568_Update {
    static constexpr const char* id = "rac1.moby.moby568.update";
    static constexpr const char* name = "Moby_Mine568_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 582, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class582_Update {
    static constexpr const char* id = "rac1.moby.path_junction.update";
    static constexpr const char* name = "Moby_Class582_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class thruster_pack_lock_green (583), each tick. Experimental: from the behaviour catalogue.
struct Moby_ThrusterPackLockGreen_Update {
    static constexpr const char* id = "rac1.moby.energy_field_emitter.update";
    static constexpr const char* name = "Moby_ThrusterPackLockGreen_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class thruster_pack_lock_red (586), each tick. Experimental: from the behaviour catalogue.
struct Moby_ThrusterPackLockRed_Update {
    static constexpr const char* id = "rac1.moby.timed_lift.update";
    static constexpr const char* name = "Moby_ThrusterPackLockRed_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class floating_platform (587), each tick. Experimental: from the behaviour catalogue.
struct Moby_FloatingPlatform587_Update {
    static constexpr const char* id = "rac1.moby.sinking_platform.update";
    static constexpr const char* name = "Moby_FloatingPlatform587_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 624, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class624_Update {
    static constexpr const char* id = "rac1.moby.falling_fireball.update";
    static constexpr const char* name = "Moby_Class624_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 628, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class628_Update {
    static constexpr const char* id = "rac1.moby.moby628.update";
    static constexpr const char* name = "Moby_Class628_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 644, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class644_Update {
    static constexpr const char* id = "rac1.moby.vendor_moby.update";
    static constexpr const char* name = "Moby_Class644_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes 885, 888, 891, 892 and 4 more, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class885_Update {
    static constexpr const char* id = "rac1.moby.hit_trigger.update";
    static constexpr const char* name = "Moby_Class885_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 983, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class983_Update {
    static constexpr const char* id = "rac1.moby.homing_spark_983.update";
    static constexpr const char* name = "Moby_Class983_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class drek_fight_missile (1355), each tick. Experimental: from the behaviour catalogue.
struct Moby_DrekFightMissile_Update {
    static constexpr const char* id = "rac1.moby.moby1355.update";
    static constexpr const char* name = "Moby_DrekFightMissile_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1381, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1381_Update {
    static constexpr const char* id = "rac1.moby.moby1381.update";
    static constexpr const char* name = "Moby_Class1381_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1392, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1392_Update {
    static constexpr const char* id = "rac1.moby.moby1392.update";
    static constexpr const char* name = "Moby_Class1392_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1402, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1402_Update {
    static constexpr const char* id = "rac1.moby.moby1402.update";
    static constexpr const char* name = "Moby_Class1402_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class drek_final_boss_robot (1422), each tick. Experimental: from the behaviour catalogue.
struct Moby_DrekFinalBossRobot_Update {
    static constexpr const char* id = "rac1.moby.final_boss.update";
    static constexpr const char* name = "Moby_DrekFinalBossRobot_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1432, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1432_Update {
    static constexpr const char* id = "rac1.moby.class_1432.update";
    static constexpr const char* name = "Moby_Class1432_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of classes deplanetizer_gun (1434), 1435, each tick. Experimental: from the behaviour catalogue.
struct Moby_DeplanetizerGun_Update {
    static constexpr const char* id = "rac1.moby.moby1434.update";
    static constexpr const char* name = "Moby_DeplanetizerGun_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class rocket_tank (1454), each tick. Experimental: from the behaviour catalogue.
struct Moby_RocketTank_Update {
    static constexpr const char* id = "rac1.moby.moby1454.update";
    static constexpr const char* name = "Moby_RocketTank_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1563, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1563_Update {
    static constexpr const char* id = "rac1.moby.moby1563.update";
    static constexpr const char* name = "Moby_Class1563_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class deplanetizer_gun (1568), each tick. Experimental: from the behaviour catalogue.
struct Moby_DeplanetizerGun1568_Update {
    static constexpr const char* id = "rac1.moby.moby1568.update";
    static constexpr const char* name = "Moby_DeplanetizerGun1568_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1584, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1584_Update {
    static constexpr const char* id = "rac1.moby.moby1584.update";
    static constexpr const char* name = "Moby_Class1584_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class 1799, each tick. Experimental: from the behaviour catalogue.
struct Moby_Class1799_Update {
    static constexpr const char* id = "rac1.moby.moby1799.update";
    static constexpr const char* name = "Moby_Class1799_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

// The update of class gadgebot (1906), each tick. Experimental: from the behaviour catalogue.
struct Moby_Gadgebot1906_Update {
    static constexpr const char* id = "rac1.moby.moby1906.update";
    static constexpr const char* name = "Moby_Gadgebot1906_Update";
    static constexpr const char* signature = "void(u32)";
    using fn = void (*)(uint32_t);
};

namespace hooks {
    namespace camera {
        namespace follow {
            using apply_overrides = rac1::FollowCamera_ApplyOverrides;
        }  // namespace follow
    }  // namespace camera
    namespace combat {
        using get_hit = rac1::Moby_GetHitMessage;
    }  // namespace combat
    namespace gold_bolt {
        using count_collected = rac1::GoldBolts_CountCollected;
    }  // namespace gold_bolt
    namespace item {
        using give = rac1::Item_Give;
    }  // namespace item
    namespace level {
        using load_gameplay = rac1::Level_LoadGameplay;
    }  // namespace level
    namespace moby {
        namespace agnogg_ship {
            using update = rac1::Moby_AgnoggShipFront_Update;
        }  // namespace agnogg_ship
        namespace airship_chain_link {
            using update = rac1::Moby_AirshipPart1181_Update;
        }  // namespace airship_chain_link
        namespace airship_part {
            using update = rac1::Moby_AirshipPart_Update;
        }  // namespace airship_part
        namespace alien_queen {
            using update = rac1::Moby_AlienQueen_Update;
        }  // namespace alien_queen
        namespace ambient_stream_l03 {
            using update = rac1::Moby_Class994_Update;
        }  // namespace ambient_stream_l03
        namespace ambient_stream_l07 {
            using update = rac1::Moby_Class317_Update;
        }  // namespace ambient_stream_l07
        namespace ammo_pickup {
            using update = rac1::Moby_DevestatorAmmo_Update;
        }  // namespace ammo_pickup
        namespace amoeboid {
            using update = rac1::Moby_LargeAmoeboid_Update;
        }  // namespace amoeboid
        namespace armed_transport {
            using update = rac1::Moby_ArmedTransport_Update;
        }  // namespace armed_transport
        namespace asteroid_belt {
            using update = rac1::Moby_Asteroid_Update;
        }  // namespace asteroid_belt
        namespace audio_stream_trigger {
            using update = rac1::Moby_OceanUmbris_Update;
        }  // namespace audio_stream_trigger
        namespace background_ship {
            using update = rac1::Moby_BackgroundShip1_Update;
        }  // namespace background_ship
        namespace banner_trigger {
            using update = rac1::Moby_Class1561_Update;
        }  // namespace banner_trigger
        namespace barrier {
            using update = rac1::Moby_Barrier_Update;
        }  // namespace barrier
        namespace batalia_bolt_thief {
            using update = rac1::Moby_BoltThief_Update;
        }  // namespace batalia_bolt_thief
        namespace beam_slot_manager {
            using update = rac1::Moby_Class1471_Update;
        }  // namespace beam_slot_manager
        namespace big_axebot {
            using update = rac1::Moby_BigAxebot_Update;
        }  // namespace big_axebot
        namespace birdbot {
            using update = rac1::Moby_Birdbot_Update;
        }  // namespace birdbot
        namespace blarg_bomb_thrower {
            using update = rac1::Moby_BlargBombthrower_Update;
        }  // namespace blarg_bomb_thrower
        namespace blarg_commander {
            using update = rac1::Moby_BlargCommander_Update;
        }  // namespace blarg_commander
        namespace blarg_fighter111 {
            using update = rac1::Moby_BlargSpaceFighter111_Update;
        }  // namespace blarg_fighter111
        namespace blarg_gun {
            using update = rac1::Moby_BlargGun_Update;
        }  // namespace blarg_gun
        namespace blarg_interceptor {
            using update = rac1::Moby_BlargHeavyInterceptor1417_Update;
        }  // namespace blarg_interceptor
        namespace blarg_ship {
            using update = rac1::Moby_BlargShip_Update;
        }  // namespace blarg_ship
        namespace blarg_space_commando {
            using update = rac1::Moby_BlargSpaceCommando1262_Update;
        }  // namespace blarg_space_commando
        namespace blarg_space_fighter {
            using update = rac1::Moby_BlargSpaceFighter_Update;
        }  // namespace blarg_space_fighter
        namespace blarg_trooper {
            using update = rac1::Moby_BlargTrooper_Update;
        }  // namespace blarg_trooper
        namespace blarg_turret {
            using update = rac1::Moby_BlargTurret_Update;
        }  // namespace blarg_turret
        namespace bobbing_lights {
            using update = rac1::Moby_Class1263_Update;
        }  // namespace bobbing_lights
        namespace bobbing_moby {
            using update = rac1::Moby_Class1091_Update;
        }  // namespace bobbing_moby
        namespace bobbing_moby_l05 {
            using update = rac1::Moby_Television_Update;
        }  // namespace bobbing_moby_l05
        namespace bobbing_platform {
            using update = rac1::Moby_Platform1117_Update;
        }  // namespace bobbing_platform
        namespace bolt {
            using update = rac1::Moby_Bolt_Update;
        }  // namespace bolt
        namespace bolt_crank {
            using update = rac1::Moby_BoltCrank_Update;
        }  // namespace bolt_crank
        namespace bolt_projectile {
            using update = rac1::Moby_Class880_Update;
        }  // namespace bolt_projectile
        namespace bolt_thief {
            using update = rac1::Moby_BoltThief923_Update;
        }  // namespace bolt_thief
        namespace bomb_230 {
            using update = rac1::Moby_Bomb_Update;
        }  // namespace bomb_230
        namespace bomb_248 {
            using update = rac1::Moby_Bomb248_Update;
        }  // namespace bomb_248
        namespace bomber_tank {
            using update = rac1::Moby_BomberTank_Update;
        }  // namespace bomber_tank
        namespace boss_slot_effect {
            using update = rac1::Moby_Class1560_Update;
        }  // namespace boss_slot_effect
        namespace bouncer {
            using update = rac1::Moby_QwarksBouncer_Update;
        }  // namespace bouncer
        namespace bouncing_debris {
            using update = rac1::Moby_Class1516_Update;
        }  // namespace bouncing_debris
        namespace bouncing_spark {
            using update = rac1::Moby_Class1193_Update;
        }  // namespace bouncing_spark
        namespace break_on_hit_blarg {
            using update = rac1::Moby_Class1660_Update;
        }  // namespace break_on_hit_blarg
        namespace break_on_hit_l02 {
            using update = rac1::Moby_WhiteFlower_Update;
        }  // namespace break_on_hit_l02
        namespace breakable_pot {
            using update = rac1::Moby_GreenMushroom_Update;
        }  // namespace breakable_pot
        namespace breakable_prop {
            using update = rac1::Moby_Class1113_Update;
        }  // namespace breakable_prop
        namespace breakable_rock {
            using update = rac1::Moby_RockFormation_Update;
        }  // namespace breakable_rock
        namespace breakable_stack {
            using update = rac1::Moby_Display_Update;
        }  // namespace breakable_stack
        namespace breakable_wall {
            using update = rac1::Moby_DestroyableCaveWall_Update;
        }  // namespace breakable_wall
        namespace bridge {
            using update = rac1::Moby_Bridge_Update;
        }  // namespace bridge
        namespace bubble {
            using update = rac1::Moby_Class1524_Update;
        }  // namespace bubble
        namespace bubbling_vent {
            using update = rac1::Moby_Class1265_Update;
        }  // namespace bubbling_vent
        namespace burning_debris {
            using update = rac1::Moby_Class567_Update;
        }  // namespace burning_debris
        namespace callable_lift {
            using update = rac1::Moby_Class1061_Update;
        }  // namespace callable_lift
        namespace camera_object {
            using update = rac1::Moby_CameraObject_Update;
        }  // namespace camera_object
        namespace camera_takeover_trigger {
            using update = rac1::Moby_Class1083_Update;
        }  // namespace camera_takeover_trigger
        namespace cannon {
            using update = rac1::Moby_Class440_Update;
        }  // namespace cannon
        namespace carrier {
            using update = rac1::Moby_GloveOfDoom_Update;
        }  // namespace carrier
        namespace challenge {
            using update = rac1::Moby_Class1347_Update;
        }  // namespace challenge
        namespace challenge_npc {
            using update = rac1::Moby_GadgetronCeo_Update;
        }  // namespace challenge_npc
        namespace checkpoint_trigger {
            using update = rac1::Moby_SpawnPoint_Update;
        }  // namespace checkpoint_trigger
        namespace child_spawner {
            using update = rac1::Moby_Spawner_Update;
        }  // namespace child_spawner
        namespace circular_saw {
            using update = rac1::Moby_CircularSawEnemy_Update;
        }  // namespace circular_saw
        namespace clank_pack {
            using update = rac1::Moby_Helipack_Update;
        }  // namespace clank_pack
        namespace class_1009 {
            using update = rac1::Moby_Class1009_Update;
        }  // namespace class_1009
        namespace class_1035 {
            using update = rac1::Moby_Class1035_Update;
        }  // namespace class_1035
        namespace class_1062 {
            using update = rac1::Moby_Class1062_Update;
        }  // namespace class_1062
        namespace class_1271 {
            using update = rac1::Moby_Class1271_Update;
        }  // namespace class_1271
        namespace class_1348 {
            using update = rac1::Moby_Class1348_Update;
        }  // namespace class_1348
        namespace class_1403 {
            using update = rac1::Moby_Class1403_Update;
        }  // namespace class_1403
        namespace class_1425 {
            using update = rac1::Moby_Class1425_Update;
        }  // namespace class_1425
        namespace class_1432 {
            using update = rac1::Moby_Class1432_Update;
        }  // namespace class_1432
        namespace class_148 {
            using update = rac1::Moby_Class148_Update;
        }  // namespace class_148
        namespace class_1557 {
            using update = rac1::Moby_Class1557_Update;
        }  // namespace class_1557
        namespace class_161 {
            using update = rac1::Moby_Class161_Update;
        }  // namespace class_161
        namespace class_1633 {
            using update = rac1::Moby_Class1633_Update;
        }  // namespace class_1633
        namespace class_1750 {
            using update = rac1::Moby_Class1750_Update;
        }  // namespace class_1750
        namespace class_1907 {
            using update = rac1::Moby_Class1907_Update;
        }  // namespace class_1907
        namespace class_1947 {
            using update = rac1::Moby_Class1947_Update;
        }  // namespace class_1947
        namespace class_256 {
            using update = rac1::Moby_Class256_Update;
        }  // namespace class_256
        namespace class_482 {
            using update = rac1::Moby_Class482_Update;
        }  // namespace class_482
        namespace class_655 {
            using update = rac1::Moby_Class655_Update;
        }  // namespace class_655
        namespace class_676 {
            using update = rac1::Moby_Class676_Update;
        }  // namespace class_676
        namespace class_684 {
            using update = rac1::Moby_Class684_Update;
        }  // namespace class_684
        namespace class_845 {
            using update = rac1::Moby_Class845_Update;
        }  // namespace class_845
        namespace class_893 {
            using update = rac1::Moby_Class893_Update;
        }  // namespace class_893
        namespace class_937 {
            using update = rac1::Moby_Class937_Update;
        }  // namespace class_937
        namespace clip_volume_door {
            using update = rac1::Moby_Gate93_Update;
        }  // namespace clip_volume_door
        namespace clip_volume_trigger {
            using update = rac1::Moby_Class6_Update;
        }  // namespace clip_volume_trigger
        namespace codebot_slot {
            using update = rac1::Moby_CodebotSlot_Update;
        }  // namespace codebot_slot
        namespace collapsing_platform {
            using update = rac1::Moby_FallingRock_Update;
        }  // namespace collapsing_platform
        namespace collectable {
            using update = rac1::Moby_UpperDoor_Update;
        }  // namespace collectable
        namespace collectible_pickup {
            using update = rac1::Moby_Morphoray1354_Update;
        }  // namespace collectible_pickup
        namespace collectible_spawner {
            using update = rac1::Moby_EnergySphereHolder_Update;
        }  // namespace collectible_spawner
        namespace collectible_trigger {
            using update = rac1::Moby_BlockCave_Update;
        }  // namespace collectible_trigger
        namespace commando {
            using update = rac1::Moby_Commando_Update;
        }  // namespace commando
        namespace construction_arm {
            using update = rac1::Moby_ConstructionArm_Update;
        }  // namespace construction_arm
        namespace controlled_gate {
            using update = rac1::Moby_Class674_Update;
        }  // namespace controlled_gate
        namespace crank_rotator {
            using update = rac1::Moby_CaveDoor_Update;
        }  // namespace crank_rotator
        namespace crank_slider {
            using update = rac1::Moby_DoorHalf_Update;
        }  // namespace crank_slider
        namespace crate {
            using update = rac1::Moby_BoltCrate_Update;
        }  // namespace crate
        namespace crate_platform {
            using update = rac1::Moby_BoltThief1401_Update;
        }  // namespace crate_platform
        namespace cutscene_beams {
            using update = rac1::Moby_Class1548_Update;
        }  // namespace cutscene_beams
        namespace cutscene_controller {
            using update = rac1::Moby_Class1156_Update;
        }  // namespace cutscene_controller
        namespace cutscene_trigger {
            using update = rac1::Moby_Class737_Update;
        }  // namespace cutscene_trigger
        namespace cutscene_volume_trigger {
            using update = rac1::Moby_Class1404_Update;
        }  // namespace cutscene_volume_trigger
        namespace debris_bounce {
            using update = rac1::Moby_NanotechCrateFragment_Update;
        }  // namespace debris_bounce
        namespace debris_moby {
            using update = rac1::Moby_Rock_Update;
        }  // namespace debris_moby
        namespace debris_particle {
            using update = rac1::Moby_Class325_Update;
        }  // namespace debris_particle
        namespace decoy_glove {
            using update = rac1::Moby_DecoyGlove_Update;
        }  // namespace decoy_glove
        namespace delete_moby {
            using update = rac1::Moby_TransportShip_Update;
        }  // namespace delete_moby
        namespace deserter {
            using update = rac1::Moby_BataliaDeserter_Update;
        }  // namespace deserter
        namespace doom_bot {
            using update = rac1::Moby_DoomBot_Update;
        }  // namespace doom_bot
        namespace door {
            using update = rac1::Moby_Door_Update;
        }  // namespace door
        namespace door_ring {
            using update = rac1::Moby_Class1021_Update;
        }  // namespace door_ring
        namespace door_switch {
            using update = rac1::Moby_HomingMineSpawner_Update;
        }  // namespace door_switch
        namespace drain_cover {
            using update = rac1::Moby_DrainCover_Update;
        }  // namespace drain_cover
        namespace drawbridge {
            using update = rac1::Moby_Class1028_Update;
        }  // namespace drawbridge
        namespace drift_spark_moby {
            using update = rac1::Moby_Leaf_Update;
        }  // namespace drift_spark_moby
        namespace drifting_blob {
            using update = rac1::Moby_Asteroid212_Update;
        }  // namespace drifting_blob
        namespace driver_lift {
            using update = rac1::Moby_LiftcarPlatform_Update;
        }  // namespace driver_lift
        namespace drone {
            using update = rac1::Moby_Drone_Update;
        }  // namespace drone
        namespace edwina {
            using update = rac1::Moby_Edwina_Update;
        }  // namespace edwina
        namespace effect_emitter {
            using update = rac1::Moby_Class1544_Update;
        }  // namespace effect_emitter
        namespace electric_gate {
            using update = rac1::Moby_ElectricGate_Update;
        }  // namespace electric_gate
        namespace electric_spark {
            using update = rac1::Moby_Class947_Update;
        }  // namespace electric_spark
        namespace elevator_21 {
            using update = rac1::Moby_Elevator21_Update;
        }  // namespace elevator_21
        namespace elevator_651 {
            using update = rac1::Moby_Elevator_Update;
        }  // namespace elevator_651
        namespace elevator_platform {
            using update = rac1::Moby_Elevator1826_Update;
        }  // namespace elevator_platform
        namespace emitter_l05 {
            using update = rac1::Moby_Class855_Update;
        }  // namespace emitter_l05
        namespace enemy_shot {
            using update = rac1::Moby_Class1258_Update;
        }  // namespace enemy_shot
        namespace enemy_spawner {
            using update = rac1::Moby_Beehive_Update;
        }  // namespace enemy_spawner
        namespace energy_field_emitter {
            using update = rac1::Moby_ThrusterPackLockGreen_Update;
        }  // namespace energy_field_emitter
        namespace env_map_a {
            using update = rac1::Moby_Class1943_Update;
        }  // namespace env_map_a
        namespace env_map_b {
            using update = rac1::Moby_Class1948_Update;
        }  // namespace env_map_b
        namespace env_map_c {
            using update = rac1::Moby_Class1953_Update;
        }  // namespace env_map_c
        namespace exhaust_vent {
            using update = rac1::Moby_Class1240_Update;
        }  // namespace exhaust_vent
        namespace extermibot {
            using update = rac1::Moby_Extermibot_Update;
        }  // namespace extermibot
        namespace extermitank {
            using update = rac1::Moby_Extermitank_Update;
        }  // namespace extermitank
        namespace fade_in_hold_out {
            using update = rac1::Moby_Class352_Update;
        }  // namespace fade_in_hold_out
        namespace falling_chunk {
            using update = rac1::Moby_Class787_Update;
        }  // namespace falling_chunk
        namespace falling_debris {
            using update = rac1::Moby_Class441_Update;
        }  // namespace falling_debris
        namespace falling_drop {
            using update = rac1::Moby_Class722_Update;
        }  // namespace falling_drop
        namespace falling_fireball {
            using update = rac1::Moby_Class624_Update;
        }  // namespace falling_fireball
        namespace falling_leaf {
            using update = rac1::Moby_Class1080_Update;
        }  // namespace falling_leaf
        namespace falling_moby {
            using update = rac1::Moby_Class1221_Update;
        }  // namespace falling_moby
        namespace falling_spark {
            using update = rac1::Moby_Class122_Update;
        }  // namespace falling_spark
        namespace fighter_jet_1379 {
            using update = rac1::Moby_FighterJet1379_Update;
        }  // namespace fighter_jet_1379
        namespace fighter_jet_69 {
            using update = rac1::Moby_FighterJet69_Update;
        }  // namespace fighter_jet_69
        namespace final_boss {
            using update = rac1::Moby_DrekFinalBossRobot_Update;
        }  // namespace final_boss
        namespace fire_bolt {
            using update = rac1::Moby_Class1292_Update;
        }  // namespace fire_bolt
        namespace fireball {
            using update = rac1::Moby_Class1017_Update;
        }  // namespace fireball
        namespace fleet_reflection {
            using update = rac1::Moby_Class1772_Update;
        }  // namespace fleet_reflection
        namespace fleet_turret {
            using update = rac1::Moby_DreksFleetTurret_Update;
        }  // namespace fleet_turret
        namespace flicker_light {
            using update = rac1::Moby_Class671_Update;
        }  // namespace flicker_light
        namespace floating_enemy {
            using update = rac1::Moby_BlargEliteCommando_Update;
        }  // namespace floating_enemy
        namespace floating_mine {
            using update = rac1::Moby_Mine933_Update;
        }  // namespace floating_mine
        namespace floating_platform_481 {
            using update = rac1::Moby_FloatingPlatform_Update;
        }  // namespace floating_platform_481
        namespace floating_platform_810 {
            using update = rac1::Moby_FloatingPlatform810_Update;
        }  // namespace floating_platform_810
        namespace floor_button {
            using update = rac1::Moby_FloorButton_Update;
        }  // namespace floor_button
        namespace flyer {
            using update = rac1::Moby_Rocket922_Update;
        }  // namespace flyer
        namespace flying_burst {
            using update = rac1::Moby_Class686_Update;
        }  // namespace flying_burst
        namespace flying_craft {
            using update = rac1::Moby_BlargShip1319_Update;
        }  // namespace flying_craft
        namespace fog_zone {
            using update = rac1::Moby_Class841_Update;
        }  // namespace fog_zone
        namespace forcefield_tower {
            using update = rac1::Moby_ForcefieldTower_Update;
        }  // namespace forcefield_tower
        namespace fred_1 {
            using update = rac1::Moby_Fred1_Update;
        }  // namespace fred_1
        namespace frog {
            using update = rac1::Moby_Frog_Update;
        }  // namespace frog
        namespace fx_group {
            using update = rac1::Moby_Hoverboard_Update;
        }  // namespace fx_group
        namespace gadgebot {
            using update = rac1::Moby_Gadgebot_Update;
        }  // namespace gadgebot
        namespace gate {
            using update = rac1::Moby_Class1248_Update;
        }  // namespace gate
        namespace gate_rise {
            using update = rac1::Moby_Class852_Update;
        }  // namespace gate_rise
        namespace gate_swing {
            using update = rac1::Moby_Class468_Update;
        }  // namespace gate_swing
        namespace gem_lock_pickup {
            using update = rac1::Moby_Forcefield_Update;
        }  // namespace gem_lock_pickup
        namespace gem_lock_sentry {
            using update = rac1::Moby_Class386_Update;
        }  // namespace gem_lock_sentry
        namespace gem_lock_turret {
            using update = rac1::Moby_BlargGunner_Update;
        }  // namespace gem_lock_turret
        namespace giant_clank_pad {
            using update = rac1::Moby_GiantClankPad_Update;
        }  // namespace giant_clank_pad
        namespace gold_bolt {
            using update = rac1::Moby_GoldBolt_Update;
        }  // namespace gold_bolt
        namespace grass {
            using update = rac1::Moby_BombthrowerGrass_Update;
        }  // namespace grass
        namespace gravity_debris {
            using update = rac1::Moby_Bomb882_Update;
        }  // namespace gravity_debris
        namespace grazing_critter {
            using update = rac1::Moby_Amoeboid_Update;
        }  // namespace grazing_critter
        namespace grind_rail {
            using update = rac1::Moby_Class679_Update;
        }  // namespace grind_rail
        namespace ground_enemy {
            using update = rac1::Moby_Class193_Update;
        }  // namespace ground_enemy
        namespace group_clear {
            using update = rac1::Moby_Class467_Update;
        }  // namespace group_clear
        namespace grow_and_fade_out {
            using update = rac1::Moby_Class775_Update;
        }  // namespace grow_and_fade_out
        namespace guide {
            using update = rac1::Moby_BigAl_Update;
        }  // namespace guide
        namespace guided_rocket {
            using update = rac1::Moby_Rocket435_Update;
        }  // namespace guided_rocket
        namespace gunship {
            using update = rac1::Moby_BomberJet_Update;
        }  // namespace gunship
        namespace heavy_tumbling_debris {
            using update = rac1::Moby_Class320_Update;
        }  // namespace heavy_tumbling_debris
        namespace height_glide {
            using update = rac1::Moby_LabDoor_Update;
        }  // namespace height_glide
        namespace helicopter {
            using update = rac1::Moby_Helicopter233_Update;
        }  // namespace helicopter
        namespace helicopter_gunship {
            using update = rac1::Moby_BlargHelicopter_Update;
        }  // namespace helicopter_gunship
        namespace help_hint_director {
            using update = rac1::Moby_Class1341_Update;
        }  // namespace help_hint_director
        namespace help_volume {
            using update = rac1::Moby_Class558_Update;
        }  // namespace help_volume
        namespace hero_bomb {
            using update = rac1::Moby_Class121_Update;
        }  // namespace hero_bomb
        namespace hideable_flower {
            using update = rac1::Moby_SwingshotVersatarget_Update;
        }  // namespace hideable_flower
        namespace hinged_bridge {
            using update = rac1::Moby_BridgeHalf_Update;
        }  // namespace hinged_bridge
        namespace hit_reaction_effect_alt {
            using update = rac1::Moby_Class154_Update;
        }  // namespace hit_reaction_effect_alt
        namespace hit_trigger {
            using update = rac1::Moby_Class885_Update;
        }  // namespace hit_trigger
        namespace homing_projectile {
            using update = rac1::Moby_Rocket50_Update;
        }  // namespace homing_projectile
        namespace homing_rocket82 {
            using update = rac1::Moby_Rocket82_Update;
        }  // namespace homing_rocket82
        namespace homing_spark_1475 {
            using update = rac1::Moby_Rocket1475_Update;
        }  // namespace homing_spark_1475
        namespace homing_spark_983 {
            using update = rac1::Moby_Class983_Update;
        }  // namespace homing_spark_983
        namespace horny_toad {
            using update = rac1::Moby_HornyToad_Update;
        }  // namespace horny_toad
        namespace hoven_challenge {
            using update = rac1::Moby_Class422_Update;
        }  // namespace hoven_challenge
        namespace hoven_homing_mine {
            using update = rac1::Moby_Mine1269_Update;
        }  // namespace hoven_homing_mine
        namespace hoven_platform {
            using update = rac1::Moby_Platform240_Update;
        }  // namespace hoven_platform
        namespace hoven_shot {
            using update = rac1::Moby_Rocket409_Update;
        }  // namespace hoven_shot
        namespace hoven_troll {
            using update = rac1::Moby_Class238_Update;
        }  // namespace hoven_troll
        namespace hover_enemy {
            using update = rac1::Moby_WaterLily_Update;
        }  // namespace hover_enemy
        namespace hoverboard {
            using update = rac1::Moby_Hoverboard439_Update;
        }  // namespace hoverboard
        namespace hoverboard_girl {
            using update = rac1::Moby_HoverboardGirl_Update;
        }  // namespace hoverboard_girl
        namespace hoverboard_racer {
            using update = rac1::Moby_TestDummyOnHoverboard_Update;
        }  // namespace hoverboard_racer
        namespace hydrodisplacer_pad {
            using update = rac1::Moby_Hydrodisplacer_Update;
        }  // namespace hydrodisplacer_pad
        namespace infobot {
            using update = rac1::Moby_Infobot_Update;
        }  // namespace infobot
        namespace item_offer {
            using update = rac1::Moby_GoldWeaponCase_Update;
        }  // namespace item_offer
        namespace jet_fighter {
            using update = rac1::Moby_JetFighter_Update;
        }  // namespace jet_fighter
        namespace kerwan_homing_rocket {
            using update = rac1::Moby_Class833_Update;
        }  // namespace kerwan_homing_rocket
        namespace kerwan_mine {
            using update = rac1::Moby_Mine627_Update;
        }  // namespace kerwan_mine
        namespace keyframe_platform {
            using update = rac1::Moby_Platform903_Update;
        }  // namespace keyframe_platform
        namespace launch_trigger {
            using update = rac1::Moby_Class1066_Update;
        }  // namespace launch_trigger
        namespace lever_door {
            using update = rac1::Moby_PlatformBooster_Update;
        }  // namespace lever_door
        namespace lift1270 {
            using update = rac1::Moby_Class1270_Update;
        }  // namespace lift1270
        namespace lift_l05 {
            using update = rac1::Moby_ElevatorWithButton_Update;
        }  // namespace lift_l05
        namespace lift_move {
            using update = rac1::Moby_PistonPlatform_Update;
        }  // namespace lift_move
        namespace light_beam {
            using update = rac1::Moby_Vendor_Update;
        }  // namespace light_beam
        namespace linked_riser {
            using update = rac1::Moby_Class1293_Update;
        }  // namespace linked_riser
        namespace linked_turret {
            using update = rac1::Moby_Gun_Update;
        }  // namespace linked_turret
        namespace lobbed_bomb {
            using update = rac1::Moby_Class1049_Update;
        }  // namespace lobbed_bomb
        namespace lobber_enemy {
            using update = rac1::Moby_Bombomatic_Update;
        }  // namespace lobber_enemy
        namespace machinegun_turret_1126 {
            using update = rac1::Moby_MachinegunTurret_Update;
        }  // namespace machinegun_turret_1126
        namespace machinegun_turret_30 {
            using update = rac1::Moby_MachinegunTurret30_Update;
        }  // namespace machinegun_turret_30
        namespace metropolis_train {
            using update = rac1::Moby_MetropolisTrainLocomotive_Update;
        }  // namespace metropolis_train
        namespace metropolis_train_carriage {
            using update = rac1::Moby_MetropolisTrainCarriage_Update;
        }  // namespace metropolis_train_carriage
        namespace mine {
            using update = rac1::Moby_Mine_Update;
        }  // namespace mine
        namespace mine_glove {
            using update = rac1::Moby_MineGlove_Update;
        }  // namespace mine_glove
        namespace mine_l14 {
            using update = rac1::Moby_Mine643_Update;
        }  // namespace mine_l14
        namespace mine_moby {
            using update = rac1::Moby_Lamp1511_Update;
        }  // namespace mine_moby
        namespace miner {
            using update = rac1::Moby_Miner_Update;
        }  // namespace miner
        namespace minibomber {
            using update = rac1::Moby_Minibomber_Update;
        }  // namespace minibomber
        namespace missile {
            using update = rac1::Moby_GoldDevastatorMissile_Update;
        }  // namespace missile
        namespace moby1000 {
            using update = rac1::Moby_Class1000_Update;
        }  // namespace moby1000
        namespace moby1005 {
            using update = rac1::Moby_Tresspasser_Update;
        }  // namespace moby1005
        namespace moby1031 {
            using update = rac1::Moby_Button_Update;
        }  // namespace moby1031
        namespace moby1033 {
            using update = rac1::Moby_Platform1033_Update;
        }  // namespace moby1033
        namespace moby1034 {
            using update = rac1::Moby_Rocket1034_Update;
        }  // namespace moby1034
        namespace moby1040 {
            using update = rac1::Moby_EnergySphere_Update;
        }  // namespace moby1040
        namespace moby1042 {
            using update = rac1::Moby_MushroomUnderside_Update;
        }  // namespace moby1042
        namespace moby1047 {
            using update = rac1::Moby_Class1047_Update;
        }  // namespace moby1047
        namespace moby1048 {
            using update = rac1::Moby_AlienSnapper_Update;
        }  // namespace moby1048
        namespace moby1054 {
            using update = rac1::Moby_Class1054_Update;
        }  // namespace moby1054
        namespace moby1068 {
            using update = rac1::Moby_BlargSpaceCommando_Update;
        }  // namespace moby1068
        namespace moby1069 {
            using update = rac1::Moby_Class1069_Update;
        }  // namespace moby1069
        namespace moby1090 {
            using update = rac1::Moby_Class1090_Update;
        }  // namespace moby1090
        namespace moby1105 {
            using update = rac1::Moby_Fred_Update;
        }  // namespace moby1105
        namespace moby1108 {
            using update = rac1::Moby_Class1108_Update;
        }  // namespace moby1108
        namespace moby1123 {
            using update = rac1::Moby_Class1123_Update;
        }  // namespace moby1123
        namespace moby1133 {
            using update = rac1::Moby_Class1133_Update;
        }  // namespace moby1133
        namespace moby1139 {
            using update = rac1::Moby_HoverboardRing_Update;
        }  // namespace moby1139
        namespace moby114 {
            using update = rac1::Moby_ResortOwner_Update;
        }  // namespace moby114
        namespace moby1142 {
            using update = rac1::Moby_Class1142_Update;
        }  // namespace moby1142
        namespace moby1143 {
            using update = rac1::Moby_VendorOrb_Update;
        }  // namespace moby1143
        namespace moby1157 {
            using update = rac1::Moby_Class1157_Update;
        }  // namespace moby1157
        namespace moby1158 {
            using update = rac1::Moby_Class1158_Update;
        }  // namespace moby1158
        namespace moby1159 {
            using update = rac1::Moby_Class1159_Update;
        }  // namespace moby1159
        namespace moby1178 {
            using update = rac1::Moby_SeesawPlatform_Update;
        }  // namespace moby1178
        namespace moby1179 {
            using update = rac1::Moby_ThrusterPackLock_Update;
        }  // namespace moby1179
        namespace moby1180 {
            using update = rac1::Moby_Class1180_Update;
        }  // namespace moby1180
        namespace moby1196 {
            using update = rac1::Moby_Screamer_Update;
        }  // namespace moby1196
        namespace moby1199 {
            using update = rac1::Moby_Class1199_Update;
        }  // namespace moby1199
        namespace moby1206 {
            using update = rac1::Moby_Class1206_Update;
        }  // namespace moby1206
        namespace moby1219 {
            using update = rac1::Moby_Class1219_Update;
        }  // namespace moby1219
        namespace moby1224 {
            using update = rac1::Moby_Class1224_Update;
        }  // namespace moby1224
        namespace moby1225 {
            using update = rac1::Moby_Class1225_Update;
        }  // namespace moby1225
        namespace moby1246 {
            using update = rac1::Moby_Puffoid_Update;
        }  // namespace moby1246
        namespace moby1257_spawner {
            using update = rac1::Moby_Class1394_Update;
        }  // namespace moby1257_spawner
        namespace moby1259 {
            using update = rac1::Moby_Class1259_Update;
        }  // namespace moby1259
        namespace moby1261 {
            using update = rac1::Moby_PoisonVessel_Update;
        }  // namespace moby1261
        namespace moby1264 {
            using update = rac1::Moby_OilTankerFront_Update;
        }  // namespace moby1264
        namespace moby1267 {
            using update = rac1::Moby_HovenTurret_Update;
        }  // namespace moby1267
        namespace moby1283 {
            using update = rac1::Moby_ThePlumberBatalia_Update;
        }  // namespace moby1283
        namespace moby1290 {
            using update = rac1::Moby_PilotsHelmet_Update;
        }  // namespace moby1290
        namespace moby1297 {
            using update = rac1::Moby_EnergySphere1297_Update;
        }  // namespace moby1297
        namespace moby1302 {
            using update = rac1::Moby_BotDoorLock_Update;
        }  // namespace moby1302
        namespace moby1331 {
            using update = rac1::Moby_Class1331_Update;
        }  // namespace moby1331
        namespace moby1343 {
            using update = rac1::Moby_Class1343_Update;
        }  // namespace moby1343
        namespace moby1344 {
            using update = rac1::Moby_Class1344_Update;
        }  // namespace moby1344
        namespace moby1345 {
            using update = rac1::Moby_Class1345_Update;
        }  // namespace moby1345
        namespace moby1349 {
            using update = rac1::Moby_Class1349_Update;
        }  // namespace moby1349
        namespace moby1350 {
            using update = rac1::Moby_Class1350_Update;
        }  // namespace moby1350
        namespace moby1353 {
            using update = rac1::Moby_Class1353_Update;
        }  // namespace moby1353
        namespace moby1355 {
            using update = rac1::Moby_DrekFightMissile_Update;
        }  // namespace moby1355
        namespace moby1368 {
            using update = rac1::Moby_Class1368_Update;
        }  // namespace moby1368
        namespace moby1378 {
            using update = rac1::Moby_Class1378_Update;
        }  // namespace moby1378
        namespace moby1380 {
            using update = rac1::Moby_MagnetElevator_Update;
        }  // namespace moby1380
        namespace moby1381 {
            using update = rac1::Moby_Class1381_Update;
        }  // namespace moby1381
        namespace moby1382 {
            using update = rac1::Moby_RobotDog1382_Update;
        }  // namespace moby1382
        namespace moby1387 {
            using update = rac1::Moby_Class1387_Update;
        }  // namespace moby1387
        namespace moby1388 {
            using update = rac1::Moby_BoltGrabber_Update;
        }  // namespace moby1388
        namespace moby1392 {
            using update = rac1::Moby_Class1392_Update;
        }  // namespace moby1392
        namespace moby1395 {
            using update = rac1::Moby_Class1395_Update;
        }  // namespace moby1395
        namespace moby1397 {
            using update = rac1::Moby_Class1397_Update;
        }  // namespace moby1397
        namespace moby1400 {
            using update = rac1::Moby_Class1400_Update;
        }  // namespace moby1400
        namespace moby1402 {
            using update = rac1::Moby_Class1402_Update;
        }  // namespace moby1402
        namespace moby1413 {
            using update = rac1::Moby_Class1413_Update;
        }  // namespace moby1413
        namespace moby1419 {
            using update = rac1::Moby_Class1419_Update;
        }  // namespace moby1419
        namespace moby1424 {
            using update = rac1::Moby_Gate2_Update;
        }  // namespace moby1424
        namespace moby1428 {
            using update = rac1::Moby_Codebot_Update;
        }  // namespace moby1428
        namespace moby1434 {
            using update = rac1::Moby_DeplanetizerGun_Update;
        }  // namespace moby1434
        namespace moby1439 {
            using update = rac1::Moby_GrindrailGate_Update;
        }  // namespace moby1439
        namespace moby1441 {
            using update = rac1::Moby_GrindrailGate1441_Update;
        }  // namespace moby1441
        namespace moby1442 {
            using update = rac1::Moby_GrindrailButton_Update;
        }  // namespace moby1442
        namespace moby1448 {
            using update = rac1::Moby_Shuttle1448_Update;
        }  // namespace moby1448
        namespace moby1454 {
            using update = rac1::Moby_RocketTank_Update;
        }  // namespace moby1454
        namespace moby1469 {
            using update = rac1::Moby_Class1469_Update;
        }  // namespace moby1469
        namespace moby1470 {
            using update = rac1::Moby_Class1470_Update;
        }  // namespace moby1470
        namespace moby1479 {
            using update = rac1::Moby_Class1479_Update;
        }  // namespace moby1479
        namespace moby1510 {
            using update = rac1::Moby_Class1510_Update;
        }  // namespace moby1510
        namespace moby152 {
            using update = rac1::Moby_Class152_Update;
        }  // namespace moby152
        namespace moby1523 {
            using update = rac1::Moby_Class1523_Update;
        }  // namespace moby1523
        namespace moby1545 {
            using update = rac1::Moby_Class1545_Update;
        }  // namespace moby1545
        namespace moby1546 {
            using update = rac1::Moby_Class1546_Update;
        }  // namespace moby1546
        namespace moby1550 {
            using update = rac1::Moby_Class1550_Update;
        }  // namespace moby1550
        namespace moby1553 {
            using update = rac1::Moby_Class1553_Update;
        }  // namespace moby1553
        namespace moby1555 {
            using update = rac1::Moby_Class1555_Update;
        }  // namespace moby1555
        namespace moby156 {
            using update = rac1::Moby_Class156_Update;
        }  // namespace moby156
        namespace moby1563 {
            using update = rac1::Moby_Class1563_Update;
        }  // namespace moby1563
        namespace moby1568 {
            using update = rac1::Moby_DeplanetizerGun1568_Update;
        }  // namespace moby1568
        namespace moby1584 {
            using update = rac1::Moby_Class1584_Update;
        }  // namespace moby1584
        namespace moby1629 {
            using update = rac1::Moby_Class1629_Update;
        }  // namespace moby1629
        namespace moby163 {
            using update = rac1::Moby_Visibomb_Update;
        }  // namespace moby163
        namespace moby1632 {
            using update = rac1::Moby_Class1632_Update;
        }  // namespace moby1632
        namespace moby1634 {
            using update = rac1::Moby_Class1634_Update;
        }  // namespace moby1634
        namespace moby1641 {
            using update = rac1::Moby_Class1641_Update;
        }  // namespace moby1641
        namespace moby1649 {
            using update = rac1::Moby_Class1649_Update;
        }  // namespace moby1649
        namespace moby166 {
            using update = rac1::Moby_Class166_Update;
        }  // namespace moby166
        namespace moby167 {
            using update = rac1::Moby_Class167_Update;
        }  // namespace moby167
        namespace moby175 {
            using update = rac1::Moby_Taunter_Update;
        }  // namespace moby175
        namespace moby1766 {
            using update = rac1::Moby_Shuttle1766_Update;
        }  // namespace moby1766
        namespace moby1789 {
            using update = rac1::Moby_Class1789_Update;
        }  // namespace moby1789
        namespace moby179 {
            using update = rac1::Moby_Class179_Update;
        }  // namespace moby179
        namespace moby1799 {
            using update = rac1::Moby_Class1799_Update;
        }  // namespace moby1799
        namespace moby180 {
            using update = rac1::Moby_Walloper_Update;
        }  // namespace moby180
        namespace moby1805 {
            using update = rac1::Moby_BlargShip1805_Update;
        }  // namespace moby1805
        namespace moby1813 {
            using update = rac1::Moby_TallShroom_Update;
        }  // namespace moby1813
        namespace moby1843 {
            using update = rac1::Moby_BlargShip1843_Update;
        }  // namespace moby1843
        namespace moby1848 {
            using update = rac1::Moby_Class1848_Update;
        }  // namespace moby1848
        namespace moby184_shot {
            using update = rac1::Moby_Class184_Update;
        }  // namespace moby184_shot
        namespace moby1855 {
            using update = rac1::Moby_Class1855_Update;
        }  // namespace moby1855
        namespace moby1856 {
            using update = rac1::Moby_Class1856_Update;
        }  // namespace moby1856
        namespace moby1873 {
            using update = rac1::Moby_Class1873_Update;
        }  // namespace moby1873
        namespace moby1876 {
            using update = rac1::Moby_Class1876_Update;
        }  // namespace moby1876
        namespace moby188 {
            using update = rac1::Moby_Trespasser_Update;
        }  // namespace moby188
        namespace moby1885 {
            using update = rac1::Moby_Spawner1885_Update;
        }  // namespace moby1885
        namespace moby1886 {
            using update = rac1::Moby_SpawnerEgg_Update;
        }  // namespace moby1886
        namespace moby1898 {
            using update = rac1::Moby_Class1898_Update;
        }  // namespace moby1898
        namespace moby19 {
            using update = rac1::Moby_Class19_Update;
        }  // namespace moby19
        namespace moby1901 {
            using update = rac1::Moby_Class1901_Update;
        }  // namespace moby1901
        namespace moby1906 {
            using update = rac1::Moby_Gadgebot1906_Update;
        }  // namespace moby1906
        namespace moby192 {
            using update = rac1::Moby_BombGlove_Update;
        }  // namespace moby192
        namespace moby209 {
            using update = rac1::Moby_SwingshotHook_Update;
        }  // namespace moby209
        namespace moby211 {
            using update = rac1::Moby_Plasmabot211_Update;
        }  // namespace moby211
        namespace moby22 {
            using update = rac1::Moby_Class22_Update;
        }  // namespace moby22
        namespace moby250 {
            using update = rac1::Moby_PlasmabotHatch_Update;
        }  // namespace moby250
        namespace moby257 {
            using update = rac1::Moby_Class257_Update;
        }  // namespace moby257
        namespace moby258 {
            using update = rac1::Moby_Class258_Update;
        }  // namespace moby258
        namespace moby279 {
            using update = rac1::Moby_Class279_Update;
        }  // namespace moby279
        namespace moby293 {
            using update = rac1::Moby_Class293_Update;
        }  // namespace moby293
        namespace moby294 {
            using update = rac1::Moby_BlargTrooper294_Update;
        }  // namespace moby294
        namespace moby302 {
            using update = rac1::Moby_GadgebotSpawnContainer_Update;
        }  // namespace moby302
        namespace moby303 {
            using update = rac1::Moby_GadgebotSpawnPad_Update;
        }  // namespace moby303
        namespace moby305 {
            using update = rac1::Moby_Class305_Update;
        }  // namespace moby305
        namespace moby309 {
            using update = rac1::Moby_Platform309_Update;
        }  // namespace moby309
        namespace moby318 {
            using update = rac1::Moby_Teleporter318_Update;
        }  // namespace moby318
        namespace moby326 {
            using update = rac1::Moby_BlargHelicopter326_Update;
        }  // namespace moby326
        namespace moby327 {
            using update = rac1::Moby_Class327_Update;
        }  // namespace moby327
        namespace moby336 {
            using update = rac1::Moby_Helicopter_Update;
        }  // namespace moby336
        namespace moby339 {
            using update = rac1::Moby_CraneArm_Update;
        }  // namespace moby339
        namespace moby341 {
            using update = rac1::Moby_HydrodisplacerFaucet_Update;
        }  // namespace moby341
        namespace moby361 {
            using update = rac1::Moby_Class361_Update;
        }  // namespace moby361
        namespace moby367 {
            using update = rac1::Moby_Class367_Update;
        }  // namespace moby367
        namespace moby37 {
            using update = rac1::Moby_Class37_Update;
        }  // namespace moby37
        namespace moby41 {
            using update = rac1::Moby_Rocket41_Update;
        }  // namespace moby41
        namespace moby417 {
            using update = rac1::Moby_Class417_Update;
        }  // namespace moby417
        namespace moby425 {
            using update = rac1::Moby_Bomb425_Update;
        }  // namespace moby425
        namespace moby428 {
            using update = rac1::Moby_Class428_Update;
        }  // namespace moby428
        namespace moby436 {
            using update = rac1::Moby_Class436_Update;
        }  // namespace moby436
        namespace moby438 {
            using update = rac1::Moby_Class438_Update;
        }  // namespace moby438
        namespace moby444 {
            using update = rac1::Moby_Class444_Update;
        }  // namespace moby444
        namespace moby458 {
            using update = rac1::Moby_Rocket458_Update;
        }  // namespace moby458
        namespace moby462 {
            using update = rac1::Moby_Shrub_Update;
        }  // namespace moby462
        namespace moby470 {
            using update = rac1::Moby_Rocket470_Update;
        }  // namespace moby470
        namespace moby471 {
            using update = rac1::Moby_ConveyorBelt471_Update;
        }  // namespace moby471
        namespace moby52 {
            using update = rac1::Moby_Seeker_Update;
        }  // namespace moby52
        namespace moby545 {
            using update = rac1::Moby_Bomb545_Update;
        }  // namespace moby545
        namespace moby546 {
            using update = rac1::Moby_WallButton_Update;
        }  // namespace moby546
        namespace moby552 {
            using update = rac1::Moby_Class552_Update;
        }  // namespace moby552
        namespace moby564 {
            using update = rac1::Moby_Rocket564_Update;
        }  // namespace moby564
        namespace moby568 {
            using update = rac1::Moby_Mine568_Update;
        }  // namespace moby568
        namespace moby573 {
            using update = rac1::Moby_RobotDog_Update;
        }  // namespace moby573
        namespace moby574 {
            using update = rac1::Moby_Class574_Update;
        }  // namespace moby574
        namespace moby584 {
            using update = rac1::Moby_Class584_Update;
        }  // namespace moby584
        namespace moby600 {
            using update = rac1::Moby_Class600_Update;
        }  // namespace moby600
        namespace moby604 {
            using update = rac1::Moby_SandmouseHouse_Update;
        }  // namespace moby604
        namespace moby605 {
            using update = rac1::Moby_Class605_Update;
        }  // namespace moby605
        namespace moby610 {
            using update = rac1::Moby_Class610_Update;
        }  // namespace moby610
        namespace moby612 {
            using update = rac1::Moby_Constructorbot_Update;
        }  // namespace moby612
        namespace moby621 {
            using update = rac1::Moby_Class621_Update;
        }  // namespace moby621
        namespace moby628 {
            using update = rac1::Moby_Class628_Update;
        }  // namespace moby628
        namespace moby639 {
            using update = rac1::Moby_Class639_Update;
        }  // namespace moby639
        namespace moby647 {
            using update = rac1::Moby_Door647_Update;
        }  // namespace moby647
        namespace moby648 {
            using update = rac1::Moby_Class648_Update;
        }  // namespace moby648
        namespace moby65 {
            using update = rac1::Moby_Class65_Update;
        }  // namespace moby65
        namespace moby650 {
            using update = rac1::Moby_TestDummyDoor_Update;
        }  // namespace moby650
        namespace moby654 {
            using update = rac1::Moby_Class654_Update;
        }  // namespace moby654
        namespace moby656 {
            using update = rac1::Moby_Bulldozer_Update;
        }  // namespace moby656
        namespace moby664 {
            using update = rac1::Moby_Platform664_Update;
        }  // namespace moby664
        namespace moby669 {
            using update = rac1::Moby_Class669_Update;
        }  // namespace moby669
        namespace moby67 {
            using update = rac1::Moby_Class67_Update;
        }  // namespace moby67
        namespace moby675 {
            using update = rac1::Moby_GrabCrane_Update;
        }  // namespace moby675
        namespace moby685 {
            using update = rac1::Moby_Elevator685_Update;
        }  // namespace moby685
        namespace moby702 {
            using update = rac1::Moby_Class702_Update;
        }  // namespace moby702
        namespace moby713 {
            using update = rac1::Moby_TransportPod_Update;
        }  // namespace moby713
        namespace moby732 {
            using update = rac1::Moby_Crane_Update;
        }  // namespace moby732
        namespace moby743 {
            using update = rac1::Moby_TresspasserDoor1_Update;
        }  // namespace moby743
        namespace moby744 {
            using update = rac1::Moby_TresspasserDoor2_Update;
        }  // namespace moby744
        namespace moby750_spark_trigger {
            using update = rac1::Moby_Class1562_Update;
        }  // namespace moby750_spark_trigger
        namespace moby755 {
            using update = rac1::Moby_SandsharkEgg_Update;
        }  // namespace moby755
        namespace moby762 {
            using update = rac1::Moby_Rocks_Update;
        }  // namespace moby762
        namespace moby77 {
            using update = rac1::Moby_Mine77_Update;
        }  // namespace moby77
        namespace moby778 {
            using update = rac1::Moby_Pipe_Update;
        }  // namespace moby778
        namespace moby78 {
            using update = rac1::Moby_Class78_Update;
        }  // namespace moby78
        namespace moby794 {
            using update = rac1::Moby_Class794_Update;
        }  // namespace moby794
        namespace moby8 {
            using update = rac1::Moby_BlargHeavyInterceptor_Update;
        }  // namespace moby8
        namespace moby800 {
            using update = rac1::Moby_Class800_Update;
        }  // namespace moby800
        namespace moby81 {
            using update = rac1::Moby_Class81_Update;
        }  // namespace moby81
        namespace moby812 {
            using update = rac1::Moby_Box_Update;
        }  // namespace moby812
        namespace moby817 {
            using update = rac1::Moby_Statue_Update;
        }  // namespace moby817
        namespace moby819 {
            using update = rac1::Moby_Rocket819_Update;
        }  // namespace moby819
        namespace moby823 {
            using update = rac1::Moby_Lamp_Update;
        }  // namespace moby823
        namespace moby83 {
            using update = rac1::Moby_Mine83_Update;
        }  // namespace moby83
        namespace moby834 {
            using update = rac1::Moby_ClankCutsceneTrigger_Update;
        }  // namespace moby834
        namespace moby835 {
            using update = rac1::Moby_Class835_Update;
        }  // namespace moby835
        namespace moby848 {
            using update = rac1::Moby_Class848_Update;
        }  // namespace moby848
        namespace moby851 {
            using update = rac1::Moby_QwarkOltanis_Update;
        }  // namespace moby851
        namespace moby854 {
            using update = rac1::Moby_OceanAridia_Update;
        }  // namespace moby854
        namespace moby859 {
            using update = rac1::Moby_Class859_Update;
        }  // namespace moby859
        namespace moby90 {
            using update = rac1::Moby_Bob_Update;
        }  // namespace moby90
        namespace moby908 {
            using update = rac1::Moby_Jet_Update;
        }  // namespace moby908
        namespace moby914 {
            using update = rac1::Moby_RobotQwark_Update;
        }  // namespace moby914
        namespace moby924 {
            using update = rac1::Moby_Sam_Update;
        }  // namespace moby924
        namespace moby934 {
            using update = rac1::Moby_Class934_Update;
        }  // namespace moby934
        namespace moby938 {
            using update = rac1::Moby_Bomb938_Update;
        }  // namespace moby938
        namespace moby99 {
            using update = rac1::Moby_Class99_Update;
        }  // namespace moby99
        namespace moby_state_relay {
            using update = rac1::Moby_Class667_Update;
        }  // namespace moby_state_relay
        namespace morphoray {
            using update = rac1::Moby_Morphoray_Update;
        }  // namespace morphoray
        namespace morphoray_chicken {
            using update = rac1::Moby_MorphorayChicken_Update;
        }  // namespace morphoray_chicken
        namespace mountable_turret {
            using update = rac1::Moby_Turret_Update;
        }  // namespace mountable_turret
        namespace mouse_critter {
            using update = rac1::Moby_Sandmouse_Update;
        }  // namespace mouse_critter
        namespace moving_moby {
            using update = rac1::Moby_Door196_Update;
        }  // namespace moving_moby
        namespace moving_moby_l07 {
            using update = rac1::Moby_Class1128_Update;
        }  // namespace moving_moby_l07
        namespace moving_platform {
            using update = rac1::Moby_MovingPlatformSmall_Update;
        }  // namespace moving_platform
        namespace nanotech_ball {
            using update = rac1::Moby_NanotechBall_Update;
        }  // namespace nanotech_ball
        namespace nanotech_vendor {
            using update = rac1::Moby_NanotechUpgradeVendor_Update;
        }  // namespace nanotech_vendor
        namespace near_hero_sparkle {
            using update = rac1::Moby_GadgebotTelepotPad_Update;
        }  // namespace near_hero_sparkle
        namespace ocean {
            using update = rac1::Moby_OceanPokitaru_Update;
        }  // namespace ocean
        namespace oltanis_jet {
            using update = rac1::Moby_Jet921_Update;
        }  // namespace oltanis_jet
        namespace orbiting_platform {
            using update = rac1::Moby_HangarDoor_Update;
        }  // namespace orbiting_platform
        namespace oscillating_spinner {
            using update = rac1::Moby_Fan_Update;
        }  // namespace oscillating_spinner
        namespace paratrooper {
            using update = rac1::Moby_BlargParatrooper_Update;
        }  // namespace paratrooper
        namespace parent_follow_fade {
            using update = rac1::Moby_Class235_Update;
        }  // namespace parent_follow_fade
        namespace particle_emitter {
            using update = rac1::Moby_Class27_Update;
        }  // namespace particle_emitter
        namespace particle_field {
            using update = rac1::Moby_ParticleSpawnerSmokeFire_Update;
        }  // namespace particle_field
        namespace path_carrier {
            using update = rac1::Moby_WaterCurrent_Update;
        }  // namespace path_carrier
        namespace path_follow_moby {
            using update = rac1::Moby_Pterodactyl_Update;
        }  // namespace path_follow_moby
        namespace path_junction {
            using update = rac1::Moby_Class582_Update;
        }  // namespace path_junction
        namespace path_keyframe_blend {
            using update = rac1::Moby_Class617_Update;
        }  // namespace path_keyframe_blend
        namespace path_lift {
            using update = rac1::Moby_Shuttle557_Update;
        }  // namespace path_lift
        namespace path_pickup {
            using update = rac1::Moby_Taxi1410_Update;
        }  // namespace path_pickup
        namespace path_platform {
            using update = rac1::Moby_NovalisElevator_Update;
        }  // namespace path_platform
        namespace path_platform_l09 {
            using update = rac1::Moby_Elevator1150_Update;
        }  // namespace path_platform_l09
        namespace path_train {
            using update = rac1::Moby_BoltcratePossiblyForMetalDetector_Update;
        }  // namespace path_train
        namespace path_vehicle {
            using update = rac1::Moby_SnowPlow_Update;
        }  // namespace path_vehicle
        namespace path_walker {
            using update = rac1::Moby_Class1667_Update;
        }  // namespace path_walker
        namespace peckbot {
            using update = rac1::Moby_Peckbot_Update;
        }  // namespace peckbot
        namespace pickup {
            using update = rac1::Moby_Class23_Update;
        }  // namespace pickup
        namespace pickup_l05 {
            using update = rac1::Moby_Taxi998_Update;
        }  // namespace pickup_l05
        namespace pipe_mounted_grating {
            using update = rac1::Moby_PipeMountedGrating_Update;
        }  // namespace pipe_mounted_grating
        namespace planetbuster_part {
            using update = rac1::Moby_PlanetbusterMaximusPart_Update;
        }  // namespace planetbuster_part
        namespace plasmabot {
            using update = rac1::Moby_Plasmabot_Update;
        }  // namespace plasmabot
        namespace poison_vessel {
            using update = rac1::Moby_PoisonVessel1257_Update;
        }  // namespace poison_vessel
        namespace pokitaru_boat {
            using update = rac1::Moby_PokitaruBoat_Update;
        }  // namespace pokitaru_boat
        namespace pokitaru_surface_far {
            using update = rac1::Moby_Class1919_Update;
        }  // namespace pokitaru_surface_far
        namespace pokitaru_surface_near {
            using update = rac1::Moby_Class1903_Update;
        }  // namespace pokitaru_surface_near
        namespace pokitaru_vendor {
            using update = rac1::Moby_FredPokitaru_Update;
        }  // namespace pokitaru_vendor
        namespace pose_slide_out {
            using update = rac1::Moby_Class843_Update;
        }  // namespace pose_slide_out
        namespace pressure_plate {
            using update = rac1::Moby_RefuelingStation_Update;
        }  // namespace pressure_plate
        namespace projectile {
            using update = rac1::Moby_Rocket49_Update;
        }  // namespace projectile
        namespace projectile_breakable {
            using update = rac1::Moby_BridgePiece1_Update;
        }  // namespace projectile_breakable
        namespace projectile_l13 {
            using update = rac1::Moby_Class1238_Update;
        }  // namespace projectile_l13
        namespace psytcopus {
            using update = rac1::Moby_Psytcopus_Update;
        }  // namespace psytcopus
        namespace pulsing_color_moby {
            using update = rac1::Moby_RedOrb_Update;
        }  // namespace pulsing_color_moby
        namespace pulsing_marker {
            using update = rac1::Moby_OceanRilgar_Update;
        }  // namespace pulsing_marker
        namespace pulsing_mesh {
            using update = rac1::Moby_Class761_Update;
        }  // namespace pulsing_mesh
        namespace purple_ship_529 {
            using update = rac1::Moby_PurpleShip_Update;
        }  // namespace purple_ship_529
        namespace purple_ship_533 {
            using update = rac1::Moby_PurpleShip533_Update;
        }  // namespace purple_ship_533
        namespace pusher {
            using update = rac1::Moby_ConveyorBelt_Update;
        }  // namespace pusher
        namespace pyrocitor {
            using update = rac1::Moby_Pyrocitor_Update;
        }  // namespace pyrocitor
        namespace quad_moby {
            using update = rac1::Moby_Class1138_Update;
        }  // namespace quad_moby
        namespace quest_giver {
            using update = rac1::Moby_Helga_Update;
        }  // namespace quest_giver
        namespace queue_effect_draw {
            using update = rac1::Moby_Class1418_Update;
        }  // namespace queue_effect_draw
        namespace qwark_starfighter_part {
            using update = rac1::Moby_QwarksStarfighterPart_Update;
        }  // namespace qwark_starfighter_part
        namespace qwark_watch {
            using update = rac1::Moby_QwarkUmbris_Update;
        }  // namespace qwark_watch
        namespace qwarks_trailer {
            using update = rac1::Moby_QwarksTrailer_Update;
        }  // namespace qwarks_trailer
        namespace race_debris {
            using update = rac1::Moby_Class403_Update;
        }  // namespace race_debris
        namespace rail_lift {
            using update = rac1::Moby_Taxi1012_Update;
        }  // namespace rail_lift
        namespace rail_moby {
            using update = rac1::Moby_Class898_Update;
        }  // namespace rail_moby
        namespace rail_spawner {
            using update = rac1::Moby_Class899_Update;
        }  // namespace rail_spawner
        namespace rail_taxi {
            using update = rac1::Moby_Taxi_Update;
        }  // namespace rail_taxi
        namespace rc_range_limiter {
            using update = rac1::Moby_Class832_Update;
        }  // namespace rc_range_limiter
        using record_death = rac1::Moby_RecordDeath;
        namespace red_button {
            using update = rac1::Moby_RedButton_Update;
        }  // namespace red_button
        namespace rider_carrier {
            using update = rac1::Moby_StationaryShip_Update;
        }  // namespace rider_carrier
        namespace ring_effect {
            using update = rac1::Moby_GrindrailBarrier_Update;
        }  // namespace ring_effect
        namespace ripple_strips_moby {
            using update = rac1::Moby_Class1405_Update;
        }  // namespace ripple_strips_moby
        namespace rise_when_collected {
            using update = rac1::Moby_Gate_Update;
        }  // namespace rise_when_collected
        namespace robosquawker {
            using update = rac1::Moby_Robosquawker_Update;
        }  // namespace robosquawker
        namespace rocket295 {
            using update = rac1::Moby_Rocket295_Update;
        }  // namespace rocket295
        namespace rocket_door1 {
            using update = rac1::Moby_RocketDoor1_Update;
        }  // namespace rocket_door1
        namespace rocket_launcher {
            using update = rac1::Moby_RocketLauncher_Update;
        }  // namespace rocket_launcher
        namespace rocket_steer {
            using update = rac1::Moby_Rocket_Update;
        }  // namespace rocket_steer
        namespace rotate_platform {
            using update = rac1::Moby_Platform_Update;
        }  // namespace rotate_platform
        namespace rotating_door {
            using update = rac1::Moby_Class127_Update;
        }  // namespace rotating_door
        namespace route_platform {
            using update = rac1::Moby_Shuttle424_Update;
        }  // namespace route_platform
        namespace ryno {
            using update = rac1::Moby_Ryno_Update;
        }  // namespace ryno
        namespace sand_mouse {
            using update = rac1::Moby_SandMouse_Update;
        }  // namespace sand_mouse
        namespace sand_shark {
            using update = rac1::Moby_SandShark_Update;
        }  // namespace sand_shark
        namespace sand_shark_nest {
            using update = rac1::Moby_SandSharkNest_Update;
        }  // namespace sand_shark_nest
        namespace scale_fade_and_delete {
            using update = rac1::Moby_Class1427_Update;
        }  // namespace scale_fade_and_delete
        namespace scenery_ship {
            using update = rac1::Moby_SceneryShip_Update;
        }  // namespace scenery_ship
        namespace scoutbot {
            using update = rac1::Moby_Scoutbot_Update;
        }  // namespace scoutbot
        namespace scrolling_layers2 {
            using update = rac1::Moby_Class678_Update;
        }  // namespace scrolling_layers2
        namespace sentrybot {
            using update = rac1::Moby_Sentrybot_Update;
        }  // namespace sentrybot
        namespace sentrybot_alarm {
            using update = rac1::Moby_SentrybotAlarm_Update;
        }  // namespace sentrybot_alarm
        namespace shady_salesman {
            using update = rac1::Moby_ShadySalesman_Update;
        }  // namespace shady_salesman
        namespace shard_debris {
            using update = rac1::Moby_Constructorbot1208_Update;
        }  // namespace shard_debris
        namespace sharkigator_1059 {
            using update = rac1::Moby_Sharkigator1059_Update;
        }  // namespace sharkigator_1059
        namespace sharkigator_35 {
            using update = rac1::Moby_Sharkigator_Update;
        }  // namespace sharkigator_35
        namespace ship_collider {
            using update = rac1::Moby_Ammocrate_Update;
        }  // namespace ship_collider
        namespace ship_path {
            using update = rac1::Moby_BackgroundShip2_Update;
        }  // namespace ship_path
        namespace ship_pickup {
            using update = rac1::Moby_Ammocrate224_Update;
        }  // namespace ship_pickup
        namespace shockwave {
            using update = rac1::Moby_Class1046_Update;
        }  // namespace shockwave
        namespace shuttle {
            using update = rac1::Moby_Shuttle_Update;
        }  // namespace shuttle
        namespace side_drift {
            using update = rac1::Moby_ConveyorBelt1250_Update;
        }  // namespace side_drift
        namespace sinking_platform {
            using update = rac1::Moby_FloatingPlatform587_Update;
        }  // namespace sinking_platform
        namespace skid_mc_marx {
            using update = rac1::Moby_SkidMcmarx717_Update;
        }  // namespace skid_mc_marx
        namespace skid_mc_marx_aridia {
            using update = rac1::Moby_SkidMcmarx_Update;
        }  // namespace skid_mc_marx_aridia
        namespace skids_agent {
            using update = rac1::Moby_SkidsAgent_Update;
        }  // namespace skids_agent
        namespace skill_point {
            using update = rac1::Moby_Class1559_Update;
        }  // namespace skill_point
        namespace sliding_door {
            using update = rac1::Moby_DoorLeft_Update;
        }  // namespace sliding_door
        namespace sliding_gate {
            using update = rac1::Moby_Gate92_Update;
        }  // namespace sliding_gate
        namespace smoke_emitter {
            using update = rac1::Moby_Class779_Update;
        }  // namespace smoke_emitter
        namespace snagglebeast {
            using update = rac1::Moby_Snagglebeast_Update;
        }  // namespace snagglebeast
        namespace sound_prop {
            using update = rac1::Moby_Class1067_Update;
        }  // namespace sound_prop
        namespace spark_burst_spawner {
            using update = rac1::Moby_Class1552_Update;
        }  // namespace spark_burst_spawner
        namespace spark_emitter {
            using update = rac1::Moby_Class1512_Update;
        }  // namespace spark_emitter
        namespace spark_ring_emitter {
            using update = rac1::Moby_Class1812_Update;
        }  // namespace spark_ring_emitter
        namespace sparks_trigger_l04 {
            using update = rac1::Moby_Class1549_Update;
        }  // namespace sparks_trigger_l04
        namespace sparks_trigger_l06 {
            using update = rac1::Moby_Class1551_Update;
        }  // namespace sparks_trigger_l06
        namespace sparks_trigger_l13 {
            using update = rac1::Moby_Class1558_Update;
        }  // namespace sparks_trigger_l13
        namespace spawn_child_and_delete {
            using update = rac1::Moby_Mushroom_Update;
        }  // namespace spawn_child_and_delete
        namespace spiderbot {
            using update = rac1::Moby_Spiderbot_Update;
        }  // namespace spiderbot
        namespace spin_fade_out {
            using update = rac1::Moby_Class112_Update;
        }  // namespace spin_fade_out
        namespace spinner {
            using update = rac1::Moby_RoboShackSign_Update;
        }  // namespace spinner
        namespace spinning_debris {
            using update = rac1::Moby_Class1085_Update;
        }  // namespace spinning_debris
        namespace spinning_object {
            using update = rac1::Moby_GadgetronSign_Update;
        }  // namespace spinning_object
        namespace spinning_pickup {
            using update = rac1::Moby_Magnetboots_Update;
        }  // namespace spinning_pickup
        namespace starfighter_part {
            using update = rac1::Moby_QwarksStarfighterPart389_Update;
        }  // namespace starfighter_part
        namespace steam_vent {
            using update = rac1::Moby_Class700_Update;
        }  // namespace steam_vent
        namespace step_on_switch {
            using update = rac1::Moby_Button1416_Update;
        }  // namespace step_on_switch
        namespace stops_platform {
            using update = rac1::Moby_Door1359_Update;
        }  // namespace stops_platform
        namespace suck_cannon {
            using update = rac1::Moby_Suckcannon_Update;
        }  // namespace suck_cannon
        namespace swarm {
            using update = rac1::Moby_Class86_Update;
        }  // namespace swarm
        namespace swarm_leader {
            using update = rac1::Moby_Class463_Update;
        }  // namespace swarm_leader
        namespace sweeping_bolt {
            using update = rac1::Moby_Class681_Update;
        }  // namespace sweeping_bolt
        namespace swing_door {
            using update = rac1::Moby_Gate1_Update;
        }  // namespace swing_door
        namespace swinging_hazard {
            using update = rac1::Moby_ToxicGasVent_Update;
        }  // namespace swinging_hazard
        namespace switch_ {
            using update = rac1::Moby_Class231_Update;
        }  // namespace switch_
        namespace switch_follower {
            using update = rac1::Moby_EudoraBridge_Update;
        }  // namespace switch_follower
        namespace swooper {
            using update = rac1::Moby_Mine1112_Update;
        }  // namespace swooper
        namespace talking_npc {
            using update = rac1::Moby_ThePlumber_Update;
        }  // namespace talking_npc
        namespace target_activator {
            using update = rac1::Moby_Class1923_Update;
        }  // namespace target_activator
        namespace target_driven_slider {
            using update = rac1::Moby_Class1013_Update;
        }  // namespace target_driven_slider
        namespace target_tracker {
            using update = rac1::Moby_MetalDetector_Update;
        }  // namespace target_tracker
        namespace taxi {
            using update = rac1::Moby_Taxi712_Update;
        }  // namespace taxi
        namespace taxi_platform_1474 {
            using update = rac1::Moby_TaxiPlatform1474_Update;
        }  // namespace taxi_platform_1474
        namespace taxi_platform_38 {
            using update = rac1::Moby_TaxiPlatform_Update;
        }  // namespace taxi_platform_38
        namespace teleporter_pad {
            using update = rac1::Moby_Teleporter_Update;
        }  // namespace teleporter_pad
        namespace test_dummy {
            using update = rac1::Moby_TestDummy_Update;
        }  // namespace test_dummy
        namespace tethered_mine {
            using update = rac1::Moby_FighterJet_Update;
        }  // namespace tethered_mine
        namespace texture_scroll {
            using update = rac1::Moby_Class809_Update;
        }  // namespace texture_scroll
        namespace thrown_bomb {
            using update = rac1::Moby_Bomb281_Update;
        }  // namespace thrown_bomb
        namespace thrown_object {
            using update = rac1::Moby_Decoy_Update;
        }  // namespace thrown_object
        namespace timed_lift {
            using update = rac1::Moby_ThrusterPackLockRed_Update;
        }  // namespace timed_lift
        namespace timed_pad {
            using update = rac1::Moby_TimedActivationPads_Update;
        }  // namespace timed_pad
        namespace timed_shooter {
            using update = rac1::Moby_BombthrowerGrass939_Update;
        }  // namespace timed_shooter
        namespace toxic_crab {
            using update = rac1::Moby_ToxicCrab_Update;
        }  // namespace toxic_crab
        namespace trackdrive_left {
            using update = rac1::Moby_TrackdriveLeft_Update;
        }  // namespace trackdrive_left
        namespace trail_moby {
            using update = rac1::Moby_Class804_Update;
        }  // namespace trail_moby
        namespace train_car {
            using update = rac1::Moby_RedShip1_Update;
        }  // namespace train_car
        namespace trespasser_puzzle {
            using update = rac1::Moby_TresspasserPad_Update;
        }  // namespace trespasser_puzzle
        namespace trigger_lists {
            using update = rac1::Moby_Class1039_Update;
        }  // namespace trigger_lists
        namespace triggered_rotor {
            using update = rac1::Moby_Class895_Update;
        }  // namespace triggered_rotor
        namespace triggered_unlock {
            using update = rac1::Moby_Class1352_Update;
        }  // namespace triggered_unlock
        namespace trooper {
            using update = rac1::Moby_MineLayer_Update;
        }  // namespace trooper
        namespace tumbling_debris {
            using update = rac1::Moby_Class313_Update;
        }  // namespace tumbling_debris
        namespace tutorial_hint {
            using update = rac1::Moby_Class1324_Update;
        }  // namespace tutorial_hint
        namespace tutorial_hint_trigger {
            using update = rac1::Moby_Class1342_Update;
        }  // namespace tutorial_hint_trigger
        namespace ultramech {
            using update = rac1::Moby_UltramechUnlimited_Update;
        }  // namespace ultramech
        namespace ultramech_scientist {
            using update = rac1::Moby_UltramechScientist_Update;
        }  // namespace ultramech_scientist
        namespace underwater_ring {
            using update = rac1::Moby_UnderwaterRing_Update;
        }  // namespace underwater_ring
        namespace unlock_planet_pad {
            using update = rac1::Moby_Lieutenant_Update;
        }  // namespace unlock_planet_pad
        namespace veldin_ship {
            using update = rac1::Moby_VeldinShip_Update;
        }  // namespace veldin_ship
        namespace vendor {
            using update = rac1::Moby_VeldinRobotEnemy_Update;
        }  // namespace vendor
        namespace vendor_moby {
            using update = rac1::Moby_Class644_Update;
        }  // namespace vendor_moby
        namespace visibomb_missile {
            using update = rac1::Moby_VisibombMissile_Update;
        }  // namespace visibomb_missile
        namespace wandering_helper {
            using update = rac1::Moby_Chomper_Update;
        }  // namespace wandering_helper
        namespace wandering_light {
            using update = rac1::Moby_Class1504_Update;
        }  // namespace wandering_light
        namespace warp_portal {
            using update = rac1::Moby_SuckCannon_Update;
        }  // namespace warp_portal
        namespace watch_mobys {
            using update = rac1::Moby_Class1577_Update;
        }  // namespace watch_mobys
        namespace watcher {
            using update = rac1::Moby_HelpDeskLady_Update;
        }  // namespace watcher
        namespace water_plane_marker {
            using update = rac1::Moby_Class982_Update;
        }  // namespace water_plane_marker
        namespace water_plate {
            using update = rac1::Moby_Water902_Update;
        }  // namespace water_plate
        namespace water_reservoir {
            using update = rac1::Moby_WaterReservoir_Update;
        }  // namespace water_reservoir
        namespace water_surface {
            using update = rac1::Moby_Water_Update;
        }  // namespace water_surface
        namespace water_surface_fx {
            using update = rac1::Moby_Class751_Update;
        }  // namespace water_surface_fx
        namespace wb_moby {
            using update = rac1::Moby_WoodcutBot_Update;
        }  // namespace wb_moby
        namespace wrench {
            using update = rac1::Moby_Wrench_Update;
        }  // namespace wrench
    }  // namespace moby
    namespace pad {
        using process_report = rac1::Pad_ProcessReport;
    }  // namespace pad
    namespace planet {
        using unlock = rac1::Planet_Unlock;
    }  // namespace planet
    namespace player {
        using advance_animation = rac1::Hero_AdvanceAnimation;
        using approach_speed = rac1::Hero_ApproachSpeed;
        using set_planar_velocity = rac1::Hero_SetPlanarVelocity;
        using take_damage = rac1::Hero_TakeDamage;
        using turn_to_heading = rac1::Hero_TurnToHeading;
        using update_motion_input = rac1::Hero_UpdateMotionInput;
    }  // namespace player
    namespace vendor {
        using build_item_list = rac1::Vendor_BuildItemList;
        using init_level_stock = rac1::Vendor_InitLevelStock;
    }  // namespace vendor
    namespace weapon {
        using take_ammo = rac1::Weapon_TakeAmmo;
    }  // namespace weapon
}  // namespace hooks

}  // namespace openrac::rac1
