// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/sky.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The RAC1 sky reader.

#include "assets/geometry/sky.h"

#include <format>

namespace openrac::assets::rac1 {

namespace {

std::size_t offset_of(s32 v, std::string_view what) {
    if (v < 0) {
        fail("sky: negative {} offset", what);
    }

    return static_cast<std::size_t>(v);
}

}  // namespace

std::vector<SkyGsVertex> sky_gs_vertices(const SkyShell& shell, const SkyCluster& cluster) {
    std::vector<SkyGsVertex> out;
    out.reserve(cluster.faces.size() * 3);

    for (const SkyFace& f : cluster.faces) {
        for (const u8 ix : f.indices) {
            const SkyVertex& v = cluster.vertices.at(ix);
            const SkyVertexAttr& a = cluster.attrs.at(ix);
            SkyGsVertex g;
            g.position = {v.x, v.y, v.z};

            if (shell.textured()) {
                g.texture = f.texture;
                g.st = a.st();
                g.rgba = {0x80, 0x80, 0x80, static_cast<u8>(v.alpha)};
            } else {
                g.rgba = a.rgba();
            }

            out.push_back(g);
        }
    }

    return out;
}

Mesh sky_shell_mesh(const SkyShell& shell) {
    Mesh mesh;

    for (const SkyCluster& cluster : shell.clusters) {
        const std::vector<SkyGsVertex> stream = sky_gs_vertices(shell, cluster);

        for (std::size_t i = 0; i + 3 <= stream.size(); i += 3) {
            const auto base = static_cast<u32>(mesh.vertices.size());

            for (std::size_t k = 0; k < 3; ++k) {
                const SkyGsVertex& g = stream[i + k];
                MeshVertex v;
                v.position =
                    {g.position[0] / 1024.0f, g.position[1] / 1024.0f, g.position[2] / 1024.0f};
                v.uv = g.st;
                v.rgba = g.rgba;
                mesh.vertices.push_back(v);
            }

            const u8 texture = stream[i].texture;
            mesh.triangles.push_back(
                {{base, base + 1, base + 2}, texture == 0xff ? -1 : s32{texture}}
            );
        }
    }

    return mesh;
}

Sky parse_sky(ByteView b) {
    Sky sky;
    sky.header = b.read<SkyHeader>(0, "sky header");
    const SkyHeader& h = sky.header;

    if (h.shell_count < 0 || h.shell_count > 8) {
        fail("sky: shell count {} out of range", h.shell_count);
    }

    if (h.fx_count > 0 && h.fx_list > 0) {
        sky.fx_list = b.sub(
                           static_cast<std::size_t>(h.fx_list),
                           static_cast<std::size_t>(h.fx_count),
                           "sky fx list"
        )
                          .to_vector();
    }

    if (h.texture_count > 0 && h.texture_defs > 0) {
        sky.texture_defs = b.read_array<SkyTextureDef>(
            static_cast<std::size_t>(h.texture_defs),
            static_cast<std::size_t>(h.texture_count),
            "sky texture defs"
        );
    }

    for (std::size_t s = 0; s < static_cast<std::size_t>(h.shell_count); ++s) {
        const std::size_t so = offset_of(h.shells[s], "shell");
        SkyShell shell;
        shell.cluster_count = b.s32_at(so);
        shell.flags = b.s32_at(so + 4);

        if (shell.cluster_count < 0 || shell.cluster_count > 10'000) {
            fail("sky: implausible cluster count {}", shell.cluster_count);
        }

        const auto headers = b.read_array<SkyClusterHeader>(
            so + 0x10, static_cast<std::size_t>(shell.cluster_count), "sky cluster headers"
        );

        for (const SkyClusterHeader& ch : headers) {
            // The game DMAs only data_size bytes of the cluster, so every
            // array must lie inside them.
            auto within = [&](s16 o, std::size_t length) {
                return o >= 0 && ch.data_size >= 0
                       && static_cast<std::size_t>(o) + length
                              <= static_cast<std::size_t>(ch.data_size);
            };
            const s16 vc = ch.vertex_count;
            const s16 tc = ch.tri_count;

            if (ch.data < 0 || vc < 0 || tc < 0 || !within(ch.vertex_offset, std::size_t(vc) * 8)
                || !within(ch.st_offset, std::size_t(vc) * 4)
                || !within(ch.tri_offset, std::size_t(tc) * 4)) {
                fail("sky cluster arrays outside data_size");
            }

            const auto d = static_cast<std::size_t>(ch.data);
            SkyCluster cluster;
            cluster.header = ch;
            cluster.vertices = b.read_array<SkyVertex>(
                d + static_cast<std::size_t>(ch.vertex_offset),
                static_cast<std::size_t>(vc),
                "sky vertices"
            );
            cluster.attrs = b.read_array<SkyVertexAttr>(
                d + static_cast<std::size_t>(ch.st_offset),
                static_cast<std::size_t>(vc),
                "sky attributes"
            );
            cluster.faces = b.read_array<SkyFace>(
                d + static_cast<std::size_t>(ch.tri_offset),
                static_cast<std::size_t>(tc),
                "sky faces"
            );

            for (const SkyFace& f : cluster.faces) {
                for (const u8 i : f.indices) {
                    if (i >= vc) {
                        fail("sky face index {} out of range", i);
                    }
                }

                if (f.texture != 0xff && f.texture >= h.texture_count) {
                    fail("sky face texture {} out of range", f.texture);
                }
            }

            shell.clusters.push_back(std::move(cluster));
        }

        sky.shells.push_back(std::move(shell));
    }

    return sky;
}

std::string SkyTexture::key() const {
    return std::format("sky/{:02}_{}x{}", index, def.width, def.height);
}

IndexedImage sky_texture_image(ByteView block, const Sky& sky, std::size_t index) {
    if (index >= sky.texture_defs.size()) {
        fail("sky texture {} out of range", index);
    }

    const SkyTextureDef& t = sky.texture_defs[index];

    if (t.width <= 0 || t.height <= 0) {
        fail("sky texture {} has no pixels", index);
    }

    const std::size_t base = offset_of(sky.header.texture_data, "texture data");
    const auto w = static_cast<u32>(t.width);
    const auto h = static_cast<u32>(t.height);
    return {
        w,
        h,
        block.sub(
            base + offset_of(t.texture_offset, "texture"), std::size_t{w} * h, "sky texture pixels"
        ),
        block.sub(base + offset_of(t.palette_offset, "palette"), 1024, "sky palette"),
    };
}

std::vector<SkyTexture> decode_sky_textures(ByteView block, const Sky& sky) {
    std::vector<SkyTexture> out;

    for (std::size_t i = 0; i < sky.texture_defs.size(); ++i) {
        out.push_back({i, sky.texture_defs[i], sky_texture_image(block, sky, i).decode()});
    }

    return out;
}

}  // namespace openrac::assets::rac1
