// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tie.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The implementation of tie.h: the RAC1 tie reader. Packets are resolved the way VU1 program 13507
// and the GIF consume them. Line labels (L10, ...) are the program's.
//
// The file has three parts: the vertex resolution (which stored vertices are written once or twice
// and to which GS slots), the walk of the GS packet that turns slots into strips and ad-gif
// choices, and the class, mesh and instance readers built on them. Every check throws an
// AssetError, because a packet that does not fit VU1's model means the format is misread. All
// functions are pure and safe to call from any thread.

#include "assets/geometry/tie.h"

#include <algorithm>

#include "assets/geometry/lighting.h"

namespace openrac::assets::rac1 {

// Packed positions are s16 in units of scale / 1024, so this converts them to class units.
std::array<f32, 3> TieVertex::class_position(f32 class_scale) const {
    const f32 k = class_scale / 1024.0f;
    return {position[0] * k, position[1] * k, position[2] * k};
}

std::array<f32, 3> TieVertex::class_morph_delta(f32 class_scale) const {
    const f32 k = class_scale / 1024.0f;
    return {morph_delta[0] * k, morph_delta[1] * k, morph_delta[2] * k};
}

// The slot's s16 normal is scaled by 1/32767; a slot past the table gives no normal.
std::optional<std::array<f32, 3>> TieClass::normal(u8 slot) const {
    // Only the class's own slots have normals.
    if (slot >= normals.size()) {
        return std::nullopt;
    }

    const auto& n = normals[slot];
    return std::array<f32, 3>{n[0] / 32767.0f, n[1] / 32767.0f, n[2] / 32767.0f};
}

namespace {

// Throws an AssetError that names the tie packet as the culprit.
template <typename... Args>
[[noreturn]] void packet_fail(std::format_string<Args...> fmt, Args&&... args) {
    fail("tie packet: {}", std::format(fmt, std::forward<Args>(args)...));
}

// The first vertex from `from` on written to `slot`.
template <typename V>
std::optional<std::size_t> find_slot(const std::vector<V>& vertices, std::size_t from, u8 slot) {
    // Scan forward for the vertex that writes to the marker slot.
    for (std::size_t i = from; i < vertices.size(); ++i) {
        // The first match is the marker vertex.
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

    // A packet without dinky vertices has no dinky phases to delimit.
    if (d > 0) {
        const auto i1 = find_slot(pk.dinky, 0, u.dinky_single_end);

        // The marker slot must belong to a stored vertex, or the packet is misread.
        if (!i1) {
            packet_fail("dinky single-write marker not found");
        }

        d_single = *i1 + 4;
        std::size_t d_end = d_single;

        // Unless the packet is single-write only, a double-write loop follows the single one.
        if (u.dinky_single_only == 0) {
            const auto i5 = find_slot(pk.dinky, d_single, u.dinky_double_end);

            // The double-write marker must be among the vertices after the single-write phase.
            if (!i5) {
                packet_fail("dinky double-write marker not found");
            }

            d_end = *i5 + 3;
        }

        // The two phases together must account for exactly the stored dinky vertices.
        if (d_end != d) {
            packet_fail("dinky phase markers do not end at the last dinky vertex");
        }
    }

    // Fat: the single-write loop (L29..L32) up to and including its marker,
    // then the double-write loop (L33..L39) up to the fat_double_end vertex.
    std::size_t f_single = 0;

    // The no_fat flag and the stored fat vertices must agree.
    if (u.no_fat != 0) {
        // Fat vertices stored although the header says there are none.
        if (f > 0) {
            packet_fail("fat vertices present with no_fat set");
        }
    } else {
        // The header promises fat vertices, so there must be some.
        if (f == 0) {
            packet_fail("no fat vertices but no_fat clear");
        }

        const auto j6 = find_slot(pk.fat, 0, u.fat_single_end);

        // The single-write marker must belong to a stored fat vertex.
        if (!j6) {
            packet_fail("fat single-write marker not found");
        }

        f_single = *j6 + 1;

        // The double-write marker must be the very last fat vertex.
        if (find_slot(pk.fat, f_single, u.fat_double_end) != std::optional<std::size_t>{f - 1}) {
            packet_fail("fat double-write marker is not the last fat vertex");
        }
    }

    const std::size_t fat_base = (d + 3) / 4 * 4;

    // The colour array must hold one index per dinky vertex and four per fat vertex.
    if (fat_base + 4 * f > pk.colors.size() || d > pk.colors.size()) {
        packet_fail("colour indices too short");
    }

    std::vector<TieVertex> out;
    out.reserve(d + f);

    // Resolve the dinky vertices; those after the single-write phase are written to two slots.
    for (std::size_t i = 0; i < d; ++i) {
        const TieDinkyVertex& v = pk.dinky[i];
        const bool twice = i >= d_single;

        // A double-written vertex needs its second slot.
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

    // Resolve the fat vertices the same way, with their three colour indices.
    for (std::size_t j = 0; j < f; ++j) {
        const TieFatVertex& v = pk.fat[j];
        const bool twice = j >= f_single;

        // A double-written vertex needs its second slot.
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
// Fills pk.draws; every vertex must be read by some strip and every strip by the GIF, or the
// packet is rejected.
void walk_gs_packet(TiePacket& pk) {
    // L1/L2: ad-gif 0 at slot 0, ad-gif k at ad_gif_dest[k-1] while that is > 0.
    std::vector<s32> placed{0};

    // Collect the ad-gif positions until one is not positive (the end marker).
    while (placed.size() <= 4 && pk.ad_gif_dest[placed.size() - 1] > 0) {
        placed.push_back(pk.ad_gif_dest[placed.size() - 1]);
    }

    // Four destinations all positive means VU1 would never stop.
    if (placed.size() > 4) {
        packet_fail("ad_gif_dest[3] > 0 (VU1 L2 would never stop)");
    }

    // The placed blocks must match the count the header says it uploads.
    if (placed.size() != pk.header.shader_count) {
        packet_fail("{} ad-gifs placed, shader_count {}", placed.size(), pk.header.shader_count);
    }

    // Each source must be a whole ad-gif block of the class table.
    for (std::size_t k = 0; k < placed.size(); ++k) {
        // A negative or unaligned source cannot index the class table of 0x50-byte blocks.
        if (pk.ad_gif_src[k] < 0 || pk.ad_gif_src[k] % 0x50 != 0) {
            packet_fail("ad_gif_src not a multiple of 0x50");
        }
    }

    // Slot -> vertex, the last writer in VU order winning. The GS packet buffer is 0x400
    // quadwords.
    constexpr std::size_t kSlots = 0x400;
    std::vector<s32> slot(kSlots, -1);
    std::vector<bool> consumed(kSlots, false);

    // Record which vertex each GS slot receives.
    for (std::size_t i = 0; i < pk.vertices.size(); ++i) {
        const TieVertex& v = pk.vertices[i];

        // A slot outside the buffer would write past it.
        if (v.gs_slot >= kSlots || v.gs_slot_2 >= kSlots) {
            packet_fail("GS slot out of range");
        }

        slot[v.gs_slot] = static_cast<s32>(i);

        // Slot 0 means no second write.
        if (v.gs_slot_2 != 0) {
            slot[v.gs_slot_2] = static_cast<s32>(i);
        }
    }

    std::size_t cursor = 0;
    std::size_t next_ad = 0;
    std::size_t si = 0;
    u8 material = 0;
    std::vector<TieDraw> draws;

    // Read the GS packet in address order: ad-gif blocks and strip GIF tags alternate as the
    // packet lays them out.
    while (si < pk.strips.size()) {
        const TieStrip& st = pk.strips[si];

        // An ad-gif block sits at the cursor: it sets the material for the strips after it.
        if (next_ad < placed.size() && static_cast<std::size_t>(placed[next_ad]) == cursor) {
            material = static_cast<u8>(pk.ad_gif_src[next_ad] / 0x50);
            ++next_ad;
            cursor += 6;
        } else if (st.gif_tag_offset == cursor) {
            // The strip's GIF tag sits at the cursor; each of its vertices takes three quadwords.
            TieDraw draw{material, st.winding, {}};

            // Collect the vertex that sits in each of the strip's slots.
            for (std::size_t n = 0; n < st.vertex_count; ++n) {
                const std::size_t s = cursor + 1 + 3 * n;

                // The strip may only read slots a vertex wrote.
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
            // Neither an ad-gif nor the next strip's tag is here: the layout is misread.
            packet_fail("no GIF tag at GS packet offset {}", cursor);
        }
    }

    // Every ad-gif block must have been met before the strips ended.
    if (next_ad != placed.size()) {
        packet_fail("ad-gif block after the last strip");
    }

    // Every written slot must have been read by a strip, or a vertex was misdecoded.
    for (const TieVertex& v : pk.vertices) {
        // A vertex none of whose slots is read was not placed where VU1 puts it.
        if (!consumed[v.gs_slot] || (v.gs_slot_2 != 0 && !consumed[v.gs_slot_2])) {
            packet_fail("vertex written to a GS slot no strip reads");
        }
    }

    pk.draws = std::move(draws);
}

// Reads one packet's records from the blob, then resolves its vertices and draws. Offsets in the
// header are relative to the LOD's packet table.
TiePacket read_packet(ByteView blob, std::size_t lod_table, const TiePacketHeader& ph) {
    // The data offset is signed in the file; a negative one would point before the table.
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

    // The control region must hold the unpack header and one record per strip.
    if (std::size_t{ph.control_size} * 16 < 12 + 4 * pk.strips.size()) {
        packet_fail("control region smaller than unpack header + strips");
    }

    const std::size_t vert_start = base + std::size_t{ph.vert_ofs} * 0x10;
    const std::size_t d = pk.unpack.dinky_count;
    const std::size_t f = pk.unpack.fat_count;

    // The vertices (0x10 and 0x18 bytes each) must fit the region the header gives them.
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

// Reads the header, the extension up to the first packet table, the normals, every LOD's packets
// and the class ad-gifs. Packets are fully resolved, so a successful return is a consistent class.
TieClass parse_tie_class(ByteView blob) {
    TieClass tc;
    tc.header = blob.read<TieClassHeader>(0, "tie class header");
    const TieClassHeader& h = tc.header;
    std::size_t first = blob.size();

    // The extension ends where the first packet table begins.
    for (std::size_t l = 0; l < 3; ++l) {
        // Only LODs that have packets contribute a table.
        if (h.packet_count[l] != 0 && h.packets[l] > 0) {
            first = std::min(first, static_cast<std::size_t>(h.packets[l]));
        }
    }

    // A table inside the 0x80-byte header would be a misread class.
    if (first < 0x80) {
        fail("tie class: packet table overlaps the class header");
    }

    tc.header_ext = blob.sub(0x80, first - 0x80, "tie header extension").to_vector();
    tc.normals = blob.read_array<std::array<s16, 4>>(h.normals, 64, "tie normals");

    // Read the packets of each LOD.
    for (std::size_t l = 0; l < 3; ++l) {
        // A LOD without packets (the coarser ones of a small class) is skipped.
        if (h.packet_count[l] == 0) {
            continue;
        }

        // Packets without a table to find them in are a corrupt header.
        if (h.packets[l] <= 0) {
            fail("tie class: LOD {} has packets but no packet table", l);
        }

        const auto table = static_cast<std::size_t>(h.packets[l]);

        // Read each packet of the LOD.
        for (const TiePacketHeader& ph :
             blob.read_array<TiePacketHeader>(table, h.packet_count[l], "tie packets")) {
            tc.lods[l].push_back(read_packet(blob, table, ph));
        }
    }

    // A class without textures has no ad-gif table to read.
    if (h.texture_count != 0) {
        tc.ad_gifs = blob.read_array<TieAdGifs>(h.ad_gif_ofs, h.texture_count, "tie ad-gifs");
    }

    return tc;
}

std::vector<TieTriangle> tie_triangles(const TiePacket& p) {
    std::vector<TieTriangle> out;

    // Triangulate each strip draw separately; a draw has its own texture and winding.
    for (const TieDraw& d : p.draws) {
        const std::size_t parity = d.winding != 0 ? 1 : 0;

        // Each vertex after the first two closes a triangle with the two before it.
        for (std::size_t i = 2; i < d.vertices.size(); ++i) {
            const u16 a = d.vertices[i - 2];
            const u16 b = d.vertices[i - 1];
            const u16 c = d.vertices[i];
            // The vertex order alternates so every triangle faces the same way; winding flips it.
            out.push_back(
                i % 2 == parity ? TieTriangle{a, b, c, d.ad_gif} : TieTriangle{c, b, a, d.ad_gif}
            );
        }
    }

    return out;
}

Mesh tie_mesh(const TieClass& tie, int lod) {
    // There are only three LODs.
    if (lod < 0 || lod > 2) {
        fail("tie LOD {} outside 0..2", lod);
    }

    Mesh mesh;

    // Append each packet's vertices and triangles; triangle indices are offset by `base`.
    for (const TiePacket& p : tie.lods[static_cast<std::size_t>(lod)]) {
        const auto base = static_cast<u32>(mesh.vertices.size());

        // Convert each stored vertex; its normal is the class normal of its light slot.
        for (const TieVertex& v : p.vertices) {
            MeshVertex out;
            out.position = v.class_position(tie.header.scale);
            out.uv = v.uv();
            out.normal = tie.normal(v.color).value_or(std::array<f32, 3>{});
            mesh.vertices.push_back(out);
        }

        // Re-base the packet's triangles onto the shared vertex array.
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

    // Parse each table entry that has a blob.
    for (const CoreClassEntry& e : table) {
        // A non-positive offset marks a class that has no data in this level.
        if (e.offset <= 0) {
            continue;
        }

        // Name the failing class in the error, since the table has hundreds of them.
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

// The loader leaves 0.01 (or 0) in [3][3]; the world matrix needs 1 there.
std::array<std::array<f32, 4>, 4> TieInstance::world_matrix() const {
    auto m = matrix;
    m[3][3] = 1.0f;
    return m;
}

std::array<f32, 3> TieInstance::transform_point(const std::array<f32, 3>& p) const {
    std::array<f32, 3> out{};

    // Each output row is the dot of the point with a matrix row, plus the translation.
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

    // A negative or absurd count is a misread section, not a level with that many ties.
    if (count < 0 || count > 100'000) {
        fail("tie instances: implausible count {}", count);
    }

    return section.read_array<TieInstance>(0x10, static_cast<std::size_t>(count), "tie instances");
}

std::vector<TieInstance> parse_tie_instances(ByteView gameplay) {
    const std::size_t offset = gameplay.u32_at(kGameplayTieInstances);

    // An offset of zero means the gameplay file has no tie section.
    if (offset == 0) {
        return {};
    }

    return parse_tie_instance_section(gameplay.tail(offset, "tie instance section"));
}

}  // namespace openrac::assets::rac1
