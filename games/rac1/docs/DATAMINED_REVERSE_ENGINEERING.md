# Datamined Reverse Engineering Intelligence

This document is automatically synthesized from headless Ghidra PS2 EmotionEngine analysis
and retail executable data mining (`SCES_509.16` + overlays).

## 1. Discovered Source Code Hierarchy

The following original C/C++ source translation units were identified through string tables, allocators, and asserts:

| Source File | Identifying Address | Evidence & Subsystem |
|---|---|---|
| `hud.cpp` | `0x0015F7B8` | Heap allocations at line 277 (`0x115`), UI HUD rendering |
| `loaders.cpp` | `0x0015FC70` | Heap allocations at line 571 (`0x23B`), Level loading subsystem |
| `map.cpp` | `0x0015FE18` | In-game minimap & world map render state |
| `camera.c` | `0x001E7A50` | `Camera_CollPrimTest` warning and collision boundary checks |
| `/usr/local/989snd/ee/989snd.c` | `0x00153D78` | Sony 989snd audio system RPC interface |

## 2. Recovered Subsystem Enums

### Memory Card State Machine (`CardState` / `CS_`)
Discovered from pointer table `D_001A04C0` and state strings at `0x0015FE78` and `0x001E83F0`:

```c
typedef enum {
    CS_INIT = 0,
    CS_GOOD_SAVE = 1,
    CS_WARNING = 2,
    CS_NOCARD = 3,
    CS_WAIT_FOR_CARD = 4,
    CS_UNFORMATTED = 5,
    CS_PROMPT_FORMAT = 6,
    CS_FORMAT_PENDING = 7,
    CS_FORMATTING = 8,
    CS_FORMATTED = 9,
    CS_CHECK_SAVE = 10,
    CS_CHECKING_SAVE = 11,
    CS_NOSAVE = 12,
    CS_PROMPT_CREATE_SAVE = 13,
    CS_CREATE_SAVE_PENDING = 14,
    CS_CREATING_SAVE = 15,
    CS_NEWCARD = 16,
    CS_FORMAT_FAILED = 17,
    CS_CREATE_FAILED = 18,
    CS_NO_ROOM = 19,
    CS_LOAD_FAILED = 20,
    CS_SAVE_FAILED = 21,
    CS_SAVING = 22,
    CS_PROMPT_BEGIN_UNFORMATTED = 23,
    CS_PROMPT_BEGIN_NOSAVE = 24
} CardState;
```

### Save Game IFF Chunk Architecture (`save.h` / `memcard.c`)
Recovered from `memcard_PrepData` (`func_0020BBC8`) and save stream chunk parsers:
- **Global Save (`game`)**:
  - `0`: `SAVE_BLOCK_LEVEL` (`int`)
  - `1`: `SAVE_BLOCK_BOLT_COUNT` (`int`)
  - `2`: `SAVE_BLOCK_GAME_COMPLETES` (`int`, playthroughs / challenge mode)
  - `3`: `SAVE_BLOCK_ELAPSED_TIME` (`int`)
  - `4`: `SAVE_BLOCK_LAST_SAVE_TIME` (`sceCdCLOCK`)
  - `5`: `SAVE_BLOCK_GLOBAL_FLAGS` (progression bitmask)
  - `7`: `SAVE_BLOCK_CHEATS_ACTIVATED`
  - `8`: `SAVE_BLOCK_SKILL_POINTS`
  - `9`: `SAVE_BLOCK_AMMO`
  - `10`: `SAVE_BLOCK_UNLOCKS` (weapon & gadget possession bitmask)
  - `12`: `SAVE_BLOCK_PURCHASABLE_VENDOR_ITEMS`
  - `14`: `SAVE_BLOCK_GALACTIC_MAP`
  - `25`: `SAVE_BLOCK_CAMERA_UP_DOWN_MODE` (invert pitch)
  - `26`: `SAVE_BLOCK_CAMERA_LEFT_RIGHT_MODE` (invert yaw)
  - `27`: `SAVE_BLOCK_CAMERA_ROTATION_SPEED` (sensitivity)
  - `37`: `SAVE_BLOCK_CHEATS_EVER_ACTIVATED`
  - `1003`: `SAVE_BLOCK_TOTAL_PLAY_TIME`
  - `1005`: `SAVE_BLOCK_TOTAL_DEATHS`
- **Level Save (`level`)**:
  - `3001`: `SAVE_LEVEL_BLOCK_VISITED` (0=unvisited, 1=visited, 2=completed)
  - `3003`: `SAVE_LEVEL_BLOCK_GOLD_BOLTS`
  - `3004`: `SAVE_LEVEL_BLOCK_SEGMENTS_COMPLETED`
  - `4000`: `SAVE_LEVEL_BLOCK_TOTAL_BOLTS`
  - `4002`: `SAVE_LEVEL_BLOCK_TOTAL_DEATHS`

### RaC1 Gadget Inventory Enum (`RaC1GadgetId`)
Complete 29-element inventory sequence (IDs 0 to 28) verified across weapon dispatch and save importing:
`BOMB_GLOVE` (0), `PYROCITOR` (1), `BLASTER` (2), `GLOVE_OF_DOOM` (3), `SUCK_CANNON` (4), `SWINGSHOT` (5), `HYDRODISPLACER` (6), `SONIC_SUMMONER` (7), `RYNO` (8), `WALLOPER` (9), `VISIBOMB` (10), `DECOY_GLOVE` (11), `TESLA_CLAW` (12), `TAUNTER` (13), `TRESPASSER` (14), `METAL_DETECTOR` (15), `MAGNEBOOTS` (16), `GRIND_BOOTS` (17), `HOVERBOARD` (18), `HELI_PACK` (19), `THRUSTER_PACK` (20), `HYDRO_PACK` (21), `O2_MASK` (22), `PILOTS_HELMET` (23), `MORPH_O_RAY` (24), `CODEBOT` (25), `HOLOGUISE` (26), `PDA` (27), `PERSUADER` (28).

### Authentic `MobyInstance` Entity Structure (256 B / 0x100)
Recovered from unstripped `.mdebug` STABS types:
- `0x00`: `BSphere bSphere` (world bounding sphere `x, y, z, rad`)
- `0x10`: `vec4 pos` (world position `x, y, z, w`)
- `0x20`: `u8 state` (`0xFE` = free slot, `0xFF` = array tail)
- `0x21`: `u8 group` (collision / grouping index)
- `0x22`: `u8 mClass` (moby class sub-type)
- `0x23`: `u8 alpha` (opacity / blend alpha, default `0x80`)
- `0x24`: `void *pClass` (pointer to class descriptor)
- `0x28`: `struct MobyInstance *pChain` (next in update chain)
- `0x34`: `u16 modeBits` (behavioral mode flags, bit `0x40` = no pre-update)
- `0x36`: `u16 modeBits2` (secondary flags, default `0x7F80`)
- `0x38`: `u64 lights` (lighting bitmask `0x40404000000000L`)
- `0x40`: `void *animSeq` / `f32 animSeqT` / `f32 animSpeed`
- `0x80`: `BSphere lSphere` (local bounding sphere)
- `0xA8`: `void (*pUpdate)(struct MobyInstance *)` (per-tick update callback)
- `0xAC`: `void *pVar` (moby private variables, 0x80 bytes)
- `0xB2`: `s16 UID` (unique actor ID in loaded level)
- `0xBC`: `s16 oClass` (Object Class ID)
- `0xC0`: `float rMtx[3][4]` (3x4 orientation matrix)
- `0xF0`: `vec4 rot` (Euler angles Pitch, Yaw, Roll)

## 3. High-Value Hub Functions (Most Called)

Functions called by the largest number of callers across the game code:

| Function | Callers | Inferred Identity / Role |
|---|---|---|
| `func_00234C98` (0x00234C98) | 48 |  |
| `FlushCache` (0x00118D80) | 42 |  |
| `func_0011B4C8` (0x0011B4C8) | 41 |  |
| `SignalSema` (0x00118C90) | 39 |  |
| `func_0011D9A8` (0x0011D9A8) | 36 |  |
| `func_0011D960` (0x0011D960) | 35 |  |
| `func_001F4630` (0x001F4630) | 34 |  |
| `func_001F4748` (0x001F4748) | 34 |  |
| `func_001FE540` (0x001FE540) | 33 | Paradox: This message does not exist |
| `func_001F98C0` (0x001F98C0) | 32 |  |
| `func_001FA888` (0x001FA888) | 32 |  |
| `func_001F4868` (0x001F4868) | 29 |  |
| `func_001FA898` (0x001FA898) | 28 |  |
| `func_001F9C30` (0x001F9C30) | 27 |  |
| `func_0012E820` (0x0012E820) | 24 | 989snd.c: RPC collision!
; snd_SendIOPCommandNoWait: BUFFER  |

## 4. Functions with Rich Diagnostic Strings

These functions contain direct assertion, error, or debug logs that reveal their original implementation:

| Function | String Address | Log / Error String |
|---|---|---|
| `func_001194C8` | `0x001527E8` | `'## internel error in libkernl.a!'` |
| `func_00119910` | `0x00152838` | `'TTY: receive error'` |
| `func_00120D28` | `0x00152FB0` | `'Ncmd fail sema cur_cmd:%d keep_cmd:%d'` |
| `func_00121040` | `0x00153010` | `'Scmd fail sema cur_cmd:%d keep_cmd:%d'` |
| `func_001236F0` | `0x00153550` | `'bind error libmc'` |
| `func_00124650` | `0x001535D8` | `'libdbc: bind failed'` |
| `func_001247E8` | `0x00153658` | `'sceDbcSetWorkAddr: rpc error'` |
| `func_00124858` | `0x00153678` | `'sceDbcCreateSocket: rpc error'` |
| `func_00124920` | `0x001536B8` | `'sceDbcGetDepNumber: rpc error'` |
| `func_00124A70` | `0x001537A0` | `'sceDbcReceiveData: rpc error'` |
| `func_001273A0` | `0x00153888` | `'Error code detected(BDEC)'` |
| `func_00127960` | `0x00153928` | `'_sliceA0(): error happens'` |
| `func_0012A558` | `0x00153AB0` | `'CSC handler error'` |
| `func_0012C420` | `0x00153BD8` | `'[MPEG ERROR]%s'` |
| `func_0012D2A0` | `0x00153D28` | `"Can't read rom error"` |
| `func_0012DB68` | `0x00153D50` | `'error: sceSifBindRpc in %s, at line %d'` |
| `func_0012DFB0` | `0x00153D98` | `"989snd.c: Sif says RPC isn't busy, but we still don't have r"` |
| `func_0012E060` | `0x00153E20` | `'989snd.c: RPC collision!'` |
| `func_0012E1C8` | `0x00153E20` | `'989snd.c: RPC collision!'` |
| `func_0012E688` | `0x00153E20` | `'989snd.c: RPC collision!'` |
| `func_0012E820` | `0x00153E20` | `'989snd.c: RPC collision!'` |
| `func_0012EB18` | `0x00153E20` | `'989snd.c: RPC collision!'` |
| `func_00208AB0` | `0x0015FE68` | `'error'` |
| `func_0020BAA8` | `0x001E8690` | `'ERROR: could not init memcard lib'` |
| `func_00217628` | `0x001E8980` | `'****Load file failed to start!****'` |
| `func_00235218` | `0x001E8D38` | `'DMAC(15) - Bus Error'` |
| `func_0023BF48` | `0x001612F8` | `'[ Error ] %s'` |
| `func_0023E298` | `0x001E8E80` | `'sceMpegGetPicture() decode error'` |

## 5. Usage in Decompilation Workflow

1. **`python3 tools/dossier.py <func>`**: Automatically references `config/strings.json` and displays all strings.
2. **`config/ghidra_callgraph.json`**: Inspect upstream callers and downstream callees when typing struct pointers.
3. **`config/ghidra_functions.json`**: Query signatures and stack bounds for any function.
