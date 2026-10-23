#version 410 core
// SPDX-License-Identifier: GPL-3.0-or-later
layout(location = 0) in vec4 centre_radius;
layout(location = 1) in vec3 status_colour;
layout(location = 2) in float region_opacity;
layout(location = 3) in int region_id;
uniform int probe_gpu_selection;
uniform int probe_region_count;
uniform vec2 probe_tile_extent;
uniform usamplerBuffer probe_region_masks;
uniform mat4 view_projection;
out vec3 v_normal;
out vec3 v_colour;
out float v_opacity;
const ivec2 corners[6] =
    ivec2[6](ivec2(0, 0), ivec2(1, 0), ivec2(1, 1), ivec2(0, 0), ivec2(1, 1), ivec2(0, 1));

void main() {
    const int longitude = 12, latitude = 8;
    int cell = gl_VertexID / 6;
    ivec2 at = ivec2(cell % longitude, cell / longitude) + corners[gl_VertexID % 6];
    float azimuth = float(at.x) * 6.28318530718 / float(longitude);
    float polar = float(at.y) * 3.14159265359 / float(latitude);
    v_normal = vec3(sin(polar) * cos(azimuth), sin(polar) * sin(azimuth), cos(polar));
    v_colour = status_colour;
    v_opacity = region_opacity;
    if (probe_gpu_selection == 1 && region_id >= 0) {
        vec4 centre = view_projection * vec4(centre_radius.xyz, 1);
        ivec2 pixel = ivec2((centre.xy / centre.w * 0.5 + 0.5) * probe_tile_extent);
        v_opacity = 0.0;
        if (centre.w > 0.0 && all(greaterThanEqual(pixel, ivec2(0)))
            && all(lessThan(pixel, ivec2(probe_tile_extent)))) {
            ivec2 tile = pixel / 16;
            int tiles_x = (int(probe_tile_extent.x) + 15) / 16;
            uint bits =
                texelFetch(
                    probe_region_masks,
                    (tile.y * tiles_x + tile.x) * ((probe_region_count + 31) / 32) + region_id / 32
                )
                    .r;
            v_opacity = (bits & (1u << uint(region_id % 32))) != 0u ? region_opacity : 0.0;
        }
    }
    gl_Position = view_projection * vec4(centre_radius.xyz + v_normal * centre_radius.w, 1.0);
    if (v_opacity <= 0.0) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
    }
}
