-- Generated from sdk/bindings/rac1.json by sdk/tools/gen_binding.py. Do not edit.
-- SPDX-License-Identifier: GPL-3.0-or-later
--
-- The `openrac` global as a script mod for rac1 sees it (docs/port/MODS.md, section 11): put this
-- file where your editor's Luau language server reads definitions (luau-lsp: --definitions).

type HookPoint = "Pad_ProcessReport" | "Hero_UpdateMotionInput" | "Hero_TurnToHeading" | "Hero_SetPlanarVelocity" | "FollowCamera_ApplyOverrides" | "Hero_AdvanceAnimation"
type MemoryType = "u8" | "i8" | "u16" | "i16" | "u32" | "i32" | "f32"

type Fields = {
    hero: {
        position: {
            x: number, -- Where the hero is (gHeroPos).
            y: number,
            z: number,
        },
        facing: number, -- The way he faces (rot.z), in radians.
        move_yaw: number, -- Where the stick moves him: the stick's angle plus the camera's yaw.
        state: number, -- read-only; His state: 0 standing, 2 running, 7 jumping, ...
    },
    bolts: number, -- The player's bolts.
}

-- Pad_ProcessReport void(u32,u32,i32): One pad report into the pad's state (PadState, at pad), each poll, before the tick reads it.
type Pad_ProcessReportFn = (pad: number, report: number, size: number) -> ()
-- Hero_UpdateMotionInput void(): The stick read into the hero, early in his update each tick.
type Hero_UpdateMotionInputFn = () -> ()
-- Hero_TurnToHeading void(i32,f32,f32,f32): Eases the hero's facing toward his move yaw.
type Hero_TurnToHeadingFn = (mode: number, accel: number, damping: number, limit: number) -> ()
-- Hero_SetPlanarVelocity void(f32): The hero's velocity at his ground speed along an angle; an angle above pi means his facing.
type Hero_SetPlanarVelocityFn = (angle: number) -> ()
-- FollowCamera_ApplyOverrides void(u32): The follow camera's per-tick requests from the hero's state; the camera moby's update calls it before it places the eye.
type FollowCamera_ApplyOverridesFn = (camera: number) -> ()
-- Hero_AdvanceAnimation void(): Ratchet's animation moved on by his animation speed, once a tick after his state's update.
type Hero_AdvanceAnimationFn = () -> ()

declare openrac: {
    game: string,
    version: "pal" | "ntsc",
    platform: string,
    api: string,
    log: (...any) -> (),
    warn: (...any) -> (),
    setting: (key: string) -> any,
    hook: (name: HookPoint, fn: (original: (...number) -> ...number, ...number) -> ...number) -> (),
    before: (name: HookPoint, fn: (...number) -> ()) -> (),
    after: (name: HookPoint, fn: (...number) -> ()) -> (),
    on_frame: (fn: () -> ()) -> (),
    on_pad: (fn: (port: number, report: buffer) -> ()) -> (),
    read: (kind: MemoryType, address: number) -> number,
    write: (kind: MemoryType, address: number, value: number) -> (),
    fields: Fields,
}
