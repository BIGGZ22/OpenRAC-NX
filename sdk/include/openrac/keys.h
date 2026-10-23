/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Names for the keyboard and mouse (interface 1.2, openrac_mod.h's openrac_input): a "bind" is one
 * key, mouse button or wheel direction, written as a name in a mod's settings and the launcher.
 *
 * Keys are named by where they are, as the web's KeyboardEvent.code names them ("KeyW" is the key
 * left of E on any layout; "Digit1", "Space", "ShiftLeft", "ArrowUp", "Numpad0"), so the launcher's
 * "press a key" control and a mod agree on every name. The mouse's are "MouseLeft", "MouseRight",
 * "MouseMiddle", "MouseBack", "MouseForward", and the wheel's "WheelUp", "WheelDown", "WheelLeft",
 * "WheelRight". A key-binding setting (type "keybind", docs/settings.md) holds up to two of them
 * separated by a comma: "KeyW,ArrowUp"; "" binds nothing.
 *
 * Header-only, C99 and C++: everything is static inline, and needs nothing but the table. */
#ifndef OPENRAC_KEYS_H
#define OPENRAC_KEYS_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "openrac/openrac_mod.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A bind: a key's HID usage (1 .. OPENRAC_KEY_COUNT-1), or one of these. 0 is no bind. */
#define OPENRAC_BIND_NONE 0
#define OPENRAC_BIND_MOUSE_LEFT 0x200
#define OPENRAC_BIND_MOUSE_MIDDLE 0x201
#define OPENRAC_BIND_MOUSE_RIGHT 0x202
#define OPENRAC_BIND_MOUSE_BACK 0x203
#define OPENRAC_BIND_MOUSE_FORWARD 0x204
#define OPENRAC_BIND_WHEEL_UP 0x210
#define OPENRAC_BIND_WHEEL_DOWN 0x211
#define OPENRAC_BIND_WHEEL_LEFT 0x212
#define OPENRAC_BIND_WHEEL_RIGHT 0x213

typedef struct openrac_bind_name {
    const char* name;
    int bind;
} openrac_bind_name;

/* Every name, in the order the launcher lists them. */
static const openrac_bind_name openrac_bind_names[] = {
    {"KeyA", 4},
    {"KeyB", 5},
    {"KeyC", 6},
    {"KeyD", 7},
    {"KeyE", 8},
    {"KeyF", 9},
    {"KeyG", 10},
    {"KeyH", 11},
    {"KeyI", 12},
    {"KeyJ", 13},
    {"KeyK", 14},
    {"KeyL", 15},
    {"KeyM", 16},
    {"KeyN", 17},
    {"KeyO", 18},
    {"KeyP", 19},
    {"KeyQ", 20},
    {"KeyR", 21},
    {"KeyS", 22},
    {"KeyT", 23},
    {"KeyU", 24},
    {"KeyV", 25},
    {"KeyW", 26},
    {"KeyX", 27},
    {"KeyY", 28},
    {"KeyZ", 29},
    {"Digit1", 30},
    {"Digit2", 31},
    {"Digit3", 32},
    {"Digit4", 33},
    {"Digit5", 34},
    {"Digit6", 35},
    {"Digit7", 36},
    {"Digit8", 37},
    {"Digit9", 38},
    {"Digit0", 39},
    {"Enter", 40},
    {"Escape", 41},
    {"Backspace", 42},
    {"Tab", 43},
    {"Space", 44},
    {"Minus", 45},
    {"Equal", 46},
    {"BracketLeft", 47},
    {"BracketRight", 48},
    {"Backslash", 49},
    {"IntlHash", 50},
    {"Semicolon", 51},
    {"Quote", 52},
    {"Backquote", 53},
    {"Comma", 54},
    {"Period", 55},
    {"Slash", 56},
    {"CapsLock", 57},
    {"F1", 58},
    {"F2", 59},
    {"F3", 60},
    {"F4", 61},
    {"F5", 62},
    {"F6", 63},
    {"F7", 64},
    {"F8", 65},
    {"F9", 66},
    {"F10", 67},
    {"F11", 68},
    {"F12", 69},
    {"PrintScreen", 70},
    {"ScrollLock", 71},
    {"Pause", 72},
    {"Insert", 73},
    {"Home", 74},
    {"PageUp", 75},
    {"Delete", 76},
    {"End", 77},
    {"PageDown", 78},
    {"ArrowRight", 79},
    {"ArrowLeft", 80},
    {"ArrowDown", 81},
    {"ArrowUp", 82},
    {"NumLock", 83},
    {"NumpadDivide", 84},
    {"NumpadMultiply", 85},
    {"NumpadSubtract", 86},
    {"NumpadAdd", 87},
    {"NumpadEnter", 88},
    {"Numpad1", 89},
    {"Numpad2", 90},
    {"Numpad3", 91},
    {"Numpad4", 92},
    {"Numpad5", 93},
    {"Numpad6", 94},
    {"Numpad7", 95},
    {"Numpad8", 96},
    {"Numpad9", 97},
    {"Numpad0", 98},
    {"NumpadDecimal", 99},
    {"IntlBackslash", 100},
    {"ContextMenu", 101},
    {"NumpadEqual", 103},
    {"ControlLeft", 224},
    {"ShiftLeft", 225},
    {"AltLeft", 226},
    {"MetaLeft", 227},
    {"ControlRight", 228},
    {"ShiftRight", 229},
    {"AltRight", 230},
    {"MetaRight", 231},
    {"MouseLeft", OPENRAC_BIND_MOUSE_LEFT},
    {"MouseRight", OPENRAC_BIND_MOUSE_RIGHT},
    {"MouseMiddle", OPENRAC_BIND_MOUSE_MIDDLE},
    {"MouseBack", OPENRAC_BIND_MOUSE_BACK},
    {"MouseForward", OPENRAC_BIND_MOUSE_FORWARD},
    {"WheelUp", OPENRAC_BIND_WHEEL_UP},
    {"WheelDown", OPENRAC_BIND_WHEEL_DOWN},
    {"WheelLeft", OPENRAC_BIND_WHEEL_LEFT},
    {"WheelRight", OPENRAC_BIND_WHEEL_RIGHT},
};

#define OPENRAC_BIND_NAME_COUNT ((int)(sizeof openrac_bind_names / sizeof openrac_bind_names[0]))

/* The bind a name is (`length` characters of it, so a name inside a longer text is read in
 * place), or OPENRAC_BIND_NONE for no name of the table. */
static inline int openrac_bind_from_name_n(const char* name, size_t length) {
    int i;

    if (name == NULL) {
        return OPENRAC_BIND_NONE;
    }

    for (i = 0; i < OPENRAC_BIND_NAME_COUNT; ++i) {
        const char* n = openrac_bind_names[i].name;

        if (strlen(n) == length && memcmp(n, name, length) == 0) {
            return openrac_bind_names[i].bind;
        }
    }

    return OPENRAC_BIND_NONE;
}

static inline int openrac_bind_from_name(const char* name) {
    return name != NULL ? openrac_bind_from_name_n(name, strlen(name)) : OPENRAC_BIND_NONE;
}

/* A bind's name, or "" for none of the table. */
static inline const char* openrac_bind_to_name(int bind) {
    int i;

    for (i = 0; i < OPENRAC_BIND_NAME_COUNT; ++i) {
        if (openrac_bind_names[i].bind == bind) {
            return openrac_bind_names[i].name;
        }
    }

    return "";
}

/* A mouse bind's OPENRAC_MOUSE_* bit; 0 for any other. */
static inline uint32_t openrac_bind_mouse_bit(int bind) {
    return bind >= OPENRAC_BIND_MOUSE_LEFT && bind <= OPENRAC_BIND_MOUSE_FORWARD
               ? (uint32_t)1u << (uint32_t)(bind - OPENRAC_BIND_MOUSE_LEFT)
               : 0u;
}

/* The wheel's notches toward a wheel bind this tick (0 for none, or any other bind). */
static inline float openrac_bind_wheel(const openrac_input* in, int bind) {
    float v = 0.0f;

    if (in == NULL) {
        return 0.0f;
    }

    switch (bind) {
        case OPENRAC_BIND_WHEEL_UP:
            v = in->wheel_y;
            break;
        case OPENRAC_BIND_WHEEL_DOWN:
            v = -in->wheel_y;
            break;
        case OPENRAC_BIND_WHEEL_RIGHT:
            v = in->wheel_x;
            break;
        case OPENRAC_BIND_WHEEL_LEFT:
            v = -in->wheel_x;
            break;
        default:
            return 0.0f;
    }

    return v > 0.0f ? v : 0.0f;
}

/* Whether a bind is held now. A wheel direction is "held" for the tick it turned in. */
static inline int openrac_bind_held(const openrac_input* in, int bind) {
    if (in == NULL || bind <= 0) {
        return 0;
    }

    if (bind < OPENRAC_KEY_COUNT) {
        return (in->keys[bind] & OPENRAC_KEY_HELD) != 0;
    }

    if (openrac_bind_mouse_bit(bind) != 0) {
        return (in->mouse_held & openrac_bind_mouse_bit(bind)) != 0;
    }

    return openrac_bind_wheel(in, bind) >= 0.5f;
}

/* Whether a bind went down since the last tick (a wheel notch counts). */
static inline int openrac_bind_pressed(const openrac_input* in, int bind) {
    if (in == NULL || bind <= 0) {
        return 0;
    }

    if (bind < OPENRAC_KEY_COUNT) {
        return (in->keys[bind] & OPENRAC_KEY_PRESSED) != 0;
    }

    if (openrac_bind_mouse_bit(bind) != 0) {
        return (in->mouse_pressed & openrac_bind_mouse_bit(bind)) != 0;
    }

    return openrac_bind_wheel(in, bind) >= 0.5f;
}

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_KEYS_H */
