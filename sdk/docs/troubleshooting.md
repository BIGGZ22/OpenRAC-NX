# Troubleshooting

## The mod is on but does nothing

1. **Look at its row in the Mods tab.** A mod that cannot load says why there,
   before Play (another disc, an interface this OpenRAC lacks, no library for
   this computer, a missing or conflicting mod) and after it (what happened at
   the last start).
2. **Read `status.json`** in the game's `plugins/` folder
   ([where mods live](where-mods-live.md)). For each mod that is on:

   | `state` | Meaning |
   |---|---|
   | `loaded` | running; `hooks` lists what it took |
   | `refused` | a check failed before it was opened; `reason` says which |
   | `failed` | its library could not be opened, its load refused (with your `fail` reason), it asked for a hook point this OpenRAC does not have or with another signature, or (a script) it raised an error or ran over its time budget: `reason` has the message |

   `hook_points` lists every hook point this OpenRAC has.
3. **Read the game's log** (**Logs** on the game's page): its header has a
   `mods` line naming the mods loaded, and your mod's lines start with its id.
   A script's errors are there with their traceback.

## Common reasons

| Reason | What to do |
|---|---|
| needs interface 1.1; this OpenRAC has 1.0 | the player needs a newer OpenRAC, or set `api` to what you really use |
| not for this disc | add the disc to `games` if the mod works there |
| no library for this platform | build for it (the template's workflow builds all desktop ones), or it is Android, which loads only script mods |
| no hook point X | check its [id](hooks.md#stable-ids), or ask for the hook point |
| signature differs | the hook point's signature is in the [reference](reference/hooks-rac1.md) |
| stopped while the game ran: hook X: ... | your script's error, or it ran over its 2 ms budget |

## Purist mode

The launcher's **Purist mode** (in Settings and on the Mods tab) starts the game
with no mods at all (`--no-mods`): the game as on the console, with only
OpenRAC's own fixes. Ask players to try it first when they report a problem.

## When the game crashes

The crash report (the game page's **Logs and crash reports**) lists every mod
that was loaded, and names the one whose code was running when it can tell: a
native mod's library, or the script mod being called. The launcher's crash
notice suggests trying again in purist mode when mods were on.

## Asking for help

Open an issue at [OpenRAC](https://github.com/OpenRAC/OpenRAC/issues) with the
mod's `status.json`, the log, and the crash report if there is one; the reports
leave out the player's home folder.
