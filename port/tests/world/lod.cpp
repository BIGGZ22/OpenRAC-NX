// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The world renderers' CPU side: terrain and tie level of detail, culling and
// fog, checked against the rules their headers document.

#include <array>
#include <cmath>

#include "renderer/world/fog.h"
#include "renderer/world/tfrag_lod.h"
#include "renderer/world/tie_lod.h"
#include "tests/check.h"

namespace {

using namespace openrac::renderer;
using namespace openrac::renderer::world;

void tfrag_lod_rule() {
    CHECK(tfrag_mode_list(6) == 2);
    CHECK(tfrag_mode_list(8) == 1);
    CHECK(tfrag_mode_list(0xa) == 1);
    CHECK(tfrag_mode_list(0xe) == 0);
    CHECK(tfrag_mode_list(0x14) == 0);

    // L = 10: D0, D1, D2 = 60, 40, 20 game units.
    const auto d = tfrag_lod_distances(10.0f);
    CHECK(d[0] == 60.0f && d[1] == 40.0f && d[2] == 20.0f);
    const auto t = tfrag_lod_thresholds(10.0f);
    CHECK(t[0] == 61440 && t[1] == 40960 && t[2] == 20480);

    CHECK(tfrag_lod_mode(70000.0f, 1000.0f, false, t) == 6);
    CHECK(tfrag_lod_mode(1000.0f, 100.0f, true, t) == 6);     // base only: always LOD 2
    CHECK(tfrag_lod_mode(60000.0f, 2000.0f, false, t) == 8);  // straddles D0
    CHECK(tfrag_lod_mode(50000.0f, 1000.0f, false, t) == 0xa);
    CHECK(tfrag_lod_mode(41000.0f, 1000.0f, false, t) == 0xe);  // straddles D1
    CHECK(tfrag_lod_mode(30000.0f, 100.0f, false, t) == 0x10);
    CHECK(tfrag_lod_mode(1000.0f, 100.0f, false, t) == 0x14);
}

TfragCull box(float x, float y, float z, float r) {
    TfragCull c;
    c.sphere = {x, y, z, r};
    int i = 0;
    for (float dx : {-r, r}) {
        for (float dy : {-r, r}) {
            for (float dz : {-r, r}) {
                c.corners[static_cast<std::size_t>(i++)] = {x + dx, y + dy, z + dz};
            }
        }
    }
    return c;
}

void tfrag_cull() {
    const auto t = tfrag_lod_thresholds(10.0f);
    const Vec3 eye{0.0f, 0.0f, 0.0f};
    const std::array<Vec3, 3> rows{Vec3{1, 0, 0}, Vec3{0, 1, 0}, Vec3{0, 0, 1}};
    const float far_cull = 512000.0f;
    // Wholly inside the view: the LOD rule.
    CHECK(tfrag_proc(box(0, 0, 30000, 100), eye, rows, t, 1.0f, 1.0f, far_cull) == 0x10);
    // Behind the camera, beyond the far cull, outside a side plane.
    CHECK(tfrag_proc(box(0, 0, -30000, 100), eye, rows, t, 1.0f, 1.0f, far_cull) == kTfragCulled);
    CHECK(tfrag_proc(box(0, 0, 600000, 100), eye, rows, t, 1.0f, 1.0f, far_cull) == kTfragCulled);
    CHECK(
        tfrag_proc(box(50000, 0, 30000, 100), eye, rows, t, 1.0f, 1.0f, far_cull) == kTfragCulled
    );
    // Across the near plane: the clipping path.
    CHECK(tfrag_proc(box(0, 0, 0, 100), eye, rows, t, 1.0f, 1.0f, far_cull) == kTfragClip);
}

void tie_lod_rule() {
    const std::array<float, 3> dist{10.0f, 20.0f, 40.0f};
    CHECK((tie_lod(5.0f, dist) == TieLodPick{0, 0.0f, 0.0f, 256.0f}));
    CHECK((tie_lod(10.0f, dist) == TieLodPick{0, 0.0f, 0.0f, 256.0f}));
    CHECK((tie_lod(50.0f, dist) == TieLodPick{2, 0.0f, 0.0f, 256.0f}));
    CHECK((tie_lod(15.0f, dist) == TieLodPick{0, 0.5f, 128.0f, 128.0f}));
    CHECK((tie_lod(30.0f, dist) == TieLodPick{1, 0.5f, 128.0f, 128.0f}));
    CHECK((tie_lod(40.0f, dist) == TieLodPick{1, 1.0f, 256.0f, 0.0f}));
}

void tie_culling() {
    CHECK(!tie_cull({0, 0, 10}, 1.0f, 0.0f, 1.0f, 1.0f));     // no draw distance
    CHECK(!tie_cull({0, 0, 200}, 1.0f, 100.0f, 1.0f, 1.0f));  // beyond it
    CHECK(!tie_cull({0, 0, -5}, 1.0f, 100.0f, 1.0f, 1.0f));   // in front of the near plane
    CHECK(!tie_cull({50, 0, 10}, 1.0f, 100.0f, 1.0f, 1.0f));  // outside a side
    CHECK(tie_cull({0, 0, 10}, 1.0f, 100.0f, 1.0f, 1.0f) == 10.0f);
    CHECK(tie_cull({0, 0, 0}, 1.0f, 100.0f, 1.0f, 1.0f) == 0.0f);  // depth clamped at 0

    const WorldFog fog;
    const auto culled = tie_record({0, 0, 200}, 1.0f, 100.0f, {10, 20, 40}, fog, 1.0f, 1.0f);
    CHECK(culled[0] == kTieCulled && culled[1] == 0 && culled[2] == 0 && culled[3] == 0);
}

void fog_line() {
    WorldFog fog;
    fog.near_dist = 0.0f;
    fog.far_dist = 100.0f * kUnits;
    fog.near_intensity = 255.0f;
    fog.far_intensity = 0.0f;
    CHECK(fog.vertex_f(0.0f) == 255.0f);
    CHECK(fog.vertex_f(fog.far_dist) == 0.0f);
    CHECK(std::fabs(fog.vertex_f(50.0f * kUnits) - 127.5f) < 0.01f);
    CHECK(fog.vertex_f(1e9f) == 0.0f);  // clamped
    CHECK(fog.vertex_f(-1e9f) == 255.0f);

    const auto p = fog_params(fog);
    CHECK(p[2] == 0.0f && p[3] == 255.0f);
    CHECK(std::fabs(p[0] * 50.0f * kUnits + p[1] - 127.5f) < 0.01f);

    CHECK(tie_fog_value(0.0f, fog) == 255);
    CHECK(tie_fog_value(200.0f, fog) == 0);
    CHECK(tie_fog_value(50.0f, fog) == 127);

    // The tie record carries F above the LOD.
    const auto r = tie_record({0, 0, 50}, 1.0f, 100.0f, {10, 20, 40}, fog, 1.0f, 1.0f);
    CHECK(r[0] == (2u | (127u << 8)));
}

}  // namespace

int main() {
    tfrag_lod_rule();
    tfrag_cull();
    tie_lod_rule();
    tie_culling();
    fog_line();
    return openrac::test::result();
}
