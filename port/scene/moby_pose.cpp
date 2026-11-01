// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "scene/moby_pose.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <span>

namespace openrac::scene {

namespace rac1 = assets::rac1;

namespace {

constexpr std::uint32_t kAddressMask = 0x01FFFFFF;
// More joints than the scratchpad holds (0x6f) is not a class.
constexpr std::size_t kMaxJoints = 0x70;

template <typename T>
T at(std::span<const std::uint8_t> ram, std::uint32_t address) {
    T value{};
    address &= kAddressMask;
    if (address + sizeof(T) <= ram.size()) {
        std::memcpy(&value, ram.data() + address, sizeof(T));
    }
    return value;
}

bool inside(std::span<const std::uint8_t> ram, std::uint32_t address, std::size_t bytes) {
    address &= kAddressMask;
    return address != 0 && address + bytes <= ram.size();
}

// A key where the moby points; none when there is no pointer or what is there is not a
// frame (a blend's snapshot slot the port never filled is all zero: no quaternions).
std::optional<rac1::MobyFrame> key_at(
    std::span<const std::uint8_t> ram, std::uint32_t address, std::size_t joints
) {
    if (!inside(ram, address, 0x10)) {
        return std::nullopt;
    }
    try {
        rac1::MobyFrame f = rac1::parse_frame(assets::ByteView(ram), address & kAddressMask);
        if (f.header.quat_bytes == 0 || f.header.quat_bytes > 8 * kMaxJoints || joints == 0) {
            return std::nullopt;
        }
        return f;
    } catch (const assets::AssetError&) {
        return std::nullopt;
    }
}

}  // namespace

namespace {

std::vector<rac1::JointMatrix> evaluate_moby(std::span<const std::uint8_t> ram, std::uint32_t moby, bool bind);

}  // namespace

std::vector<rac1::JointMatrix> moby_palette(std::span<const std::uint8_t> ram, std::uint32_t moby) {
    return evaluate_moby(ram, moby, true);
}

std::vector<rac1::JointMatrix> moby_pose_matrices(std::span<const std::uint8_t> ram, std::uint32_t moby) {
    return evaluate_moby(ram, moby, false);
}

std::vector<rac1::JointMatrix> moby_palette_reference(std::span<const std::uint8_t> ram, std::uint32_t moby) {
    return evaluate_moby(ram, moby, true);
}

bool pose_reference() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_POSE_REFERENCE");
        return v != nullptr && v[0] == '1' && v[1] == '\0';
    }();
    return on;
}

int pose_verify_every() {
    static const int n = [] {
        const char* v = std::getenv("OPENRAC_POSE_VERIFY");
        const int asked = v != nullptr ? std::atoi(v) : 0;
        return asked <= 0 ? 0 : (asked > 3600 ? 3600 : asked);
    }();
    return n;
}

namespace {

std::vector<rac1::JointMatrix> evaluate_moby(std::span<const std::uint8_t> ram, std::uint32_t moby, bool bind) {
    const auto cls = at<std::uint32_t>(ram, moby + 0x24);
    if (!inside(ram, cls, 0x48)) {
        return {};
    }
    const std::size_t joints = at<std::uint8_t>(ram, cls + 0x08);
    const auto skeleton = at<std::uint32_t>(ram, cls + 0x14);
    const auto common_trans = at<std::uint32_t>(ram, cls + 0x18);
    if (joints == 0 || joints > kMaxJoints || !inside(ram, skeleton, 0x40 * joints)
        || !inside(ram, common_trans, 0x10 * joints)) {
        return {};
    }

    // The keys and the blend (moby +0x50..+0x6c).
    const auto frame_a = at<std::uint8_t>(ram, moby + 0x50);
    const auto frame_b = at<std::uint8_t>(ram, moby + 0x51);
    const auto seq_a = at<std::uint8_t>(ram, moby + 0x52);
    const auto seq_b = at<std::uint8_t>(ram, moby + 0x53);
    float t = at<float>(ram, moby + 0x54);
    std::optional<rac1::MobyFrame> a = key_at(ram, at<std::uint32_t>(ram, moby + 0x68), joints);
    std::optional<rac1::MobyFrame> b = key_at(ram, at<std::uint32_t>(ram, moby + 0x6c), joints);
    bool plain = seq_a == seq_b && frame_b == frame_a + 1;
    if (!a && b) {
        // Key A missing (a snapshot the port has not written): the target key alone.
        a = std::move(b);
        b.reset();
        t = 0.0f;
    } else if (a && !b) {
        t = 0.0f;
    }
    if (!a) {
        return {};
    }
    if (!(t > 0.0f && t <= 1.0f)) {
        t = 0.0f;
    }

    // The class as MobyAnimEval reads it: the inverse bind matrices, the rest translations and
    // the parent words (0x70000000 + 0x40 * parent, the scratchpad record of the parent).
    rac1::MobyAnimClass anim;
    anim.joint_count = joints;
    anim.skeleton.resize(joints);
    anim.rest.resize(joints);
    anim.parent_word.resize(joints);
    for (std::size_t j = 0; j < joints; ++j) {
        for (std::size_t r = 0; r < 4; ++r) {
            for (std::size_t c = 0; c < 4; ++c) {
                anim.skeleton[j][r][c] = bind ? at<float>(
                    ram, skeleton + static_cast<std::uint32_t>(0x40 * j + 0x10 * r + 4 * c)
                ) : rac1::kIdentityJoint[r][c];
            }
        }
        const auto rec = common_trans + static_cast<std::uint32_t>(0x10 * j);
        anim.rest[j] = {at<float>(ram, rec), at<float>(ram, rec + 4), at<float>(ram, rec + 8)};
        anim.parent_word[j] = at<std::uint32_t>(ram, rec + 0xC);
    }
    // The joint-modifier list (moby +0x64, each node's next at +8): what spins a vendor's
    // hologram and its logo's text, turns an NPC's head. Its order is the game's, head first.
    std::vector<rac1::JointModifier> mods;
    for (std::uint32_t node = at<std::uint32_t>(ram, moby + 0x64); inside(ram, node, 0x40) && mods.size() < 32;
         node = at<std::uint32_t>(ram, node + 8)) {
        const std::uint32_t record = at<std::uint32_t>(ram, node + 4);
        if (record < 0x70000000u || ((record - 0x70000000u) & 0x3F) != 0) {
            continue;
        }
        rac1::JointModifier m;
        m.joint = static_cast<std::uint8_t>((record - 0x70000000u) / 0x40);
        m.mode = at<std::uint8_t>(ram, node + 3);
        m.weight = at<float>(ram, node + 0xC);
        for (std::size_t k = 0; k < 4; ++k) {
            m.quat[k] = at<float>(ram, node + 0x10 + static_cast<std::uint32_t>(4 * k));
        }
        for (std::size_t k = 0; k < 3; ++k) {
            m.scale[k] = at<float>(ram, node + 0x20 + static_cast<std::uint32_t>(4 * k));
            m.trans[k] = at<float>(ram, node + 0x30 + static_cast<std::uint32_t>(4 * k));
        }
        mods.push_back(m);
    }
    // The pose layers (moby +0x60, each node's next at +0x1C, PAL func_L00_00250060 links them in
    // front): Ratchet's weapon arm, its keys at +0x38 / +0x3C ready in his joint layout. Without them
    // his arms kept the unarmed idle and a gun hung from his lowered hand.
    std::vector<rac1::MobyFrame> layer_keys;
    layer_keys.reserve(16);
    std::vector<rac1::PoseLayer> layers;
    for (std::uint32_t node = at<std::uint32_t>(ram, moby + 0x60); inside(ram, node, 0x40) && layers.size() < 6;
         node = at<std::uint32_t>(ram, node + 0x1C)) {
        const auto count = at<std::uint16_t>(ram, node + 2);
        const auto list = at<std::uint32_t>(ram, node + 0x0C);
        if (count == 0 || count > kMaxJoints || !inside(ram, list, count)) {
            continue;
        }
        rac1::PoseLayer l;
        l.joints.assign(ram.begin() + list, ram.begin() + list + count);
        l.weight = at<float>(ram, node + 0x08);
        l.t = at<float>(ram, node + 0x24);
        if (!(l.t > 0.0f && l.t <= 1.0f)) {
            l.t = 0.0f;
        }
        l.plain = at<std::uint8_t>(ram, node + 0x22) == at<std::uint8_t>(ram, node + 0x23)
                  && at<std::uint8_t>(ram, node + 0x21) == at<std::uint8_t>(ram, node + 0x20) + 1;
        auto ka = key_at(ram, at<std::uint32_t>(ram, node + 0x38), joints);
        auto kb = l.t != 0.0f ? key_at(ram, at<std::uint32_t>(ram, node + 0x3C), joints) : std::nullopt;
        if (!ka || (l.t != 0.0f && !kb)) {
            continue;
        }
        layer_keys.push_back(std::move(*ka));
        l.a = &layer_keys.back();
        if (kb) {
            layer_keys.push_back(std::move(*kb));
            l.b = &layer_keys.back();
        }
        layers.push_back(std::move(l));
    }
    return rac1::evaluate_keys(anim, &*a, b ? &*b : nullptr, t, plain, mods, layers);
}

// The temporaries of one evaluation (moby_palette_into): evaluate_moby's, with their storage kept.
// Bounded: at most 2 keys, 6 layers with 12 keys, 32 modifiers, 0x70 joints; a key whose arrays
// came out larger than kKeepBytes is let go after the call (trim), so one odd frame is not held.
struct PoseScratch {
    rac1::MobyAnimClass anim;
    std::vector<rac1::JointModifier> mods;
    std::array<rac1::MobyFrame, 2> keys;
    std::array<rac1::MobyFrame, 12> layer_keys;
    std::array<rac1::PoseLayer, 6> layers;
    rac1::EvalScratch eval;
    bool busy = false;

    static constexpr std::size_t kKeepBytes = 16 * 1024;

    template <typename T>
    static void trim(std::vector<T>& v) {
        if (v.capacity() * sizeof(T) > kKeepBytes) {
            std::vector<T>().swap(v);
        }
    }

    static void trim(rac1::MobyFrame& f) {
        trim(f.quats);
        trim(f.scales);
        trim(f.trans);
        trim(f.payload);
    }

    void trim() {
        for (auto& f : keys) {
            trim(f);
        }
        for (auto& f : layer_keys) {
            trim(f);
        }
        trim(eval.post_list);
        trim(eval.post_a_order);
        trim(eval.post_b_order);
    }
};

// key_at into `f`: whether a key is there (when not, `f` holds nothing to use).
bool key_into(std::span<const std::uint8_t> ram, std::uint32_t address, std::size_t joints, rac1::MobyFrame& f) {
    if (!inside(ram, address, 0x10)) {
        return false;
    }
    try {
        rac1::parse_frame_into(assets::ByteView(ram), address & kAddressMask, f);
        return !(f.header.quat_bytes == 0 || f.header.quat_bytes > 8 * kMaxJoints || joints == 0);
    } catch (const assets::AssetError&) {
        return false;
    }
}

// evaluate_moby with `s` for its temporaries, into `out`: the same reads in the same order, the
// same checks, the same arithmetic (evaluate_keys_into). Every field of every temporary it uses
// is written before it is read.
void evaluate_moby_into(
    std::span<const std::uint8_t> ram, std::uint32_t moby, bool bind, PoseScratch& s,
    std::vector<rac1::JointMatrix>& out
) {
    out.clear();
    const auto cls = at<std::uint32_t>(ram, moby + 0x24);
    if (!inside(ram, cls, 0x48)) {
        return;
    }
    const std::size_t joints = at<std::uint8_t>(ram, cls + 0x08);
    const auto skeleton = at<std::uint32_t>(ram, cls + 0x14);
    const auto common_trans = at<std::uint32_t>(ram, cls + 0x18);
    if (joints == 0 || joints > kMaxJoints || !inside(ram, skeleton, 0x40 * joints)
        || !inside(ram, common_trans, 0x10 * joints)) {
        return;
    }

    const auto frame_a = at<std::uint8_t>(ram, moby + 0x50);
    const auto frame_b = at<std::uint8_t>(ram, moby + 0x51);
    const auto seq_a = at<std::uint8_t>(ram, moby + 0x52);
    const auto seq_b = at<std::uint8_t>(ram, moby + 0x53);
    float t = at<float>(ram, moby + 0x54);
    const rac1::MobyFrame* a = key_into(ram, at<std::uint32_t>(ram, moby + 0x68), joints, s.keys[0]) ? &s.keys[0] : nullptr;
    const rac1::MobyFrame* b = key_into(ram, at<std::uint32_t>(ram, moby + 0x6c), joints, s.keys[1]) ? &s.keys[1] : nullptr;
    const bool plain = seq_a == seq_b && frame_b == frame_a + 1;
    if (a == nullptr && b != nullptr) {
        a = b;
        b = nullptr;
        t = 0.0f;
    } else if (a != nullptr && b == nullptr) {
        t = 0.0f;
    }
    if (a == nullptr) {
        return;
    }
    if (!(t > 0.0f && t <= 1.0f)) {
        t = 0.0f;
    }

    rac1::MobyAnimClass& anim = s.anim;
    anim.joint_count = joints;
    anim.skeleton.resize(joints);
    anim.rest.resize(joints);
    anim.parent_word.resize(joints);
    for (std::size_t j = 0; j < joints; ++j) {
        for (std::size_t r = 0; r < 4; ++r) {
            for (std::size_t c = 0; c < 4; ++c) {
                anim.skeleton[j][r][c] = bind ? at<float>(
                    ram, skeleton + static_cast<std::uint32_t>(0x40 * j + 0x10 * r + 4 * c)
                ) : rac1::kIdentityJoint[r][c];
            }
        }
        const auto rec = common_trans + static_cast<std::uint32_t>(0x10 * j);
        anim.rest[j] = {at<float>(ram, rec), at<float>(ram, rec + 4), at<float>(ram, rec + 8)};
        anim.parent_word[j] = at<std::uint32_t>(ram, rec + 0xC);
    }

    std::vector<rac1::JointModifier>& mods = s.mods;
    mods.clear();
    for (std::uint32_t node = at<std::uint32_t>(ram, moby + 0x64); inside(ram, node, 0x40) && mods.size() < 32;
         node = at<std::uint32_t>(ram, node + 8)) {
        const std::uint32_t record = at<std::uint32_t>(ram, node + 4);
        if (record < 0x70000000u || ((record - 0x70000000u) & 0x3F) != 0) {
            continue;
        }
        rac1::JointModifier m;
        m.joint = static_cast<std::uint8_t>((record - 0x70000000u) / 0x40);
        m.mode = at<std::uint8_t>(ram, node + 3);
        m.weight = at<float>(ram, node + 0xC);
        for (std::size_t k = 0; k < 4; ++k) {
            m.quat[k] = at<float>(ram, node + 0x10 + static_cast<std::uint32_t>(4 * k));
        }
        for (std::size_t k = 0; k < 3; ++k) {
            m.scale[k] = at<float>(ram, node + 0x20 + static_cast<std::uint32_t>(4 * k));
            m.trans[k] = at<float>(ram, node + 0x30 + static_cast<std::uint32_t>(4 * k));
        }
        mods.push_back(m);
    }

    // The pose layers: a layer is filled in its slot and kept (count + 1) only when its keys are
    // there, as evaluate_moby pushes it; a layer's keys take the next free key slots.
    std::size_t layer_count = 0;
    std::size_t key_count = 0;
    for (std::uint32_t node = at<std::uint32_t>(ram, moby + 0x60); inside(ram, node, 0x40) && layer_count < s.layers.size();
         node = at<std::uint32_t>(ram, node + 0x1C)) {
        const auto count = at<std::uint16_t>(ram, node + 2);
        const auto list = at<std::uint32_t>(ram, node + 0x0C);
        if (count == 0 || count > kMaxJoints || !inside(ram, list, count)) {
            continue;
        }
        rac1::PoseLayer& l = s.layers[layer_count];
        l.joints.assign(ram.begin() + list, ram.begin() + list + count);
        l.weight = at<float>(ram, node + 0x08);
        l.t = at<float>(ram, node + 0x24);
        if (!(l.t > 0.0f && l.t <= 1.0f)) {
            l.t = 0.0f;
        }
        l.plain = at<std::uint8_t>(ram, node + 0x22) == at<std::uint8_t>(ram, node + 0x23)
                  && at<std::uint8_t>(ram, node + 0x21) == at<std::uint8_t>(ram, node + 0x20) + 1;
        rac1::MobyFrame& ka = s.layer_keys[key_count];
        rac1::MobyFrame& kb = s.layer_keys[key_count + 1];
        const bool has_a = key_into(ram, at<std::uint32_t>(ram, node + 0x38), joints, ka);
        const bool has_b = l.t != 0.0f && key_into(ram, at<std::uint32_t>(ram, node + 0x3C), joints, kb);
        if (!has_a || (l.t != 0.0f && !has_b)) {
            continue;
        }
        l.a = &ka;
        l.b = has_b ? &kb : nullptr;
        key_count += has_b ? 2 : 1;
        ++layer_count;
    }
    rac1::evaluate_keys_into(
        anim, a, b, t, plain, mods, std::span<const rac1::PoseLayer>(s.layers.data(), layer_count), s.eval, out
    );
}

}  // namespace

bool moby_palette_into(
    std::span<const std::uint8_t> ram, std::uint32_t moby, std::vector<rac1::JointMatrix>& out
) {
    if (pose_reference()) {
        out = evaluate_moby(ram, moby, true);
        return !out.empty();
    }
    // This thread's temporaries (made on its first evaluation, freed when it ends).
    static thread_local std::unique_ptr<PoseScratch> mine;
    if (!mine) {
        mine = std::make_unique<PoseScratch>();
    }
    if (mine->busy) {
        // Already evaluating on this thread: temporaries of this call's own.
        const auto own = std::make_unique<PoseScratch>();
        evaluate_moby_into(ram, moby, true, *own, out);
        return !out.empty();
    }
    struct Busy {
        PoseScratch& s;
        explicit Busy(PoseScratch& scratch) : s(scratch) { s.busy = true; }
        ~Busy() {
            s.trim();
            s.busy = false;
        }
        Busy(const Busy&) = delete;
        Busy& operator=(const Busy&) = delete;
    } busy(*mine);
    evaluate_moby_into(ram, moby, true, *mine, out);
    return !out.empty();
}

}  // namespace openrac::scene
