# Hooks

A mod changes what the game does at **hook points**: functions of the game that
OpenRAC lets mods take over. With no mod on a hook point, the game's own code
runs, exactly as on the console.

## Four ways to use a hook point

| | What your function does | Example |
|---|---|---|
| **wrap** | does something, calls `original`, maybe does more after | change an argument, then let the game go on |
| **replace** | never calls `original` | write a behaviour anew |
| **before** | runs before the rest; the game's code always runs | count jumps |
| **after** | runs after the rest | read the result, nudge a value |

In Luau, `openrac.hook(id, fn)` wraps or replaces (`fn(original, ...)`), and
`openrac.before` and `openrac.after` take a plain function. In C and C++, a
hook is a wrap or a replace; before and after are a wrap that calls
`original` first or last.

```lua
openrac.hook("rac1.player.set_planar_velocity", function(original, angle)
    original(angle)   -- the game's own, or the mod before yours
    openrac.log(`moving along {angle}`)
end)

openrac.after("rac1.player.advance_animation", function()
    -- runs after the game has moved the animation on
end)
```

## Several mods on one hook point

Mods on the same hook point form a chain, in the order of the Mods tab. The
mod **lower** in the list (loaded later) runs **first**, and its `original`
is the mod above it, down to the game's code. So:

- a mod that calls `original` lets the others run too;
- a mod that does not call `original` stops the chain there; the launcher
  shows when mods share a hook point, so players can order them;
- script and native mods share the same chain.

Take hooks only while your mod loads (in `openrac_mod_load`, in your C++
`load`, or at the top level of a script). Hook points are the same on every
disc a mod supports.

## Stable ids

Every hook point has a permanent **id** of the form
`<game>.<area>.<name>`. Ids never change, even when OpenRAC renames the game
function behind them (the decompilation renames functions as it learns what
they do, and the port's code is moving to C++). Write ids in new mods.

The names hook points had first (`Hero_TurnToHeading`, from the
decompilation) stay as **aliases**: a mod written with one keeps loading.

| Id | Alias | What the game does there |
|---|---|---|
| `rac1.pad.process_report` | `Pad_ProcessReport` | turns one pad report into the pad's state, each poll |
| `rac1.player.update_motion_input` | `Hero_UpdateMotionInput` | reads the stick into the hero, early in his update |
| `rac1.player.turn_to_heading` | `Hero_TurnToHeading` | eases the hero's facing toward where he moves |
| `rac1.player.set_planar_velocity` | `Hero_SetPlanarVelocity` | sets his ground velocity along an angle |
| `rac1.camera.follow.apply_overrides` | `FollowCamera_ApplyOverrides` | the follow camera's per-tick requests from the hero's state |
| `rac1.player.advance_animation` | `Hero_AdvanceAnimation` | moves his animation on, once a tick |

The [reference](reference/hooks-rac1.md) lists every hook point with its
signature and parameters, generated from OpenRAC's table.

## Moby updates (experimental)

The behaviour catalogue lists the game's moby classes (crates, enemies, ...),
their update functions and their per-class variables (pvars). A class's update
is a hook point with the id `rac1.moby.<class>.update` (alias
`Moby_<Class>_Update`), once OpenRAC has made it one. Several classes may share
one update function: check the moby's class in your hook. This part names the
game's internals and is **experimental**: names, fields and signatures may
change as the decompilation goes on.

## What a hook cannot do

- Hook a function OpenRAC has not made a hook point. Ask for one in an
  [issue](https://github.com/OpenRAC/OpenRAC/issues): adding a hook point
  changes nothing for players without mods.
- Take a hook point after load, or let go of one.
- Keep pointers into the game's memory across levels: a level moves its data.
