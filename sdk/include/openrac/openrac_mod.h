/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The interface between OpenRAC's native port and a native mod (docs/port/MODS.md).
 *
 * A mod is a shared library (.dylib, .so, .dll) that exports openrac_mod_query and openrac_mod_load,
 * and optionally openrac_mod_unload. The port opens it at start, before the game runs, asks
 * openrac_mod_query which interface it was built for, and calls openrac_mod_load with the table of
 * what the port offers (openrac_api). The mod uses nothing of the port but that table: it is built
 * against this header alone, and links against nothing of OpenRAC's.
 *
 * Versions. OPENRAC_MOD_API_MAJOR changes when anything in the table changes or goes: a mod built
 * for another major is refused. OPENRAC_MOD_API_MINOR grows when something is added at the end of
 * the table: a mod built for a minor above the port's is refused, one built for a lower minor loads
 * (and checks api->api_minor before it uses anything newer than what it was built for).
 *
 * Everything runs on the game's thread: load, the hooks, the callbacks, unload.
 *
 * This header is plain C (C99, and C++ through its extern "C" block): it is the ABI, and the C++
 * layer (openrac/mod.hpp) is built on it. It is laid out as: versions, helper macros, types,
 * the input state, the table (openrac_api), the entry points, then the helpers a mod uses.
 * Each item says which interface version added it ("Since 1.0" is the original table); a mod
 * checks api->api_minor before it uses anything newer than the minor it was built for.
 */
#ifndef OPENRAC_MOD_H
#define OPENRAC_MOD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------ *
 * Versions
 * ------------------------------------------------------------------------------------------ */

/* The interface's major version. Changes when anything in the table changes or goes. */
#define OPENRAC_MOD_API_MAJOR 1

/* The interface's minor version, which this header describes. It grows when something is added at
 * the end of the table. 1.1 added script mods (the table is unchanged), 1.2 the keyboard and mouse,
 * 1.3 the pointer over the game's picture (openrac_input.pointer_*). */
#define OPENRAC_MOD_API_MINOR 3

/* The interface a mod is built for, what its openrac_mod_query reports: its manifest's "api" minor,
 * which the SDK's openrac_add_mod passes in, else this header's. A mod for 1.1 built with a newer
 * SDK thus still loads on a port of 1.1 (and uses nothing newer without checking api->api_minor). */
#ifndef OPENRAC_MOD_TARGET_MINOR
#define OPENRAC_MOD_TARGET_MINOR OPENRAC_MOD_API_MINOR
#endif

/* Marks an entry point of the mod's library as exported, whatever the build's default visibility.
 * Since 1.0. */
#if defined(_WIN32)
#define OPENRAC_MOD_EXPORT __declspec(dllexport)
#else
#define OPENRAC_MOD_EXPORT __attribute__((visibility("default")))
#endif

/* ------------------------------------------------------------------------------------------ *
 * Basic types and constants
 * ------------------------------------------------------------------------------------------ */

/* A game address: an offset into game memory (the console's address). Since 1.0. */
typedef uint32_t openrac_gaddr;

/* The port's handle for one loaded mod, passed back to the functions that are the mod's own (its
 * log lines, its settings, its hooks). Opaque; valid from openrac_mod_load until the program ends.
 * Since 1.0. */
typedef struct openrac_mod openrac_mod;

/* The levels of openrac_api.log, from the most to the least talkative. Since 1.0. */
#define OPENRAC_LOG_DEBUG 0
#define OPENRAC_LOG_INFO 1
#define OPENRAC_LOG_WARN 2
#define OPENRAC_LOG_ERROR 3

/* The results of openrac_api.hook. Since 1.0. */
#define OPENRAC_HOOK_OK 0
/* This port has no hook point of that name (on this disc). */
#define OPENRAC_HOOK_UNKNOWN (-1)
/* The hook point's signature is another than the one asked for. */
#define OPENRAC_HOOK_SIGNATURE (-2)
/* Hooks are taken in openrac_mod_load only. */
#define OPENRAC_HOOK_TOO_LATE (-3)

/* ------------------------------------------------------------------------------------------ *
 * Reporting to the port
 * ------------------------------------------------------------------------------------------ */

/* What openrac_mod_query tells the port before it loads the mod. The port sets `size`; the mod
 * sets the rest (OPENRAC_MOD_DEFINE does). Since 1.0. */
typedef struct openrac_mod_info {
    /* sizeof(openrac_mod_info), set by the port. */
    uint32_t size;
    /* OPENRAC_MOD_API_MAJOR as the mod was built. */
    uint32_t api_major;
    /* OPENRAC_MOD_TARGET_MINOR as the mod was built. */
    uint32_t api_minor;
    /* The manifest's id: the port checks they agree. Static storage in the mod. */
    const char* id;
    /* The mod's version, as its manifest's. Static storage in the mod. */
    const char* version;
} openrac_mod_info;

/* ------------------------------------------------------------------------------------------ *
 * The keyboard and mouse (since 1.2)
 * ------------------------------------------------------------------------------------------ */

/* The number of entries of openrac_input.keys. A key is its USB HID usage, the same on every layout
 * and system (openrac/keys.h names them: "KeyW", "Space", "ShiftLeft"). Since 1.2. */
#define OPENRAC_KEY_COUNT 512

/* The key is down: a bit of an openrac_input.keys[k] entry. Since 1.2. */
#define OPENRAC_KEY_HELD 1u
/* The key went down since the last tick (a tap shorter than a tick too): a bit of an
 * openrac_input.keys[k] entry. Since 1.2. */
#define OPENRAC_KEY_PRESSED 2u

/* The bits of openrac_input.mouse_held and mouse_pressed. Since 1.2. */
#define OPENRAC_MOUSE_LEFT 1u
#define OPENRAC_MOUSE_MIDDLE 2u
#define OPENRAC_MOUSE_RIGHT 4u
/* The side button nearer the wrist (X1). */
#define OPENRAC_MOUSE_BACK 8u
/* The other side button (X2). */
#define OPENRAC_MOUSE_FORWARD 16u

/* A device of openrac_api.take_input: the port's own keyboard table stops driving pad 0. Since
 * 1.2. */
#define OPENRAC_INPUT_KEYBOARD 1u
/* A device of openrac_api.take_input: the mod uses the mouse in play. Reserved: the port has no
 * mouse use of its own in play. Since 1.2. */
#define OPENRAC_INPUT_MOUSE 2u

/* The keyboard and mouse as the port read them for the tick about to run: taken once a tick, after
 * the window's events and before the game reads its pads (so before the tick's pad reports).
 * Read it through openrac_api.input; it is the port's, and valid until the next tick.
 *
 * Since 1.2; the fields from `pointer_x` on are 1.3's, and exist only when `size` reaches them. */
typedef struct openrac_input {
    /* sizeof(openrac_input) as the port was built: a newer port's is larger. */
    uint32_t size;
    /* Counts the ticks: a new value, a new state. */
    uint32_t tick;
    /* By HID usage: OPENRAC_KEY_HELD | OPENRAC_KEY_PRESSED. */
    uint8_t keys[OPENRAC_KEY_COUNT];
    /* The OPENRAC_MOUSE_* buttons down now. */
    uint32_t mouse_held;
    /* The OPENRAC_MOUSE_* buttons that went down since the last tick. */
    uint32_t mouse_pressed;
    /* The mouse's motion since the last tick, in the system's pixels: raw (no pointer acceleration)
     * while captured; y grows downward. */
    float mouse_dx;
    float mouse_dy;
    /* The wheel since the last tick, in notches: x > 0 to the right, y > 0 away from the player
     * (up), whatever the system's scroll direction. */
    float wheel_x;
    float wheel_y;
    /* 1 when the game's window has the keyboard; while not, nothing is held. */
    uint8_t focused;
    /* 1 when the cursor is captured: hidden, kept in the window, motion relative. */
    uint8_t captured;
    /* 1 when the port's own overlay or menu has the input now (F3): leave it alone. */
    uint8_t overlay;
    /* Padding; zero. */
    uint8_t reserved;
    /* Since 1.3 (check `size` first): where the cursor is over the game's 4:3 picture, 0..1 across
     * and down (outside it below 0 or above 1), as the game's 2D screen of 512 by 416 pixels has
     * it: x * 512, y * 416 is the game's pixel. Meaningful while the cursor is not captured. */
    float pointer_x;
    float pointer_y;
    /* Since 1.3: 1 when the cursor is over the picture, in the focused window. */
    uint8_t pointer_inside;
    /* Padding; zero. */
    uint8_t reserved2[3];
} openrac_input;

/* ------------------------------------------------------------------------------------------ *
 * Callbacks
 * ------------------------------------------------------------------------------------------ */

/* A function the port calls after each game tick (openrac_api.on_frame), with the `user` pointer
 * the mod gave. Since 1.0. */
typedef void (*openrac_frame_fn)(void* user);

/* A function the port calls for each pad report (openrac_api.on_pad), with the `user` pointer the
 * mod gave. `report` is the report as the console's pad library hands it to the game (its bytes,
 * `size` of them), which the callback may change before the game reads it; `port` is the
 * controller's port, 0 or 1. Since 1.0. */
typedef void (*openrac_pad_fn)(void* user, int port, uint8_t* report, int size);

/* ------------------------------------------------------------------------------------------ *
 * The table the port offers
 * ------------------------------------------------------------------------------------------ */

/* What the port offers a mod. Filled by the port; valid for the life of the program. Every function
 * is called on the game's thread. Members from "1.2" on are at the end: a mod built for 1.1 never
 * sees them, and one built for 1.2 loads on a 1.2 port or newer. */
typedef struct openrac_api {
    /* sizeof(openrac_api) as the port was built: a newer port's is larger. Since 1.0. */
    uint32_t size;
    /* The port's interface version: at least what the mod was built for, else it is not loaded.
     * Since 1.0. */
    uint16_t api_major;
    uint16_t api_minor;

    /* The game's id, "rac1" (games/<game>/game.json). Since 1.0. */
    const char* game;
    /* The disc's version, "pal" or "ntsc". Since 1.0. */
    const char* version;
    /* OpenRAC's version, "0.9.0". Since 1.0. */
    const char* openrac_version;
    /* The platform, "macos-arm64", "linux-x86_64", "windows-x86_64", ... Since 1.0. */
    const char* platform;

    /* The base of game memory: game address a is memory + a. Fixed for the life of the program.
     * Since 1.0. */
    uint8_t* memory;

    /* Writes a line to the session's log, after the mod's id. `level` is an OPENRAC_LOG_* value.
     * Since 1.0. */
    void (*log)(openrac_mod* mod, int level, const char* text);

    /* Says why the mod refuses to load (status.json and the launcher show it). Only during
     * openrac_mod_load, which then returns non-zero. Since 1.0. */
    void (*fail)(openrac_mod* mod, const char* reason);

    /* Read the mod's settings (its manifest's "settings"; the values the launcher wrote). A key
     * the manifest does not declare, or of another type, gives 0, 0.0 or "". A string stays valid
     * for the life of the program. Since 1.0. */
    int (*setting_bool)(openrac_mod* mod, const char* key);
    int64_t (*setting_int)(openrac_mod* mod, const char* key);
    double (*setting_float)(openrac_mod* mod, const char* key);
    const char* (*setting_string)(openrac_mod* mod, const char* key);

    /* Takes the hook point `name` (games/<game>.h: its name and signature) for `replacement`, a
     * function of the hook point's signature, which `signature` spells. Sets `*original` to what
     * the replacement goes on to: the game's own function, or the replacement of a mod loaded
     * earlier, so the mod loaded last runs first. Returns OPENRAC_HOOK_OK or one of the other
     * OPENRAC_HOOK_* results. Only during openrac_mod_load. Since 1.0. */
    int (*hook)(
        openrac_mod* mod,
        const char* name,
        const char* signature,
        void* replacement,
        void** original
    );

    /* Calls `fn(user)` after each game tick, in load order. Returns 0, or non-zero when refused
     * (after openrac_mod_load). Only during openrac_mod_load. Since 1.0. */
    int (*on_frame)(openrac_mod* mod, openrac_frame_fn fn, void* user);

    /* Calls `fn(user, port, report, size)` for each pad report, before the game reads it, in load
     * order. Returns 0, or non-zero when refused (after openrac_mod_load). Only during
     * openrac_mod_load. Since 1.0. */
    int (*on_pad)(openrac_mod* mod, openrac_pad_fn fn, void* user);

    /* Returns a game function by its name in this disc's build ("func_001F9F90" on PAL,
     * "FUN_001f9dc8" on NTSC-U), as a host function to call with its signature; null when there is
     * none. Since 1.0. */
    void* (*function)(const char* name);

    /* Returns the function at a code address, in the level loaded now (a function pointer the game
     * keeps); null when there is none. Since 1.0. */
    void* (*function_at)(openrac_gaddr address);

    /* Return a global's address for the code loaded now: one of the executable's (guest.h,
     * OPENRAC_DATA), or of level `level`'s program (OPENRAC_LDATA). Ask each time: a level moves
     * its globals. Since 1.0. */
    openrac_gaddr (*relocate_data)(openrac_gaddr address);
    openrac_gaddr (*relocate_level_data)(int level, openrac_gaddr address);

    /* Returns the level whose program is loaded, -1 for none (the executable's). Since 1.0. */
    int (*loaded_level)(void);

    /* Gives `size` bytes (aligned to 16) of room on the game's stack for a call that takes the
     * address of a local, and keeps the stack as it was in `*saved`. Returns the room's address.
     * Pair each push with a stack_pop, innermost first. Since 1.0. */
    openrac_gaddr (*stack_push)(uint32_t size, openrac_gaddr* saved);

    /* Gives back what the matching stack_push took: `saved` is what it stored. Since 1.0. */
    void (*stack_pop)(openrac_gaddr saved);

    /* Returns the keyboard and mouse for the tick about to run. Never null: without a window (a
     * test, a server) nothing is held and nothing moves. Valid until the next tick. Since 1.2. */
    const openrac_input* (*input)(void);

    /* Takes `devices` (OPENRAC_INPUT_*) from the port: with OPENRAC_INPUT_KEYBOARD the port's own
     * keyboard table (WASD, Space, ...) no longer presses pad 0's buttons, and the mod presses
     * them through on_pad instead. A gamepad goes on as before; the port's window keys (F3, F11)
     * stay the port's. Returns 0, or OPENRAC_HOOK_TOO_LATE after openrac_mod_load. Since 1.2. */
    int (*take_input)(openrac_mod* mod, uint32_t devices);

    /* Asks for the cursor captured (`capture` 1: hidden, held in the window, motion relative, as
     * a game's mouse look wants it) or let go (0). Any time; the latest asking of any mod counts.
     * The port lets it go anyway while the window is not focused or its overlay is open, and takes
     * it back when the reason is gone and the mod still asks. Since 1.2. */
    void (*capture_mouse)(openrac_mod* mod, int capture);
} openrac_api;

/* ------------------------------------------------------------------------------------------ *
 * The entry points a mod exports
 * ------------------------------------------------------------------------------------------ */

/* Tells the port which interface the mod was built for: sets `info`'s api_major, api_minor, id and
 * version. Called before the mod is loaded. Required; OPENRAC_MOD_DEFINE writes it. Since 1.0. */
typedef void (*openrac_mod_query_fn)(openrac_mod_info* info);

/* Loads the mod: takes its hook points and callbacks from `api`, and keeps `mod` for the calls
 * that are its own. Returns 0 to load, non-zero to refuse (after openrac_api.fail gave the
 * reason). Required. Since 1.0. */
typedef int (*openrac_mod_load_fn)(const openrac_api* api, openrac_mod* mod);

/* Lets go of what the mod holds, when the program ends. Optional. Since 1.0. */
typedef void (*openrac_mod_unload_fn)(void);

/* The names the entry points are exported under, for the port's lookup. Since 1.0. */
#define OPENRAC_MOD_QUERY "openrac_mod_query"
#define OPENRAC_MOD_LOAD "openrac_mod_load"
#define OPENRAC_MOD_UNLOAD "openrac_mod_unload"

/* ------------------------------------------------------------------------------------------ *
 * Helpers for a mod
 * ------------------------------------------------------------------------------------------ */

/* The host pointer to a game address's bytes. `api` is the openrac_api, `a` a game address.
 * Since 1.0. */
#define OPENRAC_G(api, a) ((void*)((api)->memory + (openrac_gaddr)(a)))

/* A typed value at a game address, as an lvalue: OPENRAC_GREF(api, int32_t, a) = 5. The address
 * must be aligned for `T`. Since 1.0. */
#define OPENRAC_GREF(api, T, a) (*(T*)OPENRAC_G(api, a))

/* Writes the usual openrac_mod_query: OPENRAC_MOD_DEFINE("lock-strafe", "1.0.0"). `mod_id` and
 * `mod_version` are string literals, the manifest's. Since 1.0. */
#define OPENRAC_MOD_DEFINE(mod_id, mod_version)                                                    \
    OPENRAC_MOD_EXPORT void openrac_mod_query(openrac_mod_info* info) {                            \
        info->api_major = OPENRAC_MOD_API_MAJOR;                                                   \
        info->api_minor = OPENRAC_MOD_TARGET_MINOR;                                                \
        info->id = (mod_id);                                                                       \
        info->version = (mod_version);                                                             \
    }

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_MOD_H */
