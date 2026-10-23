// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), the unit tests of
// crates/rc-formats/src/tfrag.rs and tfrag_light.rs: ISC License, Copyright (c) 2026
// ReRAC contributors.
//
// Terrain (tfrag): ad-gif fields and GS registers, strips and texture
// switches, texture spheres, level of detail; the PS2 float model and the
// lighting pass.

#include "assets/geometry/tfrag.h"

#include <cmath>
#include <numbers>

#include "assets/geometry/tfrag_lighting.h"
#include "common/ps2_float.h"
#include "tests/check.h"

using namespace openrac::assets;
using namespace openrac::assets::rac1;
namespace ps2 = openrac::ps2;

namespace {

u32 fb(f32 x) {
    return ps2::bits(x);
}

AdGif gif(s32 lo, s32 hi, u8 address) {
    AdGif g;
    g.data_lo = lo;
    g.data_hi = hi;
    g.address = address;
    return g;
}

TfragAdGifs adgifs(s32 tex, s32 k, s32 mmin, s32 wms, s32 wmt) {
    return {gif(tex, 0, 6), gif(k, mmin, 0x14), gif(wms, wmt, 8), gif(0, 0, 0x34), gif(0, 0, 0x36)};
}

void ad_gif_fields() {
    const TfragAdGifs a = adgifs(17, 0xff77, 4, 1, 0);
    CHECK(a.texture_index() == 17);
    CHECK(a.wrap_s() == GsWrap::Clamp && a.wrap_t() == GsWrap::Repeat);
    CHECK(a.mag_filter() == GsFilter::Linear && a.min_filter() == GsFilter::LinearMipmapNearest);
    CHECK(a.lod_k_raw() == -137 && a.lod_k() == -8.5625f);
    const TfragAdGifs b = adgifs(0, 0xffa7, 4, 0, 1);
    CHECK(b.wrap_s() == GsWrap::Repeat && b.wrap_t() == GsWrap::Clamp);
    CHECK(b.lod_k() == -5.5625f);
    CHECK(b.mip_levels(CoreTextureEntry{0, 32, 32, 3, 0x40, 0x20, -1}) == 3);
}

void gs_registers() {
    // 128 x 64, 4 levels, palette block 0x30, mips 2 and 3 at blocks 0x100 and
    // 0xf8, the level's GS base at block 0x1000.
    const CoreTextureEntry tex{0, 128, 64, 4, 0x30, 0x100, 0xf8};
    const TfragAdGifs a = adgifs(5, 0xff80, 4, 1, 1);
    const TfragGsRegisters r = a.gs_registers(tex, 0x10'0000);
    const u64 base = 0x1000;
    CHECK(
        r.tex0
        == (u64{2} << 14 | u64{0x13} << 20 | u64{7} << 26 | u64{6} << 30 | u64{1} << 34
            | (0x30 + base) << 37 | u64{4} << 61)
    );
    CHECK(r.tex1 == (u64{3} << 2 | u64{1} << 5 | u64{4} << 6 | u64{0xff80} << 32));
    CHECK(r.clamp == (u64{1} | u64{1} << 2 | u64{5} << 24));
    CHECK(
        r.miptbp1
        == (u64{1} << 14 | (0x100 + base) << 20 | u64{1} << 34 | (0xf8 + base) << 40 | u64{1} << 54)
    );
    CHECK(r.miptbp2 == 0);
    CHECK(((r.tex1 >> 32) & 0xfff) == 0xf80);
    // 32-pixel textures: TBW and TBW1 clamp to 1.
    CoreTextureEntry small = tex;
    small.width = 32;
    small.height = 32;
    small.levels = 3;
    const TfragGsRegisters s = a.gs_registers(small, 0);
    CHECK(((s.tex0 >> 14) & 0x3f) == 1);
    CHECK(((s.tex0 >> 26) & 0xf) == 5);
    CHECK(((s.miptbp1 >> 14) & 0x3f) == 1);
    CHECK(((s.tex1 >> 2) & 7) == 2);
}

TfragStrip strip(s8 x, s8 y, s8 z) {
    return {x, y, z, 0};
}

std::vector<u16> ad_gifs_of(const Tfrag& t) {
    std::vector<u16> out;
    for (const TfragTriangle& tri : tfrag_triangles(t, 0)) {
        out.push_back(tri.ad_gif);
    }
    return out;
}

template <typename F>
bool throws(F&& f) {
    try {
        f();
    } catch (const AssetError&) {
        return true;
    }
    return false;
}

void strips_switch_textures() {
    Tfrag t;
    t.ad_gifs.resize(3);
    // Load ad-gif 1 + 3 vertices; kick without a texture (z < 0) + 3; kick
    // with ad-gif 2 + 3; a plain run of 3; load ad-gif 0 + 3; end.
    t.lod[0].strips =
        {strip(-125, 0, 5),
         strip(-125, -128, -1),
         strip(-125, -128, 10),
         strip(3, 0, 0),
         strip(-125, 0, 0),
         strip(0, 0, 0)};
    for (u8 i = 0; i < 15; ++i) {
        t.lod[0].indices.push_back(i);
    }
    CHECK((ad_gifs_of(t) == std::vector<u16>{1, 1, 2, 2, 0}));
    // The first record always loads its ad-gif, whatever y says.
    t.lod[0].strips = {strip(-125, -128, 5), strip(0, 0, 0)};
    CHECK((ad_gifs_of(t) == std::vector<u16>{1}));
    // A load past the ad-gifs, or off a record boundary, is refused.
    t.lod[0].strips = {strip(-125, 0, 15), strip(0, 0, 0)};
    CHECK(throws([&] { tfrag_triangles(t, 0); }));
    t.lod[0].strips = {strip(-125, 0, 5), strip(-125, -128, 7), strip(0, 0, 0)};
    CHECK(throws([&] { tfrag_triangles(t, 0); }));
}

void texture_spheres() {
    Tfrag t;
    t.mspheres = {{1.0f, 2.0f, 3.0f, ps2::to_float(0x2aff'0c00)}};
    const auto s = t.texture_spheres();
    CHECK(s.size() == 1);
    CHECK((s[0] == TfragTextureSphere{{1, 2, 3}, 0x0c00, 0xff, 0x2a}));
}

void level_of_detail() {
    TfragBlockHeader bh;
    bh.table_offset = 0x10;
    bh.tfrag_count = 1;
    bh.lod_base = 12.0f;
    CHECK((bh.lod_distances() == std::array<f32, 3>{72, 48, 24}));
    const auto th = bh.lod_thresholds_raw();
    CHECK((th == std::array<s32, 3>{73728, 49152, 24576}));
    Tfrag t;
    t.header.bsphere = {0, 0, 0, 4096};
    auto mode = [&](f32 d) {
        return t.draw_mode(d * 1024.0f, th);
    };
    CHECK(mode(10) == TfragDrawMode::Lod0);
    CHECK(mode(22) == TfragDrawMode::Lod0Morph);
    CHECK(mode(40) == TfragDrawMode::Lod0Morph);
    CHECK(mode(46) == TfragDrawMode::Lod0Collapse);
    CHECK(mode(60) == TfragDrawMode::Lod1Morph);
    CHECK(mode(70) == TfragDrawMode::Lod1Collapse);
    CHECK(mode(80) == TfragDrawMode::Lod2);
    CHECK(static_cast<u8>(TfragDrawMode::Lod1Collapse) == 8);
    CHECK(tfrag_draw_lod(TfragDrawMode::Lod0Collapse) == 0);
    t.header.base_only = 1;
    CHECK(t.draw_mode(10240.0f, th) == TfragDrawMode::Lod2);
    const auto d = bh.lod_distances();
    CHECK(tfrag_morph_weight(TfragMorphTier::Lod01, 48, d) == 0.0f);
    CHECK(tfrag_morph_weight(TfragMorphTier::Lod01, 60, d) == 0.5f);
    CHECK(tfrag_morph_weight(TfragMorphTier::Lod01, 90, d) == 1.0f);
    CHECK(tfrag_morph_weight(TfragMorphTier::Lod0, 30, d) == 0.25f);
    CHECK(tfrag_collapse_distance(TfragMorphTier::Lod0, d) == 48.0f);
}

void lod_links() {
    // Positions 0..3 common, 3..5 LOD-01, 5 LOD-0. Vertex infos 0..3 common,
    // 3..5 LOD-01 morphing + 5 extra, 6 LOD-0 morphing + 7 extra.
    auto vi = [](s16 p2, s16 own) {
        return TfragVertexInfo{0, 0, static_cast<s16>(p2 * 2), static_cast<s16>(own * 2)};
    };
    Tfrag t;
    t.positions.resize(6);
    t.vertex_info =
        {vi(0, 0), vi(0, 1), vi(0, 2), vi(1, 3), vi(2, 4), vi(0, 3), vi(4, 5), vi(3, 5)};
    t.vinfo_common = 3;
    t.vinfo_lod01 = 3;
    t.vinfo_lod0 = 2;
    t.parent_indices_lod01 = {0, 1, 0, 0};
    t.unk_indices_2_lod01 = {2, 0, 0, 0};
    t.parent_indices_lod0 = {3, 0, 0, 0};
    t.unk_indices_2_lod0 = {4, 0, 0, 0};
    t.vu.positions_lod_01_count = 2;
    t.vu.unk_06 = 1;
    t.vu.positions_lod_0_count = 1;
    t.vu.unk_0a = 1;
    using Pair = std::pair<std::size_t, std::size_t>;
    CHECK(!t.morph_parents(2));
    CHECK(t.morph_parents(3) == Pair(0, 1));
    CHECK(t.morph_parents(4) == Pair(1, 2));
    CHECK(!t.morph_parents(5));  // extra: collapses only
    CHECK((t.lod_link(5) == TfragLodLink{TfragMorphTier::Lod01, false, 3, 2, 2, 0}));
    CHECK((t.lod_link(6) == TfragLodLink{TfragMorphTier::Lod0, true, 5, 3, 3, 4}));
    const auto l7 = t.lod_link(7);
    CHECK(l7 && l7->parent1_vinfo == 4 && l7->parent1_position == 4 && l7->parent2_position == 3);
    CHECK(!t.lod_link(8));
}

void ps2_arithmetic() {
    CHECK(
        ps2::add(fb(65536.0f), fb(0.5f / 128.0f * 1.999f)) == fb(65536.0f)
    );  // below the 1/128 grid
    CHECK(ps2::add(fb(65536.0f), fb(1.0f)) == fb(65537.0f));
    CHECK(ps2::mul(fb(1.0f), fb(0.3f)) == fb(0.3f));
    CHECK(ps2::mul(0x3f80'0003, 0x3fc0'0001) == 0x3fc0'0005);  // truncated; IEEE nearest gives ..06
    CHECK(ps2::add(fb(1.0f), fb(-1.0f)) == 0);
    CHECK(ps2::add(fb(1.0f), fb(1e-10f)) == fb(1.0f));
    // The adder drops the smaller operand's bits below half an ULP first.
    CHECK(ps2::add(fb(1.0f), fb(-1.5f * std::ldexp(1.0f, -24))) == 0x3f7f'ffff);
    CHECK(ps2::sqrt(fb(4.0f)) == fb(2.0f));
    CHECK(ps2::sqrt(fb(2.0f)) == 0x3fb5'04f3);
    CHECK(ps2::div(fb(1.0f), fb(4.0f)) == fb(0.25f));
    CHECK(ps2::div(fb(1.0f), fb(3.0f)) == 0x3eaa'aaaa);
    CHECK(ps2::itof12(0x800) == fb(0.5f));
    CHECK(ps2::itof12(-4096 * 3) == fb(-3.0f));
    CHECK(ps2::mul(fb(1e30f), fb(1e30f)) == ps2::kMax);
}

NormalTable test_table() {
    NormalTable t;
    for (std::size_t i = 0; i < 256; ++i) {
        const double a = static_cast<double>(i) * 2 * std::numbers::pi / 256.0;
        t.entries[i] = {fb(static_cast<f32>(std::cos(a))), fb(static_cast<f32>(std::sin(a)))};
    }
    t.entries[0] = {fb(1.0f), 0};
    t.entries[64] = {0, fb(1.0f)};
    t.entries[192] = {0, fb(-1.0f)};
    return t;
}

void normals() {
    const NormalTable t = test_table();
    // Elevation 64 = straight up: the stored normal is its negation.
    const ps2::V4 up = tfrag_stored_normal(t, 0, 64);
    CHECK(up[0] == 0 && up[1] == 0 && up[2] == fb(-1.0f));
    const ps2::V4 x = tfrag_stored_normal(t, 0, 0);
    CHECK(x[0] == fb(-1.0f) && x[1] == 0 && x[2] == 0);
    const ps2::V4 down = tfrag_stored_normal(t, 64, 192);
    CHECK(down[0] == 0 && down[1] == 0 && down[2] == fb(1.0f));
    CHECK((pext5(0x8000 | 31 | 16 << 5 | 1 << 10) == std::array<u8, 4>{248, 128, 8, 128}));
}

void one_light() {
    const NormalTable t = test_table();
    LightBank bank;
    // A sun straight down, colour (1, 0.5, 0.25); a back light along +X, w = -0.5.
    bank.sets[2] =
        {{1.0f, 0.5f, 0.25f, 0}, {0, 0, -1, 0}, {0.25f, 0.25f, 0.25f, -0.5f}, {1, 0, 0, 0}};
    // Up-facing, base colour (40, 48, 56), the alpha bit set.
    TfragLight l{0, 0, 64, static_cast<u16>(0x8000 | 5 | 6 << 5 | 7 << 10), 2};
    CHECK((light_tfrag_vertex(bank, t, l) == std::array<u8, 4>{168, 112, 88, 128}));
    TfragLight facing = l;
    facing.elevation = 0;
    CHECK((light_tfrag_vertex(bank, t, facing) == std::array<u8, 4>{56, 64, 72, 128}));
    TfragLight hot = l;
    hot.color = 0x8000 | 31;
    CHECK(light_tfrag_vertex(bank, t, hot)[0] == 255);
    // Half way between set 2 and an empty set 3.
    TfragLight blend = l;
    blend.light_select = 0x80 << 8 | 3 << 4 | 2;
    CHECK((light_tfrag_vertex(bank, t, blend) == std::array<u8, 4>{104, 80, 72, 128}));
}

void point_light() {
    const PointLight pl{{1, 1, 1, 0}, {0, 0, 8, 16}};
    const ps2::V4 up = {0, 0, fb(-1.0f), 0};
    // 8 below the light, radius 16: d = 0.5, +64.
    CHECK(
        (light_tfrag_point(pl, {0, 0, 0}, {0, 0, 0}, up, {10, 20, 30, 128})
         == std::array<u8, 4>{74, 84, 94, 128})
    );
    const PointLight far{{1, 1, 1, 0}, {0, 0, 30, 16}};
    CHECK(!light_tfrag_point(far, {0, 0, 0}, {0, 0, 0}, up, {10, 20, 30, 128}));
}

}  // namespace

int main() {
    ad_gif_fields();
    gs_registers();
    strips_switch_textures();
    texture_spheres();
    level_of_detail();
    lod_links();
    ps2_arithmetic();
    normals();
    one_light();
    point_light();
    return openrac::test::result();
}
