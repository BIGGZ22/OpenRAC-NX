// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/occlusion.rs
// and crates/rc-engine/src/occlusion.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The occlusion grid, the load-time resolution and the per-frame mask.
// Addresses are NTSC-U level01.
//
// Implements occlusion.h. It has four parts: the tree lookup over the grid block, the readers of
// the grid, octants and mappings, the load-time step that turns mappings into one bit per
// object, and the per-frame OcclusionState that picks the mask for the camera's cell.

#include "assets/world/occlusion.h"

#include <algorithm>
#include <cstring>
#include <limits>

#include "assets/world/known_games.h"

namespace openrac::assets {

namespace {

// A mask with every bit set: nothing is culled.
OcclusionMask all_visible() {
    OcclusionMask m;
    m.fill(0xff);
    return m;
}

// A u16 at `at`, or nothing when it would run past the block.
std::optional<u16> u16_if(ByteView b, std::size_t at) {
    // A read past the block's end means the tree points outside it, which reads as a miss.
    if (at + 2 > b.size()) {
        return std::nullopt;
    }

    return b.u16_at(at);
}

}  // namespace

// The octant is picked by which side of the centre the camera is on, one bit per axis (x is the
// high bit).
const OcclusionMask& OcclusionOctants::mask_for(const std::array<float, 3>& camera) const {
    const std::size_t i = (0.0f < camera[0] - centre[0] ? 4u : 0u)
                          + (0.0f < camera[1] - centre[1] ? 2u : 0u)
                          + (0.0f < camera[2] - centre[2] ? 1u : 0u);
    return masks[i];
}

// Cells are 4 units wide, so the cell is the position times 0.25, truncated like the game does.
std::array<s32, 3> occlusion_cell_coords(const std::array<float, 3>& position) {
    std::array<s32, 3> c{};

    // Each axis is converted on its own.
    for (std::size_t k = 0; k < 3; ++k) {
        const float v = position[k] * 0.25f;

        // cvt.w.s truncates toward zero and saturates.
        // Below the s32 range or NaN: NaN gives 0, the rest saturate low.
        if (!(v > -2147483648.0f)) {
            c[k] = v != v ? 0 : std::numeric_limits<s32>::min();
        } else if (v >= 2147483648.0f) {
            // Above the s32 range: saturate high.
            c[k] = std::numeric_limits<s32>::max();
        } else {
            // In range: truncate toward zero.
            c[k] = static_cast<s32>(v);
        }
    }

    return c;
}

// Walks the raw block the way the game does: the header gives the Z range, each slot points to
// a Y node and each of those to an X node holding the mask indices. Nodes are {u16 base, u16
// count} followed by u16 slots, and slot offsets are in 4-byte units. Every read is bounds
// checked.
std::optional<u16> lookup_occlusion_mask(ByteView block, s32 x, s32 y, s32 z) {
    const auto z_base = u16_if(block, 4);
    const auto z_count = u16_if(block, 6);

    // A block too short for its header has no cells.
    if (!z_base || !z_count) {
        return std::nullopt;
    }

    z -= *z_base;

    // A Z outside the root's range has no cells.
    if (z < 0 || z >= *z_count) {
        return std::nullopt;
    }

    const auto z_slot = u16_if(block, 8 + static_cast<std::size_t>(z) * 2);

    // A missing or zero Z slot has no cells at this Z.
    if (!z_slot || *z_slot == 0) {
        return std::nullopt;
    }

    const std::size_t y_node = std::size_t{*z_slot} * 4;
    const auto y_base = u16_if(block, y_node);
    const auto y_count = u16_if(block, y_node + 2);

    // A node cut off by the end of the block is a miss.
    if (!y_base || !y_count) {
        return std::nullopt;
    }

    y -= *y_base;

    // A Y outside the slab's range has no cells.
    if (y < 0 || y >= *y_count) {
        return std::nullopt;
    }

    const auto y_slot = u16_if(block, y_node + 4 + static_cast<std::size_t>(y) * 2);

    // A missing or zero Y slot has no cells at this Y.
    if (!y_slot || *y_slot == 0) {
        return std::nullopt;
    }

    const std::size_t x_node = std::size_t{*y_slot} * 4;
    const auto x_base = u16_if(block, x_node);
    const auto x_count = u16_if(block, x_node + 2);

    // A node cut off by the end of the block is a miss.
    if (!x_base || !x_count) {
        return std::nullopt;
    }

    x -= *x_base;

    // An X outside the row's range has no cell.
    if (x < 0 || x >= *x_count) {
        return std::nullopt;
    }

    const auto mask = u16_if(block, x_node + 4 + static_cast<std::size_t>(x) * 2);

    // A mask index of 0xffff is the file's "no mask here".
    if (!mask || *mask == 0xffff) {
        return std::nullopt;
    }

    return mask;
}

// See lookup_occlusion_mask.
std::optional<u16> OcclusionGrid::lookup(s32 x, s32 y, s32 z) const {
    return lookup_occlusion_mask(raw, x, y, z);
}

// The cell holding `position`, with its mask index, or nothing when the cell has none.
std::optional<OcclusionCell> OcclusionGrid::cell_for(const std::array<float, 3>& position) const {
    const auto c = occlusion_cell_coords(position);
    const auto mask = lookup(c[0], c[1], c[2]);

    // The position's cell has no mask.
    if (!mask) {
        return std::nullopt;
    }

    return OcclusionCell{
        static_cast<u16>(c[0]), static_cast<u16>(c[1]), static_cast<u16>(c[2]), *mask
    };
}

// Walks the whole tree into a flat list of cells, reads the mask table, and keeps a copy of the
// raw block for lookup().
OcclusionGrid read_occlusion_grid(Game game, ByteView block) {
    require_world_layout(game, "occlusion grid");
    OcclusionGrid grid;
    grid.masks_offset = block.s32_at(0);
    grid.z_base = block.u16_at(4);
    grid.z_count = block.u16_at(6);

    // The mask table starts after the 8-byte header and must lie inside the block.
    if (grid.masks_offset <= 8 || static_cast<std::size_t>(grid.masks_offset) > block.size()) {
        fail("occlusion: bad mask offset {:#x}", grid.masks_offset);
    }

    // Walk the tree Z, then Y, then X, collecting every cell that has a mask.
    for (u16 zi = 0; zi < grid.z_count; ++zi) {
        const u16 z_slot = block.u16_at(8 + std::size_t{zi} * 2);

        // A zero slot is a Z with no cells.
        if (z_slot == 0) {
            continue;
        }

        const std::size_t y_node = std::size_t{z_slot} * 4;
        const u16 y_base = block.u16_at(y_node);
        const u16 y_count = block.u16_at(y_node + 2);

        // Each Y entry of the slab.
        for (u16 yi = 0; yi < y_count; ++yi) {
            const u16 y_slot = block.u16_at(y_node + 4 + std::size_t{yi} * 2);

            // A zero slot is a Y with no cells.
            if (y_slot == 0) {
                continue;
            }

            const std::size_t x_node = std::size_t{y_slot} * 4;
            const u16 x_base = block.u16_at(x_node);
            const u16 x_count = block.u16_at(x_node + 2);

            // Each X entry of the row.
            for (u16 xi = 0; xi < x_count; ++xi) {
                const u16 mask = block.u16_at(x_node + 4 + std::size_t{xi} * 2);

                // 0xffff marks a cell with no mask.
                if (mask == 0xffff) {
                    continue;
                }

                grid.cells.push_back({
                    static_cast<u16>(x_base + xi),
                    static_cast<u16>(y_base + yi),
                    static_cast<u16>(grid.z_base + zi),
                    mask,
                });
            }
        }
    }

    std::size_t count = 0;

    // The mask table is as long as the highest mask index in use, plus one.
    for (const OcclusionCell& c : grid.cells) {
        count = std::max(count, std::size_t{c.mask} + 1);
    }

    grid.masks = block.read_array<OcclusionMask>(
        static_cast<std::size_t>(grid.masks_offset), count, "occlusion masks"
    );
    grid.raw = block.to_vector();
    return grid;
}

// The octant block is a centre (4 floats) followed by 8 masks.
OcclusionOctants read_occlusion_octants(Game game, ByteView block) {
    require_world_layout(game, "occlusion octants");
    block.check(0, kOcclusionOctantBlockSize, "occlusion octant block");
    OcclusionOctants o;

    // The octant centre is three floats and a pad.
    for (std::size_t k = 0; k < 4; ++k) {
        o.centre[k] = block.f32_at(4 * k);
    }

    // One mask per octant, 8 of them.
    for (std::size_t i = 0; i < 8; ++i) {
        o.masks[i] = block.read<OcclusionMask>(0x10 + i * kOcclusionMaskBytes, "octant mask");
    }

    return o;
}

// Three counts (tfrags, ties, mobys), then the three mapping tables back to back after a 0x10-byte
// header.
OcclusionMappings read_occlusion_mappings(Game game, ByteView section) {
    require_world_layout(game, "occlusion mappings");
    const s32 tfrags = section.s32_at(0);
    const s32 ties = section.s32_at(4);
    const s32 mobys = section.s32_at(8);

    // Negative counts, or a total past the limit, mean a misread header.
    if (tfrags < 0 || ties < 0 || mobys < 0 || s64{tfrags} + ties + mobys > 100'000) {
        fail("occlusion mappings: bad counts {} {} {}", tfrags, ties, mobys);
    }

    const auto nt = static_cast<std::size_t>(tfrags);
    const auto ni = static_cast<std::size_t>(ties);
    const auto nm = static_cast<std::size_t>(mobys);
    OcclusionMappings out;
    // Reads `n` mappings at `at` into one table.
    auto read = [&](std::size_t at, std::size_t n, std::vector<OcclusionMapping>& into) {
        section.check(at, n * 8, "occlusion mappings");

        // Each mapping is two s32 (occlusion id and bit index), 8 bytes.
        for (std::size_t i = 0; i < n; ++i) {
            into.push_back({section.s32_at(at + 8 * i), section.s32_at(at + 8 * i + 4)});
        }
    };
    read(0x10, nt, out.tfrag);
    read(0x10 + nt * 8, ni, out.tie);
    read(0x10 + (nt + ni) * 8, nm, out.moby);
    return out;
}

// The mappings section of a gameplay file, or nothing when the level has none.
std::optional<OcclusionMappings> read_gameplay_occlusion_mappings(const GameplayFile& file) {
    const std::size_t at = file.offset(GameplaySection::OcclusionMappings);

    // A level with no mappings section has none.
    if (at == 0) {
        return std::nullopt;
    }

    return read_occlusion_mappings(file.game(), file.bytes().tail(at, "occlusion mappings"));
}

// Bit index b is bit (b & 7) of byte (b >> 3) of the frame mask.
OcclusionBits OcclusionBits::from_bit(s32 bit) {
    return {static_cast<u16>(((bit >> 3) << 8) | (1 << (bit & 7)))};
}

// A byte past the mask is not visible.
bool OcclusionBits::visible(const OcclusionMask& frame_mask) const {
    return byte() < frame_mask.size() && (frame_mask[byte()] & bit()) != 0;
}

// Tfrags are matched by position; the mapping is stale when its ids no longer agree with the
// tfrags' keys, and then every tfrag is always drawn.
void resolve_tfrag_occlusion(
    const OcclusionMappings& mappings, std::span<const u8> tfrag_keys, LevelOcclusion& out
) {
    bool stale = mappings.tfrag.size() != tfrag_keys.size();

    // The keys are stale as soon as one differs from its mapping, so stop at the first.
    for (std::size_t i = 0; !stale && i < tfrag_keys.size(); ++i) {
        stale = u32{tfrag_keys[i]} != static_cast<u32>(mappings.tfrag[i].occlusion_id);
    }

    out.tfrag_out_of_date = stale;
    out.tfrag.clear();

    // A stale table gives every tfrag the always-visible bits.
    for (std::size_t i = 0; i < tfrag_keys.size(); ++i) {
        out.tfrag.push_back(
            stale ? OcclusionBits::always() : OcclusionBits::from_bit(mappings.tfrag[i].bit_index)
        );
    }
}

// Ties are matched by position when every id agrees, otherwise by looking each id up (the
// order in the file changed); a tie with no mapping is always drawn and counted.
void resolve_tie_occlusion(
    const OcclusionMappings& mappings, std::span<const s32> tie_occlusion_index, LevelOcclusion& out
) {
    bool positional = mappings.tie.size() == tie_occlusion_index.size();

    // Ties are matched by position while every id agrees, compared as s16.
    for (std::size_t i = 0; positional && i < tie_occlusion_index.size(); ++i) {
        positional = static_cast<s16>(tie_occlusion_index[i])
                     == static_cast<s16>(mappings.tie[i].occlusion_id);
    }

    out.ties_positional = positional;
    out.ties_not_found = 0;
    out.tie.clear();

    // Resolve each tie to its bits.
    for (std::size_t i = 0; i < tie_occlusion_index.size(); ++i) {
        // The tables agree position by position, so the bit comes from the same index.
        if (positional) {
            out.tie.push_back(OcclusionBits::from_bit(mappings.tie[i].bit_index));
            continue;
        }

        const u32 key = static_cast<u16>(tie_occlusion_index[i]);
        const auto it = std::find_if(mappings.tie.begin(), mappings.tie.end(), [&](const auto& m) {
            return static_cast<u32>(m.occlusion_id) == key;
        });

        // A tie with no mapping is counted and always drawn.
        if (it == mappings.tie.end()) {
            ++out.ties_not_found;
            out.tie.push_back(OcclusionBits::always());
        } else {
            // Found: the tie's bit comes from its mapping.
            out.tie.push_back(OcclusionBits::from_bit(it->bit_index));
        }
    }
}

// Mobys are matched by their spawn id; one with a non-zero occlusion field, or with no mapping,
// is always drawn.
void resolve_moby_occlusion(
    const OcclusionMappings& mappings, std::span<const MobyInstance> mobys, LevelOcclusion& out
) {
    out.mobys_not_found = 0;
    out.moby.clear();

    // Resolve each moby instance.
    for (const MobyInstance& m : mobys) {
        // A non-zero occlusion field on the instance opts it out of culling.
        if (m.occlusion != 0) {
            out.moby.push_back(OcclusionBits::always());
            continue;
        }

        const s32 key = static_cast<s16>(m.spawn_id);
        const auto it =
            std::find_if(mappings.moby.begin(), mappings.moby.end(), [&](const auto& r) {
                return r.occlusion_id == key;
            });

        // A moby with no mapping is counted and always drawn.
        if (it == mappings.moby.end()) {
            ++out.mobys_not_found;
            out.moby.push_back(OcclusionBits::always());
        } else {
            // Found: the moby's bit comes from its mapping.
            out.moby.push_back(OcclusionBits::from_bit(it->bit_index));
        }
    }
}

// Resolves all three object kinds, or marks all of them always visible when the level has no
// mappings.
LevelOcclusion resolve_level_occlusion(
    const OcclusionMappings* mappings,
    std::span<const u8> tfrag_keys,
    std::span<const s32> tie_occlusion_index,
    std::span<const MobyInstance> mobys
) {
    LevelOcclusion out;

    // A level with no mappings section is never culled.
    if (!mappings) {
        out.tfrag.assign(tfrag_keys.size(), OcclusionBits::always());
        out.tie.assign(tie_occlusion_index.size(), OcclusionBits::always());
        out.moby.assign(mobys.size(), OcclusionBits::always());
        return out;
    }

    resolve_tfrag_occlusion(*mappings, tfrag_keys, out);
    resolve_tie_occlusion(*mappings, tie_occlusion_index, out);
    resolve_moby_occlusion(*mappings, mobys, out);
    return out;
}

// The indices of the objects whose bits are set in `frame_mask`.
std::vector<u32> visible_objects(
    std::span<const OcclusionBits> words, const OcclusionMask& frame_mask
) {
    std::vector<u32> out;

    // Collect every object whose bits are set in the frame mask.
    for (std::size_t i = 0; i < words.size(); ++i) {
        // The object's bit is set in the frame's mask, so it is visible.
        if (words[i].visible(frame_mask)) {
            out.push_back(static_cast<u32>(i));
        }
    }

    return out;
}

// Bit 0x7f:0x80 is the "always" bit every mask has set.
OcclusionMask with_always_bit(OcclusionMask mask) {
    mask[0x7f] |= 0x80;
    return mask;
}

// Starts with everything visible.
OcclusionState::OcclusionState() : m_mask(all_visible()), m_union{} {}

// Called once a frame. Off shows everything, Active rebuilds the mask for the camera, and
// Freeze keeps the last mask. Returns the mask for the frame.
const OcclusionMask& OcclusionState::update(
    const OcclusionGrid* grid,
    OcclusionMode mode,
    OcclusionFallback fallback,
    bool debug_camera,
    const std::array<float, 3>& camera
) {
    // With occlusion off, or no grid, everything is visible.
    if (mode == OcclusionMode::Off || !grid) {
        m_mask = all_visible();
    } else if (mode == OcclusionMode::Active) {
        // Only an active mode rebuilds; a frozen one keeps the last mask.
        build(*grid, fallback, debug_camera, camera);
    }

    // Freeze keeps the mask.
    return m_mask;
}

// Picks the mask: the camera's own cell if it has one, then (as `fallback` says) its neighbours,
// the octants, everything, or the last mask that was used.
void OcclusionState::build(
    const OcclusionGrid& grid,
    OcclusionFallback fallback,
    bool debug_camera,
    const std::array<float, 3>& camera
) {
    const auto [x, y, z] = occlusion_cell_coords(camera);

    // The camera's cell has a mask: use it and remember it as the last good one.
    if (const auto m = grid.lookup(x, y, z)) {
        m_missed = false;
        m_mask = grid.masks.at(*m);
        m_previous = OcclusionPrevious::Grid;
        m_previous_mask = *m;
    } else {
        // The camera is in a cell with no mask; the fallback decides what to show.
        m_missed = true;
        bool done = false;

        // The camera is outside the grid's cells: try the cells next to it first.
        if (fallback == OcclusionFallback::Neighbours) {
            // GetOcclGridFromPair (0x218f50): the fraction p * 0.25 - cell
            // below 0.5 tries the -1 side first.
            auto pair = [&](float fraction, std::array<s32, 3> lo, std::array<s32, 3> hi) {
                // A fraction of 0.5 or more means the camera is in the upper half, so try the +1
                // side first.
                if (!(fraction < 0.5f)) {
                    std::swap(lo, hi);
                }

                auto first = grid.lookup(lo[0], lo[1], lo[2]);
                return first ? first : grid.lookup(hi[0], hi[1], hi[2]);
            };
            const std::array<std::optional<u16>, 3> neighbours{
                pair(camera[0] * 0.25f - static_cast<float>(x), {x - 1, y, z}, {x + 1, y, z}),
                pair(camera[1] * 0.25f - static_cast<float>(y), {x, y - 1, z}, {x, y + 1, z}),
                pair(camera[2] * 0.25f - static_cast<float>(z), {x, y, z - 1}, {x, y, z + 1}),
            };

            // At least one neighbour has a mask.
            if (std::any_of(neighbours.begin(), neighbours.end(), [](const auto& n) {
                    return n.has_value();
                })) {
                m_union.fill(0);

                // OR together the masks of the neighbours that have one.
                for (const auto& n : neighbours) {
                    // Only neighbours that were found contribute.
                    if (n) {
                        const OcclusionMask& mask = grid.masks.at(*n);

                        // OR byte by byte.
                        for (std::size_t i = 0; i < kOcclusionMaskBytes; ++i) {
                            m_union[i] |= mask[i];
                        }
                    }
                }

                m_previous = OcclusionPrevious::Union;
                m_mask = m_union;
                done = true;
            }
        }

        // No neighbour mask was used, so fall back as the mode says.
        if (!done) {
            // The fallback is to show everything.
            if (fallback == OcclusionFallback::AllVisible) {
                m_mask = all_visible();
            } else if (fallback == OcclusionFallback::Octants && grid.octants) {
                // The level has octant masks: use the camera's octant.
                m_mask = grid.octants->mask_for(camera);
            } else if (debug_camera) {
                // A detached debug camera must not inherit the game camera's mask.
                m_mask = all_visible();
            } else if (m_previous == OcclusionPrevious::Grid) {
                // Keep showing what the last cell-based mask showed.
                m_mask = grid.masks.at(m_previous_mask);
            } else if (m_previous == OcclusionPrevious::Union) {
                // Keep showing the last neighbour union.
                m_mask = m_union;
            } else {
                // Nothing was used before: show everything.
                m_mask = all_visible();
            }
        }
    }

    m_mask[0x7f] |= 0x80;
}

}  // namespace openrac::assets
