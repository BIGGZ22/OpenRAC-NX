// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 terrain: the tfrags block of a level's core data (ReRAC
// docs/formats/tfrag_rac1.md, docs/plan/vu1_tfrag_analysis.md). A tfrag is a
// terrain fragment of up to a few hundred vertices with three levels of
// detail that morph into one another. Its data is a set of VIF lists the
// VU1 tfrag program consumes: a common list (VU header, texture ad-gifs,
// common positions and vertex infos), the LOD-2 strips, the LOD-1 strips and
// the LOD-0 refinement (more positions, vertex infos and parent links), then
// an RGBA array, a light record per position, texture-paging spheres and a
// clip box.
//
// The reader replays the lists by VU address the way the game uploads them
// and keeps every array, so a renderer can draw any LOD and morph between
// them as the game does; tfrag_mesh() gives a plain mesh of one LOD.
//
// EE addresses below are NTSC-U (SCUS_971.99): "boot" is the boot ELF,
// "level01" an in-game overlay.

#pragma once

#include <array>
#include <optional>
#include <vector>

#include "assets/bytes.h"
#include "assets/geometry/core_records.h"
#include "assets/geometry/gs_adgif.h"
#include "assets/geometry/mesh.h"

namespace openrac::assets::rac1 {

// The block header (0x10 bytes), at byte 0 of the tfrags block.
struct TfragBlockHeader {
    s32 table_offset = 0;  // 0x00: the TfragHeader table, from the block start
    s32 tfrag_count = 0;   // 0x04
    // 0x08: the LOD base distance L in world units (12..32 on the disc). The
    // level-load tfrag init (boot 0x2040e0) sets the switch distances to 6L,
    // 4L and 2L.
    f32 lod_base = 0;
    u32 unknown_c = 0;

    // [D0, D1, D2] = [6L, 4L, 2L] in world units.
    std::array<f32, 3> lod_distances() const;

    // What TfragProc compares against: trunc(D * 1024), raw units.
    std::array<s32, 3> lod_thresholds_raw() const;
};

static_assert(sizeof(TfragBlockHeader) == 0x10);

// The per-tfrag header (0x40 bytes). Offsets named *_ofs are relative to the
// tfrag's data, which starts at table_offset + data in the block.
struct TfragHeader {
    // 0x00: bounding sphere centre and radius in raw units (1024 = one world
    // unit), absolute.
    std::array<f32, 4> bsphere{};
    s32 data = 0;        // 0x10: the data, relative to table_offset
    u16 lod_2_ofs = 0;   // 0x14: the LOD-2 strip list
    u16 shared_ofs = 0;  // 0x16: the common list
    u16 lod_1_ofs = 0;   // 0x18: the LOD-1 strip list
    u16 lod_0_ofs = 0;   // 0x1a: the LOD-0+1 refinement list
    u16 tex_ofs = 0;     // 0x1c: the ad-gif payload inside the common list
    u16 rgba_ofs = 0;    // 0x1e: the RGBA array; also the end of the LOD-0 list
    u8 common_size = 0;  // 0x20: common list, quadwords
    u8 lod_2_size = 0;   // 0x21: LOD-2 + common, quadwords
    // 0x22: common + LOD-1 + LOD-01, quadwords from shared_ofs; the LOD-0-only
    // list starts at shared_ofs + lod_1_size * 16.
    u8 lod_1_size = 0;
    u8 lod_0_size = 0;        // 0x23: LOD-01 + LOD-0, quadwords from lod_0_ofs
    u8 lod_2_rgba_count = 0;  // 0x24
    u8 lod_1_rgba_count = 0;  // 0x25
    u8 lod_0_rgba_count = 0;  // 0x26
    u8 base_only = 0;         // 0x27: non-zero = always LOD 2
    u8 texture_count = 0;     // 0x28: ad-gif blocks
    u8 rgba_size = 0;         // 0x29: RGBA array, quadwords (4 colours each)
    u8 rgba_verts_loc = 0;    // 0x2a
    u8 occl_index_stash = 0;  // 0x2b
    u8 msphere_count = 0;     // 0x2c: texture spheres at msphere_ofs
    u8 flags = 0;             // 0x2d
    u16 msphere_ofs = 0;      // 0x2e
    u16 light_ofs = 0;        // 0x30: the origin quadword, then the light records
    u16 light_end_ofs = 0;    // 0x32
    u8 dir_lights_one = 0;    // 0x34: one light set for the whole tfrag; 0xff = per vertex
    u8 dir_lights_upd = 0;    // 0x35
    u16 point_lights = 0;     // 0x36: point light nibble list; 0xffff = none
    u16 cube_ofs = 0;         // 0x38: the clip box, 8 corners of 4 x s16 (x 64 = raw units)
    u16 occl_index = 0;       // 0x3a
    u8 vert_count = 0;        // 0x3c: positions (common + LOD-01 + LOD-0); also light records
    u8 tri_count = 0;         // 0x3d: LOD-0 triangles
    // 0x3e: texture paging distance, raw units: textures are paged in only
    // while the tfrag's near distance is within it.
    u16 mip_dist = 0;
};

static_assert(sizeof(TfragHeader) == 0x40);

// The VU header (20 x u16), unpacked to VU address 0. Each vertex-info tier
// (common, LOD-01, LOD-0) is one entry per position followed by extra entries
// (texture seams); the unk_* fields are the extra ranges' counts and
// addresses.
struct TfragVuHeader {
    u16 positions_common_count = 0;
    u16 unk_02 = 0;  // extra common vertex infos (never read by VU1)
    u16 positions_lod_01_count = 0;
    u16 unk_06 = 0;  // extra LOD-01 vertex infos: the length of unk_indices_2_lod01
    u16 positions_lod_0_count = 0;
    u16 unk_0a = 0;  // extra LOD-0 vertex infos: the length of unk_indices_2_lod0
    u16 positions_common_addr = 0;
    u16 vertex_info_common_addr = 0;
    u16 unk_10 = 0;
    u16 vertex_info_lod_01_addr = 0;
    u16 unk_14 = 0;
    u16 vertex_info_lod_0_addr = 0;
    u16 unk_18 = 0;
    u16 indices_addr = 0;
    u16 parent_indices_lod_01_addr = 0;
    u16 unk_indices_2_lod_01_addr = 0;
    u16 parent_indices_lod_0_addr = 0;
    u16 unk_indices_2_lod_0_addr = 0;
    u16 strips_addr = 0;
    u16 texture_ad_gifs_addr = 0;
};

static_assert(sizeof(TfragVuHeader) == 0x28);

// A position relative to Tfrag::origin, raw units (1024 per world unit), Z up.
struct TfragPosition {
    s16 x = 0;
    s16 y = 0;
    s16 z = 0;

    bool operator==(const TfragPosition&) const = default;
};

static_assert(sizeof(TfragPosition) == 6);

// A vertex info, what strip indices name.
struct TfragVertexInfo {
    s16 s = 0;  // texture S, 1/4096
    s16 t = 0;  // texture T, 1/4096
    // VU qword offset of the second LOD parent's position (position index =
    // parent / 2); meaningless for common entries.
    s16 parent = 0;
    s16 vertex = 0;  // VU qword offset of the own position (index = vertex / 2)

    bool operator==(const TfragVertexInfo&) const = default;
};

static_assert(sizeof(TfragVertexInfo) == 8);

// A strip command (see tfrag_triangles).
struct TfragStrip {
    s8 vertex_count_and_flag = 0;  // count; biased by -128 when the record also acts; 0 ends
    s8 end_of_packet_flag = 0;     // >= 0: load an ad-gif; negative: kick the GS packet
    s8 ad_gif_offset = 0;          // quadword offset into the ad-gifs (index = z / 5); -1 = keep
    s8 pad = 0;
};

static_assert(sizeof(TfragStrip) == 4);

// A vertex colour, by position index; 0x80 = 1.0.
struct TfragRgba {
    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
    u8 a = 0;

    bool operator==(const TfragRgba&) const = default;
};

static_assert(sizeof(TfragRgba) == 4);

// A light record, by position index, with the meanings the lighting pass
// (LightTfrags, tfrag_lighting.h) gives its fields.
struct TfragLight {
    u16 position_offset = 0;  // the position (3 x s16) from the tfrag data start (point lights)
    u8 azimuth = 0;           // normal azimuth, 2 pi / 256 per step
    u8 elevation = 0;         // normal elevation
    u16 color = 0;            // base colour, 5:5:5:1 (red in the low bits)
    // Light set select: bits 8..15 zero: set bits 0..3; else set bits 0..3
    // blended with set bits 4..7 by bits 8..15 / 256.
    u16 light_select = 0;
};

static_assert(sizeof(TfragLight) == 8);

// GS register values the load-time init writes over one ad-gif block.
struct TfragGsRegisters {
    u64 tex0 = 0;
    u64 tex1 = 0;
    u64 clamp = 0;
    u64 miptbp1 = 0;
    u64 miptbp2 = 0;

    bool operator==(const TfragGsRegisters&) const = default;
};

// A texture primitive: five A+D quadwords copied into the GS packet. On the
// disc the register values are packed fields the init rewrites.
struct TfragAdGifs {
    AdGif tex0;     // data_lo = index into the tfrag texture table, data_hi = 0
    AdGif tex1;     // data_lo = LOD K (s16, 1/16), data_hi = MMIN (4)
    AdGif clamp;    // data_lo = WMS, data_hi = WMT (0 repeat, 1 clamp)
    AdGif miptbp1;  // 0 on the disc, built at load
    AdGif miptbp2;  // 0

    // Index into the level's tfrag texture table.
    std::size_t texture_index() const { return static_cast<u32>(tex0.data_lo); }

    GsWrap wrap_s() const;
    GsWrap wrap_t() const;
    GsFilter mag_filter() const;  // always Linear: the init sets MMAG
    GsFilter min_filter() const;  // LinearMipmapNearest on every disc ad-gif

    // TEX1 K, 1/16 mip levels (-137..-89 on the disc), and in mip levels.
    s16 lod_k_raw() const { return assets::lod_k_raw(tex1.data_lo); }

    f32 lod_k() const { return static_cast<f32>(lod_k_raw()) / 16.0f; }

    // Mip levels including the base (the texture's level count).
    u32 mip_levels(const CoreTextureEntry& texture) const;

    // The registers the init (boot 0x2040e0) writes; `texture` is the tfrag
    // table entry this block names, `gs_base` the level texture area's GS
    // byte address.
    TfragGsRegisters gs_registers(const CoreTextureEntry& texture, u32 gs_base) const;
};

static_assert(sizeof(TfragAdGifs) == 0x50);

// The strips and index bytes of one LOD.
struct TfragLod {
    std::vector<TfragStrip> strips;  // ends with a zero-count record
    std::vector<u8> indices;         // into Tfrag::vertex_info, zero-padded to 4
};

// A texture-paging sphere: textures of a tfrag are paged in by distance.
struct TfragTextureSphere {
    std::array<f32, 3> centre{};  // absolute, raw units
    u16 radius = 0;               // raw units
    // Paging distance / 128: mip 1 is needed while view depth - radius <=
    // m * 128, the base level while <= m * 64.
    u8 mip_dist_128 = 0;
    u8 texture_index = 0;  // into the tfrag texture table

    bool operator==(const TfragTextureSphere&) const = default;
};

// The draw path TfragProc (boot 0x233fb0) picks; the value is its MSCAL
// address in the VU1 tfrag program.
enum class TfragDrawMode : u8 {
    Lod2 = 0x06,          // LOD 2, no morph
    Lod1Collapse = 0x08,  // LOD 1; LOD-01 morph, collapse past D0
    Lod1Morph = 0x0a,     // LOD 1; LOD-01 morph
    Lod0Collapse = 0x0e,  // LOD 0; LOD-01 morph, LOD-0 morph with collapse at D1
    Lod0Morph = 0x10,     // LOD 0; LOD-01 at rest, LOD-0 morph
    Lod0 = 0x14,          // LOD 0, no morph
};

// The strip list a draw mode draws (0 = highest detail).
int tfrag_draw_lod(TfragDrawMode mode);

// Which morph tier a vertex info belongs to.
enum class TfragMorphTier : u8 {
    Lod01,  // fades between D1 and D0
    Lod0,   // fades between D2 and D1
};

// The morph amount u in [0, 1] (0 = own position, 1 = the parents' midpoint)
// at view depth `depth` (world units).
f32 tfrag_morph_weight(TfragMorphTier tier, f32 depth, const std::array<f32, 3>& lod_distances);

// A vertex info is replaced by its first parent when both parents are at or
// beyond this distance.
f32 tfrag_collapse_distance(TfragMorphTier tier, const std::array<f32, 3>& lod_distances);

// The LOD linkage of one LOD-01 or LOD-0 vertex info.
struct TfragLodLink {
    TfragMorphTier tier = TfragMorphTier::Lod01;
    // The first positions_lod_XX_count entries of a tier morph (into their own
    // position's slot); extra entries only collapse.
    bool morphs = false;
    std::size_t own_position = 0;
    std::size_t parent1_vinfo = 0;  // also the collapse replacement (UV and position)
    std::size_t parent1_position = 0;
    std::size_t parent2_position = 0;

    bool operator==(const TfragLodLink&) const = default;
};

// One parsed tfrag: its headers and every array the VIF lists carry, kept as the game lays them
// out so a renderer can draw any LOD and morph between them.
struct Tfrag {
    TfragHeader header;
    TfragVuHeader vu;
    // The integer origin added to every position: the last origin STROW row
    // of the lists, else the quadword at light_ofs.
    std::array<s32, 4> origin{};
    // Common, LOD-01 and LOD-0 concatenated: the index space of rgba, lights
    // and vertex_info[].vertex / 2.
    std::vector<TfragPosition> positions;
    // The same tiers: the index space of strip and parent indices.
    std::vector<TfragVertexInfo> vertex_info;
    u32 positions_common = 0;
    u32 positions_lod01 = 0;
    u32 positions_lod0 = 0;
    u32 vinfo_common = 0;
    u32 vinfo_lod01 = 0;
    u32 vinfo_lod0 = 0;
    // For LOD-01 vertex info i (i < positions_lod_01_count, padded to 4): the
    // vertex info of its first LOD parent, always a common one; the second
    // parent is the entry's parent / 2 position.
    std::vector<u8> parent_indices_lod01;
    // Per extra LOD-01 vertex info: the collapse replacement (VU1 pass L48).
    std::vector<u8> unk_indices_2_lod01;
    // For LOD-0 vertex info i: its first parent's vertex info (common or LOD-01).
    std::vector<u8> parent_indices_lod0;
    // Per extra LOD-0 vertex info: the collapse replacement (VU1 pass L47).
    std::vector<u8> unk_indices_2_lod0;
    std::array<TfragLod, 3> lod;  // [0] = highest detail
    std::vector<TfragAdGifs> ad_gifs;
    std::vector<TfragRgba> rgba;     // rgba_size * 4 colours, by position index
    std::vector<TfragLight> lights;  // vert_count records, by position index
    // Texture spheres as stored (lane w is packed, not a float): see
    // texture_spheres().
    std::vector<std::array<f32, 4>> mspheres;
    // The clip box: 8 corners, x/y/z * 64 = absolute raw units, w = 0.
    std::array<std::array<s16, 4>, 8> cube{};

    // Vertex infos a LOD draws from (0..n): LOD 2 the common tier, LOD 1 plus
    // LOD-01, LOD 0 all.
    u32 lod_vertex_info_count(int lod) const;
    u32 lod_position_count(int lod) const;

    // The position index of a vertex info (vertex / 2); out of range is 0.
    std::size_t position_index(std::size_t vinfo) const;

    // (origin + local) / 1024: world units.
    std::array<f32, 3> world_position(std::size_t vinfo) const;

    std::vector<TfragTextureSphere> texture_spheres() const;

    // TfragProc's draw path for a centre depth (view space, raw units),
    // ignoring frustum culling and the guard-band clip test.
    TfragDrawMode draw_mode(f32 centre_depth_raw, const std::array<s32, 3>& thresholds_raw) const;

    // The LOD linkage of a vertex info; empty for common entries, padding or
    // out-of-range indices (VU1 L17-L36, L47/L48).
    std::optional<TfragLodLink> lod_link(std::size_t vinfo) const;

    // The (parent1, parent2) positions a morphing entry blends toward:
    // pos = lerp(own, (p1 + p2) / 2, u), the same for colour; UV does not
    // morph. Empty for common and extra entries.
    std::optional<std::pair<std::size_t, std::size_t>> morph_parents(std::size_t vinfo) const;
};

// A triangle of vertex-info indices and the ad-gif block in effect.
struct TfragTriangle {
    u16 a = 0;
    u16 b = 0;
    u16 c = 0;
    u16 ad_gif = 0;

    bool operator==(const TfragTriangle&) const = default;
};

// Reads the 0x10-byte header at the start of a tfrags block.
TfragBlockHeader parse_tfrag_block_header(ByteView block);

// Every tfrag of a tfrags block (the core data's "tfrags" block).
std::vector<Tfrag> parse_tfrags(ByteView block);

// A LOD's strips as triangles (GS strip order, alternating winding), lod 0..2.
//
// Texture assignment follows the VU1 strip processor (program 55907, EE
// 0x103578): the first record always loads the ad-gif at z and is always
// biased (x + 128); later, x > 0 is a plain run of x vertices, x == 0 ends
// the list, and x < 0 is a run of x + 128 vertices that loads the ad-gif at z
// when y >= 0, and otherwise kicks the packet and loads the ad-gif at z only
// if z >= 0 (-1 keeps the texture). A load whose offset is negative, not a
// multiple of 5 or past the array is an error.
std::vector<TfragTriangle> tfrag_triangles(const Tfrag& tfrag, int lod);

// One LOD as a plain mesh: a vertex per vertex info of the LOD (world
// position, UV s/4096 t/4096, the stored RGBA and the light record's normal,
// both by position), triangles carrying the tfrag texture table index.
Mesh tfrag_mesh(const Tfrag& tfrag, int lod);

}  // namespace openrac::assets::rac1
