// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/moby.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The moby class reader (moby.h). A class blob is parsed into packets of vertices and triangles,
// a skeleton and the sequence pointers. Most of the work is undoing what the console's data
// layout does: vertex-cache ids stored seven records late, duplicate vertices that copy a cached
// one, the VU0 matrix slots that decide which joints a vertex blends, and an index stream with
// escape bytes for texture switches. The cache, the slots and the current texture carry from
// packet to packet inside one LOD list, which is why a list is read in order with a ListState.
// Every offset and count is checked, so a corrupt blob throws an AssetError.

#include "assets/geometry/moby.h"

#include <cstring>

#include "assets/geometry/vif.h"

namespace openrac::assets::rac1 {

// The per-vertex colour multipliers as 4-byte records, one per transfer vertex (none for a metal
// packet).
std::vector<std::array<u8, 4>> MobyPacket::rgba_multiplier_records() const {
    const std::size_t n =
        is_metal ? 0
                 : std::min<std::size_t>(
                       std::size_t{vertex_table.transfer_vertex_count} * 4, rgba_multipliers.size()
                   );
    std::vector<std::array<u8, 4>> out(n / 4);

    // Skip the copy for an empty list: there is nothing to read from.
    if (!out.empty()) {
        std::memcpy(out.data(), rgba_multipliers.data(), out.size() * 4);
    }

    return out;
}

// The parent of a joint, or nothing for the root or an unknown joint. The stored parent is a
// byte offset in 0x40-byte records.
std::optional<std::size_t> MobySkeleton::parent(std::size_t joint) const {
    // The root joint (0) has no parent, and a joint past the table does not exist.
    if (joint == 0 || joint >= trans.size()) {
        return std::nullopt;
    }

    return trans[joint].parent_offset / 0x40u;
}

// A vertex's position in model units: its fixed-point coordinates times the class scale.
std::array<f32, 3> MobyClass::position(const MobyVertex& v) const {
    const f32 k = header.scale / 1024.0f;
    return {v.x * k, v.y * k, v.z * k};
}

// The 9-bit cache id of each of the `in_file` vertices of a packet, taken from the vertex
// records (ids come seven records after their vertex) and from the epilogue record.
std::vector<u16> vertex_cache_ids(
    std::span<const std::array<u8, 16>> records, std::size_t in_file
) {
    std::vector<u16> ids;

    // The ids are stored seven records late, so they start at the eighth record.
    for (std::size_t i = 7; i < records.size(); ++i) {
        ids.push_back(static_cast<u16>((records[i][0] | records[i][1] << 8) & 0x1ff));
    }

    // The last record also holds six more ids, for the vertices whose own record is not far enough
    // along.
    if (!records.empty()) {
        const auto& last = records.back();

        // Stop when there are as many ids as vertices in the file.
        for (std::size_t k = 0; k < 6 && ids.size() < in_file; ++k) {
            ids.push_back(static_cast<u16>((last[4 + k * 2] | last[5 + k * 2] << 8) & 0x1ff));
        }
    }

    // Fewer ids than vertices would leave some vertex without a cache slot.
    if (ids.size() < in_file) {
        fail("moby packet: not enough vertex ids");
    }

    ids.resize(in_file);
    return ids;
}

// Writes a skin into a VU0 slot; the address must be a valid slot.
void Vu0Slots::store(u8 address, const MobySkin& skin) {
    // The slots are four bytes apart, so an unaligned address cannot be a real one.
    if (address % 4 != 0) {
        fail("moby skin: unaligned VU0 store address {:#x}", address);
    }

    m_slots[address] = skin;
}

// Reads a slot that an earlier store wrote; anything else is a corrupt packet.
MobySkin Vu0Slots::load(u8 address) const {
    // The slots are four bytes apart, so an unaligned address cannot be a real one.
    if (address % 4 != 0) {
        fail("moby skin: unaligned VU0 load address {:#x}", address);
    }

    // Addresses from 0xf4 on are not matrix slots (the unit's sink and constants).
    if (address >= 0xf4) {
        fail("moby skin: load from the VU0 sink / constant area");
    }

    // Reading a slot that no earlier packet wrote would use a matrix of unknown origin.
    if (!m_slots[address]) {
        fail("moby skin: load from a VU0 slot never written ({:#x})", address);
    }

    return *m_slots[address];
}

// Reads a slot that must hold a single joint, and returns that joint.
u8 Vu0Slots::load_joint(u8 address) const {
    const MobySkin s = load(address);

    // Only a plain joint can be blended; a blend of blends is not something the game does.
    if (s.count != 1) {
        fail("moby skin: a blend input is itself a blend");
    }

    return s.joints[0];
}

// Runs a vertex's slot reads and writes for its kind, and returns the skin that results.
MobySkin Vu0Slots::vertex(MobyVertexKind kind, const std::array<u8, 8>& r) {
    const auto joint = static_cast<u8>(r[1] >> 1);

    // The vertex kind decides which slots are read and written.
    switch (kind) {
        case MobyVertexKind::TwoWay: {
            store(r[6], MobySkin::joint(joint));
            const u8 a = load_joint(r[2]);
            const u8 b = load_joint(r[3]);

            // The two weights are out of 256 and must use all of it.
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

            // The three weights are out of 256 and must use all of it.
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

// The skin of a metal vertex, whose joints and weights are stored directly in the record.
MobySkin metal_skin(const std::array<u8, 8>& r) {
    const u8 n = r[3];

    // Zero or one joint means a plain, unblended skin.
    if (n <= 1) {
        return MobySkin::joint(r[0]);
    }

    // A metal vertex has room for three joints at most.
    if (n > 3) {
        fail("moby metal vertex: joint count {} > 3", n);
    }

    MobySkin s;
    s.count = n;
    u32 sum = 0;

    // Copy the joints and their weights, summing the weights to check them.
    for (std::size_t k = 0; k < n; ++k) {
        s.joints[k] = r[k];
        s.weights[k] = r[4 + k];
        sum += r[4 + k];
    }

    // The weights are out of 256 and must use all of it.
    if (sum != 256) {
        fail("moby metal vertex: weights do not sum to 256");
    }

    return s;
}

namespace {

// The result of walking an index stream: the triangles and the texture current at its end.
struct Walk {
    std::vector<MobyTriangle> triangles;
    s32 texture = -1;
};

// Turns a packet's index stream into triangles. The stream is a triangle strip whose entries
// may have the no-kick bit, and whose zero bytes switch texture.
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

    // Walk the index bytes and turn each into a pushed vertex.
    for (const u8 byte : sub.index_bytes) {
        u32 index;
        bool no_kick;

        // A zero byte is an escape: the real index and the texture switch come from the side
        // arrays.
        if (byte == 0) {
            // Each escape consumes one secret index, so running out means a corrupt packet.
            if (secret >= sub.secret_indices.size()) {
                fail("moby packet: ran out of secret indices");
            }

            const u8 s = sub.secret_indices[secret++];

            // A secret index of 0 is the terminator of the stream.
            if (s == 0) {
                ended = true;
                break;
            }

            // A texture switch needs an ad-gif block to take its texture from.
            if (adgif >= sub.texture_indices.size()) {
                fail("moby packet: texture switch without an ad-gif block");
            }

            texture = sub.texture_indices[adgif++];
            index = s & 0x7fu;
            no_kick = true;
        } else {
            // A non-zero byte is an index with its no-kick bit in bit 7.
            index = byte & 0x7fu;
            no_kick = (byte & 0x80) != 0;
        }

        // Indices are 1-based, so 0 or a value past the vertex count is invalid.
        if (index == 0 || index > sub.vertices.size()) {
            fail("moby packet: vertex index {} out of range", index);
        }

        pushes.push_back({index - 1, no_kick, texture});
    }

    // A stream without its terminator would run off the data.
    if (!ended) {
        fail("moby packet: index stream not terminated");
    }

    // The stream ends with a trailer of three pushes that do not make a triangle, so fewer than
    // three is corrupt.
    if (pushes.size() < 3) {
        fail("moby packet: fewer than 3 indices before the terminator");
    }

    pushes.resize(pushes.size() - 3);
    Walk walk;

    // From the third push on, each push completes a triangle with the two before it (a strip).
    for (std::size_t n = 2; n < pushes.size(); ++n) {
        // A no-kick push moves the strip along without drawing a triangle.
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

// Reads one packet: its VIF list, then its vertex table, resolving the cache, the skins and
// the triangles. `state` carries over from the previous packet of the same list.
MobyPacket read_packet(ByteView blob, const MobyPacketEntry& e, bool metal, ListState& state) {
    // The VIF list: ST (regular only), the index stream, optional ad-gifs.
    const ByteView list =
        blob.sub(e.vif_list_offset, std::size_t{e.vif_list_size} * 0x10, "moby VIF list");
    std::vector<vif::Code> unpacks;

    // Collect the unpack commands, which carry the packet's data.
    for (const vif::Code& c : vif::parse(list)) {
        // Other VIF commands (flushes, microprogram calls) carry nothing the reader needs.
        if (c.is_unpack()) {
            unpacks.push_back(c);
        }
    }

    MobyPacket sub;
    sub.entry = e;
    sub.is_metal = metal;
    std::size_t next = 0;

    // Only regular packets have an ST array; metal ones have none.
    if (!metal) {
        // A regular packet needs an ST unpack and an index unpack.
        if (next >= unpacks.size()) {
            fail("moby packet: fewer than 2 unpacks");
        }

        const vif::Code& st = unpacks[next++];

        // The ST unpack must be two 16-bit components.
        if (st.vn() != 1 || st.vl() != 1) {
            fail("moby packet: the first unpack is not V2_16");
        }

        sub.st = st.data.read_array<std::array<s16, 2>>(0, st.count(), "moby ST");
    }

    // Every packet has an index unpack, after the ST one if there is one.
    if (next >= unpacks.size()) {
        fail("moby packet: {}", metal ? "missing index unpack" : "fewer than 2 unpacks");
    }

    const vif::Code& ix = unpacks[next++];

    // The index unpack must be four 8-bit components.
    if (ix.vn() != 3 || ix.vl() != 2) {
        fail("moby packet: the index unpack is not V4_8");
    }

    const std::size_t ix_bytes = std::size_t{ix.count()} * 4;
    sub.index_bytes = ix.data.sub(4, ix_bytes - 4, "moby indices").to_vector();
    sub.secret_indices.push_back(ix.data.u8_at(2));

    // Anything after the index unpack is the ad-gif (texture) unpack, which is optional.
    if (next < unpacks.size()) {
        const vif::Code& ad = unpacks[next];

        // The texture unpack must be four 32-bit components.
        if (ad.vn() != 3 || ad.vl() != 0) {
            fail("moby packet: the texture unpack is not V4_32");
        }

        // One texture index per ad-gif block of four quadwords.
        for (std::size_t b = 0; b < ad.count() / 4; ++b) {
            sub.texture_indices.push_back(ad.data.s32_at(b * 0x40 + 0x20));
            // One extra index per ad-gif block, from successive quadwords
            // (block 0 holds the first four).
            sub.secret_indices.push_back(ad.data.u8_at(b * 0x10 + 0x0c));
        }
    }

    const std::size_t vo = e.vertex_offset;

    // Metal and regular packets store their vertices differently.
    if (metal) {
        sub.metal_header =
            blob.read<MobyMetalVertexTableHeader>(vo, "moby metal vertex table header");
        const s32 count = sub.metal_header.vertex_count;

        // Reject a count that cannot be real before it sizes the read.
        if (count < 0 || count > 4096) {
            fail("moby metal packet: implausible vertex count {}", count);
        }

        sub.raw_vertices =
            blob.sub(vo + 0x10, static_cast<std::size_t>(count) * 0x10, "moby metal vertices")
                .to_vector();

        // Decode each metal vertex record into a vertex.
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
        // A regular packet has a vertex table with a header, matrix transfers and skinned vertices.
        const auto h = blob.read<MobyVertexTableHeader>(vo, "moby vertex table header");
        sub.vertex_table = h;
        const std::size_t n2 = h.two_way_blend_vertex_count;
        const std::size_t n3 = h.three_way_blend_vertex_count;
        const std::size_t in_file = n2 + n3 + h.main_vertex_count;

        // The vertex counts of the header must add up to the transfer count.
        if (h.transfer_vertex_count != in_file + h.duplicate_vertex_count) {
            fail("moby packet: transfer count mismatch");
        }

        // The packet entry repeats the transfer count, and the two must agree.
        if (h.transfer_vertex_count != e.transfer_vertex_count) {
            fail("moby packet: entry and table transfer counts differ");
        }

        const u32 tvc = e.transfer_vertex_count;

        // The entry's quadword counts for positions and colours follow from the vertex count, so a
        // mismatch means a misread.
        if (e.positions_qwc != (tvc * 6 + 15) / 16 || e.colours_qwc != (tvc + 3) / 4) {
            fail("moby packet: redundant entry fields do not match");
        }

        sub.transfers = blob.read_array<MobyMatrixTransfer>(
            vo + 0x20, h.matrix_transfer_count, "moby transfers"
        );
        std::size_t ofs = vo + 0x20 + std::size_t{h.matrix_transfer_count} * 2;

        // The array of two-byte transfers is padded to a 4-byte boundary.
        if (ofs % 4 != 0) {
            ofs += 2;
        }

        // And then to an 8-byte boundary.
        if (ofs % 8 != 0) {
            ofs += 4;
        }

        sub.duplicates =
            blob.read_array<u16>(ofs, h.duplicate_vertex_count, "moby duplicate vertices");

        // The multipliers sit after the vertex table, never before it.
        if (h.multipliers_offset < h.vertex_table_offset) {
            fail("moby packet: the multipliers come before the vertex table");
        }

        const std::size_t epilogue =
            (h.multipliers_offset - h.vertex_table_offset) / 0x10 - in_file;

        // There are one to six epilogue records after the vertices, which hold extra cache ids.
        if (epilogue < 1 || epilogue >= 7) {
            fail("moby packet: epilogue vertex count {}", static_cast<std::ptrdiff_t>(epilogue));
        }

        const std::size_t mult_size = std::size_t{e.vertex_data_size} * 0x10;

        // The multipliers must start inside the vertex data.
        if (mult_size < h.multipliers_offset) {
            fail("moby packet: the multipliers start past the vertex data");
        }

        sub.rgba_multipliers = blob.sub(
                                       vo + h.multipliers_offset,
                                       mult_size - h.multipliers_offset,
                                       "moby RGBA multipliers"
        )
                                   .to_vector();

        // The multipliers must be the transfer vertices' four bytes each rounded up to 16.
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

        // Load each matrix transfer's joint into its VU0 slot before the vertices read them.
        for (const MobyMatrixTransfer& t : sub.transfers) {
            state.slots.store(t.vu0_dest_addr, MobySkin::joint(t.joint));
        }

        // Decode the vertices in file order: two-way blends first, then three-way, then
        // single-joint ones.
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
        // Fill the vertex cache with the file's vertices, so that duplicates can copy them.
        for (const MobyVertex& v : sub.vertices) {
            state.cache[v.id & 0x1ff] = v;
        }

        // Each duplicate copies a vertex that an earlier packet (or this one) put in the cache.
        for (const u16 d : sub.duplicates) {
            const u16 id = duplicate_cache_id(d);
            MobyVertex v;

            // A duplicate whose cache entry is empty is counted, and left blank.
            if (state.cache[id]) {
                v = *state.cache[id];
            } else {
                // The cache has nothing under this id yet, so remember that the duplicate could not
                // be resolved.
                ++sub.unresolved_duplicates;
            }

            v.id = id;
            v.duplicate = true;
            sub.vertices.push_back(v);
        }

        // Every vertex needs a texture coordinate pair.
        if (sub.st.size() < sub.vertices.size()) {
            fail("moby packet: the ST array is shorter than the vertex list");
        }

        // Copy each vertex's ST pair onto it.
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

// The triangles of a packet, from its index stream.
std::vector<MobyTriangle> moby_triangles(const MobyPacket& packet) {
    return walk_indices(packet).triangles;
}

// Parses a class blob: header, sequence pointers, the three packet lists and the skeleton.
MobyClass parse_moby_class(ByteView blob) {
    MobyClass mc;
    mc.header = blob.read<MobyClassHeader>(0, "moby class header");
    const MobyClassHeader& h = mc.header;

    // Sequences are optional; a class with none has no pointers.
    if (h.sequence_count > 0) {
        mc.sequence_pointers =
            blob.read_array<s32>(0x48, h.sequence_count, "moby sequence pointers");
    }

    // A class with no packet table has no geometry.
    if (h.packet_table_offset > 0) {
        const std::size_t total = std::size_t{h.high_lod_count} + h.low_lod_count + h.metal_count;

        // The metal packets must lie inside the packet table.
        if (h.metal_count > 0 && std::size_t{h.metal_begin} + h.metal_count > total) {
            fail("moby class: metal packets past the packet table");
        }

        const auto entries = blob.read_array<MobyPacketEntry>(
            static_cast<std::size_t>(h.packet_table_offset), total, "moby packet table"
        );
        auto list =
            [&](std::size_t first, std::size_t count, bool metal, std::vector<MobyPacket>& out) {
                ListState state;

                // Read the packets of the list in order, sharing the state between them.
                for (std::size_t i = first; i < first + count; ++i) {
                    out.push_back(read_packet(blob, entries[i], metal, state));
                }
            };
        list(0, h.high_lod_count, false, mc.high_lod);
        list(h.high_lod_count, h.low_lod_count, false, mc.low_lod);
        list(h.metal_begin, h.metal_count, true, mc.metal);
    }

    const std::size_t jc = h.joint_count;

    // The skeleton matrices exist only for a class with joints.
    if (h.skeleton > 0 && jc > 0) {
        mc.skeleton.matrices = blob.read_array<std::array<std::array<f32, 4>, 4>>(
            static_cast<std::size_t>(h.skeleton), jc, "moby skeleton"
        );
    }

    // The joint transform records exist only for a class with joints.
    if (h.common_trans > 0 && jc > 0) {
        mc.skeleton.trans = blob.read_array<MobyTrans>(
            static_cast<std::size_t>(h.common_trans), jc, "moby common trans"
        );
    }

    return mc;
}

// Builds one mesh out of a set of packets, with each vertex's skin kept in step with it.
MobyMesh moby_mesh(const MobyClass& moby, std::span<const MobyPacket> packets) {
    MobyMesh out;

    // Each packet's vertices and triangles are appended, with the indices rebased.
    for (const MobyPacket& p : packets) {
        const auto base = static_cast<u32>(out.mesh.vertices.size());

        // Convert each vertex to the renderer's position, UV and normal.
        for (const MobyVertex& v : p.vertices) {
            MeshVertex vertex;
            vertex.position = moby.position(v);
            vertex.uv = {v.st[0] / 4096.0f, v.st[1] / 4096.0f};
            vertex.normal = spherical_normal(v.normal_azimuth, v.normal_elevation);
            out.mesh.vertices.push_back(vertex);
            out.skins.push_back(v.skin);
        }

        // Offset each triangle's indices by where this packet's vertices begin.
        for (const MobyTriangle& t : p.triangles) {
            out.mesh.triangles.push_back({{base + t.a, base + t.b, base + t.c}, t.texture});
        }
    }

    return out;
}

// Parses every class of a level that has a blob.
std::vector<LevelMobyClass> parse_level_moby_classes(
    std::span<const CoreClassEntry> table, ByteView core_data
) {
    std::vector<LevelMobyClass> out;

    // Classes without a blob offset (gadgets, empty slots) are skipped.
    for (const CoreClassEntry& e : table) {
        // An offset of 0 or less means the class has no blob in the core data.
        if (e.offset <= 0) {
            continue;
        }

        // A malformed class must be reported by class number, so wrap its errors.
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
