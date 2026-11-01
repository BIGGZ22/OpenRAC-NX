// SPDX-License-Identifier: GPL-3.0-or-later
#version 460
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec4 colour;
layout(location=4) in uvec4 joints;
layout(location=5) in vec4 weights;
layout(location=6) in float light_slot;
layout(std140,binding=0) uniform Draw {
    mat4 projection; mat4 model; mat4 view;
    vec4 tint; vec4 material; vec4 moby_light; vec4 ambient;
    vec4 fog_params; vec4 fog_colour;
    uvec4 flags; // textured, joint count, light colour count, cutout
};
layout(std140,binding=1) uniform Pose { mat4 joint_matrices[256]; };
layout(std140,binding=2) uniform Colours { vec4 light_colours[64]; };
layout(std140,binding=3) uniform LightSets { vec4 light_sets[64]; };
layout(location=0) out vec4 vertex_colour;
layout(location=1) out vec2 texcoord;
layout(location=2) noperspective out float fog;
void main() {
    vec4 p = vec4(position,1.0);
    vec3 n = normal;
    if (flags.y != 0u && dot(weights,vec4(1.0)) > 0.0) {
        uvec4 ids = min(joints,uvec4(flags.y-1u));
        mat4 skin = weights.x*joint_matrices[ids.x] + weights.y*joint_matrices[ids.y]
                  + weights.z*joint_matrices[ids.z] + weights.w*joint_matrices[ids.w];
        p = vec4((skin*p).xyz,1.0); n = mat3(skin)*n;
    }
    vec4 world = model*p;
    gl_Position = projection*view*world;
    // Presenter uses Deko3D's default [0,w] depth range; the game uses [-w,w].
    gl_Position.z = (gl_Position.z + gl_Position.w)*0.5;
    vertex_colour = colour*tint;
    if (light_slot >= 0.0 && uint(light_slot+0.5) < flags.z)
        vertex_colour = light_colours[uint(light_slot+0.5)]*tint;
    if (abs(moby_light.w)>0.5) {
        int s0 = (int(moby_light.x+0.5)&15)*4;
        int s1 = (int(moby_light.y+0.5)&15)*4;
        float t = moby_light.z;
        vec4 ca=light_sets[s0], cb=light_sets[s0+2];
        vec3 da=light_sets[s0+1].xyz, db=light_sets[s0+3].xyz;
        if(t>0.0) {
            ca=mix(ca,light_sets[s1],t); cb=mix(cb,light_sets[s1+2],t);
            da=normalize(mix(da,light_sets[s1+1].xyz,t)); db=normalize(mix(db,light_sets[s1+3].xyz,t));
        }
        vec3 nw=mat3(model)*n;
        if(dot(nw,nw)>0.0) nw=normalize(nw);
        float fa=dot(-da,nw), fb=dot(-db,nw);
        fa=max(fa,-abs(ca.w)*fa); fb=max(fb,-abs(cb.w)*fb);
        vertex_colour=vec4(min((ambient.rgb+ca.rgb*fa+cb.rgb*fb)*colour.rgb,vec3(255.0/128.0)),colour.a)*tint;
    }
    texcoord=uv;
    if ((flags.w&256u)!=0u) {
        vec3 e=mat3(view)*mat3(model)*n;
        if(dot(e,e)>0.0) e=normalize(e);
        texcoord=vec2(e.x*0.5+0.5,0.5-e.y*0.5);
        float centre=-(view*model[3]).z;
        vertex_colour=vec4(vec3(112.0/128.0),clamp((24.0-centre)/16.0,0.0,1.0));
        gl_Position.z+=0.5e-4*gl_Position.w;
    }
    float depth=-(view*world).z*1024.0;
    fog=trunc(clamp(depth*fog_params.x+fog_params.y,fog_params.z,fog_params.w))/255.0;
}
