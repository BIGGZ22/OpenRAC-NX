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
 */
#ifndef OPENRAC_MOD_H
#define OPENRAC_MOD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENRAC_MOD_API_MAJOR 1
#define OPENRAC_MOD_API_MINOR                                                                      \
    3 /* 1.3: the pointer over the game's picture (openrac_input.pointer_*) */

/* The interface a mod is built for, what its openrac_mod_query reports: its manifest's "api" minor,
 * which the SDK's openrac_add_mod passes in, else this header's. A mod for 1.1 built with a newer
 * SDK thus still loads on a port of 1.1 (and uses nothing newer without checking api->api_minor). */
#ifndef OPENRAC_MOD_TARGET_MINOR
#define OPENRAC_MOD_TARGET_MINOR OPENRAC_MOD_API_MINOR
#endif

#if defined(_WIN32)
#define OPENRAC_MOD_EXPORT __declspec(dllexport)
#else
#define OPENRAC_MOD_EXPORT __attribute__((visibility("default")))
#endif

/* A game address: an offset into game memory (the console's address). */
typedef uint32_t openrac_gaddr;

/* The port's handle for one loaded mod, passed back to the functions that are the mod's own (its
 * log lines, its settings, its hooks). */
typedef struct openrac_mod openrac_mod;

/* Log levels for openrac_api.log. */
#define OPENRAC_LOG_DEBUG 0
#define OPENRAC_LOG_INFO 1
#define OPENRAC_LOG_WARN 2
#define OPENRAC_LOG_ERROR 3

/* openrac_api.hook's results. */
#define OPENRAC_HOOK_OK 0
#define OPENRAC_HOOK_UNKNOWN (-1)   /* this port has no hook point of that name (on this disc) */
#define OPENRAC_HOOK_SIGNATURE (-2) /* the hook point's signature is another */
#define OPENRAC_HOOK_TOO_LATE (-3)  /* hooks are taken in openrac_mod_load only */

/* What openrac_mod_query tells the port before it loads the mod. */
typedef struct openrac_mod_info {
    uint32_t size;       /* sizeof(openrac_mod_info), set by the port */
    uint32_t api_major;  /* OPENRAC_MOD_API_MAJOR as the mod was built */
    uint32_t api_minor;  /* OPENRAC_MOD_API_MINOR as the mod was built */
    const char* id;      /* the manifest's id: the port checks they agree */
    const char* version; /* the mod's version, as its manifest's */
} openrac_mod_info;

/* 1.2: the keyboard and mouse. A key is its USB HID usage, the same on every layout and system
 * (openrac/keys.h names them: "KeyW", "Space", "ShiftLeft"). */
#define OPENRAC_KEY_COUNT 512
#define OPENRAC_KEY_HELD 1u /* openrac_input.keys[k]: the key is down */
#define OPENRAC_KEY_PRESSED                                                                        \
    2u /* it went down since the last tick (a tap shorter than a tick too) */

/* openrac_input.mouse_held and mouse_pressed's bits. */
#define OPENRAC_MOUSE_LEFT 1u
#define OPENRAC_MOUSE_MIDDLE 2u
#define OPENRAC_MOUSE_RIGHT 4u
#define OPENRAC_MOUSE_BACK 8u     /* the side button nearer the wrist (X1) */
#define OPENRAC_MOUSE_FORWARD 16u /* the other side button (X2) */

/* openrac_api.take_input's devices. */
#define OPENRAC_INPUT_KEYBOARD 1u /* the port's own keyboard table stops driving pad 0 */
#define OPENRAC_INPUT_MOUSE                                                                        \
    2u /* the mod uses the mouse in play (reserved: the port has no mouse use of its own in play) */

/* The keyboard and mouse as the port read them for the tick about to run: taken once a tick, after
 * the window's events and before the game reads its pads (so before the tick's pad reports). */
typedef struct openrac_input {
    uint32_t size; /* sizeof(openrac_input) as the port was built: a newer port's is larger */
    uint32_t tick; /* counts the ticks: a new value, a new state */
    uint8_t keys[OPENRAC_KEY_COUNT]; /* by HID usage: OPENRAC_KEY_HELD | OPENRAC_KEY_PRESSED */
    uint32_t mouse_held;             /* OPENRAC_MOUSE_* down now */
    uint32_t mouse_pressed;          /* OPENRAC_MOUSE_* that went down since the last tick */
    float mouse_dx;   /* the mouse's motion since the last tick, in the system's pixels; */
    float mouse_dy;   /* raw (no pointer acceleration) while captured; y grows downward */
    float wheel_x;    /* the wheel since the last tick, in notches: x > 0 to the right, */
    float wheel_y;    /* y > 0 away from the player (up), whatever the system's scroll direction */
    uint8_t focused;  /* the game's window has the keyboard; while not, nothing is held */
    uint8_t captured; /* the cursor is captured: hidden, kept in the window, motion relative */
    uint8_t overlay;  /* the port's own overlay or menu has the input now (F3): leave it */
    uint8_t reserved;
    /* 1.3 (check size first): where the cursor is over the game's 4:3 picture, 0..1 across and
     * down (outside it below 0 or above 1), as the game's 2D screen of 512 by 416 pixels has it:
     * x * 512, y * 416 is the game's pixel. Meaningful while the cursor is not captured. */
    float pointer_x;
    float pointer_y;
    uint8_t pointer_inside; /* the cursor is over the picture, in the focused window */
    uint8_t reserved2[3];
} openrac_input;

typedef void (*openrac_frame_fn)(void* user);
/* A pad report as the console's pad library hands it to the game (its bytes, `size` of them),
 * which the callback may change before the game reads it. `port` is the controller's port, 0 or 1. */
typedef void (*openrac_pad_fn)(void* user, int port, uint8_t* report, int size);

/* What the port offers a mod. Filled by the port; valid for the life of the program. */
typedef struct openrac_api {
    uint32_t size; /* sizeof(openrac_api) as the port was built: a newer port's is larger */
    uint16_t api_major;
    uint16_t api_minor;

    const char* game;            /* "rac1": the game's id (games/<game>/game.json) */
    const char* version;         /* "pal", "ntsc": the disc's version */
    const char* openrac_version; /* OpenRAC's version, "0.9.0" */
    const char* platform;        /* "macos-arm64", "linux-x86_64", "windows-x86_64", ... */

    /* The base of game memory: game address a is memory + a. Fixed for the life of the program. */
    uint8_t* memory;

    /* A line in the session's log, after the mod's id. */
    void (*log)(openrac_mod* mod, int level, const char* text);
    /* During openrac_mod_load: why the mod refuses to load (status.json and the launcher show it). */
    void (*fail)(openrac_mod* mod, const char* reason);

    /* The mod's settings (its manifest's "settings"; the values the launcher wrote). A key the
     * manifest does not declare, or of another type, gives 0, 0.0 or "". The string stays valid. */
    int (*setting_bool)(openrac_mod* mod, const char* key);
    int64_t (*setting_int)(openrac_mod* mod, const char* key);
    double (*setting_float)(openrac_mod* mod, const char* key);
    const char* (*setting_string)(openrac_mod* mod, const char* key);

    /* Takes the hook point `name` (games/<game>.h: its name and signature) for `replacement`, a
     * function of the hook point's signature. *original is set to what the replacement goes on to:
     * the game's own function, or the replacement of a mod loaded earlier. So the mod loaded last
     * runs first. Only during openrac_mod_load. */
    int (*hook)(
        openrac_mod* mod,
        const char* name,
        const char* signature,
        void* replacement,
        void** original
    );

    /* fn(user) after each game tick, in load order. Only during openrac_mod_load. */
    int (*on_frame)(openrac_mod* mod, openrac_frame_fn fn, void* user);
    /* fn(user, port, report, size) for each pad report, before the game reads it, in load order.
     * Only during openrac_mod_load. */
    int (*on_pad)(openrac_mod* mod, openrac_pad_fn fn, void* user);

    /* A game function by its name in this disc's build ("func_001F9F90" on PAL, "FUN_001f9dc8" on
     * NTSC-U), as a host function to call with its signature; null when there is none. */
    void* (*function)(const char* name);
    /* The function at a code address, in the level loaded now (a function pointer the game keeps);
     * null when there is none. */
    void* (*function_at)(openrac_gaddr address);

    /* A global's address for the code loaded now: one of the executable's (guest.h, OPENRAC_DATA),
     * or of level `level`'s program (OPENRAC_LDATA). Ask each time: a level moves its globals. */
    openrac_gaddr (*relocate_data)(openrac_gaddr address);
    openrac_gaddr (*relocate_level_data)(int level, openrac_gaddr address);
    /* The level whose program is loaded, -1 for none (the executable's). */
    int (*loaded_level)(void);

    /* Room on the game's stack for a call that takes the address of a local: stack_push gives
     * `size` bytes (aligned to 16) and keeps the stack as it was in *saved; stack_pop(saved) gives
     * them back. Pushes and pops pair, innermost first. */
    openrac_gaddr (*stack_push)(uint32_t size, openrac_gaddr* saved);
    void (*stack_pop)(openrac_gaddr saved);

    /* 1.2. A mod built for 1.1 never sees these; one built for 1.2 loads on a 1.2 port or newer. */

    /* The keyboard and mouse for the tick about to run (openrac_input). Never null: without a
     * window (a test, a server) nothing is held and nothing moves. Valid until the next tick. */
    const openrac_input* (*input)(void);
    /* Takes `devices` (OPENRAC_INPUT_*) from the port: with OPENRAC_INPUT_KEYBOARD the port's own
     * keyboard table (WASD, Space, ...) no longer presses pad 0's buttons, and the mod presses them
     * through on_pad instead. A gamepad goes on as before; the port's window keys (F3, F11) stay
     * the port's. 0, or OPENRAC_HOOK_TOO_LATE after openrac_mod_load. */
    int (*take_input)(openrac_mod* mod, uint32_t devices);
    /* Asks for the cursor captured (1: hidden, held in the window, motion relative, as a game's
     * mouse look wants it) or let go (0). Any time; the latest asking of any mod counts. The port
     * lets it go anyway while the window is not focused or its overlay is open, and takes it back
     * when the reason is gone and the mod still asks. */
    void (*capture_mouse)(openrac_mod* mod, int capture);
} openrac_api;

/* The entry points a mod exports. */
typedef void (*openrac_mod_query_fn)(openrac_mod_info* info);
typedef int (*openrac_mod_load_fn)(const openrac_api* api, openrac_mod* mod);
typedef void (*openrac_mod_unload_fn)(void);

#define OPENRAC_MOD_QUERY "openrac_mod_query"
#define OPENRAC_MOD_LOAD "openrac_mod_load"
#define OPENRAC_MOD_UNLOAD "openrac_mod_unload"

/* Helpers for a mod: a game address's bytes, and a typed value there. */
#define OPENRAC_G(api, a) ((void*)((api)->memory + (openrac_gaddr)(a)))
#define OPENRAC_GREF(api, T, a) (*(T*)OPENRAC_G(api, a))

/* The usual openrac_mod_query: OPENRAC_MOD_DEFINE("lock-strafe", "1.0.0"). */
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
