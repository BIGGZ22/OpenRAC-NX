// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Synthetic game memory for the pose tests (moby_pose_test.cpp): classes, keys, joint modifiers
// and pose layers laid out where scene/moby_pose.cpp reads them, from a seeded generator. Valid
// and invalid cases mixed on purpose: keys missing, all zero, cut off by the memory's end or with
// sizes that disagree; parent words that name later joints, no record or a record past the
// joints; modifier lists that loop; layers whose keys are missing. Synthetic: it covers the
// reading and the arithmetic's branches, not every class the game has.

#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <vector>

namespace openrac::test::pose_ram {

class Ram {
public:
    explicit Ram(std::size_t bytes, std::uint32_t seed) : m_bytes(bytes, 0), m_rng(seed) {}

    std::vector<std::uint8_t>& bytes() { return m_bytes; }

    // Whether less than `room` bytes are left: time for a new Ram.
    bool nearly_full(std::size_t room = 1u << 20) const { return m_next + room > m_bytes.size(); }

    // Only readable keys from here on (no missing, empty or malformed ones).
    void valid_keys_only() { m_valid_keys = true; }

    std::uint32_t alloc(std::size_t size) {
        m_next = (m_next + 15u) & ~15u;
        const std::uint32_t at = m_next;
        m_next += static_cast<std::uint32_t>(size);
        if (m_next > m_bytes.size() - 0x100) {
            // Out of room (the tests start a new Ram well before: see nearly_full). Starting again
            // over the old could leave stale pointers that loop, so this is a test bug.
            std::abort();
        }
        std::memset(m_bytes.data() + at, 0, size);
        return at;
    }

    template <typename T>
    void put(std::uint32_t at, T v) {
        std::memcpy(m_bytes.data() + at, &v, sizeof(T));
    }

    std::uint32_t u(std::uint32_t lo, std::uint32_t hi) {
        return std::uniform_int_distribution<std::uint32_t>(lo, hi)(m_rng);
    }

    float f(float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(m_rng); }

    bool chance(int percent) { return u(0, 99) < static_cast<std::uint32_t>(percent); }

    // A blend factor: in range, the ends, out of range, NaN, tiny.
    float t_value() {
        switch (u(0, 9)) {
            case 0: return 0.0f;
            case 1: return 1.0f;
            case 2: return 1.5f;
            case 3: return -0.25f;
            case 4: return std::numeric_limits<float>::quiet_NaN();
            case 5: return 1.0e-30f;
            default: return f(0.0f, 1.0f);
        }
    }

    // A key: `quats` quaternions, then scale and translation records naming joints up to a few
    // past `joints` (and, for translations, some "negative" ones).
    std::uint32_t frame(std::size_t joints, std::size_t quats) {
        const std::size_t scales = u(0, 6);
        const std::size_t trans = u(0, 8);
        const std::size_t qb = 8 * quats;
        const std::size_t to = qb + 8 * scales;
        const std::size_t qwc = (to + 8 * trans + 15) >> 4;
        const std::uint32_t at = alloc(0x10 + 16 * qwc + 16);
        put<float>(at, f(0.1f, 8.0f));
        put<std::int16_t>(at + 4, static_cast<std::int16_t>(u(0, 400)));
        put<std::uint16_t>(at + 6, static_cast<std::uint16_t>(qwc));
        put<std::uint16_t>(at + 8, static_cast<std::uint16_t>(qb));
        put<std::uint16_t>(at + 10, static_cast<std::uint16_t>(scales));
        put<std::uint16_t>(at + 12, static_cast<std::uint16_t>(to));
        put<std::uint16_t>(at + 14, static_cast<std::uint16_t>(trans));
        const std::uint32_t p = at + 0x10;
        for (std::size_t q = 0; q < quats; ++q) {
            // Roughly unit quaternions, as the game stores them, and some that are not.
            float v[4] = {f(-1, 1), f(-1, 1), f(-1, 1), f(-1, 1)};
            float n = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
            if (n < 1e-3f || chance(10)) {
                n = 1.0f;
            }
            for (int k = 0; k < 4; ++k) {
                put<std::int16_t>(p + static_cast<std::uint32_t>(8 * q + 2 * k),
                                  static_cast<std::int16_t>(std::lround(v[k] / n * 32767.0f)));
            }
        }
        for (std::size_t s = 0; s < scales; ++s) {
            const std::uint32_t r = p + static_cast<std::uint32_t>(qb + 8 * s);
            for (int k = 0; k < 3; ++k) {
                put<std::uint16_t>(r + 2 * static_cast<std::uint32_t>(k), static_cast<std::uint16_t>(u(0x0400, 0x2000)));
            }
            put<std::uint8_t>(r + 6, static_cast<std::uint8_t>(chance(5) ? 200 : u(0, static_cast<std::uint32_t>(joints + 2))));
            put<std::uint8_t>(r + 7, static_cast<std::uint8_t>(chance(50) ? 0x80 : 0));
        }
        for (std::size_t s = 0; s < trans; ++s) {
            const std::uint32_t r = p + static_cast<std::uint32_t>(to + 8 * s);
            for (int k = 0; k < 3; ++k) {
                put<std::int16_t>(r + 2 * static_cast<std::uint32_t>(k), static_cast<std::int16_t>(u(0, 2000)) - 1000);
            }
            put<std::uint8_t>(r + 6, static_cast<std::uint8_t>(chance(5) ? 0xF0 : u(0, static_cast<std::uint32_t>(joints + 2))));
        }
        return at;
    }

    // A key pointer of every kind: none, a real key, an all-zero slot, a key whose sizes disagree,
    // a key running past the memory's end.
    std::uint32_t key(std::size_t joints) {
        const std::uint32_t kind = m_valid_keys ? 19 : u(0, 19);
        if (kind == 0) {
            return 0;
        }
        if (kind == 1) {
            return alloc(0x40);  // all zero: no quaternions
        }
        if (kind == 2) {
            const std::uint32_t at = frame(joints, joints);
            put<std::uint16_t>(at + 6, 0x7777);  // qwc disagrees
            return at;
        }
        if (kind == 3) {
            return static_cast<std::uint32_t>(m_bytes.size() - 0x18);  // payload past the end
        }
        // Mostly one quaternion per joint; sometimes fewer (the class reads on into what follows)
        // or more.
        std::size_t quats = joints;
        if (chance(15)) {
            quats = u(1, static_cast<std::uint32_t>(joints));
        } else if (chance(10)) {
            quats = joints + u(1, 3);
        }
        return frame(joints, quats > 0x70 ? 0x70 : quats);
    }

    // A class of `joints` joints: inverse bind matrices, rest translations, parent words.
    std::uint32_t make_class(std::size_t joints) {
        const std::uint32_t cls = alloc(0x50);
        const std::uint32_t sk = alloc(0x40 * joints);
        const std::uint32_t ct = alloc(0x10 * joints);
        put<std::uint8_t>(cls + 0x08, static_cast<std::uint8_t>(joints));
        put<std::uint32_t>(cls + 0x14, sk);
        put<std::uint32_t>(cls + 0x18, ct);
        for (std::size_t j = 0; j < joints; ++j) {
            for (int e = 0; e < 16; ++e) {
                put<float>(sk + static_cast<std::uint32_t>(0x40 * j + 4 * e), (e % 5 == 0) ? f(0.5f, 1.5f) : f(-0.5f, 0.5f));
            }
            const std::uint32_t r = ct + static_cast<std::uint32_t>(0x10 * j);
            put<float>(r, f(-200, 200));
            put<float>(r + 4, f(-200, 200));
            put<float>(r + 8, f(-200, 200));
            std::uint32_t word = 0;
            if (j > 0) {
                const std::uint32_t kind = u(0, 29);
                if (kind == 0) {
                    word = 0x70000000u + 0x40u * u(static_cast<std::uint32_t>(j), 0x75);  // a later or absent record
                } else if (kind == 1) {
                    word = 0x70000004u;  // not a record
                } else if (kind == 2) {
                    word = 0;  // another root
                } else {
                    word = 0x70000000u + 0x40u * u(0, static_cast<std::uint32_t>(j - 1));
                }
            }
            put<std::uint32_t>(r + 0xC, word);
        }
        return cls;
    }

    // A joint-modifier list of `count` nodes (sometimes looping on itself).
    std::uint32_t modifiers(std::size_t joints, std::size_t count) {
        bool any_invalid = false;
        std::uint32_t head = 0;
        std::uint32_t prev = 0;
        for (std::size_t i = 0; i < count; ++i) {
            const std::uint32_t node = alloc(0x40);
            put<std::uint8_t>(node + 3, static_cast<std::uint8_t>(chance(50) ? 0 : u(1, 3)));
            const std::uint32_t joint = chance(5) ? 0x7A : u(0, static_cast<std::uint32_t>(joints));
            const bool not_a_record = chance(5);
            any_invalid = any_invalid || not_a_record;
            put<std::uint32_t>(node + 4, not_a_record ? 0x12345678u : 0x70000000u + 0x40u * joint);
            put<float>(node + 0xC, f(0, 1));
            float q[4] = {f(-1, 1), f(-1, 1), f(-1, 1), f(-1, 1)};
            for (int k = 0; k < 4; ++k) {
                put<float>(node + 0x10 + 4 * static_cast<std::uint32_t>(k), q[k]);
            }
            for (int k = 0; k < 3; ++k) {
                put<float>(node + 0x20 + 4 * static_cast<std::uint32_t>(k), chance(70) ? 1.0f : f(0.5f, 2.0f));
                put<float>(node + 0x30 + 4 * static_cast<std::uint32_t>(k), f(-50, 50));
            }
            if (prev != 0) {
                put<std::uint32_t>(prev + 8, node);
            } else {
                head = node;
            }
            prev = node;
        }
        // A loop: the reader stops at 32 modifiers. Only of nodes that all count: the reader skips a
        // node that names no record without counting it, so a loop of such nodes would never end
        // (in the reader as it is and always was; the game links no loops).
        if (prev != 0 && !any_invalid && chance(10)) {
            put<std::uint32_t>(prev + 8, head);
        }
        return head;
    }

    // A pose-layer list of `count` nodes.
    std::uint32_t layers(std::size_t joints, std::size_t count) {
        std::uint32_t head = 0;
        std::uint32_t prev = 0;
        for (std::size_t i = 0; i < count; ++i) {
            const std::uint32_t node = alloc(0x40);
            const std::uint32_t n = chance(5) ? 0 : u(1, static_cast<std::uint32_t>(joints < 12 ? joints : 12));
            const std::uint32_t list = alloc(n + 1);
            for (std::uint32_t k = 0; k < n; ++k) {
                put<std::uint8_t>(list + k, static_cast<std::uint8_t>(u(0, static_cast<std::uint32_t>(joints))));
            }
            put<std::uint16_t>(node + 2, static_cast<std::uint16_t>(n));
            put<float>(node + 8, chance(10) ? 0.0f : f(0, 1));
            put<std::uint32_t>(node + 0x0C, list);
            const std::uint8_t fr = static_cast<std::uint8_t>(u(0, 5));
            put<std::uint8_t>(node + 0x20, fr);
            put<std::uint8_t>(node + 0x21, static_cast<std::uint8_t>(chance(60) ? fr + 1 : u(0, 5)));
            const std::uint8_t sq = static_cast<std::uint8_t>(u(0, 3));
            put<std::uint8_t>(node + 0x22, sq);
            put<std::uint8_t>(node + 0x23, static_cast<std::uint8_t>(chance(70) ? sq : u(0, 3)));
            put<float>(node + 0x24, t_value());
            put<std::uint32_t>(node + 0x38, key(joints));
            put<std::uint32_t>(node + 0x3C, key(joints));
            if (prev != 0) {
                put<std::uint32_t>(prev + 0x1C, node);
            } else {
                head = node;
            }
            prev = node;
        }
        return head;
    }

    // A moby of class `cls` (joints `joints`) at a new address.
    std::uint32_t moby(std::uint32_t cls, std::size_t joints) {
        const std::uint32_t m = alloc(0x100);
        animate(m, cls, joints);
        return m;
    }

    // Writes a moby's animation fields anew (a transition: new keys, blend, layers, modifiers).
    void animate(std::uint32_t m, std::uint32_t cls, std::size_t joints) {
        put<std::uint32_t>(m + 0x24, cls);
        const std::uint8_t fr = static_cast<std::uint8_t>(u(0, 8));
        put<std::uint8_t>(m + 0x50, fr);
        put<std::uint8_t>(m + 0x51, static_cast<std::uint8_t>(chance(60) ? fr + 1 : u(0, 8)));
        const std::uint8_t sq = static_cast<std::uint8_t>(u(0, 4));
        put<std::uint8_t>(m + 0x52, sq);
        put<std::uint8_t>(m + 0x53, static_cast<std::uint8_t>(chance(70) ? sq : u(0, 4)));
        put<float>(m + 0x54, t_value());
        put<std::uint32_t>(m + 0x60, chance(25) ? layers(joints, u(1, 8)) : 0);
        put<std::uint32_t>(m + 0x64, chance(30) ? modifiers(joints, u(1, 6)) : 0);
        put<std::uint32_t>(m + 0x68, key(joints));
        put<std::uint32_t>(m + 0x6c, key(joints));
    }

private:
    std::vector<std::uint8_t> m_bytes;
    std::mt19937 m_rng;
    std::uint32_t m_next = 0x1000;
    bool m_valid_keys = false;
};

}  // namespace openrac::test::pose_ram
