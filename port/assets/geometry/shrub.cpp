// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/shrub.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The RAC1 shrub reader: packets replayed through VU1 program 56467's input
// and output buffers, billboards, fades, wind sway and instances.

#include "assets/geometry/shrub.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <numbers>

#include "assets/geometry/texture.h"
#include "assets/geometry/vif.h"

namespace openrac::assets::rac1 {

namespace {

template <typename... Args>
[[noreturn]] void packet_fail(std::format_string<Args...> fmt, Args&&... args) {
    fail("shrub packet: {}", std::format(fmt, std::forward<Args>(args)...));
}

// f32 to s32 as Rust's `as` does it (and the console's cvt saturates too):
// truncation, saturating, NaN to 0.
s32 trunc_s32(f32 x) {
    if (std::isnan(x)) {
        return 0;
    }

    if (x >= 2147483648.0f) {
        return std::numeric_limits<s32>::max();
    }

    if (x <= -2147483648.0f) {
        return std::numeric_limits<s32>::min();
    }

    return static_cast<s32>(x);
}

// The VU1 input buffer after the packet's unpacks, as 32-bit lanes.
class InputBuffer {
public:
    std::array<u32, 4> at(std::size_t a, std::string_view what) const {
        if (a >= kShrubInputQwc) {
            packet_fail("{} outside the input buffer", what);
        }

        if (!m_written[a]) {
            packet_fail("{} reads an input quadword no unpack wrote", what);
        }

        return m_qw[a];
    }

    template <typename T>
    T record(std::size_t a, std::string_view what) const {
        static_assert(sizeof(T) == 16);
        const auto q = at(a, what);
        T value;
        std::memcpy(&value, q.data(), sizeof(T));
        return value;
    }

    // The four lanes truncated to 16 bits: the V4_16 view of a vertex.
    std::array<s16, 4> halves(std::size_t a, std::string_view what) const {
        const auto q = at(a, what);
        return {
            static_cast<s16>(q[0]),
            static_cast<s16>(q[1]),
            static_cast<s16>(q[2]),
            static_cast<s16>(q[3])
        };
    }

    void write(std::size_t a, const std::array<u32, 4>& q) {
        m_qw[a] = q;
        m_written[a] = true;
    }

private:
    std::array<std::array<u32, 4>, kShrubInputQwc> m_qw{};
    std::array<bool, kShrubInputQwc> m_written{};
};

InputBuffer run_vif(ByteView list) {
    InputBuffer buffer;
    int unpacks = 0;

    for (const vif::Code& c : vif::parse(list)) {
        if (c.is_unpack()) {
            ++unpacks;
            const bool v4_32 = c.vn() == 3 && c.vl() == 0;
            const bool v4_16 = c.vn() == 3 && c.vl() == 1;

            if (!v4_32 && !v4_16) {
                packet_fail("unpack is neither V4_32 nor V4_16");
            }

            if (!c.flg()) {
                packet_fail("unpack without FLG (not TOPS-relative)");
            }

            if (std::size_t{c.addr()} + c.count() > kShrubInputQwc) {
                packet_fail("unpack runs past the 0x76-qw input buffer");
            }

            for (std::size_t i = 0; i < c.count(); ++i) {
                std::array<u32, 4> q{};

                for (std::size_t k = 0; k < 4; ++k) {
                    if (v4_32) {
                        q[k] = c.data.u32_at(i * 16 + k * 4);
                    } else {
                        const u16 h = c.data.u16_at(i * 8 + k * 2);
                        q[k] = c.usn() ? u32{h} : static_cast<u32>(s32{static_cast<s16>(h)});
                    }
                }

                buffer.write(c.addr() + i, q);
            }
        } else if (c.cmd == vif::kStcycl) {
            if ((c.imm & 0xff) != (c.imm >> 8)) {
                packet_fail("STCYCL with CL != WL");
            }
        } else if (c.cmd == vif::kStmod) {
            if ((c.imm & 3) != 0) {
                packet_fail("STMOD mode != 0");
            }
        } else if (c.cmd != vif::kNop) {
            packet_fail("unexpected VIF code {:#x}", c.cmd);
        }
    }

    if (unpacks != 3) {
        packet_fail("expected 3 unpacks, got {}", unpacks);
    }

    return buffer;
}

// What a GS-packet quadword holds after VU1's writes.
struct Slot {
    enum class Kind : u8 {
        Unwritten,
        Tag,
        AdGif,
        Vertex,
    };

    Kind kind = Kind::Unwritten;
    u16 index = 0;
    u8 part = 0;

    bool operator==(const Slot&) const = default;
};

}  // namespace

std::array<f32, 3> ShrubVertex::class_position(f32 class_scale) const {
    const f32 k = class_scale / 1024.0f;
    return {position[0] * k, position[1] * k, position[2] * k};
}

ShrubGsRegisters ShrubAdGifs::gs_registers(
    u8 texture_index, const CoreTextureEntry& texture, u32 gs_base
) const {
    return {
        loaded_tex1(texture.levels, tex1.data_lo, tex1.data_hi),
        loaded_clamp(clamp.data_lo, clamp.data_hi, texture_index),
        loaded_miptbp1(texture, gs_base),
        loaded_tex0(texture, gs_base),
    };
}

f32 ShrubBillboardRegisters::lod_k() const {
    return static_cast<f32>(assets::lod_k_raw(static_cast<s32>(tex1 >> 32))) / 16.0f;
}

ShrubBillboardRegisters ShrubBillboard::gs_registers(
    const CoreBillboardInfo& info, u32 gs_base
) const {
    const s64 base = static_cast<s32>(gs_base) >> 8;
    auto tbw = [&](int k) {
        return s64{std::max(s32{info.width} >> (k + 6), 1)};
    };
    auto tbp = [&](s16 v) {
        return s64{v} + base;
    };
    ShrubBillboardRegisters r;
    r.tex1 = loaded_tex1(info.levels, tex1.data_lo, tex1.data_hi);
    const s64 t0 = tbp(info.texture_block) | tbw(0) << 14 | ee_log2(info.width) << 26 | 0x0130'0000
                   | ee_log2(info.height) << 30 | tbp(info.palette_block) << 37 | s64{1} << 34;
    r.tex0 = static_cast<u64>(t0) | u64{1} << 63;
    const s64 m1 = tbp(info.mip1_block) | tbw(1) << 14 | tbp(info.mip2_block) << 20 | tbw(2) << 34
                   | tbp(info.mip3_block) << 40 | tbw(3) << 54;
    r.miptbp1 = static_cast<u64>(m1);
    return r;
}

std::optional<std::array<f32, 3>> ShrubClass::normal(u8 index) const {
    if (index >= normals.size()) {
        return std::nullopt;
    }

    const auto& n = normals[index];
    return std::array<f32, 3>{n[0] / 32767.0f, n[1] / 32767.0f, n[2] / 32767.0f};
}

ShrubPacket read_shrub_packet(ByteView list, const ShrubPacketEntry& entry, s32& texture) {
    const InputBuffer in = run_vif(list);
    const auto h = in.record<ShrubPacketHeader>(0, "packet header");

    if (h.texture_count < 1 || h.gif_tag_count < 1) {
        packet_fail("texture_count and gif_tag_count must be >= 1");
    }

    if (h.vertex_count < 0 || h.vertex_offset < 0
        || static_cast<std::size_t>(h.vertex_offset) + 2 * static_cast<std::size_t>(h.vertex_count)
               > kShrubInputQwc) {
        packet_fail("vertex tables outside the input buffer");
    }

    ShrubPacket pk;
    pk.entry = entry;
    pk.header = h;
    const auto tags = static_cast<std::size_t>(h.gif_tag_count);

    for (std::size_t k = 0; k < tags; ++k) {
        pk.gif_tags.push_back(in.record<ShrubGifTag>(1 + k, "GIF tag"));
    }

    for (std::size_t k = 0; k < static_cast<std::size_t>(h.texture_count); ++k) {
        const std::size_t base = 1 + tags + 4 * k;
        ShrubAdGifs a;
        a.tex1 = in.record<AdGif>(base, "ad-gif");
        a.clamp = in.record<AdGif>(base + 1, "ad-gif");
        a.miptbp1 = in.record<AdGif>(base + 2, "ad-gif");
        a.tex0 = in.record<AdGif>(base + 3, "ad-gif");
        pk.ad_gifs.push_back(a);
    }

    const auto p1 = static_cast<std::size_t>(h.vertex_offset);
    const auto p2 = static_cast<std::size_t>(h.vertex_offset + h.vertex_count);

    for (std::size_t i = 0; i < static_cast<std::size_t>(h.vertex_count); ++i) {
        const auto a = in.halves(p1 + i, "vertex part 1");
        const auto b = in.halves(p2 + i, "vertex part 2");
        pk.part1.push_back({a[0], a[1], a[2], a[3]});
        pk.part2.push_back({b[0], b[1], b[2], static_cast<u16>(b[3])});
    }

    // The vertex loop tests the stop bit from vertex 2 on and drains three
    // more vertices after it.
    std::size_t stop = 2;

    while ((in.at(p2 + stop, "vertex stop scan")[3] & 0x8000) == 0) {
        ++stop;
    }

    for (std::size_t i = 0; i < stop + 4; ++i) {
        const auto a = in.halves(p1 + i, "vertex part 1");
        const auto b = in.halves(p2 + i, "vertex part 2");
        const auto n = static_cast<u16>(b[3]);

        if ((n & 0x7fff) >= kShrubNormals) {
            packet_fail("normal index {} > 23", n & 0x7fff);
        }

        pk.vertices.push_back(
            {{a[0], a[1], a[2]},
             a[3],
             {b[0], b[1]},
             b[2],
             static_cast<u8>(n & 0x7fff),
             static_cast<u8>(n >> 15)}
        );
    }

    // GS-packet slots in VU1's write order: tags, ad-gif blocks, vertices;
    // later writes win.
    std::array<Slot, kShrubOutputQwc> slot{};
    auto put = [&](s32 at, Slot s) {
        if (at < 0 || static_cast<std::size_t>(at) >= kShrubOutputQwc) {
            packet_fail("GS slot {} outside the 0xa8-qw output buffer", at);
        }

        slot[static_cast<std::size_t>(at)] = s;
    };

    for (std::size_t k = 0; k < pk.gif_tags.size(); ++k) {
        put(pk.gif_tags[k].gs_packet_offset, {Slot::Kind::Tag, static_cast<u16>(k), 0});
    }

    for (std::size_t k = 0; k < pk.ad_gifs.size(); ++k) {
        for (u8 j = 0; j < 5; ++j) {
            put(pk.ad_gifs[k].gs_packet_offset() + j, {Slot::Kind::AdGif, static_cast<u16>(k), j});
        }
    }

    for (std::size_t i = 0; i < pk.vertices.size(); ++i) {
        for (u8 j = 0; j < 3; ++j) {
            put(pk.vertices[i].gs_packet_offset + j, {Slot::Kind::Vertex, static_cast<u16>(i), j});
        }
    }

    // The GIF from slot 0 to the EOP tag.
    std::size_t cursor = 0;

    for (;;) {
        if (cursor >= kShrubOutputQwc) {
            packet_fail("GIF runs past the output buffer");
        }

        const Slot s = slot[cursor];

        if (s.kind == Slot::Kind::AdGif && s.part == 0) {
            for (u8 j = 1; j < 5; ++j) {
                if (cursor + j >= kShrubOutputQwc
                    || slot[cursor + j] != Slot{Slot::Kind::AdGif, s.index, j}) {
                    packet_fail("ad-gif block partly overwritten");
                }
            }

            const s32 tex = pk.ad_gifs[s.index].tex0.data_lo;

            if (tex < 0 || tex > 15) {
                packet_fail("ad-gif texture slot {} out of range", tex);
            }

            texture = tex;
            cursor += 5;
        } else if (s.kind == Slot::Kind::Tag) {
            const ShrubGifTag& g = pk.gif_tags[s.index];

            if (g.flg() != 0 || g.nreg() != 3 || (g.tag_hi & 0xfff) != 0x412 || !g.pre()) {
                packet_fail("vertex GIF tag is not PACKED ST/RGBAQ/XYZF2 with PRIM");
            }

            if (g.prim() != 3 && g.prim() != 4) {
                packet_fail("unexpected GS primitive type {}", g.prim());
            }

            if (texture < 0) {
                packet_fail("vertices drawn before any ad-gif");
            }

            ShrubDraw draw{static_cast<u8>(texture), g.prim(), {}};

            for (std::size_t m = 0; m < g.nloop(); ++m) {
                const std::size_t q = cursor + 1 + 3 * m;

                if (q + 2 >= kShrubOutputQwc) {
                    packet_fail("GIF runs past the output buffer");
                }

                const Slot& a = slot[q];
                const Slot& b = slot[q + 1];
                const Slot& c = slot[q + 2];

                if (a.kind != Slot::Kind::Vertex || b != Slot{Slot::Kind::Vertex, a.index, 1}
                    || c != Slot{Slot::Kind::Vertex, a.index, 2} || a.part != 0) {
                    packet_fail("GIF reads a vertex slot not written by one vertex at {}", q);
                }

                draw.vertices.push_back(a.index);
            }

            pk.draws.push_back(std::move(draw));
            cursor += 1 + 3 * std::size_t{g.nloop()};

            if (g.eop()) {
                break;
            }
        } else {
            packet_fail("no GIF tag at GS slot {}", cursor);
        }
    }

    return pk;
}

ShrubClass parse_shrub_class(ByteView blob) {
    ShrubClass sc;
    sc.header = blob.read<ShrubClassHeader>(0, "shrub class header");
    const ShrubClassHeader& h = sc.header;

    if (h.packet_count < 0 || h.packet_count > 1000) {
        fail("shrub class: implausible packet count {}", h.packet_count);
    }

    const auto entries = blob.read_array<ShrubPacketEntry>(
        0x40, static_cast<std::size_t>(h.packet_count), "shrub packet table"
    );
    s32 texture = -1;  // GS texture state carries from packet to packet

    for (const ShrubPacketEntry& e : entries) {
        if (e.offset < 0 || e.size < 0) {
            fail("shrub class: negative packet offset or size");
        }

        const ByteView list = blob.sub(
            static_cast<std::size_t>(e.offset), static_cast<std::size_t>(e.size), "shrub packet"
        );
        sc.packets.push_back(read_shrub_packet(list, e, texture));
    }

    if (h.normals_offset <= 0) {
        fail("shrub class without normals");
    }

    sc.normals = blob.read_array<std::array<s16, 4>>(
        static_cast<std::size_t>(h.normals_offset), kShrubNormals, "shrub normals"
    );

    if (h.billboard_offset > 0) {
        sc.billboard = blob.read<ShrubBillboard>(
            static_cast<std::size_t>(h.billboard_offset), "shrub billboard"
        );
    }

    return sc;
}

std::vector<ShrubTriangle> shrub_triangles(const ShrubPacket& p) {
    std::vector<ShrubTriangle> out;

    for (const ShrubDraw& d : p.draws) {
        const std::vector<u16>& v = d.vertices;
        const u16 texture = d.texture;

        if (d.prim == 4) {
            for (std::size_t i = 2; i < v.size(); ++i) {
                out.push_back(
                    i % 2 == 0 ? ShrubTriangle{v[i - 2], v[i - 1], v[i], texture}
                               : ShrubTriangle{v[i], v[i - 1], v[i - 2], texture}
                );
            }
        } else {
            for (std::size_t i = 0; i + 3 <= v.size(); i += 3) {
                out.push_back({v[i], v[i + 1], v[i + 2], texture});
            }
        }
    }

    return out;
}

Mesh shrub_mesh(const ShrubClass& shrub) {
    Mesh mesh;

    for (const ShrubPacket& p : shrub.packets) {
        const auto base = static_cast<u32>(mesh.vertices.size());

        for (const ShrubVertex& v : p.vertices) {
            MeshVertex out;
            out.position = v.class_position(shrub.header.scale);
            out.uv = v.uv();
            out.normal = shrub.normal(v.normal).value_or(std::array<f32, 3>{});
            mesh.vertices.push_back(out);
        }

        for (const ShrubTriangle& t : shrub_triangles(p)) {
            mesh.triangles.push_back({{base + t.a, base + t.b, base + t.c}, t.texture});
        }
    }

    return mesh;
}

std::vector<LevelShrubClass> parse_level_shrub_classes(
    std::span<const CoreShrubClassEntry> table, ByteView core_data
) {
    std::vector<LevelShrubClass> out;

    for (const CoreShrubClassEntry& e : table) {
        if (e.base.offset <= 0) {
            continue;
        }

        try {
            out.push_back(
                {e,
                 parse_shrub_class(
                     core_data.tail(static_cast<std::size_t>(e.base.offset), "shrub class")
                 )}
            );
        } catch (const AssetError& error) {
            fail("shrub class {}: {}", e.base.o_class, error.what());
        }
    }

    return out;
}

u32 packed_column_lengths(const std::array<std::array<f32, 4>, 4>& m) {
    auto length = [](const std::array<f32, 4>& c) {
        return std::sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
    };
    const s32 lo = std::min(trunc_s32((length(m[0]) + length(m[1])) * 0.5f * 4096.0f), 0x10000);
    s32 hi = trunc_s32(length(m[2]) * 4096.0f);

    if (hi >= 0x10001) {
        hi = 0;
    }

    return static_cast<u32>(lo) | static_cast<u32>(hi) << 16;
}

std::array<f32, 3> billboard_extent(const ShrubBillboard& b, f32 class_scale, u32 packed) {
    // ShrubProc: itof12 of (lo, hi), width * lo, (height, z_ofs) * hi, all
    // times the scale; its projection takes 1024-scaled coordinates.
    const f32 lo = static_cast<f32>(packed & 0xffff) / 4096.0f;
    const f32 hi = static_cast<f32>(packed >> 16) / 4096.0f;
    return {
        b.width * lo * class_scale / 1024.0f,
        b.height * hi * class_scale / 1024.0f,
        b.z_ofs * hi * class_scale / 1024.0f,
    };
}

std::array<std::array<f32, 3>, 4> billboard_corners(
    const std::array<f32, 3>& t, const std::array<f32, 3>& eye, const std::array<f32, 3>& extent
) {
    const std::array<f32, 3> d = {t[0] - eye[0], t[1] - eye[1], t[2] - eye[2]};
    const f32 q = 1.0f / std::sqrt((d[0] * d[0] + d[1] * d[1]) + d[2] * d[2]);
    const f32 dx = d[0] * q;
    const f32 dy = d[1] * q;
    std::array<std::array<f32, 3>, 4> out{};

    for (std::size_t i = 0; i < 4; ++i) {
        const f32 h = kBillboardCorners[i][0] * extent[0];
        const f32 v = kBillboardCorners[i][1] * extent[1] + extent[2];
        out[i] = {t[0] + h * dy, t[1] - h * dx, t[2] + v};
    }

    return out;
}

ShrubFade shrub_fade(f32 z, f32 d, std::optional<u8> f) {
    const s32 dz = trunc_s32((d - z) * 4096.0f);  // vftoi12 (D - z)
    auto far = [&] {
        return static_cast<u8>(std::min(dz >> 1, 0x8000) >> 8);
    };

    if (!f) {
        return {static_cast<u8>(std::min(dz, 0x8000) >> 8), std::nullopt};
    }

    if (*f == 0) {
        return {std::nullopt, far()};
    }

    const s32 iz = trunc_s32(std::max(z, 0.0f) * 4096.0f) - (s32{*f} << 12);

    if (iz < 0) {
        return {u8{0x80}, std::nullopt};
    }

    const auto a = static_cast<u8>(std::min(iz, 0x8000) >> 8);

    if (a == 0x80) {
        return {std::nullopt, far()};
    }

    return {static_cast<u8>((0x8000 - iz) >> 8), a};
}

s8 sway_table(u32 i) {
    const double angle = 2.0 * std::numbers::pi * static_cast<double>(i & 0xff) / 256.0;
    return static_cast<s8>(std::trunc(-127.0 * std::sin(angle)));
}

std::optional<std::array<f32, 2>> wind_sway(
    u16 mode, u32 block, u32 tick, const std::array<f32, 3>& rel
) {
    if (mode == 0) {
        return std::nullopt;
    }

    const f32 d2 = (rel[0] * rel[0] + rel[1] * rel[1]) + 1.0f * (rel[2] * rel[2]);

    if (kSwayVf9[1] - d2 < 0.0f) {
        return std::nullopt;
    }

    // lb + sll 5 + vitof12: a table entry / 128.
    auto s = [](u32 i) {
        return static_cast<f32>(sway_table(i)) / 128.0f;
    };
    const f32 k = 1.0f - d2 * kSwayVf9[2];
    const bool doubled = (mode >> 1) != 0;
    const u32 u = block * 67u + tick;
    const u32 w = block * 123u + tick + (doubled ? tick : 0u);
    const f32 g = ((s(u) * s(u >> 1)) * kSwayVf9[0] + (1.0f - kSwayVf9[0])) * kSwayVf8[0];
    f32 sx = (s(w + 0x40) * kSwayVf8[1] + g * kSwayVf8[2]) * k;
    f32 sy = (s(w >> 1) * kSwayVf8[1] + g * kSwayVf8[3]) * k;

    if (!doubled) {
        sx *= kSwayVf9[3];
        sy *= kSwayVf9[3];
    }

    return std::array<f32, 2>{sx, sy};
}

std::array<std::array<f32, 4>, 4> ShrubInstance::world_matrix() const {
    auto m = matrix;
    m[3][3] = 1.0f;
    return m;
}

std::array<f32, 3> ShrubInstance::transform_point(const std::array<f32, 3>& p) const {
    std::array<f32, 3> out{};

    for (std::size_t r = 0; r < 3; ++r) {
        out[r] = matrix[0][r] * p[0] + matrix[1][r] * p[1] + matrix[2][r] * p[2] + matrix[3][r];
    }

    return out;
}

std::array<u8, 4> ShrubInstance::ambient_rgba() const {
    return {
        static_cast<u8>(colour[0]), static_cast<u8>(colour[1]), static_cast<u8>(colour[2]), 0x80
    };
}

ShrubInstance::LightSets ShrubInstance::light_sets() const {
    const auto s = static_cast<u16>(dir_lights);
    return {static_cast<u8>(s & 0xf), static_cast<u8>((s >> 4) & 0xf), static_cast<u8>(s >> 8)};
}

std::vector<ShrubInstance> parse_shrub_instance_section(ByteView section) {
    const s32 count = section.s32_at(0);

    if (count < 0 || count > 100'000) {
        fail("shrub instances: implausible count {}", count);
    }

    return section
        .read_array<ShrubInstance>(0x10, static_cast<std::size_t>(count), "shrub instances");
}

std::vector<ShrubInstance> parse_shrub_instances(ByteView gameplay) {
    const std::size_t offset = gameplay.u32_at(kGameplayShrubInstances);

    if (offset == 0) {
        return {};
    }

    return parse_shrub_instance_section(gameplay.tail(offset, "shrub instance section"));
}

std::vector<s32> parse_shrub_class_list(ByteView gameplay) {
    const std::size_t offset = gameplay.u32_at(kGameplayShrubClasses);

    if (offset == 0) {
        return {};
    }

    const s32 count = gameplay.s32_at(offset);

    if (count < 0 || count > 10'000) {
        fail("shrub class list: implausible count {}", count);
    }

    return gameplay
        .read_array<s32>(offset + 4, static_cast<std::size_t>(count), "shrub class list");
}

}  // namespace openrac::assets::rac1
