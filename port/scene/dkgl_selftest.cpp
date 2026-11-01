// SPDX-License-Identifier: GPL-3.0-or-later
// Off-screen conformance checks using the shipped game mesh shader and dkgl API.
#include "scene/dkgl_selftest.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string_view>
#include <vector>
#include "common/log.h"
#include "renderer/gl.h"
#include "renderer/shader.h"
#include "renderer/framebuffer.h"
#include "scene/level.h"
namespace openrac::scene {
using namespace openrac::gl;
namespace {
struct InstanceFixture {
    renderer::Mat4 model=renderer::identity();
    std::array<float,4> tint{1,1,1,1};
    float palette=-1,lights=-1;
    std::array<float,4> light{},ambient{};
};
static_assert(sizeof(InstanceFixture)==120);
}
void dkgl_selftest(renderer::Shader& mesh) {
    static bool ran=false;
    if(ran) return;
    ran=true;
    const auto* renderer_name=glGetString(GL_RENDERER);
    if(!renderer_name || std::string_view(reinterpret_cast<const char*>(renderer_name)).find("deko3d")==std::string_view::npos) return;
    const char* diagnostic=std::getenv("OPENRAC_DK_DIAGNOSTIC");
    if(diagnostic && *diagnostic && std::string_view(diagnostic)!="none") {
        log::warn("dkgl selftest skipped: remove OPENRAC_DK_DIAGNOSTIC to test normal shader execution");
        return;
    }
    log::info("dkgl selftest: begin shipped mesh shader GPU pixel checks");
    renderer::FrameBuffer target;
    std::string error;
    if(!target.create(16,16,error)) { log::error("dkgl selftest: target creation failed: {}",error); return; }
    unsigned vao=0,vbo=0,instances_buffer=0,ebo=0;
    std::array<unsigned,2> textures{};
    glGenVertexArrays(1,&vao); glBindVertexArray(vao);
    glGenBuffers(1,&vbo); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    std::array<Vertex,3> vertices{};
    const std::array<std::array<float,2>,3> xy{{{-.7f,-.7f},{.7f,-.7f},{0,.7f}}};
    for(unsigned i=0;i<3;++i) {
        auto& v=vertices[i]; v.position[0]=xy[i][0]+4; v.position[1]=xy[i][1];
        v.position[2]=.5f; v.normal[2]=1; v.uv[0]=v.uv[1]=.5f;
        for(auto& channel:v.colour) channel=1;
        v.light_slot=-1;
    }
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices.data(),GL_STREAM_DRAW);
    const auto attr=[&](unsigned id,int count,unsigned type,std::size_t offset,bool integer=false) {
        glEnableVertexAttribArray(id);
        if(integer) glVertexAttribIPointer(id,count,type,sizeof(Vertex),reinterpret_cast<const void*>(offset));
        else glVertexAttribPointer(id,count,type,GL_FALSE,sizeof(Vertex),reinterpret_cast<const void*>(offset));
    };
    attr(0,3,GL_FLOAT,offsetof(Vertex,position)); attr(1,3,GL_FLOAT,offsetof(Vertex,normal));
    attr(2,2,GL_FLOAT,offsetof(Vertex,uv)); attr(3,4,GL_FLOAT,offsetof(Vertex,colour));
    attr(10,4,GL_UNSIGNED_BYTE,offsetof(Vertex,joints),true);
    attr(11,4,GL_FLOAT,offsetof(Vertex,weights)); attr(12,1,GL_FLOAT,offsetof(Vertex,light_slot));
    glGenBuffers(1,&instances_buffer); glBindBuffer(GL_ARRAY_BUFFER,instances_buffer);
    InstanceFixture instance; instance.model[12]=-4; instance.tint={1,0,0,1};
    glBufferData(GL_ARRAY_BUFFER,sizeof(instance),&instance,GL_STREAM_DRAW);
    const auto inst=[&](unsigned id,int count,std::size_t offset) {
        glEnableVertexAttribArray(id);
        glVertexAttribPointer(id,count,GL_FLOAT,GL_FALSE,sizeof(instance),reinterpret_cast<const void*>(offset));
        glVertexAttribDivisor(id,1);
    };
    for(unsigned i=0;i<4;++i) inst(4+i,4,i*16);
    inst(8,4,offsetof(InstanceFixture,tint)); inst(9,1,offsetof(InstanceFixture,palette));
    inst(13,1,offsetof(InstanceFixture,lights)); inst(14,4,offsetof(InstanceFixture,light));
    inst(15,4,offsetof(InstanceFixture,ambient));
    glGenBuffers(1,&ebo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    const std::array<unsigned,6> indices{2,2,2,0,1,2};
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices.data(),GL_STATIC_DRAW);
    glGenTextures(2,textures.data());
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,textures[0]);
    const std::uint32_t white=0xffffffffu;
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,&white);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glActiveTexture(GL_TEXTURE0+1); glBindTexture(GL_TEXTURE_2D,textures[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glActiveTexture(GL_TEXTURE0+2); glBindTexture(GL_TEXTURE_2D,0);
    mesh.use();
    const auto identity=renderer::identity();
    glUniformMatrix4fv(mesh.uniform("view_projection"),1,GL_FALSE,identity.data());
    glUniformMatrix4fv(mesh.uniform("view"),1,GL_FALSE,identity.data());
    glUniform1f(mesh.uniform("picture_scale"),1);
    glUniform4f(mesh.uniform("material_colour"),1,1,1,1);
    glUniform4f(mesh.uniform("fog_colour"),0,0,0,0);
    glUniform4f(mesh.uniform("fog_params"),0,255,0,255);
    for(const char* name:{"metal_pass","probe_mode","probe_receiver","sun_shadow_enabled","hero_shadow_enabled",
                         "cutout","lighting","glow_pass","glow_split","translucent_pass","effect_pass","skinning"})
        glUniform1i(mesh.uniform(name),0);
    glUniform1i(mesh.uniform("tex_T0"),0); glUniform1i(mesh.uniform("palette_texture"),1); glUniform1i(mesh.uniform("light_texture"),2);
    glBindFramebuffer(GL_FRAMEBUFFER,target.id()); glViewport(0,0,16,16);
    glDisable(GL_BLEND); glDisable(GL_CULL_FACE); glDisable(GL_DEPTH_TEST); glDisable(GL_STENCIL_TEST); glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    unsigned passed=0,total=0;
    const auto check=[&](const char* name,bool red) {
        glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
        glDrawElementsInstanced(GL_TRIANGLES,3,GL_UNSIGNED_INT,reinterpret_cast<const void*>(3*sizeof(unsigned)),1);
        std::array<unsigned char,4> pixel{};
        glReadPixels(8,8,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel.data());
        const auto failure=glGetError();
        const bool ok=std::abs(int(pixel[0])-(red ? 255 : 0))<=2 && pixel[1]<=2 && pixel[2]<=2 && pixel[3]>=253 && failure==GL_NO_ERROR;
        ++total; if(ok) ++passed;
        log::info("dkgl selftest {}: {}; pixel {} {} {} {}, GL error {:#x}",ok ? "PASS" : "FAIL",name,
                  unsigned(pixel[0]),unsigned(pixel[1]),unsigned(pixel[2]),unsigned(pixel[3]),failure);
    };
    check("rigid matrix instance + index byte offset",true);
    instance.model=identity; instance.palette=0;
    glBindBuffer(GL_ARRAY_BUFFER,instances_buffer); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
    for(auto& v:vertices) { v.joints[0]=1; v.weights[0]=1; }
    glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices.data());
    std::array<renderer::Mat4,2> pose{identity,identity}; pose[0][12]=4; pose[1][12]=-4;
    glActiveTexture(GL_TEXTURE0+1); glBindTexture(GL_TEXTURE_2D,textures[1]);
    glTexImage2D(GL_TEXTURE_2D,0,0x8814,8,1,0,GL_RGBA,GL_FLOAT,pose.data());
    glUniform1i(mesh.uniform("skinning"),1);
    check("RGBA32F vertex fetch + integer joint index",true);
    pose[1][12]=4;
    glTexSubImage2D(GL_TEXTURE_2D,0,4,0,4,1,GL_RGBA,GL_FLOAT,pose[1].data());
    check("joint texture update moves triangle off screen",false);
    pose[1][12]=-4;
    glTexSubImage2D(GL_TEXTURE_2D,0,4,0,4,1,GL_RGBA,GL_FLOAT,pose[1].data());
    check("joint texture update restores triangle",true);
    std::vector<renderer::Mat4> rows(512); rows[256]=pose[1];
    for(auto& v:vertices) v.joints[0]=0;
    glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices.data());
    instance.palette=256;
    glBindBuffer(GL_ARRAY_BUFFER,instances_buffer); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
    glTexImage2D(GL_TEXTURE_2D,0,0x8814,1024,2,0,GL_RGBA,GL_FLOAT,rows.data());
    check("joint palette crosses 1024-texel row",true);
    // Unlike the checks above, queue both draws before reading back. A readback
    // between draws hides resource lifetime and texture read/write hazards.
    const auto draw=[&] {
        glDrawElementsInstanced(GL_TRIANGLES,3,GL_UNSIGNED_INT,reinterpret_cast<const void*>(3*sizeof(unsigned)),1);
    };
    const auto pixel_check=[&](const char* name,int x,int y,bool red) {
        std::array<unsigned char,4> pixel{};
        glReadPixels(x,y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel.data());
        const auto failure=glGetError();
        const bool ok=std::abs(int(pixel[0])-(red ? 255 : 0))<=2 && pixel[1]<=2 && pixel[2]<=2 && pixel[3]>=253 && failure==GL_NO_ERROR;
        ++total; if(ok) ++passed;
        log::info("dkgl selftest {}: {}; pixel {} {} {} {}, GL error {:#x}",ok ? "PASS" : "FAIL",name,
                  unsigned(pixel[0]),unsigned(pixel[1]),unsigned(pixel[2]),unsigned(pixel[3]),failure);
    };
    // Scissor separates the old and updated palette draws without a CPU wait.
    glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST); glScissor(0,0,8,16); draw();
    rows[256][12]=4;
    glTexSubImage2D(GL_TEXTURE_2D,0,0,1,4,1,GL_RGBA,GL_FLOAT,rows[256].data());
    glScissor(8,0,8,16); draw();
    pixel_check("queued palette draw retains old texture contents",7,8,true);
    pixel_check("queued palette draw observes updated texture contents",8,8,false);
    glDisable(GL_SCISSOR_TEST);
    // Queue a model-buffer change and a uniform change without waiting either.
    glUniform1i(mesh.uniform("skinning"),0);
    instance.model=identity; instance.model[12]=-4;
    glBindBuffer(GL_ARRAY_BUFFER,instances_buffer);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
    glClear(GL_COLOR_BUFFER_BIT); glEnable(GL_SCISSOR_TEST); glScissor(0,0,8,16); draw();
    instance.model[12]=4;
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
    glUniform1f(mesh.uniform("picture_scale"),.9f);
    glScissor(8,0,8,16); draw();
    pixel_check("queued draw retains old instance and uniforms",7,8,true);
    pixel_check("queued draw observes new instance and uniforms",8,8,false);
    glDisable(GL_SCISSOR_TEST);
    // Reverse-Z: a farther black triangle must fail against the nearer red one.
    glUniform1f(mesh.uniform("picture_scale"),1);
    instance.model[12]=-4; instance.model[14]=.25f;
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_GEQUAL); glDepthMask(GL_TRUE);
    glClearDepth(0); glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); draw();
    instance.model[14]=-.25f; instance.tint={0,0,0,1};
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance); draw();
    pixel_check("reversed depth rejects farther queued geometry",8,8,true);
    glDisable(GL_DEPTH_TEST);
    // Two simultaneous instances must step all four matrix columns and tint.
    std::array<InstanceFixture,2> pair{};
    for(unsigned i=0;i<2;++i) {
        pair[i].model[0]=.45f; pair[i].model[5]=.8f;
        pair[i].model[12]=-1.8f+(i ? .5f : -.5f);
        pair[i].tint={1,0,0,1};
    }
    glBufferData(GL_ARRAY_BUFFER,sizeof(pair),pair.data(),GL_STREAM_DRAW);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElementsInstanced(GL_TRIANGLES,3,GL_UNSIGNED_INT,reinterpret_cast<const void*>(3*sizeof(unsigned)),2);
    pixel_check("multiple instances: first matrix",4,8,true);
    pixel_check("multiple instances: second matrix",12,8,true);
    // The earlier fixtures deliberately disabled culling. Real models need
    // both face classifications to agree with GL after the shader Y flip.
    instance.model=identity; instance.model[12]=-4; instance.tint={1,0,0,1};
    glBufferData(GL_ARRAY_BUFFER,sizeof(instance),&instance,GL_STREAM_DRAW);
    glEnable(GL_CULL_FACE); glFrontFace(GL_CCW); glCullFace(GL_BACK);
    check("CCW triangle survives back-face culling",true);
    glCullFace(GL_FRONT);
    check("CCW triangle rejected by front-face culling",false);
    glFrontFace(GL_CW); glCullFace(GL_BACK);
    check("CW convention rejects CCW triangle",false);
    glCullFace(GL_FRONT);
    check("CW convention keeps CCW back face",true);
    glDisable(GL_CULL_FACE); glFrontFace(GL_CCW); glCullFace(GL_BACK);
    // Real skinned vertices blend four independent affine joint matrices,
    // unlike the single translated joint used by the original checks.
    std::array<renderer::Mat4,4> mixed_pose{};
    for(unsigned j=0;j<4;++j) {
        auto& matrix=mixed_pose[j]; matrix=identity;
        const float angle=(float(j)-1.5f)*.3f;
        const float c=std::cos(angle),s=std::sin(angle);
        matrix[0]=c; matrix[1]=s; matrix[4]=-s; matrix[5]=c;
        matrix[12]=-4*c; matrix[13]=-4*s;
    }
    for(auto& v:vertices) for(unsigned j=0;j<4;++j) {
        v.joints[j]=static_cast<unsigned char>(j); v.weights[j]=float(j+1)*.1f;
    }
    glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices.data());
    instance.model=identity; instance.palette=0;
    glBindBuffer(GL_ARRAY_BUFFER,instances_buffer); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
    glActiveTexture(GL_TEXTURE0+1); glBindTexture(GL_TEXTURE_2D,textures[1]);
    glTexImage2D(GL_TEXTURE_2D,0,0x8814,16,1,0,GL_RGBA,GL_FLOAT,mixed_pose.data());
    glUniform1i(mesh.uniform("skinning"),1);
    check("four weighted rotated joint matrices",true);
    // Copy-engine ordering at game sizes. Texture uploads and mip level 0 go through deko3d's copy
    // engine; the small fixtures above finish copying before the GPU reaches the next draw, so
    // they cannot show a missing copy-engine barrier. A 4 MiB texture keeps the copy busy.
    {
        glUniform1i(mesh.uniform("skinning"),0);
        instance.model=identity; instance.model[12]=-4; instance.palette=-1; instance.tint={1,1,1,1};
        glBindBuffer(GL_ARRAY_BUFFER,instances_buffer); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(instance),&instance);
        for(auto& v:vertices) { v.uv[0]=v.uv[1]=.5f; for(unsigned j=0;j<4;++j) { v.joints[j]=0; v.weights[j]=0; } }
        glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices.data());
        constexpr int kBig=1024;
        std::vector<std::uint32_t> red(std::size_t(kBig)*kBig,0xff0000ffu), green(std::size_t(kBig)*kBig,0xff00ff00u);
        // Blue clear: an undrawn pixel, a stale (or never written, zero) texel and the right
        // texel all read differently.
        const auto expect=[&](const char* name,int x,int y,unsigned r,unsigned gr,unsigned b) {
            std::array<unsigned char,4> pixel{};
            glReadPixels(x,y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,pixel.data());
            const auto failure=glGetError();
            const bool ok=std::abs(int(pixel[0])-int(r))<=2 && std::abs(int(pixel[1])-int(gr))<=2
                          && std::abs(int(pixel[2])-int(b))<=2 && failure==GL_NO_ERROR;
            ++total; if(ok) ++passed;
            log::info("dkgl selftest {}: {}; pixel {} {} {} {} (want {} {} {}), GL error {:#x}",ok ? "PASS" : "FAIL",name,
                      unsigned(pixel[0]),unsigned(pixel[1]),unsigned(pixel[2]),unsigned(pixel[3]),r,gr,b,failure);
        };
        unsigned big=0; glGenTextures(1,&big);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,big);
        glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glClearColor(0,0,1,1);
        // Read after write: the draw right after a large upload samples the new texels.
        glClear(GL_COLOR_BUFFER_BIT);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,kBig,kBig,0,GL_RGBA,GL_UNSIGNED_BYTE,red.data());
        draw();
        expect("draw observes a 4 MiB texture upload queued just before it",8,8,255,0,0);
        // Write after read: a queued draw keeps the texels it was recorded with.
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_SCISSOR_TEST); glScissor(0,0,8,16); draw();
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,kBig,kBig,GL_RGBA,GL_UNSIGNED_BYTE,green.data());
        glScissor(8,0,8,16); draw();
        glDisable(GL_SCISSOR_TEST);
        expect("queued draw keeps texels a later 4 MiB upload replaces",7,8,255,0,0);
        expect("draw after the 4 MiB replacement observes it",8,8,0,255,0);
        // A mip chain as TexturePool makes every 3D texture (glTexImage2D + glGenerateMipmap),
        // sampled at its smallest level: wide texture coordinates over a few pixels.
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,kBig,kBig,0,GL_RGBA,GL_UNSIGNED_BYTE,red.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,0x2700);  // GL_NEAREST_MIPMAP_NEAREST
        const std::array<std::array<float,2>,3> spread{{{0,0},{400,0},{0,400}}};
        for(unsigned i=0;i<3;++i) { vertices[i].uv[0]=spread[i][0]; vertices[i].uv[1]=spread[i][1]; }
        glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices.data());
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        expect("smallest level of a generated 1024x1024 mip chain",8,8,255,0,0);
        glClearColor(0,0,0,1);
        glDeleteTextures(1,&big);
    }
    log::info("dkgl selftest summary: {}/{} checks passed; see individual results above",passed,total);
    glBindFramebuffer(GL_FRAMEBUFFER,0); glBindVertexArray(0); glBindBuffer(GL_ARRAY_BUFFER,0);
    glDeleteBuffers(1,&vbo); glDeleteBuffers(1,&instances_buffer); glDeleteBuffers(1,&ebo);
    glDeleteVertexArrays(1,&vao); glDeleteTextures(2,textures.data());
    glActiveTexture(GL_TEXTURE0); glUseProgram(0);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_GEQUAL); glDepthMask(GL_TRUE);
}
}
