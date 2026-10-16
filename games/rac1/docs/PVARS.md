# Moby Private Variables (pvars)

A reference guide for moby instance private variables (`pvars`) in *Ratchet &
Clank* (PS2, 2002), how the engine manages object-specific state, and the
reverse-engineered class layouts adapted from **Wrench**.

The matching C declarations are in [`include/moby_pvars.h`](../include/moby_pvars.h).

---

## 1. Engine Architecture

Every interactive actor or entity in the game — Ratchet, Clank, NPCs, enemies,
hazards, moving platforms, projectiles, and collectables — is represented by a
[`MobyInstance`](../include/structs.h) record in memory.

While all mobys share common header fields (position, rotation matrix, bounding
sphere, animation sequence, class ID, update function pointer), each class has
unique private state:

```c
typedef struct MobyInstance {
    /* 0x00 */ BSphere       bSphere;              /* bounding sphere in world space */
    /* 0x10 */ vec4          pos;                  /* world position (x, y, z, w) */
    /* 0x20 */ unsigned char state;                /* current moby state */
    ...
    /* 0xA8 */ void        (*pUpdate)(struct MobyInstance *); /* per-frame tick */
    /* 0xAC */ void         *pVar;                 /* private moby state (pvars) */
    ...
    /* 0xBC */ short         oClass;               /* Object Class ID */
    ...
} MobyInstance;
```

When a moby's update function (`pUpdate`) executes, it casts `moby->pVar` to its
class-specific structure:

```c
void func_L01_NovalisLift_Update(MobyInstance *m) {
    PVar_Moby726_NovalisLift *v = (PVar_Moby726_NovalisLift *)m->pVar;
    
    if (v->movePath != 0) {
        /* integrate along spline path using v->moveSpeed */
    }
}
```

### Why exact pvars layouts matter for decompilation

For matching decompilation under the SN Systems ProDG compiler (GCC 2.95.3),
struct layouts dictate the exact machine instructions generated:
- `lwc1` / `swc1` for `float`
- `lh` / `lhu` for `s16` / `u16`
- `lb` / `lbu` for `s8` / `u8`
- Struct alignment, padding, and member order determine register allocation
  and pointer offset math. Having exact struct definitions allows candidates to
  compile directly to matching machine code without ad-hoc pointer casts.

---

## 2. Common Subsystems

Many moby classes share common sub-structures embedded at the start of their
`pVar` block:

### NPC Dialogue State Machine (`npcVars`, `npcStep`, `npcstring`)
Interactive NPCs (the Plumber, Skid McMarx, Big Al, Edwina, etc.) share a
standard dialogue and cutscene progression system:

- **`npcstring`** (`0x14` bytes): Array of 5 pointers to text strings, one for
  each PAL language: English, French, German, Spanish, Italian.
- **`npcStep`** (`0x1C` bytes): A single dialogue state step:
  - `offer`: Bolt cost for goods or services (e.g. Infobots, upgrades).
  - `scene`: Cutscene ID to trigger (`-1` if none).
  - `dest`: Next dialogue step ID upon completion.
  - `cond_type` / `cond_val`: Prerequisites (checks missions, items, or bolts).
  - `true_dest` / `false_dest`: Branching destinations based on condition evaluation.
- **`npcVars`** (`0x40` bytes): Runtime tracking:
  - `type`, `msg`: Current interaction step and message index.
  - `autoTalk`: Flag determining whether NPC engages automatically on proximity.
  - `prevStep`, `prevMsg`: History for resuming interrupted dialogue.

### Head-Tracking & IK (`Tweaker`, `Manipulator`)
NPCs and complex props look at the player as Ratchet walks past:
- **`Manipulator`** (`0x40` bytes): Controls bone transforms, interpolation
  timers, rotation quaternion, scale, and translation vectors.
- **`Tweaker`** (`0x80` bytes): Embeds a `Manipulator` along with target vector,
  angular speed, joint index, and target `MobyInstance` pointer (`pMoby`).

---

## 3. Reverse-Engineered Class Catalog

The following moby classes and pvars were reverse-engineered by **Wrench** and
adapted into [`include/moby_pvars.h`](../include/moby_pvars.h):

| Class ID | Friendly Name | Planet / Level | PVar Struct | Size | Description & Key Fields |
|---|---|---|---|---:|---|
| **90** | Bob | Pokitaru (L08) | `PVar_Moby90_Bob` | `0x190` | Robo-Trainer NPC (embeds `npcVars`) |
| **114** | Resort Owner | Pokitaru (L08) | `PVar_Moby114_ResortOwner` | `0x2E0` | Resort Owner NPC (`npcVars`, dialogue chains) |
| **282** | Miner | Novalis (L01) | `PVar_Moby282_Miner` | — | Novalis Waterworks Miner NPC |
| **298** | Fred | Pokitaru (L08) | `PVar_Moby298_Fred` | `0x190` | Pokitaru Fred NPC (`npcVars`) |
| **328** | Edwina | Kerwan (L03) | `PVar_Moby328_Edwina` | `0x190` | Robo-Workshop proprietor (`SubVars`, `npcVars`) |
| **613** | Water Current | Various | `PVar_Moby613_WaterCurrent` | `0x30` | Water flow stream (`pathlink flowPath`, `float speed`) |
| **726** | Novalis Lift | Novalis (L01) | `PVar_Moby726_NovalisLift` | `0xD0` | Elevator (`SubVars`, `moveSpeed`, `movePath`) |
| **750** | Infobot | Various | `PVar_Moby750_Infobot` | `0xD0` | Infobot pickup / cutscene trigger (`int levelId`) |
| **774** | Plumber | Novalis (L01) | `PVar_Moby774_Plumber` | `0x1A0` | Plumber NPC (`npcVars`, `mobylink infobot`) |
| **786** | Skid's Agent | Aridia (L02) | `PVar_Moby786_SkidsAgent` | `0x190` | Skid McMarx's Agent NPC (`npcVars`) |
| **788** | Skid McMarx | Aridia (L02) | `PVar_Moby788_SkidMcMarx` | `0x1B0` | Skid McMarx NPC (`npcVars`, hoverboard trigger) |
| **851** | Qwark (Oltanis) | Oltanis (L10) | `PVar_Moby851_QwarkOltanis` | `0x1A0` | Captain Qwark NPC vendor on Oltanis (`npcVars`) |
| **909** | Big Al | Kerwan (L03) | `PVar_Moby909_BigAl` | `0x1A0` | Big Al NPC on Kerwan (`npcVars`, Heli-pack upgrade) |
| **914** | Robot Qwark | Umbris (L09) | `PVar_Moby914_RobotQwark` | `0x50` | Holographic/Robot Qwark training guide (`npcVars`) |
| **918** | Hoverboard Girl | Rilgar (L04) | `PVar_Moby918_HoverboardGirl` | `0x280` | Race coordinator (`npcVars`, `Tweaker tweaks[4]`) |
| **924** | Sam | Batalia (L06) | `PVar_Moby924_Sam` | `0x1A0` | Scrap Merchant / Tool Vendor (`npcVars`) |
| **1111** | Ocean Surface | Pokitaru (L08) | `PVar_Moby1111_PokitaruOcean` | `0x30` | Water surface animation (`float surfaceZ`) |
| **1135** | Teleporter | Various | `PVar_Moby1135_Teleporter` | `0x50` | Teleporter pad (`mobylink destination`) |
| **1455** | Gadgetron CEO | Kalebo II (L14) | `PVar_Moby1455_GadgetronCEO` | `0x200` | Boardroom CEO hologram (`SubVars`, `npcVars`) |

---

## 4. How to Use in Level Decompilation

When decompiling functions in [`src/overlays/`](../src/overlays/):

1. Include the header:
   ```c
   #include "moby_pvars.h"
   ```
2. Identify the moby class handled by the function:
   - Check `tools/extract/moby_classes.tsv` for the class name and ID.
   - Check the level's class dispatch table in the overlay setup function.
3. Replace raw offset arithmetic (e.g. `*(int *)(m->pVar + 0x40)`) with struct
   member accesses.
4. Verify byte-matching:
   ```sh
   bash tools/docker/run.sh python3 tools/try_func.py func_LNN_XXXXXXXX candidate.c --diff
   ```

---

## 5. Sources and Attribution

The structures in this document and [`include/moby_pvars.h`](../include/moby_pvars.h)
are adapted from **[Wrench](https://github.com/chaoticgd/wrench)** by
**chaoticgd and contributors**, licensed under the **GNU General Public
License v3.0 or later (GPL-3.0-or-later)**.

Specific upstream files:
- `data/overlay/src/game_common/links.h`: link identifier types
- `data/overlay/src/game_common/npc.h`: NPC state machine (`npcVars`, `npcStep`, `npcstring`)
- `data/overlay/src/game_common/mobyfunc.h`: IK `Manipulator`
- `data/overlay/src/game_common/mobyutil.h`: head-tracking `Tweaker`
- `data/overlay/src/game_rac/mobyutil.h`: R&C 1 common moby sub-vars
- `data/overlay/src/game_rac/update/moby*.h`: level-specific moby update structs

See [`THIRD_PARTY_NOTICES.md`](../THIRD_PARTY_NOTICES.md) for full licensing terms.
