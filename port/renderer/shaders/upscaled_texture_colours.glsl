// SPDX-License-Identifier: GPL-3.0-or-later
// Colour reconstruction for magnified world surfaces only.
// Alpha stays on the original sampler so depth/coverage remains unchanged.
uniform sampler2D upscaled_texture_colours;
uniform int upscaled_texture_colours_ready;
vec4 sample_upscaled_texture_colours(vec2 uv) {
  vec4 original = texture(tex_T0, uv);
  if (upscaled_texture_colours_ready == 0) {
    return original;
  }
  vec2 size = vec2(textureSize(tex_T0, 0));
  vec2 dx = dFdx(uv) * size, dy = dFdy(uv) * size;
  float footprint = max(length(dx), length(dy));
  if (footprint >= 1.0) {
    return original;
  }
  original.rgb = mix(textureLod(upscaled_texture_colours, uv, 0.0).rgb, original.rgb,
                     smoothstep(0.5, 1.0, footprint));
  return original;
}
