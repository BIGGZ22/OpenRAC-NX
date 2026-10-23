# Every weapon, and how to test it in the port

A weapon in the port has to do what it does on the console: come out when it
is chosen, fire on the button it fires on, use its ammo, hit what it should
hit, sound as it should, and — for a mod that reads the game — report the
right thing while it fires. This document is the practical half of that: how
to put any weapon in Ratchet's hands on demand, how to fire it from a
script, and what to look at. It complements
[RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md) (the run line, the scripted
switches, reading game memory), [CONTROLLERS.md](CONTROLLERS.md) (what the
game asks a controller for) and [MODS.md](MODS.md) (a mod's view of the
same).

Written against the PAL build. The NTSC-U copy keeps the executable's data
`0x100` lower (so every address below that is in the executable, not in a
level, is `0x100` less there); the level's own data (`D_L00_*`) moves with
each level and is reached through `OPENRAC_LDATA`, not a raw address
([RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md) §5).

## 1. The weapons

Item ids are the game's own (`games/rac1/include/names.h`,
`games/rac1/docs/kb/datamining.md`). Every weapon is an *item*; the game
distinguishes weapons (they go into the quick select) from gadgets by
`D_L00_00179BC0[id].t` at +0x08 — `0` is a weapon
(`games/rac1/src/overlays/shared/mobyutil_00258BC8.c:2935`).

| id | item | fires on | how | ammo |
|---|---|---|---|---|
| 0x08 | OmniWrench | Square | a swing; R1 held while Square is pressed throws it | none |
| 0x09 | Suck Cannon | Circle | holds what it sucks, then fires it | none |
| 0x0a | Bomb Glove | Circle | a thrown bomb | yes |
| 0x0b | Devastator | Circle | a rocket, while held at its own rate | yes |
| 0x0c | Swingshot | Circle | a grapple, at the marked target | none |
| 0x0d | Visibomb Gun | Circle | launches a missile the player steers; Circle again detonates | yes |
| 0x0e | Taunter | Circle | taunts nearby enemies | none |
| 0x0f | Blaster | Circle | fires while held, at its own rate | yes |
| 0x10 | Pyrocitor | Circle | a flame while held | yes |
| 0x11 | Mine Glove | Circle | drops a mine | yes |
| 0x12 | Walloper | Circle | a punch; it plays a sound and has its own hero state | none |
| 0x13 | Tesla Claw | Circle | lightning while held | yes |
| 0x14 | Glove of Doom | Circle | releases the agents | yes |
| 0x15 | Morph-o-Ray | Circle | a beam while held | none |
| 0x16 | Hydrodisplacer | — | a gadget, not a weapon | — |
| 0x17 | R.Y.N.O. | Circle | a salvo of seven missiles | yes |
| 0x18 | Drone Device | — | a drone that damages nearby enemies on its own | yes |
| 0x19 | Decoy Glove | Circle | drops a decoy | yes |

The **eleven items with an ammo limit** are exactly 0x0a, 0x0b, 0x0d, 0x0f,
0x10, 0x11, 0x13, 0x14, 0x17, 0x18 and 0x19: the table `D_L00_001C43B0` is
nonzero for those and zero for the rest
([datamining.md](../../games/rac1/docs/kb/datamining.md) §1.3). A zero limit
is unlimited — `TakeAmmo` returns 1 without subtracting
(`games/rac1/src/overlays/shared/help_00232560.c:1080-1085`).

The button is not hard-coded in the hero: each weapon's own moby carries its
button mask in `hero.items[0].button_mask` (`games/rac1/include/hero.h:88`,
`HeroItemSlot.button_mask` at slot +0x10), and its update tests
`held & button_mask` (`games/rac1/src/overlays/shared/vendor_002B33E8.c:156`).
**Verified in the port**: with the wrench out the mask reads `0x80` (Square),
with the Blaster out `0x20` (Circle); see §4.

## 2. What gives a weapon: `GiveItem`

Everything the game does when a weapon is awarded is in one function:

`GiveItem(id, flag)` — `games/rac1/src/overlays/shared/mobyutil_00258BC8.c:2884`

1. `D_0013D5F0[id] = 1` — **obtained** (`:2895`; the second flag, read by
   the mission predicates, `games/rac1/src/game/missionfunc.c:203`).
2. The banner (`:2896-2898`) — shown even for an item already owned.
3. If `D_0013D5C8[id]` is not yet set (`:2899`):
   - `D_0013D5C8[id] = 1` — **owned** (`:2901`; the flag the pause menu's
     item list reads);
   - **starting ammo**: `D_0013D530[id]` becomes `max(now,
     D_L00_001C43B0[id].b)` when the record's `a` (+0x08) is not 0
     (`:2904-2910`);
   - the item is marked in the slot list `D_0015EED0[]` (`:2913-2923`);
   - with `flag != 0`, `hero.pending_item[t] = id` (`t` at
     `D_L00_00179BC0[id].t`): Ratchet takes it out (`:2925-2933`);
   - **quick select**: the first empty entry of `D_00141FA0[8]` becomes the
     id, when the item's type is 0, a weapon (`:2935-2941`).

**The trap** is step 3's condition: the ammo, the quick-select entry and the
equip are all inside `if (D_0013D5C8[id] == 0)`. Anything that sets the owned
flag *without* going through `GiveItem` — the cheat codes, a memory write —
takes the grant away for the rest of the session: a later `GiveItem` shows
the banner and nothing else (§3.4).

The vendor is the one in-game path that calls it with the flag:
`GiveItem(sel, 1)` at `games/rac1/src/overlays/shared/vendor_002A5218.c:1470`.

## 3. Getting a weapon into your hands

### 3.1 Play to it — the faithful way (vendors)

A vendor sells the stock its own slot list holds (`VendorA.slots`), and
buying calls `GiveItem(sel, 1)`.
Prices and the ammo limits are in the executable's table `D_001E02B0`, 0x18
bytes a record: price at +0x08, the alternate price at +0x0A, the ammo limit
at +0x0E (`games/rac1/src/game/vendor.c:610-620`). Bolts are `gBolts` =
`0x0015EE98` (`include/names.h:902`).

For a test that needs the whole award path (banner, sound, ammo, quick
select, the weapon coming out), this is the route to use, and the flow is
already scripted: teleport to the vendor, press Triangle, then Cross
([RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md) §4). Give yourself the bolts
first:

```sh
OPENRAC_WRITE="2400:15EE98:000F423F:60"     # gBolts = 999999, held 60 frames
```

(Money is never a fix, only a way to reach a screen in a test.)

### 3.2 `OPENRAC_WRITE` — give any item, then switch to it

This is the recipe the rest of this document uses. It is **verified in the
port**: on Novalis, with the Blaster (0x0f), it put the Blaster out and the
Blaster answered Circle (§6 has the run and what it does and does not
prove).

Three words do the work, all in the executable's data:

| What | Address | Value |
|---|---|---|
| owned flag, byte `id` of `D_0013D5C8` | `0x13D5C8 + (id & ~3)` | `1 << (8 * (id & 3))` |
| ammo, `D_0013D530[id]` | `0x13D530 + id * 4` | the weapon's limit, or any number for a weapon with no limit |
| what Triangle switches to with the wrench out, `D_00141760` | `0x141760` | `id` |

`OPENRAC_WRITE` writes a **32-bit word**, which is why the owned flag is
written a word at a time: the word covers items `id & ~3` to `id | 3`, and
the byte for `id` is set while the other three are written **0**. So this is
also a way to *clear* the neighbours: if any of those items were owned, they
are not any more — invisible to the pause menu, and a later `GiveItem` would
grant them afresh. Firing does not consult the flag (the switch and the
weapon's own moby do not read `D_0013D5C8`), so a test that only fires one
weapon is unaffected; a test that awards several should write each word once,
after the set of items is decided.

Then **Triangle** switches to it through the game's own code — not by
writing `hero.pending_item[0]`, which on its own is not enough (§6, trap 3):

```
Triangle  OPENRAC_PRESS mask 0x1000    (the game's pad bit 0x0010)
Circle    OPENRAC_PRESS mask 0x2000    (the game's pad bit 0x0020)
```

The switch is `games/rac1/src/overlays/l01_novalis/help_00243078.c:164-181`:
on Triangle's release, `next = D_0015EE8C` (the item in use), or
`D_00141760` when the item out is the wrench; it writes
`pending_item[0] = next`. Setting `D_00141760` is therefore how a script
says "switch to this one" while the wrench is out.

A shell helper for any id (the arithmetic above, and the two presses):

```sh
# give <id> at frame <f>, holding the words <n> frames, then Triangle at
# <f2> and Circle at <f3>
id=15; f=2500; n=5200; f2=6000; f3=6200
w=$(printf '%08X' $(( 1 << (8 * (id & 3)) )))     # the owned byte's word
owned=$(printf '%X' $(( 0x13D5C8 + (id & ~3) )))
ammo=$(printf '%X' $(( 0x13D530 + id * 4 )))
OPENRAC_WRITE="$f:$owned:$w:$n,$f:$ammo:000000C8:$n,$f:141760:$(printf '%08X' $id):$n" \
OPENRAC_PRESS="$f2:1000:5,$f3:2000:5" ...
```

For the Blaster that gives `w=01000000`, `owned=13D5D4`, `ammo=13D56C`.

### 3.3 The pause menu lists, and does not equip

The pause menu's Weapons page draws each owned item with its ammo
(`Pause_DrawItemCount`, `games/rac1/src/game/pause.c:4890-4935`: it reads
`D_0013D5C8[id]` and `D_0013D530[id]`). It is a *list*: it does not set
`pending_item`, so it cannot put a weapon in Ratchet's hands. Equipping is
the in-game quick select (Triangle held / Select) or the Triangle switch
above. The quick select's eight entries are `D_00141FA0`, loaded and saved by
`Pause_LoadQuickSelect` / `Pause_SaveQuickSelect`
(`games/rac1/src/game/pause.c:3028-3060`); its **in-game wheel** is drawn
from the level's own table `D_L00_0015FB78`, which is why a level can show a
different wheel from the pause page's list
(`games/rac1/src/overlays/shared/hud_00235960.c:1100-1160`).

### 3.4 The cheat codes — and why not to use them for weapons

The game has a hidden button-code system: hold **R2 + L1** in the pause menu
and press 20 buttons; codes `0x00`–`0x24` set an item's flags, `0x25`–`0x36`
unlock planets, `0x3d`–`0x5a` set skill points
([games/rac1/docs/kb/datamining.md](../../games/rac1/docs/kb/datamining.md)
§1, with all 91 sequences in §1.4; the table there is a static derivation
and **has not been played**). The matcher walks a 256-byte table,
`D_L00_001B9F20`, once the 20th symbol is in
(`games/rac1/src/overlays/shared/pause_00277208.c:1186`).

For weapons they are the wrong tool, twice over:

- they set **`D_0013D5C8[c]` and `D_0013D5F0[c]` only** — no ammo, no
  quick-select entry, no equip
  (`games/rac1/docs/kb/datamining.md` §1.3);
- and because they set the owned flag, a later `GiveItem` (the vendor, a
  mission reward) takes the `if (D_0013D5C8[id] == 0)` branch and grants the
  ammo and the quick-select entry **never**. The item stays at 0 ammo for
  the session.

Use them to test the codes themselves, not to arm a weapon test.

### 3.5 The I-5 debug menu — ammo and bolts, not weapons

The game's debug menu, `game mode -1`, is a real menu with an **AMMO** page
(`games/rac1/docs/kb/datamining.md` §2; the update is
`games/rac1/src/overlays/shared/draw_001F3A78.c:488`, the page is `case 3`
at `:646-679`). Each row writes **one thing**: the weapon's ammo
`D_0013D530[id]` (the row's id comes from the level's table
`D_L00_0015F0D0`), or `gBolts` for the last row. It does **not** call
`GiveItem`, so it does not own the item, fill the quick select or equip it.
It is not reachable with buttons: the only way in is writing the game mode
(`gGameMode`, level data `D_L00_0015F6A8`) to `0xFFFFFFFF`. The mode's
`case -1` is present in the port
(`games/rac1/src/overlays/shared/map_002465F8.c:204-208`). **Untested in the
port**: no button path exists, and writing a level's data from outside needs
`OPENRAC_LDATA`'s level, not a raw address
([RAC1_PAL_TESTING.md](RAC1_PAL_TESTING.md) §5).

### 3.6 A save

A memory card from a finished game carries the owned flags, the ammo and the
quick select, so a test that starts at a *late* weapon can start from one:
`--cards DIR` points the port at a card directory. Everything else should
run with `--no-card`, so a test never writes to one.

## 4. Which button, and what the game is doing

Two button numberings appear in the port's tools and in the game's code, and
they are **not** the same:

| Button | Game's pad bit | `OPENRAC_PRESS` mask |
|---|---|---|
| L2 | 0x0001 | 0x0100 |
| R2 | 0x0002 | 0x0200 |
| L1 | 0x0004 | 0x0400 |
| R1 | 0x0008 | 0x0800 |
| Triangle | 0x0010 | 0x1000 |
| Circle | 0x0020 | 0x2000 |
| Cross | 0x0040 | 0x4000 |
| Square | 0x0080 | 0x8000 |
| Select | 0x0100 | 0x0001 |
| Start | 0x0800 | 0x0008 |
| Up | 0x1000 | 0x0010 |
| Right | 0x2000 | 0x0020 |
| Down | 0x4000 | 0x0040 |
| Left | 0x8000 | 0x0080 |

The mask is the game's bit with its two bytes swapped, because
`OPENRAC_PRESS` clears bits in the pad's **reported** word and the game reads
that word big-endian
(`port/game/common/frontend.cpp:3082-3122`; `OPENRAC_PRESS` comment,
"mask bits as the pad reports them, 0x4000 cross, 0x8 start"). The swap is a
trap: in an `OPENRAC_PRESS` line `0x4000` is **Cross**, not Down, and
`0x8000` is **Square**, not Left.

## 5. What each weapon should look, sound and feel like

The game's side is the decompilation's own weapon mobies; this table is what
a tester watches. "Feels" is the DualSense mod's per-weapon vibration
(`rac1-mods/dualsense`, a sibling repository) — the column is here because
that mod is the reason to fire every weapon in one session and check that
each one gets its own feedback.

| id | item | what to watch | the mod's feel |
|---|---|---|---|
| 0x09 | Suck Cannon | sucks a crate or an enemy, holds it, throws it | a steady resistance |
| 0x0a | Bomb Glove | the bomb leaves the hand and explodes | a light click |
| 0x0b | Devastator | a rocket, smoke, a bigger blast | a heavy click |
| 0x0d | Visibomb Gun | the camera follows the missile; Circle detonates | a light click |
| 0x0e | Taunter | the taunt animation and the enemies' reaction | a steady resistance |
| 0x0f | Blaster | a bolt per shot while Circle is held; the HUD count | a light resistance and a kick per shot |
| 0x10 | Pyrocitor | the flame while held | a hum while firing |
| 0x11 | Mine Glove | the mine on the ground, then the blast | a light click |
| 0x12 | Walloper | the punch, its own hero state (`0x20`), no fire flag | a heavy click |
| 0x13 | Tesla Claw | the arc while held | a hum while firing |
| 0x14 | Glove of Doom | the agents appear and run off | a light click |
| 0x15 | Morph-o-Ray | the beam, the morph | a hum while firing |
| 0x17 | R.Y.N.O. | seven missiles; the HUD count | a heavy buzz through the salvo |
| 0x19 | Decoy Glove | the decoy lands and draws fire | a light click |

Two weapons read the pad themselves rather than firing through the moby's
button mask: the **Visibomb Gun** and the **Taunter**. Watch the shot itself
for those (the missile, the taunt animation) rather than a fire flag.

## 6. A scripted test, end to end

Verified 2026-10-22 in the port with the PAL disc, the DualSense connected,
and the mod loaded. Novalis is the quickest level to reach: the direct start
still plays the arrival scene, so the level is drawn anywhere from 2970 to
5470 frames in, and the switch below landed at 6705–6805 in these runs.

```sh
DATA=$HOME/.local/share/dev.openrac.launcher/active/rac1/data
cd port
TRI=$(for f in $(seq 4700 300 7100); do printf "%d:1000:5," $f; done)   # Triangle
CIR=$(for f in $(seq 5200 200 7400); do printf "%d:2000:5," $f; done)   # Circle
SDL_AUDIODRIVER=dummy OPENRAC_UNCAPPED=1 OPENRAC_DIRECT=1 \
OPENRAC_WATCH=13D5D4:1:i,140508:1:i,1404F0:1:i,1414F8:1:i \
OPENRAC_WRITE="2500:13D5D4:01000000:5200,2500:13D56C:000000C8:5200,2500:141760:0000000F:5200" \
OPENRAC_PRESS="${TRI}${CIR%,}" \
  ./build/release/openrac-rac1-pal --data "$DATA" --window --no-card \
  --level 1 --skip-movies --frames 7600 > /tmp/weapon.log 2>&1
grep watch /tmp/weapon.log | awk '{t=$5"|"$7"|"$9"|"$11; if(t!=p){print $3,t; p=t}}'
```

The watched fields, in order: the owned word `0x13D5D4` (bit 24 = item 0x0f),
`hero.items[0].item_id` (`0x140508`), `hero.items[0].button_mask`
(`0x1404F0`), and `hero.unk20A8` (`0x1414F8`, the fire flag in the word's
low byte). The run printed:

```
5145 16777216|8|128|0          the wrench is out; its button mask is 0x80 (Square)
6805 16777216|15|32|16777216   the Blaster is out; its mask is 0x20 (Circle)
7001 16777216|15|32|16842753   the flag's byte went 0 -> 1 on a Circle press
7011 16777216|15|32|16777216   and back on release
```

`16842753` is `0x01010001`: byte +0x20A8 = 1, +0x20AA = 1. The fire flag
pulses, so the Circle press reaches the game and the Blaster's own moby
answers it. What this run does **not** prove is that a round left the gun:
`D_0013D530[0x0f]` stayed at 200, and the Blaster is one of the eleven items
with an ammo limit (§1), so a shot should have taken one. Watch the ammo, the
projectile and the sound together before calling a weapon *fired*: the flag
is the input arriving, not the shot.

The same recipe with the **Walloper** (`id=0x12`, owned word
`0x13D5D8:00010000`, no ammo word, `0x141760:00000012`) puts it out (item id
18) and every Circle press holds `hero.state.current` at **0x20** for about
36 frames:

```
6705 18|32|0|0
6801 18|32|32|0        a Circle press: hero state 0x20
6837 18|32|0|0         back to 0
6901 18|32|32|0        the next press
```

That is the Walloper's own firing state
(`games/rac1/src/overlays/shared/help_00221A98.c:3267`): it raises no fire
flag, which is why a test of it watches the hero state, not `unk20A8`.

### The traps this recipe had to get past

1. **A level load wipes the three words.** `D_0013D530` is reset when a level
   starts, so a write during the arrival scene or the flight is gone by the
   time Ratchet has control. Hold the writes long, or start them after
   control (§6's run holds from 2500 to 7700, through the load).
2. **The boot's timing is not fixed.** The direct start fires only once the
   title's camera is up (`port/game/rac1/host/boot.c:132-152`), and the
   arrival scene's length varies: level 1 was drawn at frames 2970, 3772,
   4470, 4870, 5170 and 5470 in six runs. Absolute-frame scripts must cover
   the window (the looped presses above) or count from control with `cFRAME`
   (`c100:...`, `c300:2000:5`).
3. **`hero.pending_item[0]` alone does not equip.** Writing it makes the
   hero's item-switch code see a pending item, but the switch that creates
   the moby is the game's own (it runs on Triangle's release); four pulsed
   writes of `pending_item[0]` in one run left the wrench out. Setting
   `D_00141760` and letting Triangle set the same field is what worked.
4. **The owned word's address is a word, not a byte.** `0x13D5C8 + id` is
   the byte; `OPENRAC_WRITE` needs `0x13D5C8 + (id & ~3)` and the byte's bit
   inside the word (`1 << (8 * (id & 3))`). Writing `0x13D5CC` for item
   `0x0f` is `0x13D5C8 + 4`, which sets items 0x04–0x07 — the mistake this
   document was written after.
5. **`OPENRAC_PRESS` masks are byte-swapped** (§4): Triangle is `0x1000`
   and Circle `0x2000`, but `0x4000` is Cross and `0x8000` Square.

### Watching a shot without the mod

For guns the fire flag is `hero.unk20A8` (`0x001414F8` PAL, low byte of the
watched word). The Walloper has no flag — it fires from its own hero state
`0x20` (`games/rac1/src/overlays/shared/help_00221A98.c:3267`). The
Visibomb Gun and the Taunter read the pad themselves. For the ones that fire
on a counter, `hero.items[0].moby` (`0x1404E0`), then `+0x78` (the moby's
pvars) and `+0x00` is the shot's cooldown; the R.Y.N.O.'s `+0x04` counts
its missiles (`rac1-mods/dualsense/src/weapons.cpp`, `weapon_moment`).

## 7. What is verified, and what is not

Verified in the port, 2026-10-22 (PAL, `--level 1`, the run in §6):

- the item ids and names (§1), from the decompilation and
  `games/rac1/docs/kb/datamining.md`;
- the owned word, the ammo word and `D_00141760`, and the Triangle switch
  putting the Blaster and the Walloper out (§3.2, §6);
- `items[0].button_mask` being `0x80` for the wrench and `0x20` for the
  Blaster and the Walloper, the fire flag pulsing on Circle (§6), and the
  Walloper's hero state 0x20 on each punch (§6).

Not verified:

- the **cheat codes**: `games/rac1/docs/kb/datamining.md` §1.4 is a static
  derivation and says so. §3.4's warning about the owned flag follows from
  the code (`mobyutil_00258BC8.c:2899`), not from a played run;
- the **I-5 debug menu** in the port (§3.5);
- a shot actually leaving the gun: the input reaches the weapon and the flag
  pulses (§6), and the Walloper's punch state is the punch (§6), but the
  Blaster's ammo did not move — so each weapon's *effect* (its projectile,
  blast, beam, sound) still needs its own run, watching the ammo or the
  thing the weapon puts in the world;
- the ammo limits' values: which items have one is §1's list, but how many
  rounds each holds (`D_L00_001C43B0[id].max`, +0x0E) was not read back;
- the thrown weapons (Bomb Glove, Mine Glove, Decoy Glove, Glove of Doom,
  Visibomb) may want a longer Circle hold or a press and release rather than
  the 5-frame press that the Walloper answered.

## 8. Where the code is

| What | Where |
|---|---|
| `GiveItem` | `games/rac1/src/overlays/shared/mobyutil_00258BC8.c:2884` |
| the vendor's buy | `games/rac1/src/overlays/shared/vendor_002A5218.c:1458-1471` |
| the vendor's prices | `games/rac1/src/game/vendor.c:610-700` |
| the hero's item slots | `games/rac1/include/hero.h:88-110`, `:637`, `:752` |
| creating the item moby | `games/rac1/src/overlays/shared/help_0020CDF0.c:898-940` |
| the Triangle switch | `games/rac1/src/overlays/l01_novalis/help_00243078.c:164-181` |
| the quick select, pause page | `games/rac1/src/game/pause.c:3028-3060` |
| the quick select, in-game wheel | `games/rac1/src/overlays/shared/hud_00235960.c:1100-1160` |
| ammo take/add/get | `games/rac1/src/overlays/shared/help_00232560.c:1072-1111` |
| the I-5 menu's AMMO page | `games/rac1/src/overlays/shared/draw_001F3A78.c:646-679` |
| the button-code system | `games/rac1/docs/kb/datamining.md` §1; the matcher, `games/rac1/src/overlays/shared/pause_00277208.c:1186` |
| `OPENRAC_PRESS`, `OPENRAC_WRITE` | `port/game/common/frontend.cpp:3082`, `port/game/common/main.cpp:613` |
