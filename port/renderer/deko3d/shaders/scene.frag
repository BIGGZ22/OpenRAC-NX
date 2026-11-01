// SPDX-License-Identifier: GPL-3.0-or-later
#version 460
layout(std140,binding=0) uniform Draw {
    mat4 projection; mat4 model; mat4 view;
    vec4 tint; vec4 material; vec4 moby_light; vec4 ambient;
    vec4 fog_params; vec4 fog_colour;
    uvec4 flags; // textured, joint count, light colour count, cutout
};
layout(binding=0) uniform sampler2D diffuse;
layout(location=0) in vec4 vertex_colour;
layout(location=1) in vec2 texcoord;
layout(location=2) noperspective in float fog;
layout(location=0) out vec4 color;
void main() {
    vec4 value=vertex_colour*material;
    if(flags.x!=0u) value*=texture(diffuse,texcoord);
    uint test=flags.w&255u;
    if(test==1u && value.a<0.5) discard;
    if(test==2u && value.a<0.99) discard;
    if(test==3u && (value.a>=0.99 || value.a<0.02)) discard;
    if(fog_colour.w>0.5 && (flags.w&512u)==0u) value.rgb=mix(fog_colour.rgb,value.rgb,fog);
    color=clamp(value,0.0,1.0);
}
