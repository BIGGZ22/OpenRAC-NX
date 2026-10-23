// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/moby.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The RAC1 moby class reader.

#include "assets/geometry/moby.h"

#include <cstring>

#include "assets/geometry/vif.h"

namespace openrac::assets::rac1 {

std::vector<std::array<u8, 4>> MobyPacket::rgba_multiplier_records() const {
    const std::size_t n =
        is_metal ? 0
                 : std::min<std::size_t>(
                       std::size_t{vertex_table.transfer_vertex_count} * 4, rgba_multipliers.size()
                   );
    std::vector<std::array<u8, 4>> out(n / 4);

    if (!out.empty()) {
        std::memcpy(out.data(), rgba_multipliers.data(), out.size() * 4);
    }

    return out;
}

std::optional<std::size_t> MobySkeleton::parent(std::size_t joint) const {
    if (joint == 0 || joint >= trans.size()) {
        return std::nullopt;
    }

    return trans[joint].parent_offset / 0x40u;
}

std::array<f32, 3> MobyClass::position(const MobyVertex& v) const {
    const f32 k = header.scale / 1024.0f;
    return {v.x * k, v.y * k, v.z * k};
}

std::vector<u16> vertex_cache_ids(
    std::span<const std::array<u8, 16>> records, std::size_t in_file
) {
    std::vector<u16> ids;

    for (std::size_t i = 7; i < records.size(); ++i) {
        ids.push_back(static_cast<u16>((records[i][0] | records[i][1] << 8) & 0x1ff));
    }

    if (!records.empty()) {
        const auto& last = records.back();

        for (std::size_t k = 0; k < 6 && ids.size() < in_file; ++k) {
            ids.push_back(static_cast<u16>((last[4 + k * 2] | last[5 + k * 2] << 8) & 0x1ff));
        }
    }

    if (ids.size() < in_file) {
        fail("moby packet: not enough vertex ids");
    }

    ids.resize(in_file);
    return ids;
}

void Vu0Slots::store(u8 address, const MobySkin& skin) {
    if (address % 4 != 0) {
        fail("moby skin: unaligned VU0 store address {:#x}", address);
    }

    m_slots[address] = skin;
}

MobySkin Vu0Slots::load(u8 address) const {
    if (address % 4 != 0) {
        fail("moby skin: unaligned VU0 load address {:#x}", address);
    }

    if (address >= 0xf4) {
        fail("moby skin: load from the VU0 sink / constant area");
    }

    if (!m_slots[address]) {
        fail("moby skin: load from a VU0 slot never written ({:#x})", address);
    }

    return *m_slots[address];
}

u8 Vu0Slots::load_joint(u8 address) const {
    const MobySkin s = load(address);

    if (s.count != 1) {
        fail("moby skin: a blend input is itself a blend");
    }

    return s.joints[0];
}

MobySkin Vu0Slots::vertex(MobyVertexKind kind, const std::array<u8, 8>& r) {
    const auto joint = static_cast<u8>(r[1] >> 1);

    switch (kind) {
        case MobyVertexKind::TwoWay: {
            store(r[6], MobySkin::joint(joint));
            const u8 a = load_joint(r[2]);
            const u8 b = load_joint(r[3]);

            if (r[4] + r[5] != 256) {
                fail("moby skin: 2-way weights do not sum to 256");
            }

            const MobySkin m{2, {a, b, 0}, {r[4], r[5], 0}};
            store(r[7], m);
            return m;
        }
        case MobyVertexKind::ThreeWay: {
            const u8 a = load_joint(r[2]);
            const u8 b = load_joint(r[3]);
            const u8 c = load_joint(static_cast<u8>(r[1] & 0xfe));

            if (r[4] + r[5] + r[6] != 256) {
                fail("moby skin: 3-way weights do not sum to 256");
            }

            const MobySkin m{3, {a, b, c}, {r[4], r[5], r[6]}};
            store(r[7], m);
            return m;
        }
        case MobyVertexKind::Single:
            store(r[3], MobySkin::joint(joint));
            return load(r[2]);
        case MobyVertexKind::Metal:
            break;
    }

    fail("moby skin: metal vertices do not use VU0 slots");
}

MobySkin metal_skin(const std::array<u8, 8>& r) {
    const u8 n = r[3];

    if (n <= 1) {
        return MobySkin::joint(r[0]);
    }

    if (n > 3) {
        fail("moby metal vertex: joint count {} > 3", n);
    }

    MobySkin s;
    s.count = n;
    u32 sum = 0;

    for (std::size_t k = 0; k < n; ++k) {
        s.joints[k] = r[k];
        s.weights[k] = r[4 + k];
        sum += r[4 + k];
    }

    if (sum != 256) {
        fail("moby metal vertex: weights do not sum to 256");
    }

    return s;
}

namespace {

struct Walk {
    std::vector<MobyTriangle> triangles;
    s32 texture = -1;
};

Walk walk_indices(const MobyPacket& sub) {
    struct Push {
        u32 index;
        bool no_kick;
        s32 texture;
    };

    s32 texture = sub.initial_texture;
    std::vector<Push> pushes;
    std::size_t secret = 0;
    std::size_t adgif = 0;
    bool ended = false;

    for (const u8 byte : sub.index_bytes) {
        u32 index;
        bool no_kick;

        if (byte == 0) {
            if (secret >= sub.secret_indices.size()) {
                fail("moby packet: ran out of secret indices");
            }

            const u8 s = sub.secret_indices[secret++];

            if (s == 0) {
                ended = true;
                break;
            }

            if (adgif >= sub.texture_indices.size()) {
                fail("moby packet: texture switch without an ad-gif block");
            }

            texture = sub.texture_indices[adgif++];
            index = s & 0x7fu;
            no_kick = true;
        } else {
            index = byte & 0x7fu;
            no_kick = (byte & 0x80) != 0;
        }

        if (index == 0 || index > sub.vertices.size()) {
            fail("moby packet: vertex index {} out of range", index);
        }

        pushes.push_back({index - 1, no_kick, texture});
    }

    if (!ended) {
        fail("moby packet: index stream not terminated");
    }

    if (pushes.size() < 3) {
        fail("moby packet: fewer than 3 indices before the terminator");
    }

    pushes.resize(pushes.size() - 3);
    Walk walk;

    for (std::size_t n = 2; n < pushes.size(); ++n) {
        if (!pushes[n].no_kick) {
            walk.triangles.push_back(
                {pushes[n - 2].index, pushes[n - 1].index, pushes[n].index, pushes[n].texture}
            );
        }
    }

    walk.texture = texture;
    return walk;
}

// What carries across the packets of one LOD list.
struct ListState {
    std::vector<std::optional<MobyVertex>> cache = std::vector<std::optional<MobyVertex>>(512);
    Vu0Slots slots;
    s32 texture = -1;
};

MobyPacket read_packet(ByteView blob, const MobyPacketEntry& e, bool metal, ListState& state) {
    // The VIF list: ST (regular only), the index stream, optional ad-gifs.
    const ByteView list =
        blob.sub(e.vif_list_offset, std::size_t{e.vif_list_size} * 0x10, "moby VIF list");
    std::vector<vif::Code> unpacks;

    for (const vif::Code& c : vif::parse(list)) {
        if (c.is_unpack()) {
            unpacks.push_back(c);
        }
    }

    MobyPacket sub;
    sub.entry = e;
    sub.is_metal = metal;
    std::size_t next = 0;

    if (!metal) {
        if (next >= unpacks.size()) {
            fail("moby packet: fewer than 2 unpacks");
        }

        const vif::Code& st = unpacks[next++];

        if (st.vn() != 1 || st.vl() != 1) {
            fail("moby packet: the first unpack is not V2_16");
        }

        sub.st = st.data.read_array<std::array<s16, 2>>(0, st.count(), "moby ST");
    }

    if (next >= unpacks.size()) {
        fail("moby packet: {}", metal ? "missing index unpack" : "fewer than 2 unpacks");
    }

    const vif::Code& ix = unpacks[next++];

    if (ix.vn() != 3 || ix.vl() != 2) {
        fail("moby packet: the index unpack is not V4_8");
    }

    const std::size_t ix_bytes = std::size_t{ix.count()} * 4;
    sub.index_bytes = ix.data.sub(4, ix_bytes - 4, "moby indices").to_vector();
    sub.secret_indices.push_back(ix.data.u8_at(2));

    if (next < unpacks.size()) {
        const vif::Code& ad = unpacks[next];

        if (ad.vn() != 3 || ad.vl() != 0) {
            fail("moby packet: the texture unpack is not V4_32");
        }

        for (std::size_t b = 0; b < ad.count() / 4; ++b) {
            sub.texture_indices.push_back(ad.data.s32_at(b * 0x40 + 0x20));
            // One extra index per ad-gif block, from successive quadwords
            // (block 0 holds the first four).
            sub.secret_indices.push_back(ad.data.u8_at(b * 0x10 + 0x0c));
        }
    }

    const std::size_t vo = e.vertex_offset;

    if (metal) {
        sub.metal_header =
            blob.read<MobyMetalVertexTableHeader>(vo, "moby metal vertex table header");
        const s32 count = sub.metal_header.vertex_count;

        if (count < 0 || count > 4096) {
            fail("moby metal packet: implausible vertex count {}", count);
        }

        sub.raw_vertices =
            blob.sub(vo + 0x10, static_cast<std::size_t>(count) * 0x10, "moby metal vertices")
                .to_vector();

        for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
            const u8* r = sub.raw_vertices.data() + i * 16;
            MobyVertex v;
            std::memcpy(v.raw.data(), r + 8, 8);
            v.normal_azimuth = r[6];
            v.normal_elevation = r[7];
            v.x = static_cast<s16>(r[0] | r[1] << 8);
            v.y = static_cast<s16>(r[2] | r[3] << 8);
            v.z = static_cast<s16>(r[4] | r[5] << 8);
            v.id = static_cast<u16>(i);
            v.kind = MobyVertexKind::Metal;
            v.skin = metal_skin(v.raw);
            sub.vertices.push_back(v);
        }
    } else {
        const auto h = blob.read<MobyVertexTableHeader>(vo, "moby vertex table header");
        sub.vertex_table = h;
        const std::size_t n2 = h.two_way_blend_vertex_count;
        const std::size_t n3 = h.three_way_blend_vertex_count;
        const std::size_t in_file = n2 + n3 + h.main_vertex_count;

        if (h.transfer_vertex_count != in_file + h.duplicate_vertex_count) {
            fail("moby packet: transfer count mismatch");
        }

        if (h.transfer_vertex_count != e.transfer_vertex_count) {
            fail("moby packet: entry and table transfer counts differ");
        }

        const u32 tvc = e.transfer_vertex_count;

        if (e.positions_qwc != (tvc * 6 + 15) / 16 || e.colours_qwc != (tvc + 3) / 4) {
            fail("moby packet: redundant entry fields do not match");
        }

        sub.transfers = blob.read_array<MobyMatrixTransfer>(
            vo + 0x20, h.matrix_transfer_count, "moby transfers"
        );
        std::size_t ofs = vo + 0x20 + std::size_t{h.matrix_transfer_count} * 2;

        if (ofs % 4 != 0) {
            ofs += 2;
        }

        if (ofs % 8 != 0) {
            ofs += 4;
        }

        sub.duplicates =
            blob.read_array<u16>(ofs, h.duplicate_vertex_count, "moby duplicate vertices");

        if (h.multipliers_offset < h.vertex_table_offset) {
            fail("moby packet: the multipliers come before the vertex table");
        }

        const std::size_t epilogue =
            (h.multipliers_offset - h.vertex_table_offset) / 0x10 - in_file;

        if (epilogue < 1 || epilogue >= 7) {
            fail("moby packet: epilogue vertex count {}", static_cast<std::ptrdiff_t>(epilogue));
        }

        const std::size_t mult_size = std::size_t{e.vertex_data_size} * 0x10;

        if (mult_size < h.multipliers_offset) {
            fail("moby packet: the multipliers start past the vertex data");
        }

        sub.rgba_multipliers = blob.sub(
                                       vo + h.multipliers_offset,
                                       mult_size - h.multipliers_offset,
                                       "moby RGBA multipliers"
        )
                                   .to_vector();

        if (sub.rgba_multipliers.size()
            != (std::size_t{h.transfer_vertex_count} * 4 + 15) / 16 * 16) {
            fail("moby packet: the multipliers are not align16(4 * transfer_vertex_count)");
        }

        sub.raw_vertices =
            blob.sub(vo + h.vertex_table_offset, (in_file + epilogue) * 0x10, "moby vertices")
                .to_vector();
        std::vector<std::array<u8, 16>> records(in_file + epilogue);
        std::memcpy(records.data(), sub.raw_vertices.data(), sub.raw_vertices.size());
        const std::vector<u16> ids = vertex_cache_ids(records, in_file);

        for (const MobyMatrixTransfer& t : sub.transfers) {
            state.slots.store(t.vu0_dest_addr, MobySkin::joint(t.joint));
        }

        for (std::size_t i = 0; i < in_file; ++i) {
            const auto& r = records[i];
            MobyVertex v;
            std::memcpy(v.raw.data(), r.data(), 8);
            v.kind = i < n2        ? MobyVertexKind::TwoWay
                     : i < n2 + n3 ? MobyVertexKind::ThreeWay
                                   : MobyVertexKind::Single;
            v.skin = state.slots.vertex(v.kind, v.raw);
            v.normal_azimuth = r[8];
            v.normal_elevation = r[9];
            v.x = static_cast<s16>(r[10] | r[11] << 8);
            v.y = static_cast<s16>(r[12] | r[13] << 8);
            v.z = static_cast<s16>(r[14] | r[15] << 8);
            v.id = ids[i];
            sub.vertices.push_back(v);
        }

        // Duplicates copy an earlier vertex (position, normal, skin) from the
        // 512-entry cache.
        for (const MobyVertex& v : sub.vertices) {
            state.cache[v.id & 0x1ff] = v;
        }

        for (const u16 d : sub.duplicates) {
            const u16 id = duplicate_cache_id(d);
            MobyVertex v;

            if (state.cache[id]) {
                v = *state.cache[id];
            } else {
                ++sub.unresolved_duplicates;
            }

            v.id = id;
            v.duplicate = true;
            sub.vertices.push_back(v);
        }

        if (sub.st.size() < sub.vertices.size()) {
            fail("moby packet: the ST array is shorter than the vertex list");
        }

        for (std::size_t i = 0; i < sub.vertices.size(); ++i) {
            sub.vertices[i].st = sub.st[i];
        }
    }

    sub.initial_texture = state.texture;
    Walk walk = walk_indices(sub);
    sub.triangles = std::move(walk.triangles);
    state.texture = walk.texture;
    return sub;
}

}  // namespace

std::vector<MobyTriangle> moby_triangles(const MobyPacket& packet) {
    return walk_indices(packet).triangles;
}

MobyClass parse_moby_class(ByteView blob) {
    MobyClass mc;
    mc.header = blob.read<MobyClassHeader>(0, "moby class header");
    const MobyClassHeader& h = mc.header;

    if (h.sequence_count > 0) {
        mc.sequence_pointers =
            blob.read_array<s32>(0x48, h.sequence_count, "moby sequence pointers");
    }

    if (h.packet_table_offset > 0) {
        const std::size_t total = std::size_t{h.high_lod_count} + h.low_lod_count + h.metal_count;

        if (h.metal_count > 0 && std::size_t{h.metal_begin} + h.metal_count > total) {
            fail("moby class: metal packets past the packet table");
        }

        const auto entries = blob.read_array<MobyPacketEntry>(
            static_cast<std::size_t>(h.packet_table_offset), total, "moby packet table"
        );
        auto list =
            [&](std::size_t first, std::size_t count, bool metal, std::vector<MobyPacket>& out) {
                ListState state;

                for (std::size_t i = first; i < first + count; ++i) {
                    out.push_back(read_packet(blob, entries[i], metal, state));
                }
            };
        list(0, h.high_lod_count, false, mc.high_lod);
        list(h.high_lod_count, h.low_lod_count, false, mc.low_lod);
        list(h.metal_begin, h.metal_count, true, mc.metal);
    }

    const std::size_t jc = h.joint_count;

    if (h.skeleton > 0 && jc > 0) {
        mc.skeleton.matrices = blob.read_array<std::array<std::array<f32, 4>, 4>>(
            static_cast<std::size_t>(h.skeleton), jc, "moby skeleton"
        );
    }

    if (h.common_trans > 0 && jc > 0) {
        mc.skeleton.trans = blob.read_array<MobyTrans>(
            static_cast<std::size_t>(h.common_trans), jc, "moby common trans"
        );
    }

    return mc;
}

MobyMesh moby_mesh(const MobyClass& moby, std::span<const MobyPacket> packets) {
    MobyMesh out;

    for (const MobyPacket& p : packets) {
        const auto base = static_cast<u32>(out.mesh.vertices.size());

        for (const MobyVertex& v : p.vertices) {
            MeshVertex vertex;
            vertex.position = moby.position(v);
            vertex.uv = {v.st[0] / 4096.0f, v.st[1] / 4096.0f};
            vertex.normal = spherical_normal(v.normal_azimuth, v.normal_elevation);
            out.mesh.vertices.push_back(vertex);
            out.skins.push_back(v.skin);
        }

        for (const MobyTriangle& t : p.triangles) {
            out.mesh.triangles.push_back({{base + t.a, base + t.b, base + t.c}, t.texture});
        }
    }

    return out;
}

std::vector<LevelMobyClass> parse_level_moby_classes(
    std::span<const CoreClassEntry> table, ByteView core_data
) {
    std::vector<LevelMobyClass> out;

    for (const CoreClassEntry& e : table) {
        if (e.offset <= 0) {
            continue;
        }

        try {
            out.push_back(
                {e,
                 parse_moby_class(core_data.tail(static_cast<std::size_t>(e.offset), "moby class"))}
            );
        } catch (const AssetError& error) {
            fail("moby class {}: {}", e.o_class, error.what());
        }
    }

    return out;
}

}  // namespace openrac::assets::rac1
