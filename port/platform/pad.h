// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The state of one DualShock 2, as the game's pad library hands it to the
// game. The replacement of that library (scePadRead and its neighbours) copies
// a Pad into the game's buffers; the platform layer fills the Pad from SDL's
// keyboard and gamepads (input.h). Nothing here touches SDL, so the mapping is
// tested on its own.
//
// What the console reports, from public knowledge of the DualShock 2's
// controller protocol (as documented by homebrew projects and controller
// adapters; no Sony material):
//
//   - Sixteen digital buttons in one 16-bit word, **active low**: a button
//     that is held reads 0, a released one 1. The bit order is fixed by the
//     controller: the low byte is the first button byte the pad sends, the
//     high byte the second (Button below).
//   - Two analog sticks, one byte per axis: 0x00 is fully left or up, 0xFF
//     fully right or down, 0x80 the centre. Sticks at rest read near 0x80,
//     never exactly, so games apply a dead zone of their own.
//   - In pressure mode, one byte per pressure-sensitive button (the
//     directions, the four face buttons, L1, R1, L2, R2): 0x00 released,
//     0xFF pressed hard.
//
// The read buffer libpad fills (32 bytes, DualShock 2 in pressure mode):
//
//   +0   0x00 when the read succeeded
//   +1   the mode: high nibble the type (4 digital, 7 analog), low nibble
//        the number of 16-bit words that follow (0x41, 0x73, 0x79)
//   +2   buttons, low byte (active low)
//   +3   buttons, high byte (active low)
//   +4   right stick X     +5  right stick Y
//   +6   left stick X      +7  left stick Y
//   +8   pressure: right, left, up, down (+8..+11)
//   +12  pressure: triangle, circle, cross, square (+12..+15)
//   +16  pressure: L1, R1, L2, R2 (+16..+19)

#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace openrac::platform {

// The button bits in the console's order. A bit is CLEAR while the button is
// held (Pad::buttons is active low).
enum class Button : std::uint16_t {
    Select = 1u << 0,
    L3 = 1u << 1,
    R3 = 1u << 2,
    Start = 1u << 3,
    Up = 1u << 4,
    Right = 1u << 5,
    Down = 1u << 6,
    Left = 1u << 7,
    L2 = 1u << 8,
    R2 = 1u << 9,
    L1 = 1u << 10,
    R1 = 1u << 11,
    Triangle = 1u << 12,
    Circle = 1u << 13,
    Cross = 1u << 14,
    Square = 1u << 15,
};

// The pressure bytes, in the order the controller sends them.
enum class Pressure : std::uint8_t {
    Right,
    Left,
    Up,
    Down,
    Triangle,
    Circle,
    Cross,
    Square,
    L1,
    R1,
    L2,
    R2,
    Count,
};

// The pressure byte of a pressure-sensitive button, or Pressure::Count for a
// button without one (Select, Start, L3, R3).
Pressure pressure_of(Button button);

inline constexpr std::uint8_t kStickCentre = 0x80;
inline constexpr std::size_t kPadBufferSize = 32;

// The controller's mode byte (+1 of the read buffer).
inline constexpr std::uint8_t kModeDigital = 0x41;
inline constexpr std::uint8_t kModeAnalog = 0x73;
inline constexpr std::uint8_t kModePressure = 0x79;

struct Pad {
    bool connected = false;
    std::uint16_t buttons = 0xFFFF;  // active low: all released
    std::uint8_t right_x = kStickCentre;
    std::uint8_t right_y = kStickCentre;
    std::uint8_t left_x = kStickCentre;
    std::uint8_t left_y = kStickCentre;
    std::array<std::uint8_t, static_cast<std::size_t>(Pressure::Count)> pressure{};
    // Rumble the game asked for (scePadSetActDirect): the small motor is on or
    // off, the large one has a speed. The platform layer plays it.
    bool small_motor = false;
    std::uint8_t large_motor = 0;

    bool held(Button button) const { return (buttons & static_cast<std::uint16_t>(button)) == 0; }

    // Press or release a button. A pressure-sensitive button's pressure is
    // set too: `amount` when held (0xFF for a key or a digital button), 0 when
    // released.
    void set(Button button, bool down, std::uint8_t amount = 0xFF);

    // The 32-byte read buffer above, in pressure mode. A disconnected pad
    // writes a failed read (first byte 0xFF, the rest zero).
    void write_read_buffer(std::span<std::uint8_t, kPadBufferSize> out) const;
};

// The conversions from SDL's ranges, kept here so they are tested without SDL.

// A stick axis, -32768..32767 with positive right or down, to the console's
// byte. 0 maps to 0x80.
std::uint8_t stick_byte(std::int16_t axis);

// A stick's two axes (-1..1, positive right or down) as the DualShock 2 gives them. Its sticks
// reach their corners: pushed fully to a diagonal, both bytes are near 0x00 or 0xFF, and the game
// counts on it (Ratchet runs only on a stick pushed all the way). A round stick (most gamepads
// through SDL, the on-screen pad's) reaches about 0.71 on each axis there, and Ratchet walks. So a
// push near the round gate's rim is stretched out to the square: on the rim the larger axis becomes
// the length of the push (held to 1), the direction stays. Inside 0.8 of the rim nothing changes,
// so a partial push and a resting stick's drift are as the gamepad gives them.
void square_stick(float& x, float& y);

// What one gamepad stick has shown of its gate. A stick that has reported a position well past a
// circle (a diagonal near the square's corner) is square already, as some handhelds' are: from
// then on it is left as it reports, unstretched.
struct StickGate {
    bool square = false;
};

// square_stick() for a gamepad's stick, unless `gate` has shown it square (and learning so).
void shape_stick(float& x, float& y, StickGate& gate);

// A gamepad's stick, both of SDL's axes, to the console's bytes through shape_stick().
void stick_bytes(
    std::int16_t x, std::int16_t y, StickGate& gate, std::uint8_t& out_x, std::uint8_t& out_y
);

// A trigger, 0..32767, to a pressure byte.
std::uint8_t trigger_byte(std::int16_t axis);

// A digital stick from keys: -1, 0 or +1 to 0x00, 0x80 or 0xFF.
std::uint8_t key_stick_byte(int direction);

}  // namespace openrac::platform
