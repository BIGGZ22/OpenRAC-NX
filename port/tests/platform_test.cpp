// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The platform layer without a window: the DualShock 2 state and its read
// buffer, the keyboard table, the conversions from SDL's ranges, the frame
// pacer on a fake clock, and the audio output on SDL's dummy driver.

#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "check.h"
#include "platform/audio.h"
#include "platform/clock.h"
#include "platform/input.h"
#include "platform/pad.h"
#include "platform/window.h"

namespace {

using namespace openrac::platform;

void pad_bits() {
    // The console's order: Select is bit 0, Left bit 7, L2 bit 8, Square bit 15.
    CHECK(static_cast<std::uint16_t>(Button::Select) == 0x0001);
    CHECK(static_cast<std::uint16_t>(Button::Start) == 0x0008);
    CHECK(static_cast<std::uint16_t>(Button::Up) == 0x0010);
    CHECK(static_cast<std::uint16_t>(Button::Left) == 0x0080);
    CHECK(static_cast<std::uint16_t>(Button::L2) == 0x0100);
    CHECK(static_cast<std::uint16_t>(Button::R1) == 0x0800);
    CHECK(static_cast<std::uint16_t>(Button::Triangle) == 0x1000);
    CHECK(static_cast<std::uint16_t>(Button::Cross) == 0x4000);
    CHECK(static_cast<std::uint16_t>(Button::Square) == 0x8000);

    Pad pad;
    CHECK(pad.buttons == 0xFFFF);  // active low: nothing held
    pad.set(Button::Cross, true, 0xC0);
    pad.set(Button::Start, true);
    CHECK(pad.held(Button::Cross));
    CHECK(pad.held(Button::Start));
    CHECK(!pad.held(Button::Circle));
    CHECK(pad.buttons == static_cast<std::uint16_t>(0xFFFF & ~0x4000 & ~0x0008));
    CHECK(pad.pressure[static_cast<std::size_t>(Pressure::Cross)] == 0xC0);
    pad.set(Button::Cross, false);
    CHECK(!pad.held(Button::Cross));
    CHECK(pad.pressure[static_cast<std::size_t>(Pressure::Cross)] == 0);
    CHECK(pressure_of(Button::Select) == Pressure::Count);
    CHECK(pressure_of(Button::R2) == Pressure::R2);
}

void read_buffer() {
    Pad pad;
    std::array<std::uint8_t, kPadBufferSize> out{};
    pad.write_read_buffer(out);
    CHECK(out[0] == 0xFF);  // disconnected: a failed read

    pad.connected = true;
    pad.set(Button::Left, true, 0x55);
    pad.set(Button::R2, true, 0x99);
    pad.left_x = 0x00;
    pad.right_y = 0xFF;
    pad.write_read_buffer(out);
    CHECK(out[0] == 0x00);
    CHECK(out[1] == kModePressure);
    CHECK(out[2] == static_cast<std::uint8_t>(~0x80));  // Left, low byte
    CHECK(out[3] == static_cast<std::uint8_t>(~0x02));  // R2, high byte
    CHECK(out[4] == 0x80);                              // right stick X
    CHECK(out[5] == 0xFF);                              // right stick Y
    CHECK(out[6] == 0x00);                              // left stick X
    CHECK(out[7] == 0x80);                              // left stick Y
    CHECK(out[9] == 0x55);                              // pressure: left
    CHECK(out[19] == 0x99);                             // pressure: R2
}

void conversions() {
    CHECK(stick_byte(0) == 0x80);
    CHECK(stick_byte(-32768) == 0x00);
    CHECK(stick_byte(32767) == 0xFF);
    CHECK(stick_byte(-128) == 0x7F);
    CHECK(trigger_byte(0) == 0);
    CHECK(trigger_byte(32767) == 0xFF);
    CHECK(trigger_byte(-5) == 0);
    CHECK(key_stick_byte(-1) == 0x00);
    CHECK(key_stick_byte(0) == 0x80);
    CHECK(key_stick_byte(1) == 0xFF);
}

// A round stick's diagonal reaches the square's corner, as a DualShock 2's does; along an axis,
// partly pushed, or already square, it stays where it was.
void squared_sticks() {
    const auto near = [](float a, float b) {
        return std::abs(a - b) < 1e-4f;
    };
    const auto squared = [](float x, float y) {
        square_stick(x, y);
        return std::pair(x, y);
    };
    auto [x, y] = squared(0.7071f, -0.7071f);
    CHECK(near(x, 1.0f) && near(y, -1.0f));
    std::tie(x, y) = squared(0.5f, 0.0f);
    CHECK(near(x, 0.5f) && near(y, 0.0f));
    // Inside 0.8 of the rim, a push and a drift are left as they are.
    std::tie(x, y) = squared(0.25f, 0.25f);
    CHECK(near(x, 0.25f) && near(y, 0.25f));
    std::tie(x, y) = squared(0.55f, 0.55f);
    CHECK(near(x, 0.55f) && near(y, 0.55f));
    std::tie(x, y) = squared(0.03f, -0.04f);
    CHECK(near(x, 0.03f) && near(y, -0.04f));
    // Between, it eases out towards the corner, the direction kept.
    std::tie(x, y) = squared(0.6364f, 0.6364f);
    CHECK(x > 0.6364f && x < 0.9f && near(x, y));
    std::tie(x, y) = squared(1.0f, 0.5f);
    CHECK(near(x, 1.0f) && near(y, 0.5f));
    std::tie(x, y) = squared(0.0f, 0.0f);
    CHECK(x == 0.0f && y == 0.0f);

    // A gamepad's stick that reports a square (a handheld's) is left as it is once its corner
    // has shown it so; a round one keeps being stretched; a new gamepad starts afresh.
    StickGate round;
    float rx = 0.7071f;
    float ry = 0.7071f;
    shape_stick(rx, ry, round);
    CHECK(!round.square && near(rx, 1.0f) && near(ry, 1.0f));
    StickGate square;
    float sx = 1.0f;
    float sy = 1.0f;
    shape_stick(sx, sy, square);
    CHECK(square.square && near(sx, 1.0f) && near(sy, 1.0f));
    sx = 0.7f;
    sy = 0.7f;
    shape_stick(sx, sy, square);
    CHECK(near(sx, 0.7f) && near(sy, 0.7f));

    StickGate gate;
    std::uint8_t bx = 0;
    std::uint8_t by = 0;
    stick_bytes(0, 0, gate, bx, by);
    CHECK(bx == 0x80 && by == 0x80);
    stick_bytes(23170, 23170, gate, bx, by);  // a round stick at 45 degrees, down and right
    CHECK(bx == 0xFF && by == 0xFF);
    stick_bytes(-23170, -23170, gate, bx, by);
    CHECK(bx == 0x00 && by == 0x00);
    stick_bytes(32767, 0, gate, bx, by);
    CHECK(bx == 0xFF && by == 0x80);
    stick_bytes(11585, 11585, gate, bx, by);  // half way: as before the stretch
    CHECK(bx == 0xAD && by == 0xAD);
}

void keyboard() {
    std::unique_ptr<bool[]> keys(new bool[SDL_SCANCODE_COUNT]());
    keys[SDL_SCANCODE_SPACE] = true;
    keys[SDL_SCANCODE_W] = true;
    keys[SDL_SCANCODE_L] = true;
    keys[SDL_SCANCODE_RETURN] = true;
    Pad pad;
    pad.connected = true;
    apply_keyboard(pad, std::span<const bool>(keys.get(), SDL_SCANCODE_COUNT));
    CHECK(pad.held(Button::Cross));
    CHECK(pad.held(Button::Start));
    CHECK(!pad.held(Button::Square));
    CHECK(pad.left_y == 0x00);  // W: up
    CHECK(pad.left_x == 0x80);
    CHECK(pad.right_x == 0xFF);  // L: right
    CHECK(pad.right_y == 0x80);
}

void pacer() {
    // A fake clock that sleeps exactly as asked.
    Nanoseconds now = 1'000'000;
    std::vector<Nanoseconds> sleeps;
    FramePacer::Clock clock{
        [&] { return now; },
        [&](Nanoseconds d) {
            sleeps.push_back(d);
            now += d;
        }
    };
    CHECK(field_rate(VideoStandard::Pal).hz() == 50.0);
    CHECK(field_rate(VideoStandard::Ntsc).numerator == 60000);

    FramePacer pal(field_rate(VideoStandard::Pal), clock);
    CHECK(pal.wait() == 1);
    CHECK(sleeps.size() == 1 && sleeps[0] == 20'000'000);  // a 50 Hz field
    now += 5'000'000;                                      // a short frame
    CHECK(pal.wait() == 1);
    CHECK(sleeps.back() == 15'000'000);
    now += 50'000'000;  // a long frame: two and a half fields
    CHECK(pal.wait() == 2);
    CHECK(sleeps.size() == 2);  // late: no sleep
    CHECK(pal.wait() == 1);     // back on the schedule
    CHECK(now == pal.deadline(5));
    now += 1'000'000'000;  // a breakpoint: start again
    CHECK(pal.wait() > static_cast<int>(FramePacer::kResyncFields));
    CHECK(pal.wait() == 1);

    // NTSC stays exact over a long run: 60000 fields are 1001 seconds.
    FramePacer ntsc(field_rate(VideoStandard::Ntsc), clock);
    CHECK(ntsc.deadline(60000) - ntsc.deadline(0) == 1001ull * 1'000'000'000ull);
    CHECK(ntsc.deadline(1) - ntsc.deadline(0) == 16'683'333ull);
}

void audio() {
    // The dummy driver (CTest sets SDL_AUDIO_DRIVER=dummy) plays nothing but
    // takes the stream like a device.
    Platform sdl(kAudio);
    if (!sdl.ok()) {
        std::printf("audio skipped: %s\n", sdl.error().c_str());
        return;
    }
    std::string error;
    auto out = AudioOutput::open(error);
    if (!out) {
        std::printf("audio skipped: %s\n", error.c_str());
        return;
    }
    out->pause(true);
    std::vector<std::int16_t> samples(960 * 2, 1000);  // 20 ms of stereo
    CHECK(out->queue(samples));
    CHECK(out->queued_frames() == 960);
    out->set_gain(0.5f);

    int calls = 0;
    auto pulled = AudioOutput::open(error, [&](std::span<std::int16_t> buffer) {
        ++calls;
        CHECK(buffer.size() % 2 == 0);
    });
    CHECK(pulled != nullptr);
}

}  // namespace

int main() {
    pad_bits();
    read_buffer();
    conversions();
    squared_sticks();
    keyboard();
    pacer();
    audio();
    return openrac::test::result();
}
