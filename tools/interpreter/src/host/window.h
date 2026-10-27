// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * An SDL3 window that shows the pictures, plays the sound and reads the keyboard and controller.
 *
 * Nothing else in the runtime calls SDL. It leaves out menus and settings: a window shows pictures
 * and nothing more.
 */

#pragma once

#include <cstddef>

#include "ps2/gs.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Gamepad;
struct SDL_AudioStream;

namespace host {

/**
 * A controller as the games read it: buttons as a bit mask in the console's
 * order (select, L3, R3, start, up, right, down, left, L2, R2, L1, R1,
 * triangle, circle, cross, square) and two sticks, 0x80 centred.
 */
struct Controls {
    /** The pressed buttons, one bit each, bit 0 being select and bit 15 square. */
    unsigned buttons = 0;

    /** The stick axes, 0x80 when centred; left and right stick, horizontal and vertical. */
    unsigned char left_x = 0x80, left_y = 0x80, right_x = 0x80, right_y = 0x80;
};

/**
 * A window that shows one image per frame, scaled to fit with a fixed aspect
 * ratio. The games' frame buffers are not square-pixelled (512 by 448 shown
 * at 4:3), so the ratio is given, not derived from the image.
 *
 * It also reads the keyboard and the first game controller, and plays the sound through the
 * default output device. Every method is called from the program's main thread, the one that runs
 * the machine. It owns the SDL window, renderer, texture, controller and audio stream, so its copy
 * operations are deleted; the destructor closes them.
 */
class Window {
public:
    /** Makes a window object that is not open yet. */
    Window() = default;

    /** Closes whatever is open. */
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * Starts SDL and opens the window with its renderer.
     *
     * @param title The window's title bar text.
     * @param width Initial width of the window.
     * @param height Initial height of the window.
     * @return True when the window opened; the error has been written to `stderr` otherwise.
     */
    bool open(const char* title, int width, int height);

    /**
     * Handles pending events. False once the user has asked to quit.
     *
     * Also picks up a game controller when one is plugged in and lets it go when it is removed.
     * Escape counts as a request to quit.
     *
     * @return True while the program should go on running.
     */
    bool pump();

    /**
     * What is held now, on the keyboard and on the first game controller.
     *
     * Keys: W A S D the left stick, I J K L the right stick, the arrows the
     * direction pad, space cross, F square, E circle, R triangle, Q L1, left
     * shift R1, Z L2, C R2, return start, backspace select.
     *
     * @return The buttons and sticks; a controller adds to the keyboard.
     */
    Controls controls() const;

    /**
     * Sound to be heard: pairs of left and right samples at `rate`. What does
     * not come in time is silence; what would pile up is dropped.
     *
     * @param samples Interleaved left and right samples, signed 16 bits.
     * @param frames Number of left and right pairs in `samples`.
     * @param rate Sampling rate in Hz; the audio stream is reopened when it changes.
     */
    void play(const short* samples, std::size_t frames, int rate);

    /**
     * Shows one picture, scaled to the window with black bars where the shapes differ.
     *
     * @param image The picture; its pixels hold red in the low byte.
     * @param aspect Width over height the picture is to be shown at, such as 4/3.
     */
    void present(const ps2::Image& image, float aspect);

    /** Releases the SDL objects and shuts SDL down; safe to call on a window that is not open. */
    void close();

private:
    /** The SDL window, or null while closed. */
    SDL_Window* window_ = nullptr;

    /** The renderer of `window_`, or null while closed. */
    SDL_Renderer* renderer_ = nullptr;

    /** The streaming texture the picture is uploaded to; null until the first picture. */
    SDL_Texture* texture_ = nullptr;

    /** The first game controller, or null when none is connected. */
    SDL_Gamepad* gamepad_ = nullptr;

    /** The audio output stream, or null until the first `play`. */
    SDL_AudioStream* audio_ = nullptr;

    /** The sampling rate `audio_` was opened with. */
    int audio_rate_ = 0;

    /** The size in pixels of `texture_`, so a change in the picture's size is noticed. */
    int texture_width_ = 0, texture_height_ = 0;
};

}  // namespace host
