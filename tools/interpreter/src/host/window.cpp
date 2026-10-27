// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The SDL3 window: opening it, the event loop, the keyboard and controller mapping, the picture
 * upload and the sound output.
 *
 * Sources: the SDL3 interface as publicly documented, and the console's button order as in
 * `Controls`.
 */

#include "window.h"

#include <cstdio>

#include <SDL3/SDL.h>

namespace host {

Window::~Window() {
    // Releases everything that was opened, in the order `close` does.
    close();
}

bool Window::open(const char* title, int width, int height) {
    // Video for the window, the gamepad subsystem for controllers, audio for `play`.
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    // The window can be resized and uses the display's full pixel density where it has one.
    if (!SDL_CreateWindowAndRenderer(
            title,
            width,
            height,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
            &window_,
            &renderer_
        )) {
        std::fprintf(stderr, "SDL_CreateWindowAndRenderer: %s\n", SDL_GetError());
        return false;
    }

    // One buffer swap per display refresh, so the picture does not tear.
    SDL_SetRenderVSync(renderer_, 1);
    return true;
}

bool Window::pump() {
    SDL_Event event;
    bool running = true;

    // Drain the event queue; the loop ends when SDL has no event left.
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            // The user closed the window or the system asked the program to quit.
            running = false;
        } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            // Escape closes the window too.
            running = false;
        } else if (event.type == SDL_EVENT_GAMEPAD_ADDED && !gamepad_) {
            // The first controller plugged in is the one that is read; later ones are ignored.
            gamepad_ = SDL_OpenGamepad(event.gdevice.which);
        } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED && gamepad_
                   && SDL_GetGamepadID(gamepad_) == event.gdevice.which) {
            // The controller in use was unplugged: let it go, so a later one can take its place.
            SDL_CloseGamepad(gamepad_);
            gamepad_ = nullptr;
        }
    }

    return running;
}

Controls Window::controls() const {
    /** The bits of `Controls::buttons`, in the console's button order. */
    enum : unsigned {
        kSelect = 1u << 0,
        kL3 = 1u << 1,
        kR3 = 1u << 2,
        kStart = 1u << 3,
        kUp = 1u << 4,
        kRight = 1u << 5,
        kDown = 1u << 6,
        kLeft = 1u << 7,
        kL2 = 1u << 8,
        kR2 = 1u << 9,
        kL1 = 1u << 10,
        kR1 = 1u << 11,
        kTriangle = 1u << 12,
        kCircle = 1u << 13,
        kCross = 1u << 14,
        kSquare = 1u << 15,
    };

    Controls c;

    // Stick positions, -127..127 from each source; the keyboard and the controller add up.
    int left_x = 0, left_y = 0, right_x = 0, right_y = 0;

    const bool* keys = SDL_GetKeyboardState(nullptr);

    /** Which keyboard key presses which button. */
    static constexpr struct {
        SDL_Scancode key;
        unsigned button;
    } key_buttons[] = {
        {SDL_SCANCODE_UP, kUp},
        {SDL_SCANCODE_DOWN, kDown},
        {SDL_SCANCODE_LEFT, kLeft},
        {SDL_SCANCODE_RIGHT, kRight},
        {SDL_SCANCODE_SPACE, kCross},
        {SDL_SCANCODE_F, kSquare},
        {SDL_SCANCODE_E, kCircle},
        {SDL_SCANCODE_R, kTriangle},
        {SDL_SCANCODE_Q, kL1},
        {SDL_SCANCODE_LSHIFT, kR1},
        {SDL_SCANCODE_Z, kL2},
        {SDL_SCANCODE_C, kR2},
        {SDL_SCANCODE_RETURN, kStart},
        {SDL_SCANCODE_BACKSPACE, kSelect},
    };

    // A held key sets its button's bit.
    for (const auto& k : key_buttons) {
        if (keys[k.key]) {
            c.buttons |= k.button;
        }
    }

    // The letter keys push a stick all the way (127) one way or the other; up is negative.
    left_x += (keys[SDL_SCANCODE_D] ? 127 : 0) - (keys[SDL_SCANCODE_A] ? 127 : 0);
    left_y += (keys[SDL_SCANCODE_S] ? 127 : 0) - (keys[SDL_SCANCODE_W] ? 127 : 0);
    right_x += (keys[SDL_SCANCODE_L] ? 127 : 0) - (keys[SDL_SCANCODE_J] ? 127 : 0);
    right_y += (keys[SDL_SCANCODE_K] ? 127 : 0) - (keys[SDL_SCANCODE_I] ? 127 : 0);

    // A controller adds to what the keyboard gave.
    if (gamepad_) {
        /** Which controller button presses which of the console's buttons. */
        static constexpr struct {
            SDL_GamepadButton pad;
            unsigned button;
        } pad_buttons[] = {
            {SDL_GAMEPAD_BUTTON_BACK, kSelect},
            {SDL_GAMEPAD_BUTTON_LEFT_STICK, kL3},
            {SDL_GAMEPAD_BUTTON_RIGHT_STICK, kR3},
            {SDL_GAMEPAD_BUTTON_START, kStart},
            {SDL_GAMEPAD_BUTTON_DPAD_UP, kUp},
            {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, kRight},
            {SDL_GAMEPAD_BUTTON_DPAD_DOWN, kDown},
            {SDL_GAMEPAD_BUTTON_DPAD_LEFT, kLeft},
            {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, kL1},
            {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, kR1},
            {SDL_GAMEPAD_BUTTON_NORTH, kTriangle},
            {SDL_GAMEPAD_BUTTON_EAST, kCircle},
            {SDL_GAMEPAD_BUTTON_SOUTH, kCross},
            {SDL_GAMEPAD_BUTTON_WEST, kSquare},
        };

        // A held button sets its bit.
        for (const auto& b : pad_buttons) {
            if (SDL_GetGamepadButton(gamepad_, b.pad)) {
                c.buttons |= b.button;
            }
        }

        // SDL gives the triggers as axes (0 to 32767); past 8000 counts as a press.
        if (SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 8000) {
            c.buttons |= kL2;
        }
        if (SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 8000) {
            c.buttons |= kR2;
        }

        // SDL's stick axes run -32768 to 32767; dividing by 258 brings them to about -127..127.
        left_x += SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTX) / 258;
        left_y += SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTY) / 258;
        right_x += SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHTX) / 258;
        right_y += SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHTY) / 258;
    }

    // Called below for each axis, on this thread: clamp to -127..127 and centre on 0x80.
    auto stick = [](int value) {
        value = value < -127 ? -127 : value > 127 ? 127 : value;
        return static_cast<unsigned char>(0x80 + value);
    };

    c.left_x = stick(left_x);
    c.left_y = stick(left_y);
    c.right_x = stick(right_x);
    c.right_y = stick(right_y);

    return c;
}

void Window::present(const ps2::Image& image, float aspect) {
    // Not open, or an empty picture: nothing to show.
    if (!renderer_ || image.width <= 0 || image.height <= 0) {
        return;
    }

    // The texture is made again whenever the picture changes size.
    if (!texture_ || texture_width_ != image.width || texture_height_ != image.height) {
        // Free the old texture before replacing it.
        if (texture_) {
            SDL_DestroyTexture(texture_);
        }

        // Image stores R in the low byte, which is RGBA in memory order.
        texture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            image.width,
            image.height
        );

        // SDL could not make the texture; skip this picture.
        if (!texture_) {
            std::fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
            return;
        }

        SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_LINEAR);
        texture_width_ = image.width;
        texture_height_ = image.height;
    }

    // Four bytes a pixel: the row pitch of the image.
    SDL_UpdateTexture(texture_, nullptr, image.pixels.data(), image.width * 4);

    int out_w = 0, out_h = 0;
    SDL_GetRenderOutputSize(renderer_, &out_w, &out_h);
    float w = static_cast<float>(out_w), h = static_cast<float>(out_h);
    SDL_FRect dst;

    // The window is wider than the picture's shape: fit the height, bars at the sides.
    if (w / h > aspect) {
        dst.h = h;
        dst.w = h * aspect;
    } else {
        // Taller than the picture's shape: fit the width, bars above and below.
        dst.w = w;
        dst.h = w / aspect;
    }

    // Centre the rectangle in the window.
    dst.x = (w - dst.w) / 2;
    dst.y = (h - dst.h) / 2;

    // Black bars: clear to opaque black, then draw the picture over the clear.
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, &dst);
    SDL_RenderPresent(renderer_);
}

void Window::play(const short* samples, std::size_t frames, int rate) {
    // The first call, or a new rate: open the output stream again.
    if (!audio_ || audio_rate_ != rate) {
        // The old stream was for the old rate.
        if (audio_) {
            SDL_DestroyAudioStream(audio_);
        }

        // Signed 16-bit samples, two channels (left, right).
        SDL_AudioSpec spec{SDL_AUDIO_S16, 2, rate};
        audio_ =
            SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
        audio_rate_ = rate;

        // No output device: the program goes on without sound.
        if (!audio_) {
            std::fprintf(stderr, "no sound: %s\n", SDL_GetError());
            return;
        }

        // SDL opens a device stream paused.
        SDL_ResumeAudioStreamDevice(audio_);
    }

    // Over a fifth of a second queued means the machine ran ahead of the loudspeaker: drop it.
    if (SDL_GetAudioStreamQueued(audio_) > rate * 4 / 5) {
        // The queue counts bytes, four to a frame: rate * 4 / 5 bytes is a fifth of a second.
        SDL_ClearAudioStream(audio_);
    }

    // Four bytes a frame: two channels of 16 bits.
    SDL_PutAudioStreamData(audio_, samples, static_cast<int>(frames * 4));
}

void Window::close() {
    // Release in the reverse order of creation; each pointer is cleared so a second close is safe.
    if (audio_) {
        SDL_DestroyAudioStream(audio_);
        audio_ = nullptr;
    }
    if (gamepad_) {
        SDL_CloseGamepad(gamepad_);
        gamepad_ = nullptr;
    }
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    // SDL is shut down with the window, the call that `open` paired with SDL_Init.
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
    }
}

}  // namespace host
