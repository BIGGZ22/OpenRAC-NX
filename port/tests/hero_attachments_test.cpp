// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Synthetic RAC1 equipment slots test body-first ownership, detached objects and bounded reads.
#include <array>
#include <cstdint>
#include <cstring>

#include "game/rac1/host/hero_attachments.h"
#include "tests/check.h"

namespace {
// Synthetic RAM holds a hero state followed by independent complete moby records.
struct Fixture {
    std::array<std::uint8_t, 0x5000> ram{};     // Fits state at 0x100 and mobys from 0x3000.
    std::array<std::uint32_t, 32> addresses{};  // Larger than the body's seven equipment slots.

    // Writes a synthetic pointer with the byte layout read by the production C helper.
    void pointer(std::size_t offset, std::uint32_t value) {
        std::memcpy(ram.data() + offset, &value, sizeof(value));
    }

    // Extracts from a fixed state address, with an optional output capacity restriction.
    unsigned read(unsigned capacity = 32) {
        return openrac_rac1_hero_attachments(
            ram.data(), ram.size(), 0x100, addresses.data(), capacity
        );
    }
};

// Body-first ordering and deduplication are independent of the renderer's submission order.
void test_body_and_attached_equipment() {
    Fixture fixture;
    fixture.pointer(0x100 + 0x2080, 0x3000);
    fixture.pointer(0x100 + 0x1070 + 0x20, 0x3100);
    fixture.pointer(0x100 + 0x1070 + 0x24, 0x3200);
    fixture.pointer(0x100 + 0x1070 + 0x2C, 0x3300);
    fixture.pointer(0x100 + 0x1070 + 0x50 + 0x20, 0x3100);
    CHECK(fixture.read() == 4);
    CHECK(fixture.addresses[0] == 0x3000);
    CHECK(fixture.addresses[1] == 0x3100);
    CHECK(fixture.addresses[2] == 0x3200);
    CHECK(fixture.addresses[3] == 0x3300);
    CHECK(fixture.read(1) == 1);
    CHECK(fixture.addresses[0] == 0x3000);
}

// Detached equipment and invalid pointers cannot become part of the hero-only caster list.
void test_detached_and_invalid_equipment() {
    Fixture fixture;
    fixture.pointer(0x100 + 0x2080, 0x3000);
    fixture.pointer(0x100 + 0x1070 + 0x20, 0x3100);
    fixture.ram[0x100 + 0x1070 + 0x3A] = 1;
    fixture.pointer(0x100 + 0x1070 + 0x50 + 0x20, 0x4FFF);
    CHECK(fixture.read() == 1);
    CHECK(fixture.read(0) == 0);
    CHECK(
        openrac_rac1_hero_attachments(
            fixture.ram.data(), 0x1000, 0x100, fixture.addresses.data(), 32
        )
        == 0
    );
    fixture.pointer(0x100 + 0x2080, 0);
    CHECK(fixture.read() == 0);
}
}  // namespace

int main() {
    test_body_and_attached_equipment();
    test_detached_and_invalid_equipment();
    return openrac::test::result();
}
