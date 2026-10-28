// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#ifndef __SWITCH__
#include <SDL3/SDL.h>
#else
// The Switch portlibs provide SDL2. Keep the frontend's small SDL3-facing
// surface explicit, rather than pretending the two complete APIs are identical.
#define SDL_Event SDL2_Event
#define SDL_GetWindowSize SDL2_GetWindowSize
#define SDL_GetMouseState SDL2_GetMouseState
#include <SDL2/SDL.h>
#undef SDL_Event
#undef SDL_GetWindowSize
#undef SDL_GetMouseState
// Renaming the declarations must still link to SDL2's original C symbols.
extern "C" void SDL2_GetWindowSize(SDL_Window*, int*, int*) __asm__("SDL_GetWindowSize");
extern "C" Uint32 SDL2_GetMouseState(int*, int*) __asm__("SDL_GetMouseState");
union SDL_Event {
    Uint32 type;
    struct { Uint32 type; SDL_Keycode key; Uint16 mod; Uint8 repeat; } key;
    struct { Uint32 type; int data1, data2; } window;
};
enum {
    SDL_EVENT_QUIT = SDL_QUIT,
    SDL_EVENT_KEY_DOWN = SDL_KEYDOWN,
    SDL_EVENT_WINDOW_DISPLAY_CHANGED = 0x10001,
    SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED,
    SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED,
    SDL_EVENT_WINDOW_ENTER_FULLSCREEN,
    SDL_EVENT_WINDOW_LEAVE_FULLSCREEN,
    SDL_EVENT_WINDOW_RESIZED,
    SDL_EVENT_WINDOW_FOCUS_LOST,
    SDL_EVENT_WINDOW_FOCUS_GAINED,
    SDL_KMOD_ALT = KMOD_ALT
};
inline bool SDL_GetWindowSize(SDL_Window* w, int* x, int* y) {
    SDL2_GetWindowSize(w, x, y); return true;
}
inline bool SDL_GetWindowSafeArea(SDL_Window* w, SDL_Rect* r) {
    r->x = r->y = 0; SDL2_GetWindowSize(w, &r->w, &r->h); return true;
}
inline Uint32 SDL_GetMouseState(float* x, float* y) {
    int a = 0, b = 0; auto result = SDL2_GetMouseState(&a, &b);
    *x = static_cast<float>(a); *y = static_cast<float>(b); return result;
}
inline bool SDL_GetWindowRelativeMouseMode(SDL_Window*) { return SDL_GetRelativeMouseMode() == SDL_TRUE; }
inline bool SDL_SetWindowRelativeMouseMode(SDL_Window*, bool value) {
    return SDL_SetRelativeMouseMode(value ? SDL_TRUE : SDL_FALSE) == 0;
}
#endif
