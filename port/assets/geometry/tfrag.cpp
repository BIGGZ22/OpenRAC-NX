// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The RAC1 tfrag reader: the VIF list walk, strips, ad-gif registers and LOD
// selection. EE addresses are NTSC-U.

#include "assets/geometry/tfrag.h"

#include <algorithm>
#include <bit>
#include <cstring>

#include "assets/geometry/texture.h"
#include "assets/geometry/vif.h"

namespace openrac::assets::rac1 {

std::array<f32, 3> TfragBlockHeader::lod_distances() const {
    return {lod_base * 6.0f, lod_base * 4.0f, lod_base + lod_base};
}

std::array<s32, 3> TfragBlockHeader::lod_thresholds_raw() const {
    const auto d = lod_distances();
    return {
        static_cast<s32>(d[0] * 1024.0f),
        static_cast<s32>(d[1] * 1024.0f),
        static_cast<s32>(d[2] * 1024.0f)
    };
}

GsWrap TfragAdGifs::wrap_s() const {
    return gs_wrap(loaded_clamp(clamp.data_lo, clamp.data_hi, tex0.data_lo));
}

GsWrap TfragAdGifs::wrap_t() const {
    return gs_wrap(loaded_clamp(clamp.data_lo, clamp.data_hi, tex0.data_lo) >> 2);
}

GsFilter TfragAdGifs::mag_filter() const {
    return gs_filter(loaded_tex1(1, tex1.data_lo, tex1.data_hi) >> 5 & 1);
}

GsFilter TfragAdGifs::min_filter() const {
    return gs_filter(loaded_tex1(1, tex1.data_lo, tex1.data_hi) >> 6);
}

u32 TfragAdGifs::mip_levels(const CoreTextureEntry& texture) const {
    return static_cast<u32>((loaded_tex1(texture.levels, tex1.data_lo, tex1.data_hi) >> 2 & 7) + 1);
}

TfragGsRegisters TfragAdGifs::gs_registers(const CoreTextureEntry& texture, u32 gs_base) const {
    // The init also copies the texture index into CLAMP bits 24..31 (MINV,
    // unused by REPEAT and CLAMP), where the per-frame patcher reads it back.
    return {
        loaded_tex0(texture, gs_base),
        loaded_tex1(texture.levels, tex1.data_lo, tex1.data_hi),
        loaded_clamp(clamp.data_lo, clamp.data_hi, tex0.data_lo),
        loaded_miptbp1(texture, gs_base),
        0,
    };
}

int tfrag_draw_lod(TfragDrawMode mode) {
    switch (mode) {
        case TfragDrawMode::Lod2:
            return 2;
        case TfragDrawMode::Lod1Collapse:
        case TfragDrawMode::Lod1Morph:
            return 1;
        default:
            return 0;
    }
}

f32 tfrag_morph_weight(TfragMorphTier tier, f32 depth, const std::array<f32, 3>& d) {
    // VU1 L17/L18: t = clamp(w * qw666.xy + qw668.xy, 0, (0.5, 1)) with w the
    // scaled depth, which is (u / 2, 1 - u).
    const f32 near = tier == TfragMorphTier::Lod01 ? d[1] : d[2];
    const f32 far = tier == TfragMorphTier::Lod01 ? d[0] : d[1];
    return std::clamp((depth - near) / (far - near), 0.0f, 1.0f);
}

f32 tfrag_collapse_distance(TfragMorphTier tier, const std::array<f32, 3>& d) {
    return tier == TfragMorphTier::Lod01 ? d[0] : d[1];
}

u32 Tfrag::lod_vertex_info_count(int l) const {
    switch (l) {
        case 0:
            return vinfo_common + vinfo_lod01 + vinfo_lod0;
        case 1:
            return vinfo_common + vinfo_lod01;
        default:
            return vinfo_common;
    }
}

u32 Tfrag::lod_position_count(int l) const {
    switch (l) {
        case 0:
            return positions_common + positions_lod01 + positions_lod0;
        case 1:
            return positions_common + positions_lod01;
        default:
            return positions_common;
    }
}

namespace {

// A VU qword offset (s16) as a position index; a negative one wraps far out
// of range, as the reference export's unsigned arithmetic does.
std::size_t position_of(s16 offset) {
    return static_cast<std::size_t>(static_cast<s64>(offset)) / 2;
}

}  // namespace

std::size_t Tfrag::position_index(std::size_t vinfo) const {
    const std::size_t p = position_of(vertex_info.at(vinfo).vertex);
    return p >= positions.size() ? 0 : p;
}

std::array<f32, 3> Tfrag::world_position(std::size_t vinfo) const {
    const TfragPosition& p = positions.at(position_index(vinfo));
    auto axis = [](s32 o, s16 v) {
        return static_cast<f32>(static_cast<s32>(static_cast<u32>(o) + static_cast<u32>(s32{v})))
               / 1024.0f;
    };
    return {axis(origin[0], p.x), axis(origin[1], p.y), axis(origin[2], p.z)};
}

std::vector<TfragTextureSphere> Tfrag::texture_spheres() const {
    std::vector<TfragTextureSphere> out;

    for (const auto& s : mspheres) {
        const u32 w = std::bit_cast<u32>(s[3]);
        out.push_back(
            {{s[0], s[1], s[2]},
             static_cast<u16>(w),
             static_cast<u8>(w >> 16),
             static_cast<u8>(w >> 24)}
        );
    }

    return out;
}

TfragDrawMode Tfrag::draw_mode(f32 centre_depth_raw, const std::array<s32, 3>& thresholds) const {
    // near / far = depth -+ radius, truncated (vftoi0).
    const f32 r = header.bsphere[3];
    const auto far = static_cast<s32>(centre_depth_raw + r);
    const auto near = static_cast<s32>(centre_depth_raw - r);
    const auto [d0, d1, d2] = thresholds;

    if (header.base_only != 0 || near >= d0) {
        return TfragDrawMode::Lod2;
    }

    if (far >= d0) {
        return TfragDrawMode::Lod1Collapse;
    }

    if (near >= d1) {
        return TfragDrawMode::Lod1Morph;
    }

    if (far >= d1) {
        return TfragDrawMode::Lod0Collapse;
    }

    if (near >= d2 || far >= d2) {
        return TfragDrawMode::Lod0Morph;
    }

    return TfragDrawMode::Lod0;
}

std::optional<TfragLodLink> Tfrag::lod_link(std::size_t vinfo) const {
    const std::size_t l01_begin = vinfo_common;
    const std::size_t l01_end = l01_begin + vinfo_lod01;
    const std::size_t l0_end = l01_end + vinfo_lod0;
    TfragMorphTier tier;
    std::size_t k;
    std::size_t primaries;
    const std::vector<u8>* parents;
    const std::vector<u8>* extras;

    if (vinfo >= l01_begin && vinfo < l01_end) {
        tier = TfragMorphTier::Lod01;
        k = vinfo - l01_begin;
        primaries = vu.positions_lod_01_count;
        parents = &parent_indices_lod01;
        extras = &unk_indices_2_lod01;
    } else if (vinfo >= l01_end && vinfo < l0_end) {
        tier = TfragMorphTier::Lod0;
        k = vinfo - l01_end;
        primaries = vu.positions_lod_0_count;
        parents = &parent_indices_lod0;
        extras = &unk_indices_2_lod0;
    } else {
        return std::nullopt;
    }

    const bool morphs = k < primaries;
    const std::vector<u8>& list = morphs ? *parents : *extras;
    const std::size_t at = morphs ? k : k - primaries;

    if (at >= list.size() || vinfo >= vertex_info.size()) {
        return std::nullopt;
    }

    const std::size_t p1 = list[at];

    if (p1 >= vertex_info.size()) {
        return std::nullopt;
    }

    auto pos = [&](s16 v) -> std::optional<std::size_t> {
        const std::size_t p = position_of(v);

        if (p < positions.size()) {
            return p;
        }

        return std::nullopt;
    };
    const TfragVertexInfo& e = vertex_info[vinfo];
    const auto own = pos(e.vertex);
    const auto parent1 = pos(vertex_info[p1].vertex);
    const auto parent2 = pos(e.parent);

    if (!own || !parent1 || !parent2) {
        return std::nullopt;
    }

    return TfragLodLink{tier, morphs, *own, p1, *parent1, *parent2};
}

std::optional<std::pair<std::size_t, std::size_t>> Tfrag::morph_parents(std::size_t vinfo) const {
    const auto link = lod_link(vinfo);

    if (!link || !link->morphs) {
        return std::nullopt;
    }

    return std::pair{link->parent1_position, link->parent2_position};
}

namespace {

template <typename T>
std::vector<T> records(ByteView data, u32 count, std::string_view what) {
    if (data.size() < std::size_t{count} * sizeof(T)) {
        fail("tfrag {}: unpack payload smaller than its element count", what);
    }

    return data.read_array<T>(0, count, what);
}

// The value the vertex-info STROW rows carry in lane x: 2048.0f, the UV bias.
constexpr s32 kUvBiasRow = 0x4500'0000;

// Classifies each unpack of a VIF list by format and VU address, the way the
// reference extractor did.
class ListWalk {
public:
    explicit ListWalk(const TfragHeader& header, const TfragVuHeader& vu = {}) {
        m_tfrag.header = header;
        m_tfrag.vu = vu;
    }

    Tfrag& tfrag() { return m_tfrag; }

    bool have_origin() const { return m_have_origin; }

    void run(ByteView list) {
        if (list.empty()) {
            return;
        }

        Tfrag& t = m_tfrag;

        for (const vif::Code& code : vif::parse(list)) {
            if (code.cmd == vif::kStrow) {
                const auto row = code.data.read<std::array<s32, 4>>(0, "STROW row");

                // The origin row precedes every position unpack; the other
                // rows are the index and vertex-info bases.
                if (row[0] != kUvBiasRow
                    && static_cast<u32>(row[0]) != t.vu.vertex_info_common_addr) {
                    t.origin = row;
                    m_have_origin = true;
                }

                continue;
            }

            if (!code.is_unpack()) {
                continue;
            }

            const u16 a = code.addr();
            const u32 n = code.count();
            const TfragVuHeader vu = t.vu;
            const int vn = code.vn();
            const int vl = code.vl();

            if (vn == 3 && vl == 1 && code.usn() && a == 0) {
                t.vu = code.data.read<TfragVuHeader>(0, "tfrag VU header");
            } else if (vn == 3 && vl == 0) {
                t.ad_gifs = records<TfragAdGifs>(code.data, n / 5, "ad-gifs");
            } else if (vn == 2 && vl == 1) {
                auto pos = records<TfragPosition>(code.data, n, "positions");

                if (a == vu.positions_common_addr) {
                    t.positions_common = n;
                } else if (n != 0 && t.positions_lod01 == 0 && t.positions_lod0 == 0
                           && a
                                  == static_cast<u32>(vu.positions_common_addr)
                                         + 2u * t.positions_common) {
                    t.positions_lod01 = n;
                } else {
                    t.positions_lod0 = n;
                }

                t.positions.insert(t.positions.end(), pos.begin(), pos.end());
            } else if (vn == 3 && vl == 1) {
                auto vi = records<TfragVertexInfo>(code.data, n, "vertex infos");

                if (a == vu.vertex_info_common_addr) {
                    t.vinfo_common = n;
                } else if (a == vu.vertex_info_lod_01_addr) {
                    t.vinfo_lod01 = n;
                } else {
                    t.vinfo_lod0 = n;
                }

                t.vertex_info.insert(t.vertex_info.end(), vi.begin(), vi.end());
            } else if (vn == 3 && vl == 2) {
                auto bytes = [&] {
                    return code.data.sub(0, std::size_t{n} * 4, "V4_8 unpack").to_vector();
                };

                // An empty region shares its VU address with the next one (with
                // no LOD-01 extras, unk_indices_2_lod_01_addr equals
                // parent_indices_lod_0_addr in most tfrags), so a region only
                // matches when its count in the VU header is non-zero.
                if (!code.usn() && a == vu.strips_addr) {
                    m_strips = records<TfragStrip>(code.data, n, "strips");
                } else if (a == vu.indices_addr) {
                    m_indices = bytes();
                } else if (a == vu.parent_indices_lod_01_addr && vu.positions_lod_01_count != 0) {
                    t.parent_indices_lod01 = bytes();
                } else if (a == vu.unk_indices_2_lod_01_addr && vu.unk_06 != 0) {
                    t.unk_indices_2_lod01 = bytes();
                } else if (a == vu.parent_indices_lod_0_addr && vu.positions_lod_0_count != 0) {
                    t.parent_indices_lod0 = bytes();
                } else if (a == vu.unk_indices_2_lod_0_addr && vu.unk_0a != 0) {
                    t.unk_indices_2_lod0 = bytes();
                } else {
                    fail("tfrag: unexpected V4_8 unpack to VU address {:#x}", a);
                }
            } else {
                fail("tfrag: unexpected unpack format V{}_{}", vn + 1, 32 >> vl);
            }
        }
    }

    void take_lod(int l) {
        m_tfrag.lod[static_cast<std::size_t>(l)].strips = std::move(m_strips);
        m_tfrag.lod[static_cast<std::size_t>(l)].indices = std::move(m_indices);
        m_strips.clear();
        m_indices.clear();
    }

private:
    Tfrag m_tfrag;
    bool m_have_origin = false;
    std::vector<TfragStrip> m_strips;
    std::vector<u8> m_indices;
};

}  // namespace

TfragBlockHeader parse_tfrag_block_header(ByteView block) {
    return block.read<TfragBlockHeader>(0, "tfrag block header");
}

std::vector<Tfrag> parse_tfrags(ByteView block) {
    const TfragBlockHeader bh = parse_tfrag_block_header(block);

    if (bh.tfrag_count < 0 || bh.tfrag_count > 100'000 || bh.table_offset < 0x10) {
        fail(
            "tfrag block: implausible header ({} tfrags at {:#x})", bh.tfrag_count, bh.table_offset
        );
    }

    std::vector<Tfrag> out;
    out.reserve(static_cast<std::size_t>(bh.tfrag_count));

    for (std::size_t i = 0; i < static_cast<std::size_t>(bh.tfrag_count); ++i) {
        const auto h = block.read<TfragHeader>(
            static_cast<std::size_t>(bh.table_offset) + i * sizeof(TfragHeader), "tfrag header"
        );
        const s64 signed_base = s64{bh.table_offset} + h.data;

        if (signed_base < 0) {
            fail("tfrag {}: data offset before the block", i);
        }

        const auto base = static_cast<std::size_t>(signed_base);
        auto slice = [&](std::size_t from, std::size_t to) {
            if (to < from) {
                fail("tfrag {}: VIF list ends before it starts ({:#x}..{:#x})", i, from, to);
            }

            return block.sub(base + from, to - from, "tfrag VIF list");
        };
        const std::size_t lod0_start = h.shared_ofs + std::size_t{h.lod_1_size} * 0x10;

        // The common list carries the VU header: read it first so the
        // addresses are known, then walk the lists in transfer order.
        ListWalk first(h);
        first.run(slice(h.shared_ofs, h.lod_1_ofs));
        ListWalk walk(h, first.tfrag().vu);
        walk.run(slice(h.shared_ofs, h.lod_1_ofs));  // header, ad-gifs, common
        walk.run(slice(h.lod_2_ofs, h.shared_ofs));  // LOD-2 strips
        walk.take_lod(2);
        walk.run(slice(h.lod_1_ofs, h.lod_0_ofs));  // LOD-1 strips
        walk.take_lod(1);
        walk.run(slice(h.lod_0_ofs, lod0_start));  // LOD-01 parents, vertex infos, positions
        walk.run(slice(lod0_start, h.rgba_ofs));   // LOD-0 positions, strips, parents, infos
        walk.take_lod(0);
        const bool have_origin = walk.have_origin();
        Tfrag t = std::move(walk.tfrag());

        t.rgba = block.read_array<TfragRgba>(
            base + h.rgba_ofs, std::size_t{h.rgba_size} * 4, "tfrag rgba"
        );
        const auto origin = block.read<std::array<s32, 4>>(base + h.light_ofs, "tfrag origin");

        if (!have_origin) {
            t.origin = origin;
        }

        t.lights =
            block.read_array<TfragLight>(base + h.light_ofs + 0x10, h.vert_count, "tfrag lights");
        t.mspheres = block.read_array<std::array<f32, 4>>(
            base + h.msphere_ofs, h.msphere_count, "tfrag spheres"
        );
        t.cube = block.read<std::array<std::array<s16, 4>, 8>>(base + h.cube_ofs, "tfrag cube");
        out.push_back(std::move(t));
    }

    return out;
}

std::vector<TfragTriangle> tfrag_triangles(const Tfrag& t, int lod) {
    if (lod < 0 || lod > 2) {
        fail("tfrag LOD {} outside 0..2", lod);
    }

    const TfragLod& l = t.lod[static_cast<std::size_t>(lod)];
    std::vector<TfragTriangle> tris;
    std::size_t cursor = 0;
    u16 ad_gif = 0;

    for (std::size_t k = 0; k < l.strips.size(); ++k) {
        const TfragStrip& s = l.strips[k];
        int n = s.vertex_count_and_flag;

        if (k == 0 || n <= 0) {
            if (k != 0 && n == 0) {
                break;
            }

            if (k == 0 || s.end_of_packet_flag >= 0 || s.ad_gif_offset >= 0) {
                const int z = s.ad_gif_offset;

                if (z < 0 || z % 5 != 0 || static_cast<std::size_t>(z / 5) >= t.ad_gifs.size()) {
                    fail("tfrag strip {}: ad-gif offset {} outside the ad-gif array", k, z);
                }

                ad_gif = static_cast<u16>(z / 5);
            }

            n += 128;
        }

        const auto count = static_cast<std::size_t>(n);

        if (cursor + count > l.indices.size()) {
            fail("tfrag strip {} runs past the index array", k);
        }

        for (std::size_t i = 2; i < count; ++i) {
            const u16 a = l.indices[cursor + i - 2];
            const u16 b = l.indices[cursor + i - 1];
            const u16 c = l.indices[cursor + i];
            tris.push_back(
                (i & 1) != 0 ? TfragTriangle{b, a, c, ad_gif} : TfragTriangle{a, b, c, ad_gif}
            );
        }

        cursor += count;
    }

    return tris;
}

Mesh tfrag_mesh(const Tfrag& t, int lod) {
    Mesh mesh;
    const std::size_t n = std::min<std::size_t>(t.lod_vertex_info_count(lod), t.vertex_info.size());
    mesh.vertices.reserve(n);

    for (std::size_t v = 0; v < n; ++v) {
        MeshVertex vertex;
        vertex.position = t.world_position(v);
        vertex.uv = {t.vertex_info[v].s / 4096.0f, t.vertex_info[v].t / 4096.0f};
        const std::size_t p = t.position_index(v);

        if (p < t.rgba.size()) {
            const TfragRgba& c = t.rgba[p];
            vertex.rgba = {c.r, c.g, c.b, c.a};
        }

        if (p < t.lights.size()) {
            vertex.normal = spherical_normal(t.lights[p].azimuth, t.lights[p].elevation);
        }

        mesh.vertices.push_back(vertex);
    }

    for (const TfragTriangle& tri : tfrag_triangles(t, lod)) {
        if (tri.a >= n || tri.b >= n || tri.c >= n) {
            fail("tfrag triangle names a vertex info past its LOD");
        }

        mesh.triangles.push_back(
            {{tri.a, tri.b, tri.c}, static_cast<s32>(t.ad_gifs[tri.ad_gif].texture_index())}
        );
    }

    return mesh;
}

}  // namespace openrac::assets::rac1
