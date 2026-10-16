// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "viewer/moby_pose.h"

#include <cstring>
#include <optional>

namespace openrac::viewer {

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
    return rac1::evaluate_keys(anim, &*a, b ? &*b : nullptr, t, plain, mods);
}

}  // namespace

}  // namespace openrac::viewer
