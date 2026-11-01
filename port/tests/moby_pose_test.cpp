// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// scene/moby_pose.h: moby_palette_into (temporaries kept per thread) gives moby_palette_reference's
// palettes bit for bit, over synthetic game memory (moby_pose_ram.h): many classes and joint counts
// one after another on the same thread (so every temporary is reused dirty), keys missing or
// malformed, blends, pose layers, joint modifiers, a moby rewritten between calls (a transition, a
// class moved, a slot reused), and four threads at once. Also: evaluate_keys_into with a dirty
// scratch against evaluate_keys, and how many allocations each path makes.

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <thread>
#include <vector>

#include "scene/moby_pose.h"
#include "tests/check.h"
#include "tests/moby_pose_ram.h"

namespace {

std::atomic<std::uint64_t> g_allocations{0};

}  // namespace

// Counts this program's allocations (a test-only replacement of the global operator new; the
// matching deletes free what it took from malloc, which GCC's heuristic cannot tell).
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
void* operator new(std::size_t size) {
    g_allocations.fetch_add(1, std::memory_order_relaxed);
    if (void* p = std::malloc(size != 0 ? size : 1)) {
        return p;
    }
    throw std::bad_alloc();
}

void operator delete(void* p) noexcept {
    std::free(p);
}

void operator delete(void* p, std::size_t) noexcept {
    std::free(p);
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace {

namespace rac1 = openrac::assets::rac1;
namespace scene = openrac::scene;
using openrac::test::pose_ram::Ram;

bool same(const std::vector<rac1::JointMatrix>& a, const std::vector<rac1::JointMatrix>& b) {
    return a.size() == b.size()
           && (a.empty() || std::memcmp(a.data(), b.data(), a.size() * sizeof(a[0])) == 0);
}

std::size_t random_joints(Ram& r) {
    switch (r.u(0, 9)) {
        case 0: return 1;
        case 1: return 0x6f;
        case 2: return 0x70;
        default: return r.u(2, 40);
    }
}

// Many mobys of many classes, each checked as it is made.
void random_mobys() {
    auto r = std::make_unique<Ram>(32u << 20, 1234u);
    std::uint32_t seed = 1234u;
    std::vector<rac1::JointMatrix> out;
    int nonempty = 0;
    int differ = 0;
    for (int i = 0; i < 6000; ++i) {
        if (r->nearly_full()) {
            r = std::make_unique<Ram>(32u << 20, ++seed);
        }
        Ram& rr = *r;
        const std::size_t joints = random_joints(rr);
        const std::uint32_t cls = rr.make_class(joints);
        const std::uint32_t m = rr.moby(cls, joints);
        const auto want = scene::moby_palette_reference(rr.bytes(), m);
        const bool got = scene::moby_palette_into(rr.bytes(), m, out);
        CHECK(got == !want.empty());
        if (!same(want, out)) {
            ++differ;
        }
        CHECK(same(want, scene::moby_palette(rr.bytes(), m)));
        nonempty += want.empty() ? 0 : 1;
    }
    CHECK(differ == 0);
    // Enough of them real palettes for the comparison to mean something.
    CHECK(nonempty > 3000);
    std::printf("random mobys: 6000 checked, %d with a palette, %d differ\n", nonempty, differ);
}

// One moby slot rewritten between calls: new keys and blends, layers and modifiers coming and
// going, its class swapped for one of another joint count at the same address, the class
// pointer emptied and restored, the key bytes changed under the same pointer.
void transitions() {
    Ram r(32u << 20, 99u);
    std::vector<rac1::JointMatrix> out;
    int differ = 0;
    std::size_t joints = 24;
    std::uint32_t cls = r.make_class(joints);
    const std::uint32_t m = r.moby(cls, joints);
    int steps = 0;
    for (int step = 0; step < 3000 && !r.nearly_full(); ++step, ++steps) {
        switch (r.u(0, 5)) {
            case 0: r.animate(m, cls, joints); break;
            case 1: {
                // A class of another size where the old one was (a reused class slot).
                joints = random_joints(r);
                const std::uint32_t fresh = r.make_class(joints);
                std::memcpy(r.bytes().data() + cls, r.bytes().data() + fresh, 0x50);
                r.animate(m, cls, joints);
                break;
            }
            case 2: r.put<std::uint32_t>(m + 0x24, 0); break;
            case 3: r.put<std::uint32_t>(m + 0x24, cls); break;
            case 4: r.put<float>(m + 0x54, r.t_value()); break;
            default: {
                // The bytes of key A changed under the same pointer.
                std::uint32_t key = 0;
                std::memcpy(&key, r.bytes().data() + m + 0x68, 4);
                if (key != 0 && key + 0x20 < r.bytes().size()) {
                    r.put<std::int16_t>(key + 0x10 + 2 * r.u(0, 3), static_cast<std::int16_t>(r.u(0, 60000)));
                }
                break;
            }
        }
        const auto want = scene::moby_palette_reference(r.bytes(), m);
        scene::moby_palette_into(r.bytes(), m, out);
        if (!same(want, out)) {
            ++differ;
        }
    }
    CHECK(differ == 0);
    CHECK(steps > 1000);
    std::printf("transitions: %d steps, %d differ\n", steps, differ);
}

// Four threads at once, each with its own temporaries, against answers worked out first.
void threads() {
    Ram r(32u << 20, 7u);
    std::vector<std::uint32_t> mobys;
    std::vector<std::vector<rac1::JointMatrix>> want;
    // Built in one go and kept (no wrap-around: 400 mobys fit).
    for (int i = 0; i < 400; ++i) {
        const std::size_t joints = r.u(1, 30);
        mobys.push_back(r.moby(r.make_class(joints), joints));
    }
    for (std::uint32_t m : mobys) {
        want.push_back(scene::moby_palette_reference(r.bytes(), m));
    }
    std::atomic<int> differ{0};
    std::vector<std::thread> pool;
    for (int t = 0; t < 4; ++t) {
        pool.emplace_back([&, t] {
            std::vector<rac1::JointMatrix> out;
            for (int round = 0; round < 20; ++round) {
                for (std::size_t i = static_cast<std::size_t>(t); i < mobys.size(); i += 3) {
                    scene::moby_palette_into(r.bytes(), mobys[i], out);
                    if (!same(want[i], out)) {
                        differ.fetch_add(1);
                    }
                }
            }
        });
    }
    for (auto& th : pool) {
        th.join();
    }
    CHECK(differ.load() == 0);
}

// evaluate_keys_into with one scratch reused across classes against evaluate_keys (new temporaries).
void keys_into() {
    Ram r(32u << 20, 4242u);
    rac1::EvalScratch scratch;
    std::vector<rac1::JointMatrix> out;
    int differ = 0;
    for (int i = 0; i < 3000 && !r.nearly_full(); ++i) {
        const std::size_t joints = i % 50 == 0 ? 0 : random_joints(r);
        rac1::MobyAnimClass anim;
        anim.joint_count = joints;
        anim.skeleton.resize(joints);
        anim.rest.resize(joints);
        anim.parent_word.resize(joints);
        for (std::size_t j = 0; j < joints; ++j) {
            for (auto& row : anim.skeleton[j]) {
                for (auto& x : row) {
                    x = r.f(-1, 1);
                }
            }
            anim.rest[j] = {r.f(-100, 100), r.f(-100, 100), r.f(-100, 100)};
            anim.parent_word[j] = j == 0 ? 0 : 0x70000000u + 0x40u * r.u(0, static_cast<std::uint32_t>(j + 2));
        }
        const std::size_t keyjoints = joints == 0 ? 1 : joints;
        const rac1::MobyFrame a = rac1::parse_frame(openrac::assets::ByteView(r.bytes()), r.frame(keyjoints, keyjoints));
        const rac1::MobyFrame b = rac1::parse_frame(openrac::assets::ByteView(r.bytes()), r.frame(keyjoints, keyjoints));
        const float t = r.t_value();
        const bool plain = r.chance(50);
        const rac1::MobyFrame* pb = r.chance(10) ? nullptr : &b;
        const auto want = rac1::evaluate_keys(anim, &a, pb, t, plain);
        rac1::evaluate_keys_into(anim, &a, pb, t, plain, {}, {}, scratch, out);
        if (!same(want, out)) {
            ++differ;
        }
        // A frame read into a used one equals a frame read anew.
        rac1::MobyFrame reused = b;
        const std::uint32_t at = r.frame(keyjoints, r.u(1, static_cast<std::uint32_t>(keyjoints)));
        rac1::parse_frame_into(openrac::assets::ByteView(r.bytes()), at, reused);
        const rac1::MobyFrame fresh = rac1::parse_frame(openrac::assets::ByteView(r.bytes()), at);
        CHECK(reused.quats == fresh.quats && reused.scales == fresh.scales && reused.trans == fresh.trans
              && reused.payload == fresh.payload
              && std::memcmp(&reused.header, &fresh.header, sizeof(fresh.header)) == 0);
    }
    CHECK(differ == 0);
    // The same failure for a bad frame either way.
    {
        const std::uint32_t at = r.frame(4, 4);
        r.put<std::uint16_t>(at + 6, 0x7777);
        bool threw_new = false;
        bool threw_into = false;
        try {
            (void)rac1::parse_frame(openrac::assets::ByteView(r.bytes()), at);
        } catch (const openrac::assets::AssetError&) {
            threw_new = true;
        }
        rac1::MobyFrame f;
        try {
            rac1::parse_frame_into(openrac::assets::ByteView(r.bytes()), at, f);
        } catch (const openrac::assets::AssetError&) {
            threw_into = true;
        }
        CHECK(threw_new && threw_into);
    }
    // post_scale_list_into on a used list.
    {
        std::vector<std::uint8_t> out_list{9, 9, 9};
        const std::vector<std::uint8_t> a{3, 1, 4};
        const std::vector<std::uint8_t> b{1, 5, 9, 4};
        rac1::post_scale_list_into(a, b, out_list);
        CHECK(out_list == rac1::post_scale_list(a, b));
    }
}

// Allocations a palette takes on each path, once warmed up (the same moby list over and over).
void allocations() {
    // Readable keys only: a malformed key throws (the exception and its message allocate, on
    // either path), which the game's own keys do not.
    Ram r(32u << 20, 31337u);
    r.valid_keys_only();
    std::vector<std::uint32_t> mobys;
    for (int i = 0; i < 200; ++i) {
        const std::size_t joints = r.u(8, 40);
        mobys.push_back(r.moby(r.make_class(joints), joints));
    }
    std::vector<std::vector<rac1::JointMatrix>> outs(mobys.size());
    for (std::size_t i = 0; i < mobys.size(); ++i) {
        scene::moby_palette_into(r.bytes(), mobys[i], outs[i]);  // warm up
    }
    std::uint64_t before = g_allocations.load();
    for (int round = 0; round < 5; ++round) {
        for (std::size_t i = 0; i < mobys.size(); ++i) {
            scene::moby_palette_into(r.bytes(), mobys[i], outs[i]);
        }
    }
    const double into = static_cast<double>(g_allocations.load() - before) / (5.0 * static_cast<double>(mobys.size()));
    before = g_allocations.load();
    for (int round = 0; round < 5; ++round) {
        for (std::size_t i = 0; i < mobys.size(); ++i) {
            outs[i] = scene::moby_palette_reference(r.bytes(), mobys[i]);
        }
    }
    const double reference = static_cast<double>(g_allocations.load() - before) / (5.0 * static_cast<double>(mobys.size()));
    std::printf("allocations a palette: reference %.2f, reused temporaries %.2f\n", reference, into);
    CHECK(into < 0.01);
    CHECK(reference > 5.0);
}

}  // namespace

int main() {
    keys_into();
    random_mobys();
    transitions();
    threads();
    allocations();
    return openrac::test::result();
}
