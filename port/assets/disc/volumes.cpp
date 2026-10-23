// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/volumes.rs and
// crates/rc-formats/src/gameplay.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The volume sections of the gameplay file.

#include "assets/disc/volumes.h"

namespace openrac::assets::disc {

namespace {

// A section's `s32 count` header, or nothing for an absent section.
std::optional<std::pair<std::size_t, std::size_t>> section_count(
    ByteView g, std::size_t pointer, std::string_view what
) {
    const u32 s = g.u32_at(pointer);

    if (s == 0) {
        return std::nullopt;
    }

    const s32 n = g.s32_at(s);

    if (n < 0 || n > 0x10000) {
        fail("implausible {} count {}", what, n);
    }

    return std::pair<std::size_t, std::size_t>{s, static_cast<std::size_t>(n)};
}

// A spline: s32 count, pad[3], count vec4.
std::vector<Vec4> spline(ByteView data, s32 offset, std::string_view what, std::size_t i) {
    if (offset < 0) {
        fail("{} {}: negative offset", what, i);
    }

    const s32 k = data.s32_at(static_cast<std::size_t>(offset));

    if (k < 0) {
        fail("{} {}: negative point count", what, i);
    }

    return data.read_array<Vec4>(
        static_cast<std::size_t>(offset) + 0x10, static_cast<std::size_t>(k), what
    );
}

}  // namespace

std::size_t shape_pointer(ShapeKind kind) {
    switch (kind) {
        case ShapeKind::Cuboid:
            return kCuboidsPointer;
        case ShapeKind::Sphere:
            return kSpheresPointer;
        case ShapeKind::Cylinder:
            return kCylindersPointer;
        case ShapeKind::Pill:
            return kPillsPointer;
    }

    return kCuboidsPointer;
}

s32 shape_grid_type(ShapeKind kind) {
    switch (kind) {
        case ShapeKind::Cuboid:
            return 3;
        case ShapeKind::Sphere:
            return 5;
        case ShapeKind::Cylinder:
            return 6;
        case ShapeKind::Pill:
            return 7;
    }

    return 0;
}

std::optional<ShapeKind> shape_from_grid_type(s32 type) {
    for (const ShapeKind k :
         {ShapeKind::Cuboid, ShapeKind::Sphere, ShapeKind::Cylinder, ShapeKind::Pill}) {
        if (shape_grid_type(k) == type) {
            return k;
        }
    }

    return std::nullopt;
}

Vec3 Shape::centre() const {
    return {matrix[3][0], matrix[3][1], matrix[3][2]};
}

// As every test computes it (NTSC-U 0x2211b8, then VU0 0x2215e0).
Vec3 Shape::local(Vec3 p) const {
    const Vec3 c = centre();
    const Vec3 d = {p[0] - c[0], p[1] - c[1], p[2] - c[2]};
    Vec3 l{};

    for (std::size_t k = 0; k < 3; ++k) {
        l[k] = inverse[0][k] * d[0] + inverse[1][k] * d[1] + inverse[2][k] * d[2];
    }

    return l;
}

Vec3 Shape::world(Vec3 l) const {
    Vec3 w{};

    for (std::size_t k = 0; k < 3; ++k) {
        w[k] = l[0] * matrix[0][k] + l[1] * matrix[1][k] + l[2] * matrix[2][k] + matrix[3][k];
    }

    return w;
}

std::span<const Shape> Volumes::shapes(ShapeKind kind) const {
    switch (kind) {
        case ShapeKind::Cuboid:
            return cuboids;
        case ShapeKind::Sphere:
            return spheres;
        case ShapeKind::Cylinder:
            return cylinders;
        case ShapeKind::Pill:
            return pills;
    }

    return {};
}

const Shape* Volumes::shape(ShapeKind kind, s32 index) const {
    const auto s = shapes(kind);

    if (index < 0 || static_cast<std::size_t>(index) >= s.size()) {
        return nullptr;
    }

    return &s[static_cast<std::size_t>(index)];
}

std::optional<f32> Volumes::pill_cap_radius(s32 index) const {
    if (index < 0 || static_cast<std::size_t>(index) >= pill_tail.size()) {
        return std::nullopt;
    }

    return pill_tail[static_cast<std::size_t>(index)];
}

std::vector<Shape> parse_shapes(ByteView gameplay, ShapeKind kind) {
    const auto s = section_count(gameplay, shape_pointer(kind), "shape");

    if (!s) {
        return {};
    }

    return gameplay.read_array<Shape>(s->first + 0x10, s->second, "shape records");
}

std::vector<std::vector<Vec4>> parse_paths(ByteView gameplay) {
    const u32 s = gameplay.u32_at(kPathsPointer);

    if (s == 0) {
        return {};
    }

    const s32 count = gameplay.s32_at(s);
    const s32 data_offset = gameplay.s32_at(s + 4);
    const s32 data_size = gameplay.s32_at(s + 8);

    if (count < 0 || data_offset < 0 || data_size < 0) {
        fail("negative path section field");
    }

    const ByteView data = gameplay.sub(
        s + static_cast<std::size_t>(data_offset), static_cast<std::size_t>(data_size), "path data"
    );
    std::vector<std::vector<Vec4>> out;

    for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
        out.push_back(spline(data, gameplay.s32_at(s + 0x10 + 4 * i), "spline", i));
    }

    return out;
}

std::vector<GrindPath> parse_grind_paths(ByteView gameplay) {
    const auto sc = section_count(gameplay, kGrindPathsPointer, "grind path");

    if (!sc) {
        return {};
    }

    const auto [s, n] = *sc;
    const s32 data_offset = gameplay.s32_at(s + 4);
    const s32 data_size = gameplay.s32_at(s + 8);

    if (data_offset < 0 || data_size < 0) {
        fail("negative grind path data field");
    }

    const ByteView data = gameplay.sub(
        s + static_cast<std::size_t>(data_offset),
        static_cast<std::size_t>(data_size),
        "grind path data"
    );
    const std::size_t offsets = s + 0x10 + n * kGrindPathSize;
    std::vector<GrindPath> out;

    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t r = s + 0x10 + i * kGrindPathSize;
        GrindPath g;
        g.bsphere = gameplay.read<Vec4>(r, "grind path sphere");
        g.flag = gameplay.s32_at(r + 0x10);
        g.points = spline(data, gameplay.s32_at(offsets + 4 * i), "grind path", i);
        out.push_back(std::move(g));
    }

    return out;
}

Volumes parse_volumes(ByteView gameplay) {
    Volumes v;
    v.cuboids = parse_shapes(gameplay, ShapeKind::Cuboid);
    v.spheres = parse_shapes(gameplay, ShapeKind::Sphere);
    v.cylinders = parse_shapes(gameplay, ShapeKind::Cylinder);
    v.pills = parse_shapes(gameplay, ShapeKind::Pill);

    if (const auto s = section_count(gameplay, kPillsPointer, "pill")) {
        // The loader's 0x90-byte copy: +0x80 of pill i is the word after its record.
        for (std::size_t i = 0; i < s->second; ++i) {
            v.pill_tail.push_back(gameplay.f32_at(s->first + 0x10 + i * kShapeSize + kShapeSize));
        }
    }

    v.paths = parse_paths(gameplay);
    v.grind_paths = parse_grind_paths(gameplay);
    return v;
}

}  // namespace openrac::assets::disc
