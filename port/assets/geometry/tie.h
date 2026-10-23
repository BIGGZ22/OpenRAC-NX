// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tie.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 ties: instanced static props (ReRAC docs/formats/tie_rac1.md). A tie
// class is a blob in the core data with three LODs of packets; the gameplay
// file places instances of it. The packet semantics follow the game, not only
// Wrench: the EE DMA builder (TieProc, NTSC-U level01 0x2a9a90) fixes what each
// packet-header byte uploads, and the VU1 tie program 13507 (EE 0x108e28) fixes
// how stored vertices become GS-packet writes (spec 3.4):
//
//   - a packet uploads its ad-gifs, an unpack header and strips, the vertices
//     and a per-vertex colour-index array;
//   - VU1 writes each vertex's ST / RGBA / XYZ to its gs_slot and, in the
//     double-write phases the unpack header's slot markers delimit, also to
//     gs_slot_2;
//   - the GIF then reads ad-gif blocks (at slot 0 and at ad_gif_dest[k-1]) and
//     strip GIF tags with NLOOP = vertex count, in address order.
//
// Lighting inputs are kept: every vertex carries a light slot 0..63 that
// selects one of the class's 64 normals and one of the instance's 64 ambient
// colours; tie_lighting.h lights the slots per instance.

#pragma once

#include <array>
#include <optional>
#include <span>
#include <vector>

#include "assets/bytes.h"
#include "assets/geometry/core_records.h"
#include "assets/geometry/gs_adgif.h"
#include "assets/geometry/mesh.h"

namespace openrac::assets::rac1 {

// Per-LOD totals in the class header; they match the packets on every class.
struct TieLodInfo {
    u32 strip_vertex_count = 0;
    u32 triangle_count = 0;
    u32 strip_count = 0;
    u32 pad = 0;
};

static_assert(sizeof(TieLodInfo) == 0x10);

// The class header at byte 0 of a class blob (0x80 bytes); offsets are
// blob-relative.
struct TieClassHeader {
    std::array<s32, 3> packets{};  // 0x00: packet-header tables of LOD 0 (most detail), 1, 2
    u32 normals = 0;               // 0x0c: 64 light-slot normals ([s16; 4], 0x200 bytes)
    f32 near_dist = 0;             // 0x10: LOD distances
    f32 mid_dist = 0;
    f32 far_dist = 0;
    f32 unknown_1c = 0;                // equals unknown_48; TieProc reuses it as counters
    std::array<u8, 3> packet_count{};  // 0x20: packets per LOD
    u8 texture_count = 0;              // 0x23: ad-gif blocks at ad_gif_ofs
    // 0x24: render mode bits (TieProc: & 9 skips the class, (& 6) >> 1 picks
    // the path).
    u16 flags_24 = 0;
    u16 instance_count_26 = 0;     // run time
    u32 instance_list_28 = 0;      // run time
    u32 ad_gif_ofs = 0;            // 0x2c: texture_count x 0x50 bytes
    std::array<f32, 4> bsphere{};  // 0x30: bounding sphere (xyz, radius)
    f32 scale = 0;                 // 0x40: class units = packed s16 * scale / 1024
    s32 o_class = 0;               // 0x44
    f32 unknown_48 = 0;            // a distance-like float (5, 10, 20, ...)
    u32 unknown_4c = 0;
    std::array<TieLodInfo, 3> lod_info{};  // 0x50
};

static_assert(sizeof(TieClassHeader) == 0x80);

// A packet header (0x10 bytes); offsets and sizes are quadwords relative to
// the packet data. Meanings from the DMA chain TieProc builds (spec 3.2).
struct TiePacketHeader {
    s32 data = 0;          // 0x0: the packet data, relative to the LOD's packet table
    u8 shader_count = 0;   // 0x4: ad-gif blocks uploaded
    u8 ad_gif_qwc = 0;     // 0x5: 5 * shader_count
    u8 control_count = 0;  // 0x6: 3 + strip_count
    u8 control_size = 0;   // 0x7: quadwords of unpack header + strips
    u8 vert_ofs = 0;       // 0x8: the vertex region
    u8 vert_size = 0;      // 0x9
    // 0xa: the colour-index region: two copies (VU buffers A and B) of
    // color_count x 4 bytes, each padded to a quadword.
    u8 color_ofs = 0;
    u8 color_count = 0;         // 0xb
    u8 slot_table_ofs = 0;      // 0xc: per-strip-vertex GS slot steps (Wrench's "scissor")
    u8 slot_table_size = 0;     // 0xd
    u8 strip_count = 0;         // 0xe
    u8 strip_vertex_count = 0;  // 0xf
};

static_assert(sizeof(TiePacketHeader) == 0x10);

// The unpack header at packet data + 0x20 (12 bytes). The four *_end bytes are
// GS slots at which VU1's vertex loops change phase.
struct TieUnpackHeader {
    u8 dinky_single_only = 0;  // non-zero: no double-write phase for dinky vertices
    u8 no_fat = 0;             // non-zero: no fat vertices
    u8 unknown_2 = 0;
    u8 strip_count = 0;
    u8 dinky_single_end = 0;  // the dinky vertex ending the single-write loop (3 more follow)
    u8 dinky_double_end = 0;  // the dinky vertex ending the double-write loop (2 more follow)
    u8 fat_single_end = 0;    // the last single-write fat vertex
    u8 fat_double_end = 0;    // the last fat vertex
    u8 dinky_qwc_plus_four = 0;
    u8 fat_qwc_plus_six = 0;
    u8 dinky_count = 0;
    u8 fat_count = 0;
};

static_assert(sizeof(TieUnpackHeader) == 12);

// A strip record of a packet's unpack header: how many vertices one GS strip has and where its
// GIF tag sits in the GS packet.
struct TieStrip {
    u8 vertex_count = 0;  // the strip GIF tag's NLOOP
    u8 pad = 0;
    u8 gif_tag_offset = 0;  // GS-packet quadword of the strip's GIF tag
    u8 winding = 0;         // non-zero flips the triangle parity (0 on the disc)
};

static_assert(sizeof(TieStrip) == 4);

// A stored "dinky" vertex (0x10 bytes).
struct TieDinkyVertex {
    s16 x = 0;
    s16 y = 0;
    s16 z = 0;
    u16 gs_slot = 0;
    s16 s = 0;  // 1/4096
    s16 t = 0;
    u16 q = 0;
    u16 gs_slot_2 = 0;  // used only in the double-write phase
};

static_assert(sizeof(TieDinkyVertex) == 0x10);

// A stored "fat" vertex (0x18 bytes) that morphs toward the next LOD: VU1
// draws it at position + k * delta with a per-instance factor k.
struct TieFatVertex {
    s16 dx = 0;
    s16 dy = 0;
    s16 dz = 0;
    u16 gs_slot = 0;
    s16 x = 0;
    s16 y = 0;
    s16 z = 0;
    u16 pad = 0;
    s16 s = 0;
    s16 t = 0;
    u16 q = 0;
    u16 gs_slot_2 = 0;
};

static_assert(sizeof(TieFatVertex) == 0x18);

// A class's ad-gif block (0x50 bytes). TEX0's TBP/CBP are 0 on the disc
// (filled in at run time).
struct TieAdGifs {
    AdGif tex0;
    AdGif tex1;
    AdGif miptbp1;
    AdGif clamp;
    AdGif miptbp2;
};

static_assert(sizeof(TieAdGifs) == 0x50);

// A vertex as VU1 processes it (dinky first, then fat), resolved.
struct TieVertex {
    std::array<s16, 3> position{};     // class space (fat: the base); x scale / 1024
    std::array<s16, 3> morph_delta{};  // fat: drawn at position + k * delta; 0 for dinky
    std::array<s16, 2> st{};           // 1/4096
    u16 q = 0;                         // 0x1000 = 1.0 on the disc
    u16 gs_slot = 0;
    u16 gs_slot_2 = 0;  // the second slot in double-write phases, else 0
    u8 color = 0;       // light slot 0..63
    // Fat: the two slots whose average VU1 blends in as the vertex morphs;
    // `color` twice for dinky.
    std::array<u8, 2> morph_colors{};
    bool fat = false;

    std::array<f32, 3> class_position(f32 class_scale) const;
    std::array<f32, 3> class_morph_delta(f32 class_scale) const;

    std::array<f32, 2> uv() const { return {st[0] / 4096.0f, st[1] / 4096.0f}; }

    bool operator==(const TieVertex&) const = default;
};

// One GS triangle strip in GS-packet order.
struct TieDraw {
    u8 ad_gif = 0;  // the class ad-gif block, also the class texture slot
    u8 winding = 0;
    std::vector<u16> vertices;  // into TiePacket::vertices

    bool operator==(const TieDraw&) const = default;
};

// A triangle of packet vertex indices and the class ad-gif block (texture slot) in effect.
struct TieTriangle {
    u16 a = 0;
    u16 b = 0;
    u16 c = 0;
    u16 ad_gif = 0;

    bool operator==(const TieTriangle&) const = default;
};

// One packet of a LOD.
struct TiePacket {
    TiePacketHeader header;
    // GS-packet quadword of ad-gif block k+1 (block 0 is at 0); VU1 stops at
    // the first entry <= 0.
    std::array<s32, 4> ad_gif_dest{};
    // Byte offset of ad-gif block k in the class ad-gif table (/ 0x50 = index).
    std::array<s32, 4> ad_gif_src{};
    TieUnpackHeader unpack;
    std::vector<TieStrip> strips;
    std::vector<TieDinkyVertex> dinky;
    std::vector<TieFatVertex> fat;
    // Colour indices for VU buffer A: one per dinky vertex, then (c0, c1, c2,
    // 0xff) per fat vertex from the next 4-byte boundary.
    std::vector<u8> colors;
    std::vector<u8> colors_b;    // buffer B: every used index + 0x40
    std::vector<u8> slot_table;  // raw, not decoded
    std::vector<TieVertex> vertices;
    std::vector<TieDraw> draws;
};

// One parsed tie class: the header, the light-slot normals, the packets of each LOD (0 = most
// detail) and the class's ad-gif blocks.
struct TieClass {
    TieClassHeader header;
    // Bytes 0x80 up to the first packet table: bounding box min / max (w = 1)
    // then, when present, 8 corner points.
    std::vector<u8> header_ext;
    std::vector<std::array<s16, 4>> normals;  // 64 light-slot normals, unit in 1/32767
    std::array<std::vector<TiePacket>, 3> lods;
    std::vector<TieAdGifs> ad_gifs;

    // The unit normal of a light slot.
    std::optional<std::array<f32, 3>> normal(u8 slot) const;
};

// Parses a class blob (the header at byte 0, offsets relative to it) and resolves every packet's
// vertices. Throws if the blob is truncated or inconsistent.
TieClass parse_tie_class(ByteView blob);

// Fills packet.vertices from its dinky and fat records, colour indices and
// unpack header the way VU1 processes them; parse_tie_class calls it.
void resolve_tie_vertices(TiePacket& packet);

// A packet's strips as triangles: (i-2, i-1, i) when i % 2 == winding, else
// (i, i-1, i-2) (the GS draws both; the order only gives one facing).
std::vector<TieTriangle> tie_triangles(const TiePacket& packet);

// One LOD of a class as a plain mesh in class units (packed x scale / 1024),
// at morph factor 0 (full detail). Vertex normals are the light slots'
// normals; colours are left at 1.0 (they are per instance, tie_lighting.h).
// Triangles carry the class texture slot (the ad-gif block index).
Mesh tie_mesh(const TieClass& tie, int lod);

// A level tie class: its core table entry and the parsed blob.
struct LevelTieClass {
    CoreClassEntry entry;
    TieClass tie;
};

// Every class of the tie class table that has a blob, in table order;
// `core_data` is the whole decompressed core data.
std::vector<LevelTieClass> parse_level_tie_classes(
    std::span<const CoreClassEntry> table, ByteView core_data
);

// The gameplay file's tie instance section pointer.
constexpr std::size_t kGameplayTieInstances = 0x34;

// A gameplay tie instance (0xe0 bytes).
struct TieInstance {
    s32 o_class = 0;
    // An s32 in world units, not a float as Wrench has it: TieProc converts it
    // with cvt.s.w and caps it at the loader's 720.0. 0 = never drawn.
    s32 draw_distance = 0;
    s32 pad_8 = 0;
    s32 occlusion_index = 0;
    // Class to world, column-major (matrix[column][row]), translation in
    // column 3. [3][3] is 0.01 (or 0) on the disc and kept raw.
    std::array<std::array<f32, 4>, 4> matrix{};
    std::array<u16, 64> ambient_rgbas{};  // RGBA5551 per light slot
    s32 directional_lights = 0;           // light set select (0..8, 15, 0xff00 on the disc)
    s32 uid = 0;
    s32 pad_d8 = 0;
    s32 pad_dc = 0;

    // The matrix with [3][3] = 1.
    std::array<std::array<f32, 4>, 4> world_matrix() const;

    // A class-space point (world units: already x scale / 1024) in world space.
    std::array<f32, 3> transform_point(const std::array<f32, 3>& p) const;

    // A slot's ambient colour expanded like PEXT5.
    std::array<u8, 4> ambient_rgba(u8 slot) const;
};

static_assert(sizeof(TieInstance) == 0xe0);

// An instance section: s32 count, 12 bytes of padding, count records.
std::vector<TieInstance> parse_tie_instance_section(ByteView section);

// The tie instances of a decompressed gameplay file.
std::vector<TieInstance> parse_tie_instances(ByteView gameplay);

}  // namespace openrac::assets::rac1
