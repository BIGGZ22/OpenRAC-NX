// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-game/src/collision_query.rs
// and crates/rc-game/src/ps2v.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The collision kernels, written once over an arithmetic: ConsoleMath (the
// console's rounding on float bit patterns) or NativeMath (IEEE floats). Each
// vector helper names the VU0 instruction sequence it stands for, so a kernel
// follows the game's operation order step for step. Addresses are NTSC-U
// level01.

#include "assets/world/collision_query.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>

#include "common/ps2_float.h"

namespace openrac::assets {

namespace {

namespace cf = ps2;

// Constants, by their bits.
constexpr u32 kOneBits = 0x3f80'0000;
constexpr u32 k1024Bits = 0x4480'0000;     // world -> x1024
constexpr u32 kInv1024Bits = 0x3a80'0000;  // x1024 -> world
constexpr u32 k2048Bits = 0x4500'0000;     // half a cell, x1024
constexpr u32 k4096Bits = 0x4580'0000;     // a cell, x1024
constexpr u32 k64Bits = 0x4280'0000;       // the hero groups' x64
constexpr u32 kInv64Bits = 0x3c80'0000;
// The end marker of the cell walk's per-axis lists (a float > 1): never
// selected by the merge, which runs for the counted crossings only.
constexpr u32 kWalkEndBits = 0x5000'0000;

struct ConsoleMath {
    using F = u32;

    static F lit(u32 bits) { return bits; }

    static F in(float x) { return std::bit_cast<u32>(x); }

    static float out(F x) { return cf::to_host(x); }

    static F add(F a, F b) { return cf::add(a, b); }

    static F sub(F a, F b) { return cf::sub(a, b); }

    static F mul(F a, F b) { return cf::mul(a, b); }

    static F div(F a, F b) { return cf::div(a, b); }

    static F sqrt(F x) { return cf::sqrt(x); }

    static F min(F a, F b) { return cf::min(a, b); }

    static F max(F a, F b) { return cf::max(a, b); }

    static F itof(s32 i) { return cf::itof0(i); }

    static s32 ftoi(F x) { return cf::ftoi(x, 0); }

    // The sign bit (-0 counts), what bltz tests.
    static bool neg(F x) { return (x & cf::kSign) != 0; }

    // Above +0, what bgtz on the raw word tests.
    static bool pos(F x) { return static_cast<s32>(x) > 0; }

    static bool pos_zero(F x) { return x == 0; }

    // Comparisons of non-negative floats done on their bits as integers.
    static bool less(F a, F b) { return static_cast<s32>(a) - static_cast<s32>(b) < 0; }

    static bool less_equal(F a, F b) { return static_cast<s32>(a) - static_cast<s32>(b) <= 0; }

    // pminw / pmaxw of the bits as integers: negatives become +0.
    static F clamp01(F t) {
        return static_cast<u32>(std::max(std::min(static_cast<s32>(t), s32{kOneBits}), 0));
    }

    static bool equal(F a, F b) { return a == b; }

    // |x| < 1 on the bits.
    static bool magnitude_below_one(F x) {
        return static_cast<s32>(x & ~cf::kSign) - static_cast<s32>(kOneBits) < 0;
    }
};

struct NativeMath {
    using F = float;

    static F lit(u32 bits) { return std::bit_cast<float>(bits); }

    static F in(float x) { return x; }

    static float out(F x) { return x; }

    static F add(F a, F b) { return a + b; }

    static F sub(F a, F b) { return a - b; }

    static F mul(F a, F b) { return a * b; }

    // As the console, a zero divisor gives the largest magnitude, which keeps
    // every later step finite.
    static F div(F a, F b) {
        if (b == 0.0f) {
            return std::signbit(a) != std::signbit(b) ? -std::numeric_limits<float>::max()
                                                      : std::numeric_limits<float>::max();
        }

        return a / b;
    }

    static F sqrt(F x) { return std::sqrt(std::fabs(x)); }

    static F min(F a, F b) { return b < a ? b : a; }

    static F max(F a, F b) { return b > a ? b : a; }

    static F itof(s32 i) { return static_cast<float>(i); }

    static s32 ftoi(F x) {
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

    static bool neg(F x) { return std::signbit(x); }

    static bool pos(F x) { return x > 0.0f; }

    static bool pos_zero(F x) { return x == 0.0f && !std::signbit(x); }

    static bool less(F a, F b) { return a < b; }

    static bool less_equal(F a, F b) { return a <= b; }

    static F clamp01(F t) {
        if (std::isnan(t) || t > 1.0f) {
            return 1.0f;
        }

        return t > 0.0f ? t : 0.0f;
    }

    static bool equal(F a, F b) { return a == b; }

    static bool magnitude_below_one(F x) { return std::fabs(x) < 1.0f; }
};

using Int3 = std::array<s32, 3>;

template <typename M>
struct Kernels {
    using F = typename M::F;
    using V3 = std::array<F, 3>;

    static F one() { return M::lit(kOneBits); }

    static V3 zero3() { return {M::lit(0), M::lit(0), M::lit(0)}; }

    static V3 in3(const std::array<float, 3>& v) { return {M::in(v[0]), M::in(v[1]), M::in(v[2])}; }

    static std::array<float, 3> out3(const V3& v) {
        return {M::out(v[0]), M::out(v[1]), M::out(v[2])};
    }

    // vsub.xyz
    static V3 vsub(const V3& a, const V3& b) {
        return {M::sub(a[0], b[0]), M::sub(a[1], b[1]), M::sub(a[2], b[2])};
    }

    // vadd.xyz
    static V3 vadd(const V3& a, const V3& b) {
        return {M::add(a[0], b[0]), M::add(a[1], b[1]), M::add(a[2], b[2])};
    }

    // vmul.xyz
    static V3 vmul(const V3& a, const V3& b) {
        return {M::mul(a[0], b[0]), M::mul(a[1], b[1]), M::mul(a[2], b[2])};
    }

    // vmulx.xyz (or vmulq): every lane times one scalar.
    static V3 vmuls(const V3& a, F s) {
        return {M::mul(a[0], s), M::mul(a[1], s), M::mul(a[2], s)};
    }

    // vadda ACC, vf0, a; vmaddx d, b, s: a + b * s per lane, the product
    // rounded first.
    static V3 vmadds(const V3& a, const V3& b, F s) {
        return {
            M::add(a[0], M::mul(b[0], s)),
            M::add(a[1], M::mul(b[1], s)),
            M::add(a[2], M::mul(b[2], s)),
        };
    }

    // vopmula ACC, a, b; vopmsub d, b, a: a x b, both products rounded before
    // the subtraction.
    static V3 cross(const V3& a, const V3& b) {
        return {
            M::sub(M::mul(a[1], b[2]), M::mul(b[1], a[2])),
            M::sub(M::mul(a[2], b[0]), M::mul(b[2], a[0])),
            M::sub(M::mul(a[0], b[1]), M::mul(b[0], a[1])),
        };
    }

    // vmul t, a, b; vadday.x ACC, t, t; vmaddz.x d, one, t: (x + y) + 1 * z.
    static F dot_x(const V3& a, const V3& b) {
        const V3 p = vmul(a, b);
        return M::add(M::add(p[0], p[1]), M::mul(one(), p[2]));
    }

    // vaddax.y ACC, t, t; vmaddz.y d, one, t: (y + x) + 1 * z, the form the
    // kernels use where the sign is read from bit 63.
    static F dot_y(const V3& a, const V3& b) {
        const V3 p = vmul(a, b);
        return M::add(M::add(p[1], p[0]), M::mul(one(), p[2]));
    }

    // A decoded cell vertex as the kernels leave it in scratchpad.
    struct Vertex {
        V3 p;      // offset from the cell centre, x1024: (x * 64, y * 64, z * 16)
        u32 code;  // per-axis outcode bytes: bit 0 below the query box, bit 1 above
    };

    // 16-bit wrapping compares (psubh) of the halfword coordinates.
    static u32 outcode(
        const std::array<s16, 3>& h, const std::array<s16, 3>& lo, const std::array<s16, 3>& hi
    ) {
        u32 code = 0;

        for (std::size_t k = 0; k < 3; ++k) {
            const bool below = static_cast<s16>(h[k] - lo[k]) < 0;
            const bool above = static_cast<s16>(hi[k] - h[k]) < 0;
            code |= (u32{below} | u32{above} << 1) << (8 * k);
        }

        return code;
    }

    // The cell's vertices with their outcodes against the box [lo, hi] (x1024
    // relative to the centre, already truncated to 16 bits like ppach).
    static std::vector<Vertex> decode_vertices(
        const CollisionCell& cell, const std::array<s16, 3>& lo, const std::array<s16, 3>& hi
    ) {
        std::vector<Vertex> out;
        out.reserve(cell.packed.size());

        for (const PackedCollisionVertex& pv : cell.packed) {
            const auto f = pv.fields();
            const std::array<s16, 3> h{
                static_cast<s16>(f[0] << 6),
                static_cast<s16>(f[1] << 6),
                static_cast<s16>(f[2] << 4),
            };
            out.push_back({{M::itof(h[0]), M::itof(h[1]), M::itof(h[2])}, outcode(h, lo, hi)});
        }

        return out;
    }

    struct Triangle {
        std::array<u8, 3> v;
        u8 type;
    };

    // The triangles the kernels test, in order: every face record as (v0, v1,
    // v2) (the face count read as a byte, lbu), then the first quad_count
    // records again as (v0, v2, v3).
    static std::vector<Triangle> cell_triangles(const CollisionCell& cell) {
        const std::size_t faces =
            std::min<std::size_t>(cell.header.face_count & 0xff, cell.faces.size());
        const std::size_t quads =
            std::min({std::size_t{cell.header.quad_count}, cell.faces.size(), cell.quad_v3.size()});
        std::vector<Triangle> out;
        out.reserve(faces + quads);

        for (std::size_t i = 0; i < faces; ++i) {
            out.push_back({cell.faces[i].v, cell.faces[i].type});
        }

        for (std::size_t i = 0; i < quads; ++i) {
            const auto& f = cell.faces[i];
            out.push_back({{f.v[0], f.v[2], cell.quad_v3[i]}, f.type});
        }

        return out;
    }

    // Flags 0x80 (type bit 7) and 0x20 (surface id == (flags >> 8) & 0x1f).
    static bool excluded(u32 flags, u8 type) {
        return (flags & 0x80 & type) != 0
               || ((flags & 0x20) != 0 && (((flags >> 8) ^ type) & 0x1f) == 0);
    }

    static const CollisionCell* cell_of(const CollisionMesh& mesh, const Int3& c) {
        const CollisionCell* cell = mesh.find_cell(c[0], c[1], c[2]);
        // A cell without vertices is skipped (none is on the disc).
        return cell && cell->header.vertex_count != 0 ? cell : nullptr;
    }

    // The running best hit.
    struct Best {
        s32 kind;
        V3 point;
        V3 normal;
        std::array<V3, 3> tri;
    };

    // A triangle hit: `point` is P - v0, the vertices relative to `centre`.
    static Best record(
        u8 type,
        const V3& point,
        const V3& normal,
        const V3& v0,
        const V3& e1,
        const V3& e2,
        const V3& centre
    ) {
        const V3 v0_abs = vadd(v0, centre);
        return {
            0x1000 + type, vadd(point, v0_abs), normal, {v0_abs, vadd(e1, v0_abs), vadd(e2, v0_abs)}
        };
    }

    static CollisionHit output(const Best& b, F best, const std::optional<V3>& pushed, F scale) {
        CollisionHit hit;
        hit.kind = b.kind;
        hit.point = out3(vmuls(b.point, scale));

        if (pushed) {
            hit.pushed_centre = out3(*pushed);
        }

        hit.normal = out3(b.normal);

        for (std::size_t k = 0; k < 3; ++k) {
            hit.triangle[k] = out3(vmuls(b.tri[k], scale));
        }

        hit.best = M::out(best);
        return hit;
    }

    // ------------------------------------------------------------------
    // The segment (CollLine_Fix 0x211870)

    // vmini / vmax of the ends, then the [0, 1024)^3 test the kernel reads
    // from the MAC sign flags.
    static bool in_world(const V3& a, const V3& b) {
        for (std::size_t k = 0; k < 3; ++k) {
            if (M::neg(M::sub(M::min(a[k], b[k]), M::lit(0)))) {
                return false;
            }

            if (!M::neg(M::sub(M::max(a[k], b[k]), M::lit(k1024Bits)))) {
                return false;
            }
        }

        return true;
    }

    struct WalkEntry {
        F t;
        Int3 cell;
    };

    // The cell list of the segment (0x2119b0..0x211b38) in walk order,
    // without the t = 1 terminator. Nothing for a zero-length segment: both
    // ends in one cell at the same truncated x1024 point return "no hit" at
    // once (0x211b18).
    static std::optional<std::vector<WalkEntry>> walk(const V3& a1024, const V3& b1024) {
        const Int3 ia{M::ftoi(a1024[0]), M::ftoi(a1024[1]), M::ftoi(a1024[2])};
        const Int3 ib{M::ftoi(b1024[0]), M::ftoi(b1024[1]), M::ftoi(b1024[2])};
        Int3 s{};
        Int3 e{};

        for (std::size_t k = 0; k < 3; ++k) {
            // psrlw 12: floor(p / 4) for p >= 0.
            s[k] = static_cast<s32>(static_cast<u32>(ia[k]) >> 12);
            e[k] = static_cast<s32>(static_cast<u32>(ib[k]) >> 12);
        }

        if (s == e) {
            if (ia == ib) {
                return std::nullopt;
            }

            return std::vector<WalkEntry>{{M::lit(0), s}};
        }

        const V3 d = vsub(b1024, a1024);
        std::array<std::vector<F>, 3> lists;
        Int3 dir{};

        for (std::size_t k = 0; k < 3; ++k) {
            const F q = M::div(one(), d[k]);  // vdiv Q, vf0w, vf1x
            s32 at = e[k] - s[k] >= 0 ? s[k] : s[k] + 1;
            const s32 end = e[k] - s[k] >= 0 ? e[k] : e[k] + 1;
            const s32 step = e[k] - s[k] >= 0 ? 1 : -1;
            dir[k] = step;

            while (at != end) {
                at += step;
                // The plane at*4 in x1024 units; t = (plane - a) / d clamped
                // to [0, 1] on its bits.
                const F t = M::mul(M::sub(M::itof(at * 4096), a1024[k]), q);
                lists[k].push_back(M::clamp01(t));
            }
        }

        // Merge on the bits; ties go x, then y, then z.
        const std::size_t total = lists[0].size() + lists[1].size() + lists[2].size();
        std::vector<WalkEntry> out;
        out.reserve(total + 1);
        out.push_back({M::lit(0), s});
        Int3 cell = s;
        std::array<std::size_t, 3> next{};
        auto head = [&](std::size_t k) {
            return next[k] < lists[k].size() ? lists[k][next[k]] : M::lit(kWalkEndBits);
        };

        for (std::size_t n = 0; n < total; ++n) {
            const F x = head(0);
            const F y = head(1);
            const F z = head(2);
            std::size_t k = 0;

            if (M::less(y, x)) {
                k = M::less(z, y) ? 2 : 1;
            } else if (M::less(z, x)) {
                k = 2;
            }

            cell[k] += dir[k];
            out.push_back({head(k), cell});
            ++next[k];
        }

        return out;
    }

    static std::optional<std::vector<CollisionLineCell>> line_cells(
        const std::array<float, 3>& p0, const std::array<float, 3>& p1
    ) {
        const V3 a = in3(p0);
        const V3 b = in3(p1);

        if (!in_world(a, b)) {
            return std::nullopt;
        }

        const auto list = walk(vmuls(a, M::lit(k1024Bits)), vmuls(b, M::lit(k1024Bits)));

        if (!list) {
            return std::nullopt;
        }

        std::vector<CollisionLineCell> out;

        for (const WalkEntry& w : *list) {
            if (M::equal(w.t, one())) {
                break;
            }

            out.push_back({M::out(w.t), w.cell});
        }

        return out;
    }

    struct LineTriangleHit {
        F t;
        V3 p;  // P - v0
        V3 n;
        V3 e1;
        V3 e2;
    };

    // The segment against one triangle: a hit strictly before best_t.
    static std::optional<LineTriangleHit> line_triangle(
        u32 flags,
        const V3& d,
        const V3& a_rel,
        const V3& b_rel,
        const V3& v0,
        const V3& v1,
        const V3& v2,
        F best_t
    ) {
        const V3 e1 = vsub(v1, v0);
        const V3 e2 = vsub(v2, v0);
        const V3 n = cross(e2, e1);
        const F dn = dot_x(d, n);
        const F s0 = dot_x(vsub(v0, a_rel), n);
        const F s1 = dot_x(vsub(v0, b_rel), n);
        const F q = M::div(s0, dn);

        if ((flags & 0x10) == 0 && (!M::neg(s0) || !M::pos(s1))) {
            return std::nullopt;
        }

        if (M::neg(s0) == M::neg(s1)) {
            return std::nullopt;
        }

        const V3 p = vmadds(vsub(a_rel, v0), d, q);
        const F c0 = dot_x(cross(p, e1), n);
        const F c1 = dot_x(cross(e2, p), n);
        const F c2 = dot_x(cross(vsub(p, e1), vsub(e2, e1)), n);
        const F margin = M::sub(best_t, q);

        if (M::neg(c0) || M::neg(c1) || M::neg(c2) || !M::pos(margin)) {
            return std::nullopt;
        }

        return LineTriangleHit{q, p, n, e1, e2};
    }

    static Int3 centre_int(const Int3& c) {
        return {(c[0] << 12) + 2048, (c[1] << 12) + 2048, (c[2] << 12) + 2048};
    }

    static std::optional<CollisionHit> line(
        const CollisionMesh& mesh,
        const std::array<float, 3>& p0,
        const std::array<float, 3>& p1,
        u32 flags
    ) {
        const V3 a = in3(p0);
        const V3 b = in3(p1);

        if (!in_world(a, b)) {
            return std::nullopt;
        }

        const V3 a1024 = vmuls(a, M::lit(k1024Bits));
        const V3 b1024 = vmuls(b, M::lit(k1024Bits));
        const auto list = walk(a1024, b1024);

        if (!list || (flags & 0x1) != 0) {
            return std::nullopt;
        }

        const V3 d = vsub(b1024, a1024);  // vf13
        F best_t = one();                 // vf24.x
        std::optional<Best> best;

        for (std::size_t i = 0; i < list->size(); ++i) {
            const F t_enter = (*list)[i].t;
            const Int3 c = (*list)[i].cell;
            const F t_next = i + 1 < list->size() ? (*list)[i + 1].t : one();

            if (M::equal(t_enter, one())) {
                break;
            }

            if (best && M::less_equal(best_t, t_enter)) {
                break;
            }

            const CollisionCell* cell = cell_of(mesh, c);

            if (!cell) {
                continue;
            }

            // The box of the cell's stretch of segment, integer x1024
            // relative to the centre, widened by one.
            const Int3 ci = centre_int(c);
            const V3 pe = vmadds(a1024, d, t_enter);
            const V3 pn = vmadds(a1024, d, t_next);
            std::array<s16, 3> lo{};
            std::array<s16, 3> hi{};

            for (std::size_t k = 0; k < 3; ++k) {
                const s32 enter =
                    static_cast<s32>(static_cast<u32>(M::ftoi(pe[k])) - static_cast<u32>(ci[k]));
                const s32 leave =
                    static_cast<s32>(static_cast<u32>(M::ftoi(pn[k])) - static_cast<u32>(ci[k]));
                lo[k] = static_cast<s16>(std::min(enter, leave) - 1);
                hi[k] = static_cast<s16>(std::max(enter, leave) + 1);
            }

            const V3 centre{M::itof(ci[0]), M::itof(ci[1]), M::itof(ci[2])};  // vf16
            const V3 a_rel = vsub(a1024, centre);                             // vf14
            const V3 b_rel = vsub(b1024, centre);                             // vf15
            const auto verts = decode_vertices(*cell, lo, hi);

            for (const Triangle& tri : cell_triangles(*cell)) {
                const Vertex& v0 = verts[tri.v[0]];
                const Vertex& v1 = verts[tri.v[1]];
                const Vertex& v2 = verts[tri.v[2]];

                if ((v0.code & v1.code & v2.code) != 0 || excluded(flags, tri.type)) {
                    continue;
                }

                const auto hit = line_triangle(flags, d, a_rel, b_rel, v0.p, v1.p, v2.p, best_t);

                if (!hit) {
                    continue;
                }

                best_t = hit->t;
                best = record(tri.type, hit->p, hit->n, v0.p, hit->e1, hit->e2, centre);
            }
        }

        if (!best) {
            return std::nullopt;
        }

        return output(*best, best_t, std::nullopt, M::lit(kInv1024Bits));
    }

    // ------------------------------------------------------------------
    // The sphere (0x212960) and the capsule (0x2135a0)

    struct Volume {
        V3 c1024;  // vf31: the centre, or the capsule's base
        V3 lo;     // the query box in world units (vf19 / vf20 before scaling)
        V3 hi;
        V3 lo1024;
        V3 hi1024;
        F r1024;  // vf30.x
        F h1024;  // vf30.z, the capsule only
        F rr;     // (r * 1024)^2, vf24.x before the shrink
    };

    // The bounds checks: c - r >= 0 and hi - 1024 < 0 on every axis (MAC
    // sign flags), r > +0 (and h > +0 for the capsule) on the bits.
    static std::optional<Volume> volume(
        const std::array<float, 3>& centre, float radius, std::optional<float> height
    ) {
        const V3 c = in3(centre);
        const F r = M::in(radius);
        Volume v{};

        for (std::size_t k = 0; k < 3; ++k) {
            v.lo[k] = M::sub(c[k], r);
            v.hi[k] = M::add(c[k], r);
        }

        if (height) {
            v.hi[2] = M::add(v.hi[2], M::in(*height));
        }

        v.r1024 = M::mul(r, M::lit(k1024Bits));

        for (std::size_t k = 0; k < 3; ++k) {
            if (M::neg(M::sub(v.lo[k], M::lit(0)))) {
                return std::nullopt;
            }
        }

        if (!M::pos(r)) {
            return std::nullopt;
        }

        for (std::size_t k = 0; k < 3; ++k) {
            if (!M::neg(M::sub(v.hi[k], M::lit(k1024Bits)))) {
                return std::nullopt;
            }
        }

        if (height && !M::pos(M::in(*height))) {
            return std::nullopt;
        }

        v.c1024 = vmuls(c, M::lit(k1024Bits));
        v.lo1024 = vmuls(v.lo, M::lit(k1024Bits));
        v.hi1024 = vmuls(v.hi, M::lit(k1024Bits));
        v.h1024 = height ? M::mul(M::in(*height), M::lit(k1024Bits)) : M::lit(0);
        v.rr = M::mul(v.r1024, v.r1024);
        return v;
    }

    // Every cell of the box (vftoi0(p) >> 2), Z outer, Y, X inner, kept when
    // the squared distance from the centre to the cell's box is below r^2:
    // ((-r^2 + dx^2) + dy^2) + dz^2 < 0, unshrunk. For the capsule this is the
    // base sphere only.
    static std::vector<Int3> volume_cells(const Volume& v) {
        Int3 lo{};
        Int3 hi{};

        for (std::size_t k = 0; k < 3; ++k) {
            lo[k] = static_cast<s32>(static_cast<u32>(M::ftoi(v.lo[k])) >> 2);
            hi[k] = static_cast<s32>(static_cast<u32>(M::ftoi(v.hi[k])) >> 2);
        }

        const F negative_rr = M::sub(M::lit(0), v.rr);  // vf14.y
        std::vector<Int3> out;

        for (s32 z = lo[2]; z <= hi[2]; ++z) {
            for (s32 y = lo[1]; y <= hi[1]; ++y) {
                for (s32 x = lo[0]; x <= hi[0]; ++x) {
                    const Int3 c{x, y, z};
                    V3 p{};

                    for (std::size_t k = 0; k < 3; ++k) {
                        const F cmin = M::itof(c[k] * 4096);
                        const F cmax = M::add(cmin, M::lit(k4096Bits));
                        p[k] = M::min(M::max(cmin, v.c1024[k]), cmax);
                    }

                    const V3 sq = vmul(vsub(p, v.c1024), vsub(p, v.c1024));
                    const F s = M::add(
                        M::add(M::add(negative_rr, sq[0]), M::mul(one(), sq[1])),
                        M::mul(one(), sq[2])
                    );

                    if (M::neg(s)) {
                        out.push_back(c);
                    }
                }
            }
        }

        return out;
    }

    struct CellFrame {
        V3 centre;  // vf23
        V3 c_rel;   // vf22: the centre relative to the cell centre
        std::array<s16, 3> lo;
        std::array<s16, 3> hi;
    };

    // The outcode box: vftoi0 of the query box relative to the cell centre,
    // truncated to 16 bits, with no margin.
    static CellFrame cell_frame(const Volume& v, const Int3& c) {
        CellFrame f{};

        for (std::size_t k = 0; k < 3; ++k) {
            f.centre[k] = M::add(M::itof(c[k] * 4096), M::lit(k2048Bits));
            f.lo[k] = static_cast<s16>(M::ftoi(M::sub(v.lo1024[k], f.centre[k])));
            f.hi[k] = static_cast<s16>(M::ftoi(M::sub(v.hi1024[k], f.centre[k])));
        }

        f.c_rel = vsub(v.c1024, f.centre);
        return f;
    }

    // The push-out on return (0x213510 / 0x2143c8): Q = r / sqrt(best);
    // sphere: ((c - hit) * Q) / 1024 + hit / 1024; the capsule the same from
    // the axis point at height k = clamp(hit.z - c.z, 0, h), k taken off again
    // after the scale.
    static V3 pushed(const Volume& v, const Best& b, F best, bool capsule) {
        const F q = M::div(v.r1024, M::sqrt(best));
        V3 d = vsub(v.c1024, b.point);
        F k = M::lit(0);

        if (capsule) {
            const F up = M::sub(b.point[2], v.c1024[2]);
            k = M::max(M::min(up, v.h1024), M::lit(0));
            d[2] = M::add(d[2], k);
        }

        d = vmuls(d, q);

        if (capsule) {
            d[2] = M::sub(d[2], k);
        }

        return vadd(vmuls(d, M::lit(kInv1024Bits)), vmuls(b.point, M::lit(kInv1024Bits)));
    }

    // The nearest point of edge a -> b to p (all relative to v0):
    // t = ((p - a).(b - a)) / |b - a|^2 clamped to [0, 1], a + (b - a) * t.
    static V3 edge_point(const V3& p, const V3& a, const V3& b) {
        const V3 ba = vsub(b, a);
        F t = M::div(dot_x(vsub(p, a), ba), dot_x(ba, ba));
        t = M::min(M::max(M::add(M::lit(0), t), M::lit(0)), one());
        return vmadds(a, ba, t);
    }

    struct VolumeTriangleHit {
        V3 point;  // the nearest point - v0
        F d2;
        V3 n;
        V3 e1;
        V3 e2;
    };

    // The shared tail of the volume tests: p is the plane projection (relative
    // to v0) at squared distance d2; the edge tests pick the first failing edge
    // (v0v1, then v2v0, then v1v2) and its nearest point to p, whose squared
    // distance to `from` must not exceed best.
    static std::optional<std::pair<V3, F>> nearest_on_triangle(
        const V3& p, F d2, const V3& from, const V3& e1, const V3& e2, const V3& n, F best
    ) {
        if (M::neg(M::sub(best, d2))) {
            return std::nullopt;
        }

        const F c0 = dot_y(cross(p, e1), n);
        const F c1 = dot_y(cross(e2, p), n);
        const F c2 = dot_y(cross(vsub(p, e1), vsub(e2, e1)), n);
        V3 a{};
        V3 b{};

        if (M::neg(c0)) {
            a = zero3();
            b = e1;
        } else if (M::neg(c1)) {
            a = e2;
            b = zero3();
        } else if (!M::neg(c2)) {
            return std::pair{p, d2};
        } else {
            a = e1;
            b = e2;
        }

        const V3 q = edge_point(p, a, b);
        const V3 diff = vsub(q, from);
        const F d2_edge = dot_x(diff, diff);

        if (M::neg(M::sub(best, d2_edge))) {
            return std::nullopt;
        }

        return std::pair{q, d2_edge};
    }

    static F plane_distance2(const V3& nq) {
        const V3 sq = vmul(nq, nq);
        return M::add(M::add(sq[0], sq[1]), M::mul(one(), sq[2]));
    }

    // The sphere against one triangle (0x212d88..0x212f34).
    static std::optional<VolumeTriangleHit> sphere_triangle(
        u32 flags, const V3& c_rel, const V3& v0, const V3& v1, const V3& v2, F best
    ) {
        const V3 e1 = vsub(v1, v0);
        const V3 e2 = vsub(v2, v0);
        const V3 w = vsub(c_rel, v0);  // vf4
        const V3 n = cross(e2, e1);
        const F s = dot_x(w, n);
        const F nn = dot_x(n, n);
        const F q = M::div(s, nn);

        if ((flags & 0x10) == 0 && M::neg(s)) {
            return std::nullopt;
        }

        const V3 nq = vmuls(n, q);
        const V3 p = vmadds(w, vsub(zero3(), n), q);  // ACC = w; + (-N) * Q
        const auto nearest = nearest_on_triangle(p, plane_distance2(nq), w, e1, e2, n, best);

        if (!nearest) {
            return std::nullopt;
        }

        return VolumeTriangleHit{nearest->first, nearest->second, n, e1, e2};
    }

    // The capsule against one triangle (0x2139a8..): `c_rel` is the base
    // relative to the cell centre, h the height (x1024).
    static std::optional<VolumeTriangleHit> capsule_triangle(
        u32 flags, const V3& c_rel, F h, const V3& v0, const V3& v1, const V3& v2, F best
    ) {
        const V3 e1 = vsub(v1, v0);
        const V3 e2 = vsub(v2, v0);
        const V3 bottom = vsub(c_rel, v0);  // vf4: base - v0
        V3 top = bottom;                    // vf5: top - v0
        top[2] = M::add(top[2], h);
        const V3 n = cross(e2, e1);
        const F sb = dot_x(bottom, n);
        const F st = dot_x(top, n);

        if ((flags & 0x10) == 0 && M::neg(sb) && M::neg(st)) {
            return std::nullopt;
        }

        const F nn = dot_x(n, n);
        // The axis point (vf13) and its plane offset N * Q (vf8): 0x213bd8,
        // k = clamp(pz - base.z, 0, h), axis = (base.x, base.y, k + base.z).
        auto axis_at = [&](F pz) {
            const F k = M::max(M::min(M::sub(pz, bottom[2]), h), M::lit(0));
            const V3 axis{bottom[0], bottom[1], M::add(k, bottom[2])};
            return std::pair{axis, vmuls(n, M::div(dot_x(axis, n), nn))};
        };
        std::pair<V3, V3> axis;

        if (M::magnitude_below_one(n[2])) {
            axis = axis_at(M::lit(0));  // a vertical face: the height of v0
        } else {
            const F q = M::div(sb, M::mul(h, n[2]));
            const F b1z = M::sub(bottom[2], e1[2]);
            const F b2z = M::sub(bottom[2], e2[2]);
            const F t1z = M::sub(top[2], e1[2]);
            const F t2z = M::sub(top[2], e2[2]);

            if (!M::neg(bottom[2]) && !M::neg(b1z) && !M::neg(b2z)) {
                axis = {bottom, vmuls(n, M::div(sb, nn))};
            } else if (M::neg(top[2]) && M::neg(t1z) && M::neg(t2z)) {
                axis = {top, vmuls(n, M::div(st, nn))};
            } else {
                // Where the axis meets the plane: (base.xy, base.z - h * Q).
                const V3 x{bottom[0], bottom[1], M::sub(bottom[2], M::mul(h, q))};
                const F c0 = dot_y(cross(x, e1), n);
                const F c1 = dot_y(cross(e2, x), n);
                const F c2 = dot_y(cross(vsub(x, e1), vsub(e2, e1)), n);
                V3 p = x;

                if (M::neg(c0)) {
                    p = edge_point(x, zero3(), e1);
                } else if (M::neg(c1)) {
                    p = edge_point(x, e2, zero3());
                } else if (M::neg(c2)) {
                    p = edge_point(x, e1, e2);
                }

                axis = axis_at(p[2]);
            }
        }

        const V3 p = vsub(axis.first, axis.second);
        const auto nearest =
            nearest_on_triangle(p, plane_distance2(axis.second), axis.first, e1, e2, n, best);

        if (!nearest) {
            return std::nullopt;
        }

        return VolumeTriangleHit{nearest->first, nearest->second, n, e1, e2};
    }

    static std::optional<CollisionHit> volume_query(
        const CollisionMesh& mesh, const Volume& v, bool capsule, u32 flags
    ) {
        const F shrink = M::lit(kCollisionBestShrinkBits);
        F best_d = M::mul(v.rr, shrink);
        std::optional<Best> best;

        if ((flags & 0x1) == 0) {
            for (const Int3& c : volume_cells(v)) {
                const CollisionCell* cell = cell_of(mesh, c);

                if (!cell) {
                    continue;
                }

                const CellFrame f = cell_frame(v, c);
                const auto verts = decode_vertices(*cell, f.lo, f.hi);

                for (const Triangle& tri : cell_triangles(*cell)) {
                    const Vertex& v0 = verts[tri.v[0]];
                    const Vertex& v1 = verts[tri.v[1]];
                    const Vertex& v2 = verts[tri.v[2]];

                    if ((v0.code & v1.code & v2.code) != 0 || excluded(flags, tri.type)) {
                        continue;
                    }

                    const auto hit =
                        capsule
                            ? capsule_triangle(flags, f.c_rel, v.h1024, v0.p, v1.p, v2.p, best_d)
                            : sphere_triangle(flags, f.c_rel, v0.p, v1.p, v2.p, best_d);

                    if (!hit) {
                        continue;
                    }

                    best_d = M::mul(hit->d2, shrink);
                    best = record(tri.type, hit->point, hit->n, v0.p, hit->e1, hit->e2, f.centre);
                }
            }
        }

        if (!best) {
            return std::nullopt;
        }

        return output(*best, best_d, pushed(v, *best, best_d, capsule), M::lit(kInv1024Bits));
    }

    // ------------------------------------------------------------------
    // The hero groups (0x214d70): everything at x64, absolute coordinates.

    static std::optional<CollisionHit> hero_groups(
        const CollisionMesh& mesh, const std::array<float, 3>& centre, float radius
    ) {
        if (mesh.hero_groups.empty()) {
            return std::nullopt;
        }

        const F shrink = M::lit(kCollisionBestShrinkBits);
        const F r64 = M::mul(M::in(radius), M::lit(k64Bits));  // vf30.x
        const V3 c64 = vmuls(in3(centre), M::lit(k64Bits));    // vf31
        std::array<s16, 3> lo{};
        std::array<s16, 3> hi{};

        for (std::size_t k = 0; k < 3; ++k) {
            lo[k] = static_cast<s16>(M::ftoi(M::sub(c64[k], r64)));
            hi[k] = static_cast<s16>(M::ftoi(M::add(c64[k], r64)));
        }

        F best_d = M::mul(r64, r64);  // vf24.x, not shrunk
        std::optional<Best> best;

        for (const HeroCollisionGroup& g : mesh.hero_groups) {
            // The bounding-sphere cull: ((dy^2 - (r + R)^2) + dx^2) + dz^2,
            // kept when negative, or when it and dx^2 are both +0 (the kernel
            // tests the y:x lanes as one 64-bit word).
            const V3 s{M::itof(g.sphere[0]), M::itof(g.sphere[1]), M::itof(g.sphere[2])};
            const F rr = M::add(r64, M::itof(g.sphere[3]));
            const V3 d = vsub(s, c64);
            const V3 sq = vmul(d, d);
            const F cull = M::add(
                M::add(M::sub(sq[1], M::mul(rr, rr)), M::mul(one(), sq[0])), M::mul(one(), sq[2])
            );

            if (!(M::neg(cull) || (M::pos_zero(cull) && M::pos_zero(sq[0])))) {
                continue;
            }

            // Vertices zero-extended, outcodes on their low 16 bits against
            // the query box (no margin, no cell centre).
            std::vector<Vertex> verts;

            for (const auto& v : g.vertices) {
                const std::array<s16, 3> h{
                    static_cast<s16>(v[0]), static_cast<s16>(v[1]), static_cast<s16>(v[2])
                };
                verts.push_back(
                    {{M::itof(v[0]), M::itof(v[1]), M::itof(v[2])}, outcode(h, lo, hi)}
                );
            }

            const std::size_t count =
                std::min<std::size_t>(g.triangle_count & 0xff, g.triangles.size());

            for (std::size_t t = 0; t < count; ++t) {
                const auto& idx = g.triangles[t];

                if (idx[0] >= verts.size() || idx[1] >= verts.size() || idx[2] >= verts.size()) {
                    continue;
                }

                const Vertex& v0 = verts[idx[0]];
                const Vertex& v1 = verts[idx[1]];
                const Vertex& v2 = verts[idx[2]];

                if ((v0.code & v1.code & v2.code) != 0) {
                    continue;
                }

                const auto hit = sphere_triangle(0, c64, v0.p, v1.p, v2.p, best_d);

                if (!hit) {
                    continue;
                }

                best_d = M::mul(hit->d2, shrink);
                // The type is the record's pad byte, zero on every level.
                best = record(0, hit->point, hit->n, v0.p, hit->e1, hit->e2, zero3());
            }
        }

        if (!best) {
            return std::nullopt;
        }

        const F q = M::div(r64, M::sqrt(best_d));  // vrsqrt Q, vf30.x, vf24.x
        const V3 d = vmuls(vmuls(vsub(c64, best->point), q), M::lit(kInv64Bits));
        const V3 push = vadd(d, vmuls(best->point, M::lit(kInv64Bits)));
        return output(*best, best_d, push, M::lit(kInv64Bits));
    }
};

template <template <typename> class Op, typename... Args>
auto dispatch(FloatModel model, Args&&... args) {
    if (model == FloatModel::Console) {
        return Op<ConsoleMath>::run(std::forward<Args>(args)...);
    }

    return Op<NativeMath>::run(std::forward<Args>(args)...);
}

template <typename M>
struct LineOp {
    static std::optional<CollisionHit> run(
        const CollisionMesh& mesh,
        const std::array<float, 3>& p0,
        const std::array<float, 3>& p1,
        u32 flags
    ) {
        return Kernels<M>::line(mesh, p0, p1, flags);
    }
};

template <typename M>
struct SphereOp {
    static std::optional<CollisionHit> run(
        const CollisionMesh& mesh, const std::array<float, 3>& centre, float radius, u32 flags
    ) {
        const auto v = Kernels<M>::volume(centre, radius, std::nullopt);
        return v ? Kernels<M>::volume_query(mesh, *v, false, flags) : std::nullopt;
    }
};

template <typename M>
struct CapsuleOp {
    static std::optional<CollisionHit> run(
        const CollisionMesh& mesh,
        const std::array<float, 3>& base,
        float height,
        float radius,
        u32 flags
    ) {
        const auto v = Kernels<M>::volume(base, radius, height);
        return v ? Kernels<M>::volume_query(mesh, *v, true, flags) : std::nullopt;
    }
};

template <typename M>
struct HeroOp {
    static std::optional<CollisionHit> run(
        const CollisionMesh& mesh, const std::array<float, 3>& centre, float radius
    ) {
        return Kernels<M>::hero_groups(mesh, centre, radius);
    }
};

template <typename M>
struct LineCellsOp {
    static std::optional<std::vector<CollisionLineCell>> run(
        const std::array<float, 3>& p0, const std::array<float, 3>& p1
    ) {
        return Kernels<M>::line_cells(p0, p1);
    }
};

template <typename M>
struct VolumeCellsOp {
    static std::optional<std::vector<Int3>> run(
        const std::array<float, 3>& centre, float radius, std::optional<float> height
    ) {
        const auto v = Kernels<M>::volume(centre, radius, height);

        if (!v) {
            return std::nullopt;
        }

        return Kernels<M>::volume_cells(*v);
    }
};

}  // namespace

int CollisionHit::surface_id() const {
    return kind >= 0 && (kind & 0x1f) != 0x1f ? kind & 0x1f : -1;
}

int CollisionHit::sound_class() const {
    return kind >= 0 && (kind & 0x60) != 0x60 ? (kind & 0x60) >> 5 : 0;
}

std::optional<u8> CollisionHit::face_type() const {
    if (kind >= 0x1000) {
        return static_cast<u8>(kind & 0xff);
    }

    return std::nullopt;
}

std::optional<CollisionHit> collide_line(
    const CollisionMesh& mesh,
    const std::array<float, 3>& p0,
    const std::array<float, 3>& p1,
    u32 flags,
    FloatModel model
) {
    return dispatch<LineOp>(model, mesh, p0, p1, flags);
}

std::optional<CollisionHit> collide_sphere(
    const CollisionMesh& mesh,
    const std::array<float, 3>& centre,
    float radius,
    u32 flags,
    FloatModel model
) {
    return dispatch<SphereOp>(model, mesh, centre, radius, flags);
}

std::optional<CollisionHit> collide_capsule(
    const CollisionMesh& mesh,
    const std::array<float, 3>& base,
    float height,
    float radius,
    u32 flags,
    FloatModel model
) {
    return dispatch<CapsuleOp>(model, mesh, base, height, radius, flags);
}

std::optional<CollisionHit> collide_sphere_hero_groups(
    const CollisionMesh& mesh, const std::array<float, 3>& centre, float radius, FloatModel model
) {
    return dispatch<HeroOp>(model, mesh, centre, radius);
}

std::optional<std::vector<CollisionLineCell>> collision_line_cells(
    const std::array<float, 3>& p0, const std::array<float, 3>& p1, FloatModel model
) {
    return dispatch<LineCellsOp>(model, p0, p1);
}

std::optional<std::vector<std::array<s32, 3>>> collision_sphere_cells(
    const std::array<float, 3>& centre, float radius, FloatModel model
) {
    return dispatch<VolumeCellsOp>(model, centre, radius, std::optional<float>{});
}

std::optional<std::vector<std::array<s32, 3>>> collision_capsule_cells(
    const std::array<float, 3>& base, float height, float radius, FloatModel model
) {
    return dispatch<VolumeCellsOp>(model, base, radius, std::optional<float>{height});
}

}  // namespace openrac::assets
