// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tie.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The RAC1 tie reader: packets resolved the way VU1 program 13507 and the
// GIF consume them. Line labels (L10, ...) are the program's.

#include "assets/geometry/tie.h"

#include <algorithm>

#include "assets/geometry/lighting.h"

namespace openrac::assets::rac1 {

std::array<f32, 3> TieVertex::class_position(f32 class_scale) const {
    const f32 k = class_scale / 1024.0f;
    return {position[0] * k, position[1] * k, position[2] * k};
}

std::array<f32, 3> TieVertex::class_morph_delta(f32 class_scale) const {
    const f32 k = class_scale / 1024.0f;
    return {morph_delta[0] * k, morph_delta[1] * k, morph_delta[2] * k};
}

std::optional<std::array<f32, 3>> TieClass::normal(u8 slot) const {
    if (slot >= normals.size()) {
        return std::nullopt;
    }

    const auto& n = normals[slot];
    return std::array<f32, 3>{n[0] / 32767.0f, n[1] / 32767.0f, n[2] / 32767.0f};
}

namespace {

template <typename... Args>
[[noreturn]] void packet_fail(std::format_string<Args...> fmt, Args&&... args) {
    fail("tie packet: {}", std::format(fmt, std::forward<Args>(args)...));
}

// The first vertex from `from` on written to `slot`.
template <typename V>
std::optional<std::size_t> find_slot(const std::vector<V>& vertices, std::size_t from, u8 slot) {
    for (std::size_t i = from; i < vertices.size(); ++i) {
        if (vertices[i].gs_slot == slot) {
            return i;
        }
    }

    return std::nullopt;
}

}  // namespace

// Resolves the stored vertices the way VU1 writes them (spec 3.4): dinky,
// then fat, with double-write phases delimited by the slot markers.
void resolve_tie_vertices(TiePacket& pk) {
    const TieUnpackHeader& u = pk.unpack;
    const std::size_t d = pk.dinky.size();
    const std::size_t f = pk.fat.size();
    // Dinky: the single-write loop (L10) exits after storing its marker
    // vertex; the three already in its pipeline are stored single-write (L13,
    // L21). The double-write loop (L14..L17) exits on its marker and flushes
    // two more (L18..L20).
    std::size_t d_single = 0;

    if (d > 0) {
        const auto i1 = find_slot(pk.dinky, 0, u.dinky_single_end);

        if (!i1) {
            packet_fail("dinky single-write marker not found");
        }

        d_single = *i1 + 4;
        std::size_t d_end = d_single;

        if (u.dinky_single_only == 0) {
            const auto i5 = find_slot(pk.dinky, d_single, u.dinky_double_end);

            if (!i5) {
                packet_fail("dinky double-write marker not found");
            }

            d_end = *i5 + 3;
        }

        if (d_end != d) {
            packet_fail("dinky phase markers do not end at the last dinky vertex");
        }
    }

    // Fat: the single-write loop (L29..L32) up to and including its marker,
    // then the double-write loop (L33..L39) up to the fat_double_end vertex.
    std::size_t f_single = 0;

    if (u.no_fat != 0) {
        if (f > 0) {
            packet_fail("fat vertices present with no_fat set");
        }
    } else {
        if (f == 0) {
            packet_fail("no fat vertices but no_fat clear");
        }

        const auto j6 = find_slot(pk.fat, 0, u.fat_single_end);

        if (!j6) {
            packet_fail("fat single-write marker not found");
        }

        f_single = *j6 + 1;

        if (find_slot(pk.fat, f_single, u.fat_double_end) != std::optional<std::size_t>{f - 1}) {
            packet_fail("fat double-write marker is not the last fat vertex");
        }
    }

    const std::size_t fat_base = (d + 3) / 4 * 4;

    if (fat_base + 4 * f > pk.colors.size() || d > pk.colors.size()) {
        packet_fail("colour indices too short");
    }

    std::vector<TieVertex> out;
    out.reserve(d + f);

    for (std::size_t i = 0; i < d; ++i) {
        const TieDinkyVertex& v = pk.dinky[i];
        const bool twice = i >= d_single;

        if (twice && v.gs_slot_2 == 0) {
            packet_fail("double-write dinky vertex with gs_slot_2 = 0");
        }

        const u8 c = pk.colors[i];
        TieVertex r;
        r.position = {v.x, v.y, v.z};
        r.st = {v.s, v.t};
        r.q = v.q;
        r.gs_slot = v.gs_slot;
        r.gs_slot_2 = twice ? v.gs_slot_2 : u16{0};
        r.color = c;
        r.morph_colors = {c, c};
        out.push_back(r);
    }

    for (std::size_t j = 0; j < f; ++j) {
        const TieFatVertex& v = pk.fat[j];
        const bool twice = j >= f_single;

        if (twice && v.gs_slot_2 == 0) {
            packet_fail("double-write fat vertex with gs_slot_2 = 0");
        }

        const u8* c = pk.colors.data() + fat_base + 4 * j;
        TieVertex r;
        r.position = {v.x, v.y, v.z};
        r.morph_delta = {v.dx, v.dy, v.dz};
        r.st = {v.s, v.t};
        r.q = v.q;
        r.gs_slot = v.gs_slot;
        r.gs_slot_2 = twice ? v.gs_slot_2 : u16{0};
        r.color = c[0];
        r.morph_colors = {c[1], c[2]};
        r.fat = true;
        out.push_back(r);
    }

    pk.vertices = std::move(out);
}

namespace {

// Walks the GS packet as the GIF consumes it: ad-gif blocks where VU1's L2
// loop put them, strip GIF tags with NLOOP = vertex_count.
void walk_gs_packet(TiePacket& pk) {
    // L1/L2: ad-gif 0 at slot 0, ad-gif k at ad_gif_dest[k-1] while that is > 0.
    std::vector<s32> placed{0};

    while (placed.size() <= 4 && pk.ad_gif_dest[placed.size() - 1] > 0) {
        placed.push_back(pk.ad_gif_dest[placed.size() - 1]);
    }

    if (placed.size() > 4) {
        packet_fail("ad_gif_dest[3] > 0 (VU1 L2 would never stop)");
    }

    if (placed.size() != pk.header.shader_count) {
        packet_fail("{} ad-gifs placed, shader_count {}", placed.size(), pk.header.shader_count);
    }

    for (std::size_t k = 0; k < placed.size(); ++k) {
        if (pk.ad_gif_src[k] < 0 || pk.ad_gif_src[k] % 0x50 != 0) {
            packet_fail("ad_gif_src not a multiple of 0x50");
        }
    }

    // Slot -> vertex, the last writer in VU order winning.
    constexpr std::size_t kSlots = 0x400;
    std::vector<s32> slot(kSlots, -1);
    std::vector<bool> consumed(kSlots, false);

    for (std::size_t i = 0; i < pk.vertices.size(); ++i) {
        const TieVertex& v = pk.vertices[i];

        if (v.gs_slot >= kSlots || v.gs_slot_2 >= kSlots) {
            packet_fail("GS slot out of range");
        }

        slot[v.gs_slot] = static_cast<s32>(i);

        if (v.gs_slot_2 != 0) {
            slot[v.gs_slot_2] = static_cast<s32>(i);
        }
    }

    std::size_t cursor = 0;
    std::size_t next_ad = 0;
    std::size_t si = 0;
    u8 material = 0;
    std::vector<TieDraw> draws;

    while (si < pk.strips.size()) {
        const TieStrip& st = pk.strips[si];

        if (next_ad < placed.size() && static_cast<std::size_t>(placed[next_ad]) == cursor) {
            material = static_cast<u8>(pk.ad_gif_src[next_ad] / 0x50);
            ++next_ad;
            cursor += 6;
        } else if (st.gif_tag_offset == cursor) {
            TieDraw draw{material, st.winding, {}};

            for (std::size_t n = 0; n < st.vertex_count; ++n) {
                const std::size_t s = cursor + 1 + 3 * n;

                if (s >= kSlots || slot[s] < 0) {
                    packet_fail("strip reads unwritten GS slot {}", s);
                }

                consumed[s] = true;
                draw.vertices.push_back(static_cast<u16>(slot[s]));
            }

            cursor += 1 + 3 * std::size_t{st.vertex_count};
            draws.push_back(std::move(draw));
            ++si;
        } else {
            packet_fail("no GIF tag at GS packet offset {}", cursor);
        }
    }

    if (next_ad != placed.size()) {
        packet_fail("ad-gif block after the last strip");
    }

    for (const TieVertex& v : pk.vertices) {
        if (!consumed[v.gs_slot] || (v.gs_slot_2 != 0 && !consumed[v.gs_slot_2])) {
            packet_fail("vertex written to a GS slot no strip reads");
        }
    }

    pk.draws = std::move(draws);
}

TiePacket read_packet(ByteView blob, std::size_t lod_table, const TiePacketHeader& ph) {
    if (ph.data < 0) {
        packet_fail("negative data offset");
    }

    const std::size_t base = lod_table + static_cast<std::size_t>(ph.data);
    TiePacket pk;
    pk.header = ph;
    pk.ad_gif_dest = blob.read<std::array<s32, 4>>(base, "tie ad_gif_dest");
    pk.ad_gif_src = blob.read<std::array<s32, 4>>(base + 0x10, "tie ad_gif_src");
    pk.unpack = blob.read<TieUnpackHeader>(base + 0x20, "tie unpack header");
    pk.strips = blob.read_array<TieStrip>(base + 0x2c, pk.unpack.strip_count, "tie strips");

    if (std::size_t{ph.control_size} * 16 < 12 + 4 * pk.strips.size()) {
        packet_fail("control region smaller than unpack header + strips");
    }

    const std::size_t vert_start = base + std::size_t{ph.vert_ofs} * 0x10;
    const std::size_t d = pk.unpack.dinky_count;
    const std::size_t f = pk.unpack.fat_count;

    if (d * 0x10 + f * 0x18 > std::size_t{ph.vert_size} * 0x10) {
        packet_fail("vertices overrun vert_size");
    }

    pk.dinky = blob.read_array<TieDinkyVertex>(vert_start, d, "tie dinky vertices");
    pk.fat = blob.read_array<TieFatVertex>(vert_start + d * 0x10, f, "tie fat vertices");
    const std::size_t color_start = base + std::size_t{ph.color_ofs} * 0x10;
    const std::size_t color_bytes = std::size_t{ph.color_count} * 4;
    const std::size_t color_qw = (std::size_t{ph.color_count} + 3) / 4;
    pk.colors = blob.sub(color_start, color_bytes, "tie colour indices").to_vector();
    pk.colors_b =
        blob.sub(color_start + color_qw * 0x10, color_bytes, "tie colour indices B").to_vector();
    pk.slot_table = blob.sub(
                            base + std::size_t{ph.slot_table_ofs} * 0x10,
                            std::size_t{ph.slot_table_size} * 0x10,
                            "tie slot table"
    )
                        .to_vector();
    resolve_tie_vertices(pk);
    walk_gs_packet(pk);
    return pk;
}

}  // namespace

TieClass parse_tie_class(ByteView blob) {
    TieClass tc;
    tc.header = blob.read<TieClassHeader>(0, "tie class header");
    const TieClassHeader& h = tc.header;
    std::size_t first = blob.size();

    for (std::size_t l = 0; l < 3; ++l) {
        if (h.packet_count[l] != 0 && h.packets[l] > 0) {
            first = std::min(first, static_cast<std::size_t>(h.packets[l]));
        }
    }

    if (first < 0x80) {
        fail("tie class: packet table overlaps the class header");
    }

    tc.header_ext = blob.sub(0x80, first - 0x80, "tie header extension").to_vector();
    tc.normals = blob.read_array<std::array<s16, 4>>(h.normals, 64, "tie normals");

    for (std::size_t l = 0; l < 3; ++l) {
        if (h.packet_count[l] == 0) {
            continue;
        }

        if (h.packets[l] <= 0) {
            fail("tie class: LOD {} has packets but no packet table", l);
        }

        const auto table = static_cast<std::size_t>(h.packets[l]);

        for (const TiePacketHeader& ph :
             blob.read_array<TiePacketHeader>(table, h.packet_count[l], "tie packets")) {
            tc.lods[l].push_back(read_packet(blob, table, ph));
        }
    }

    if (h.texture_count != 0) {
        tc.ad_gifs = blob.read_array<TieAdGifs>(h.ad_gif_ofs, h.texture_count, "tie ad-gifs");
    }

    return tc;
}

std::vector<TieTriangle> tie_triangles(const TiePacket& p) {
    std::vector<TieTriangle> out;

    for (const TieDraw& d : p.draws) {
        const std::size_t parity = d.winding != 0 ? 1 : 0;

        for (std::size_t i = 2; i < d.vertices.size(); ++i) {
            const u16 a = d.vertices[i - 2];
            const u16 b = d.vertices[i - 1];
            const u16 c = d.vertices[i];
            out.push_back(
                i % 2 == parity ? TieTriangle{a, b, c, d.ad_gif} : TieTriangle{c, b, a, d.ad_gif}
            );
        }
    }

    return out;
}

Mesh tie_mesh(const TieClass& tie, int lod) {
    if (lod < 0 || lod > 2) {
        fail("tie LOD {} outside 0..2", lod);
    }

    Mesh mesh;

    for (const TiePacket& p : tie.lods[static_cast<std::size_t>(lod)]) {
        const auto base = static_cast<u32>(mesh.vertices.size());

        for (const TieVertex& v : p.vertices) {
            MeshVertex out;
            out.position = v.class_position(tie.header.scale);
            out.uv = v.uv();
            out.normal = tie.normal(v.color).value_or(std::array<f32, 3>{});
            mesh.vertices.push_back(out);
        }

        for (const TieTriangle& t : tie_triangles(p)) {
            mesh.triangles.push_back({{base + t.a, base + t.b, base + t.c}, t.ad_gif});
        }
    }

    return mesh;
}

std::vector<LevelTieClass> parse_level_tie_classes(
    std::span<const CoreClassEntry> table, ByteView core_data
) {
    std::vector<LevelTieClass> out;

    for (const CoreClassEntry& e : table) {
        if (e.offset <= 0) {
            continue;
        }

        try {
            out.push_back(
                {e,
                 parse_tie_class(core_data.tail(static_cast<std::size_t>(e.offset), "tie class"))}
            );
        } catch (const AssetError& error) {
            fail("tie class {}: {}", e.o_class, error.what());
        }
    }

    return out;
}

std::array<std::array<f32, 4>, 4> TieInstance::world_matrix() const {
    auto m = matrix;
    m[3][3] = 1.0f;
    return m;
}

std::array<f32, 3> TieInstance::transform_point(const std::array<f32, 3>& p) const {
    std::array<f32, 3> out{};

    for (std::size_t r = 0; r < 3; ++r) {
        out[r] = matrix[0][r] * p[0] + matrix[1][r] * p[1] + matrix[2][r] * p[2] + matrix[3][r];
    }

    return out;
}

std::array<u8, 4> TieInstance::ambient_rgba(u8 slot) const {
    return pext5(ambient_rgbas[slot & 63]);
}

std::vector<TieInstance> parse_tie_instance_section(ByteView section) {
    const s32 count = section.s32_at(0);

    if (count < 0 || count > 100'000) {
        fail("tie instances: implausible count {}", count);
    }

    return section.read_array<TieInstance>(0x10, static_cast<std::size_t>(count), "tie instances");
}

std::vector<TieInstance> parse_tie_instances(ByteView gameplay) {
    const std::size_t offset = gameplay.u32_at(kGameplayTieInstances);

    if (offset == 0) {
        return {};
    }

    return parse_tie_instance_section(gameplay.tail(offset, "tie instance section"));
}

}  // namespace openrac::assets::rac1
