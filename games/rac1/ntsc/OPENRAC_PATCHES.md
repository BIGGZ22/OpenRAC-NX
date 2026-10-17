# OpenRAC's patches to the NTSC decomp

OpenRAC builds the native NTSC-U port (`port/game/rac1-ntsc`) from this copy
of the NTSC decomp. Where the decomp's C relies on something the PS2 build
never sees (a value left in a register by the caller, a name the linker
resolves by address), the C is made explicit here so the port's translation
(hostgen) reads what the console ran. Each patch is meant to compile to the
same PS2 instructions; each was checked against the function's assembly
(`config/us/overlays/asm/` or `config/us/expected/asm/`), not by a rebuild
with the matching toolchain. They are to be offered upstream, and kept when
the decomp is synced again (`tools/sources.py sync rac1/ntsc`).

| File | Function | What | Why |
|---|---|---|---|
| `src/ui/help/get_help_message_text.c` | `get_help_message_text` (`FUN_001fdd10`) | the message id is a parameter, passed on to the index search | the C took no argument and relied on the id still in `a0`; translated, every message was message 0's (the title menu showed a weapon tip). Commit 0a6feb65 |
| `src/overlays/shared/world/loaders/load_level_gameplay.c` | `load_level_gameplay` (`FUN_L00_00241940`) | `func_0020D348_m` declared with `__asm__("FUN_0020c4f8")` | `func_0020D348_m` is the PAL decompilation's name for CreateMoby (PAL's address); the retail code calls `FUN_0020c4f8` here. Without the label the name means PAL's address, which NTSC-U has nothing at: the ship Ratchet arrives in was never created |
| `src/overlays/shared/gameplay/camera/init_camera_system.c` | `setup_level_cameras` (`FUN_L00_001ed358`) | `clear_vector` and `normalize_vector_xyz` declared with `__asm__("func_001F99F8")` and `__asm__("FUN_001f9bf8")` | the plain names are no symbol of this build; the retail code calls those two (`jal` at 0x1ED408 and 0x1ED428) |
| same file | `FUN_L00_001ed630` | takes `(a, b, c)` and passes `b`, `c` to `subtract_vector_xyz` | the C took one argument and called the subtraction with one; the retail code leaves the caller's `a1` and `a2` in place for it, and every caller passes them |
| `src/overlays/shared/gameplay/entities/vendor_orb.c` | `FUN_L00_002e48a0` | takes the scale as a second (float) parameter and passes it to the first `FUN_001f9a68` | the C dropped it; the retail code passes the caller's `$f12` on. Translated, the scale came out as 0 |
| `config/us/port_symbols.txt` (new) | 44 readable names | each name's address, for the port only (`port/game/rac1-ntsc/hostgen.json` "symbols") | files call `subtract_vector_xyz`, `clear_vector`, `fast_sin` and others by a plain name that other files give an `__asm__` label; without an address the port had no function for them (Kerwan's hero code called `subtract_vector_xyz` 1,700 times a run, doing nothing). The address is the one the labelled declarations agree on; the decomp's own build does not read the file |
| `src/math/random/random_float_between.c` | `random_float_between` (`FUN_002132a8`) | the unused first parameter (`s32 arg0`) is gone | no caller passes it and the function never reads it; translated, every call (50 call sites) was reported as short of an argument and given a 0 |
| `src/math/random/random_angle_radians.c` | `random_angle_radians` (`FUN_00213308`) | the unused parameter (`s32 arg0`) is gone | as above (13 call sites) |
| `src/rendering/refresh_point_light.c` | `refresh_point_light` (`FUN_00201f58`) | `detach_point_light` and `create_point_light` declared with their `s32` parameter, and the light index passed to the first | the C called `detach_point_light()` with nothing and relied on `a0` still holding the index (the retail code moves it to `s0` only for the second call, in the first `jal`'s delay slot). The call goes through each level's own copy (a folded wrapper), so translated it went through a K&R pointer with no argument: on Aridia (level 2) the light code read a host address as a game one and the program stopped |
| `src/audio/music/process_bgm_display_text_event.c` | `FUN_001f6638` (scaled text) | the TEX0 parameter is `s64`, and passed on to `FUN_001f5808` as the `u64` that function takes | the C kept the texture word in an `s32`; on the console the register holds all 64 bits (the routine only moves it on with `daddu`, which the `s64` emits too: `build/overlay-asm/level_18/split/f002042D8/FUN_001f6638.s`). Translated, the word lost its high half and came back sign-extended: the menus' titles and labels (the large and scaled fonts) were drawn as grey blocks |
