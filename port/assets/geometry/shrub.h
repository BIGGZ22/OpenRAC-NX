// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/shrub.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// RAC1 shrubs: small instanced props with a far-LOD billboard (ReRAC
// docs/formats/shrub_sky_rac1.md part 1, docs/plan/shrub_lighting.md). The
// packet semantics follow the VU1 shrub program 56467 (EE 0x101768, uploaded
// by ShrubProc, NTSC-U level01 0x29cdf0; spec 1.3b), not only Wrench:
//
//   - a packet's three UNPACKs (FLG, TOPS-relative) fill a 0x76-quadword
//     input buffer; VU1 reads the packet header at qw 0, the GIF tags from qw
//     1, the ad-gif blocks after them, vertex part 1 at vertex_offset and part
//     2 at vertex_offset + vertex_count, by address;
//   - VU1 copies every GIF tag to its GS-packet slot, then every ad-gif block
//     (its own A+D tag and the four stored quadwords), then writes ST / RGBAQ
//     / XYZF2 of vertices 0 ..= stop + 3 to slots off .. off + 3, where stop is
//     the first vertex from index 2 on with bit 15 of n_and_stop set (the
//     pipelined loop never tests vertices 0 and 1 and drains three after the
//     flag); later writes win (padding vertices rewrite their original's);
//   - the GIF starts at slot 0 of the 0xa8-qw output buffer and reads A+D
//     blocks (5 qw) and vertex tags (1 + 3 * NLOOP qw) up to the tag with EOP.
//
// Lighting inputs are kept: every vertex carries a normal index 0..23 that
// selects one of the class's 24 normals and the matching entry of the
// instance's palette (shrub_lighting.h). EE addresses are NTSC-U.

#pragma once

#include <array>
#include <bit>
#include <optional>
#include <span>
#include <vector>

#include "assets/bytes.h"
#include "assets/geometry/core_records.h"
#include "assets/geometry/gs_adgif.h"
#include "assets/geometry/mesh.h"

namespace openrac::assets::rac1 {

// The VU1 input buffer (quadwords; double buffered at VU 0x02 / 0x78).
constexpr std::size_t kShrubInputQwc = 0x76;
// One GS-packet output buffer (quadwords; triple buffered).
constexpr std::size_t kShrubOutputQwc = 0xa8;
// Normals per class, palette entries per instance.
constexpr std::size_t kShrubNormals = 24;

// The class header at byte 0 of a class blob (0x40 bytes); offsets are
// blob-relative.
struct ShrubClassHeader {
    std::array<f32, 4> bsphere{};  // 0x00: in units of `scale` world units
    f32 mip_distance = 0;          // 0x10
    u16 mode_bits = 0;             // 0x14: & 6 selects the wind sway
    s16 instance_count = 0;        // 0x16: run time
    s32 instances_pointer = 0;     // 0x18: run time
    s32 billboard_offset = 0;      // 0x1c: ShrubBillboard, 0 = none
    f32 scale = 0;                 // 0x20: class units = packed s16 * scale / 1024
    s16 o_class = 0;               // 0x24
    s16 s_class = 0;               // 0x26
    s16 packet_count = 0;          // 0x28: the packet table at 0x40
    s16 pad_2a = 0;
    s32 normals_offset = 0;  // 0x2c: 24 normals
    s32 pad_30 = 0;
    s16 drawn_count = 0;  // 0x34..: run-time counters
    s16 scis_count = 0;
    s16 billboard_count = 0;
    std::array<s16, 3> pad_3a{};
};

static_assert(sizeof(ShrubClassHeader) == 0x40);

// A packet table entry (8 bytes) at blob + 0x40.
struct ShrubPacketEntry {
    s32 offset = 0;  // the VIF list, blob-relative
    s32 size = 0;

    bool operator==(const ShrubPacketEntry&) const = default;
};

static_assert(sizeof(ShrubPacketEntry) == 8);

// Input-buffer quadword 0.
struct ShrubPacketHeader {
    s32 texture_count = 0;  // ad-gif blocks (>= 1: VU1's copy loop is a do-while)
    s32 gif_tag_count = 0;  // vertex GIF tags (>= 1)
    s32 vertex_count = 0;   // entries in each vertex table
    s32 vertex_offset = 0;  // input-buffer qw of part 1; part 2 follows vertex_count later
};

static_assert(sizeof(ShrubPacketHeader) == 0x10);

// A vertex GIF tag: the GIFtag VU1 copies, and its GS slot in the fourth word.
struct ShrubGifTag {
    u64 tag = 0;     // NLOOP, EOP, PRE, PRIM, FLG, NREG
    u32 tag_hi = 0;  // REGS bits 0..31 (0x412: ST, RGBAQ, XYZF2)
    s32 gs_packet_offset = 0;

    u32 nloop() const { return static_cast<u32>(tag & 0x7fff); }

    bool eop() const { return ((tag >> 15) & 1) != 0; }

    bool pre() const { return ((tag >> 46) & 1) != 0; }

    // GS primitive: 3 = triangle list, 4 = triangle strip.
    u8 prim() const { return static_cast<u8>((tag >> 47) & 7); }

    u8 flg() const { return static_cast<u8>((tag >> 58) & 3); }

    u8 nreg() const { return static_cast<u8>(tag >> 60); }
};

static_assert(sizeof(ShrubGifTag) == 0x10);

// The four register values the class init writes over an ad-gif block.
struct ShrubGsRegisters {
    u64 tex1 = 0;
    u64 clamp = 0;
    u64 miptbp1 = 0;
    u64 tex0 = 0;

    bool operator==(const ShrubGsRegisters&) const = default;
};

// An ad-gif block (4 A+D quadwords). VU1 writes its own A+D GIF tag in front,
// so a block takes 5 GS-packet quadwords.
struct ShrubAdGifs {
    AdGif tex1;
    AdGif clamp;
    AdGif miptbp1;
    AdGif tex0;  // data_lo = the class texture slot (patched to a real TEX0 at load)

    // The block's GS-packet slot: the w lane of the TEX1 quadword.
    s32 gs_packet_offset() const { return tex1.w_lane(); }

    // TEX1 K, 1/16 mip levels (-142..-110 on the disc), and in mip levels.
    s16 lod_k_raw() const { return assets::lod_k_raw(tex1.data_lo); }

    f32 lod_k() const { return static_cast<f32>(lod_k_raw()) / 16.0f; }

    // CLAMP WMS / WMT: true = clamp, false = repeat.
    std::pair<bool, bool> clamp_st() const {
        return {(clamp.data_lo & 1) != 0, (clamp.data_hi & 1) != 0};
    }

    // The class init's conversion (boot 0x203b08), bit for bit the tfrag
    // rule: `texture_index` is the class entry's textures[tex0.data_lo] (a
    // shrub texture table index), `texture` that entry.
    ShrubGsRegisters gs_registers(
        u8 texture_index, const CoreTextureEntry& texture, u32 gs_base
    ) const;
};

static_assert(sizeof(ShrubAdGifs) == 0x40);

// Vertex part 1 and part 2 as VU1 sees them (V4_16 signed).
struct ShrubVertexPart1 {
    s16 x = 0;
    s16 y = 0;
    s16 z = 0;
    s16 gs_packet_offset = 0;
};

struct ShrubVertexPart2 {
    s16 s = 0;
    s16 t = 0;
    s16 q = 0;
    u16 n_and_stop = 0;
};

// One vertex as VU1 processes it.
struct ShrubVertex {
    std::array<s16, 3> position{};  // class space; x scale / 1024
    s16 gs_packet_offset = 0;       // the ST slot (RGBAQ +1, XYZF2 +2)
    std::array<s16, 2> st{};        // 1/4096
    s16 q = 0;                      // multiplies the perspective Q (0x1000 on the disc)
    u8 normal = 0;                  // 0..23: class normal and palette entry
    u8 stop = 0;                    // 1 on the vertex whose flag ends VU1's loop

    std::array<f32, 3> class_position(f32 class_scale) const;

    std::array<f32, 2> uv() const { return {st[0] / 4096.0f, st[1] / 4096.0f}; }

    bool operator==(const ShrubVertex&) const = default;
};

// One vertex GIF tag in GS order.
struct ShrubDraw {
    // The class texture slot in effect (tex0.data_lo of the last ad-gif block
    // the GIF read; GS state carries across packets).
    u8 texture = 0;
    u8 prim = 0;                // 3 = list, 4 = strip (every disc draw)
    std::vector<u16> vertices;  // into ShrubPacket::vertices

    bool operator==(const ShrubDraw&) const = default;
};

struct ShrubTriangle {
    u16 a = 0;
    u16 b = 0;
    u16 c = 0;
    u16 texture = 0;

    bool operator==(const ShrubTriangle&) const = default;
};

struct ShrubPacket {
    ShrubPacketEntry entry;
    ShrubPacketHeader header;
    std::vector<ShrubGifTag> gif_tags;
    std::vector<ShrubAdGifs> ad_gifs;
    std::vector<ShrubVertexPart1> part1;  // vertex_count entries
    std::vector<ShrubVertexPart2> part2;
    std::vector<ShrubVertex> vertices;  // the stop + 4 vertices VU1 writes
    std::vector<ShrubDraw> draws;
};

// The billboard registers after the class init (see ShrubBillboard).
struct ShrubBillboardRegisters {
    u64 tex1 = 0;
    u64 tex0 = 0;
    u64 miptbp1 = 0;

    u32 mxl() const { return static_cast<u32>((tex1 >> 2) & 7); }

    f32 lod_k() const;
};

// The far-LOD billboard at billboard_offset (0x40 bytes). The class init
// (boot 0x203b08) rewrites its three A+D words; ShrubProc loads qw 0 and sends
// qw 1..3 verbatim (ReRAC docs/plan/shrub_lighting.md 6).
struct ShrubBillboard {
    // 0x00: F = trunc(fade_distance) as a byte (256 wraps to 0 = billboard
    // only); the loader raises the instance draw distance to at least F + 24.
    f32 fade_distance = 0;
    f32 width = 0;   // quad width, class units, x the mean of the x/y column lengths
    f32 height = 0;  // quad height, class units, x the z column length
    f32 z_ofs = 0;   // the quad's bottom edge above the origin, like height
    AdGif tex1;      // data_lo = K, data_hi = MMIN
    AdGif tex0;      // overwritten at load
    AdGif miptbp1;   // built at load

    // TEX1 = MXL (levels - 1) << 2 | MMIN << 6 | MMAG << 5 | K << 32; TEX0 with
    // TBP0 = texture_block + base (billboard textures are resident); MIPTBP1 =
    // TBP1..3 = mip1..3 + base with TBW_k = max(1, width >> (6 + k)).
    ShrubBillboardRegisters gs_registers(const CoreBillboardInfo& info, u32 gs_base) const;
};

static_assert(sizeof(ShrubBillboard) == 0x40);

struct ShrubClass {
    ShrubClassHeader header;
    std::vector<ShrubPacket> packets;
    std::vector<std::array<s16, 4>> normals;  // 24, unit in 1/32767
    std::optional<ShrubBillboard> billboard;

    std::optional<std::array<f32, 3>> normal(u8 index) const;
};

ShrubClass parse_shrub_class(ByteView blob);

// One packet's VIF list walked the way VU1 and the GIF consume it. `texture`
// is the GS texture slot in effect, carried from packet to packet (-1 = none
// yet); parse_shrub_class calls it.
ShrubPacket read_shrub_packet(ByteView list, const ShrubPacketEntry& entry, s32& texture);

// Strips as (i-2, i-1, i) for even i and (i, i-1, i-2) for odd i (facing is
// not guaranteed by the disc), lists three at a time.
std::vector<ShrubTriangle> shrub_triangles(const ShrubPacket& packet);

// The class as a plain mesh in class units: normals from the class's 24,
// colours left at 1.0 (they are per instance), triangles carrying the class
// texture slot.
Mesh shrub_mesh(const ShrubClass& shrub);

// A level shrub class: its core table entry and the parsed blob.
struct LevelShrubClass {
    CoreShrubClassEntry entry;
    ShrubClass shrub;
};

std::vector<LevelShrubClass> parse_level_shrub_classes(
    std::span<const CoreShrubClassEntry> table, ByteView core_data
);

// --- Billboards (ShrubProc, level01 0x29cdf0 = boot 0x228be8)

// The matrix block's column-2 w word the level loader builds (level01
// 0x255958): lo | hi << 16 with lo = min(trunc((|c0| + |c1|) / 2 * 4096),
// 0x10000) and hi = trunc(|c2| * 4096), or 0 above 0x10000 (lo = 0x10000
// spills into bit 16, as in the game).
u32 packed_column_lengths(const std::array<std::array<f32, 4>, 4>& matrix);

// The billboard quad of one instance in world units: (width, height, z_ofs).
std::array<f32, 3> billboard_extent(
    const ShrubBillboard& billboard, f32 class_scale, u32 packed_lengths
);

// The quad's corners in GS order (a 4-vertex strip), (y, z, s, t): positions
// (0, y, z, 1) and ST (s, t, 1). Format constants of the game's drawing code.
constexpr std::array<std::array<f32, 4>, 4> kBillboardCorners = {{
    {-0.5f, 1.0f, 0.0f, 0.0f},
    {0.5f, 1.0f, 1.0f, 0.0f},
    {-0.5f, 0.0f, 0.0f, 1.0f},
    {0.5f, 0.0f, 1.0f, 1.0f},
}};

// World corners of an instance's billboard (Z up), facing the eye: with d =
// unit(t - eye), corner = t + y * W * (d.y, -d.x, 0) + (z * H + Z) * z-hat. The
// horizontal axis has length cos(elevation), so the quad narrows from above.
std::array<std::array<f32, 3>, 4> billboard_corners(
    const std::array<f32, 3>& origin,
    const std::array<f32, 3>& eye,
    const std::array<f32, 3>& extent
);

// ShrubProc's mesh and billboard alphas (GS 0..0x80) for an instance that
// passed the distance and frustum tests: z = view depth of the bounding-sphere
// centre, d = the run-time draw distance, f = the class's billboard byte F
// (empty: no billboard). A billboard alpha of 0 is listed but never drawn.
struct ShrubFade {
    std::optional<u8> mesh;
    std::optional<u8> billboard;

    bool operator==(const ShrubFade&) const = default;
};

ShrubFade shrub_fade(f32 z, f32 d, std::optional<u8> f);

// --- Wind sway (mode_bits & 6; level01 0x29d4f8..0x29d670)

// ShrubProc's sway constants (its vf8 and vf9): gust scale, sway amplitude, x
// and y lean weights; gust depth, range squared (world units), 1 / range
// squared, the mode-1 factor.
constexpr std::array<f32, 4> kSwayVf8 = {0.1f, 0.02f, 1.0f, 0.0f};
// The third lane is stored as the float bits 0x392ec33e (about 1 / 6000).
constexpr std::array<f32, 4> kSwayVf9 = {0.2f, 6000.0f, std::bit_cast<f32>(u32{0x392e'c33e}), 0.5f};

// The sway table ShrubProc reads: entry i = trunc(-127 sin(2 pi i / 256)).
s8 sway_table(u32 i);

// The wind shear of one instance this frame, (sx, sy): ShrubProc replaces the
// scaled instance columns c by (c.x + sx c.z, c.y + sy c.z, c.z). `mode` =
// (mode_bits & 6) >> 1, `block` = EE address of the instance's matrix block,
// `tick` = the frame counter, `rel` = origin - camera (world units). Empty for
// mode 0 or beyond the range.
std::optional<std::array<f32, 2>> wind_sway(
    u16 mode, u32 block, u32 tick, const std::array<f32, 3>& rel
);

// --- Instances

constexpr std::size_t kGameplayShrubClasses = 0x38;  // s32 count, count x s32
constexpr std::size_t kGameplayShrubInstances = 0x3c;

// A gameplay shrub instance (0x70 bytes). Field use from the level loader
// (level01 0x255958). There is no uid: an instance is its section index.
struct ShrubInstance {
    s32 o_class = 0;
    f32 draw_distance = 0;  // world units; the loader clamps it to >= 16 (and F + 24)
    s32 unused_08 = 0;
    s32 unused_0c = 0;  // no occlusion index: shrubs are not occlusion-culled
    // Class to world, column-major, translation in column 3; [3][3] is 0.01
    // on the disc and kept raw.
    std::array<std::array<f32, 4>, 4> matrix{};
    // Base colour r, g, b 0..255; the loader packs r | g << 8 | b << 16 | 0x80 << 24.
    std::array<s32, 3> colour{};
    s32 unused_5c = 0;
    // Light set select (low 16 bits): bits 0..3 set A, 4..7 set B, 8..15 the
    // blend weight of B (0 = A only).
    s32 dir_lights = 0;
    std::array<s32, 3> unused_64{};

    std::array<std::array<f32, 4>, 4> world_matrix() const;
    std::array<f32, 3> transform_point(const std::array<f32, 3>& p) const;

    // The ambient as the loader packs it, alpha 0x80.
    std::array<u8, 4> ambient_rgba() const;

    struct LightSets {
        u8 a = 0;
        u8 b = 0;
        u8 weight_of_b = 0;  // 1/256

        bool operator==(const LightSets&) const = default;
    };

    LightSets light_sets() const;
};

static_assert(sizeof(ShrubInstance) == 0x70);

std::vector<ShrubInstance> parse_shrub_instance_section(ByteView section);
std::vector<ShrubInstance> parse_shrub_instances(ByteView gameplay);

// The gameplay file's shrub class list (the distinct instance classes).
std::vector<s32> parse_shrub_class_list(ByteView gameplay);

}  // namespace openrac::assets::rac1
