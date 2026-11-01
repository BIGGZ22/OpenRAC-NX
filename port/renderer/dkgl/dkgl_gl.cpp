// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// dkgl's GL functions (renderer/gl.h): GL's state machine, its objects, and each draw recorded
// into the frame's deko3d command list.

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "common/env_interval.h"
#include "common/log.h"
#include "renderer/dkgl/dkgl_internal.h"
#include "renderer/dkgl/vertex_bind_compat.h"
#include "renderer/dkgl/draw_state_compat.h"
#include "renderer/dkgl/transfer_validation.h"
#include "renderer/dkgl/geometry_validation.h"
#include "renderer/dkgl/sampler_refresh.h"

namespace openrac::dkgl::detail {

namespace {

// GL constants renderer/gl.h does not name.
constexpr GLenum kGlPoints = 0x0000, kGlLines = 0x0001, kGlLineLoop = 0x0002, kGlLineStrip = 0x0003,
                 kGlTriangleStrip = 0x0005, kGlTriangleFan = 0x0006;
constexpr GLenum kGlByte = 0x1400;
constexpr GLenum kGlRed = 0x1903, kGlR8 = 0x8229, kGlRg8 = 0x822B, kGlR32f = 0x822E, kGlRg16f = 0x822F;
constexpr GLenum kGlRgba32f = 0x8814, kGlRgba32ui = 0x8D70, kGlR32ui = 0x8236;
constexpr GLenum kGlInvalidEnum = 0x0500, kGlInvalidValue = 0x0501;
constexpr GLenum kGlColorBufferBit = 0x4000, kGlDepthBufferBit = 0x0100, kGlStencilBufferBit = 0x0400;
constexpr GLenum kGlIncr = 0x1E02, kGlDecr = 0x1E03, kGlReplace = 0x1E01, kGlInvert = 0x150A;
constexpr GLenum kGlMin = 0x8007, kGlMax = 0x8008;
constexpr GLenum kGlSrcAlphaSaturate = 0x0308;
constexpr GLenum kGlMaxTextureImageUnits = 0x8872, kGlMaxVertexAttribs = 0x8869;
constexpr GLenum kGlDepthClamp = 0x864F;
constexpr GLenum kGlPolygonOffsetFill = 0x8037;
constexpr GLenum kGlColor = 0x1800, kGlDepth = 0x1801, kGlStencil = 0x1802;

void fail(GLenum error) {
    if (g->state.error == GL_NO_ERROR) {
        g->state.error = error;
    }
}

// The caps glEnable switches, as places in State::caps.
int cap_index(GLenum cap) {
    switch (cap) {
        case GL_CULL_FACE: return 0;
        case GL_DEPTH_TEST: return 1;
        case GL_STENCIL_TEST: return 2;
        case GL_BLEND: return 3;
        case GL_SCISSOR_TEST: return 4;
        case GL_CLIP_DISTANCE0: return 5;
        case GL_CLIP_DISTANCE1: return 6;
        case GL_MULTISAMPLE: return 7;
        case GL_PRIMITIVE_RESTART: return 8;
        case kGlDepthClamp: return 9;
        case kGlPolygonOffsetFill: return 10;
        default: return -1;
    }
}

bool cap(GLenum c) {
    const int i = cap_index(c);
    return i >= 0 && g->state.caps[std::size_t(i)];
}

// ---- Formats ----

struct Format {
    DkImageFormat dk = DkImageFormat_None;
    int bytes = 0;       // a texel's bytes in the image
    int components = 0;  // GL components a texel has
};

Format format_of(GLenum internal) {
    switch (internal) {
        case GL_RGBA8: case GL_RGBA: case GL_RGB: return {DkImageFormat_RGBA8_Unorm, 4, 4};
        case kGlR8: case kGlRed: return {DkImageFormat_R8_Unorm, 1, 1};
        case kGlRg8: case GL_RG: return {DkImageFormat_RG8_Unorm, 2, 2};
        case kGlR32ui: return {DkImageFormat_R32_Uint, 4, 1};
        case kGlRgba32ui: return {DkImageFormat_RGBA32_Uint, 16, 4};
        case kGlRgba32f: return {DkImageFormat_RGBA32_Float, 16, 4};
        case GL_RGBA16F: return {DkImageFormat_RGBA16_Float, 8, 4};
        case GL_R11F_G11F_B10F: return {DkImageFormat_RG11B10_Float, 4, 3};
        case GL_RG32F: return {DkImageFormat_RG32_Float, 8, 2};
        case kGlR32f: return {DkImageFormat_R32_Float, 4, 1};
        case kGlRg16f: return {DkImageFormat_RG16_Float, 4, 2};
        case GL_DEPTH_COMPONENT: case GL_DEPTH_COMPONENT24: return {DkImageFormat_Z24X8, 4, 1};
        case GL_DEPTH_COMPONENT32F: return {DkImageFormat_ZF32, 4, 1};
        case GL_DEPTH24_STENCIL8: case GL_DEPTH_STENCIL: return {DkImageFormat_Z24S8, 4, 2};
        default: return {};
    }
}

// The bytes a pixel of client data has, for (format, type).
int client_bytes(GLenum format, GLenum type) {
    int components = 4;
    switch (format) {
        case kGlRed: case GL_DEPTH_COMPONENT: components = 1; break;
        case GL_RG: components = 2; break;
        case GL_RGB: components = 3; break;
        default: components = 4; break;
    }
    switch (type) {
        case GL_UNSIGNED_BYTE: case kGlByte: return components;
        case GL_HALF_FLOAT: case GL_SHORT: case GL_UNSIGNED_SHORT: return components * 2;
        case GL_UNSIGNED_INT_24_8: case GL_UNSIGNED_INT_10F_11F_11F_REV: return 4;
        default: return components * 4;
    }
}

// ---- Objects in use ----

Texture* bound_texture() {
    return g->textures.find(g->state.textures[g->state.active_unit]);
}

GLuint* buffer_binding(GLenum target) {
    switch (target) {
        case GL_ARRAY_BUFFER: return &g->state.array_buffer;
        case GL_ELEMENT_ARRAY_BUFFER: {
            static GLuint none = 0;
            VertexArray* va = g->vertex_arrays.find(g->state.vertex_array);
            return va != nullptr ? &va->elements : &none;
        }
        default: {
            static GLuint other = 0;  // uniform and storage buffers: kept, not used by the game
            return &other;
        }
    }
}

VertexArray& vertex_array() {
    static VertexArray none;
    VertexArray* va = g->vertex_arrays.find(g->state.vertex_array);
    return va != nullptr ? *va : none;
}

Image* attachment_image(const Attachment& a) {
    if (a.texture != 0) {
        Texture* t = g->textures.find(a.texture);
        return t != nullptr ? t->image.get() : nullptr;
    }
    if (a.renderbuffer != 0) {
        Renderbuffer* r = g->renderbuffers.find(a.renderbuffer);
        return r != nullptr ? r->image.get() : nullptr;
    }
    return nullptr;
}

}  // namespace

Target current_target(GLuint framebuffer) {
    Target t;
    if (framebuffer == 0) {
        t.colour = g->default_colour.get();
        t.depth = g->default_depth.get();
    } else if (Framebuffer* f = g->framebuffers.find(framebuffer)) {
        t.colour = attachment_image(f->colour);
        t.depth = attachment_image(f->depth);
    }
    const Image* sized = t.colour != nullptr ? t.colour : t.depth;
    if (sized != nullptr) {
        t.width = sized->width;
        t.height = sized->height;
    }
    return t;
}

void reset_bound_state() {
    g->vertex_setup.valid=false;
    g->target_bound = false;
    g->bound_target = {};
    g->sent = {};
}

// OPENRAC_DK_STATE_CACHE=0: every draw sends all its state again, as before.
bool state_cache() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_DK_STATE_CACHE");
        const bool enabled = v == nullptr || std::string_view(v) != "0";
        log::info("dkgl: repeated draw state {} (OPENRAC_DK_STATE_CACHE={})", enabled ? "skipped" : "sent every draw", enabled ? 1 : 0);
        return enabled;
    }();
    return on;
}

namespace {

// Binds the draw framebuffer's images as the render targets, when they are not already.
bool bind_target(Target& t) {
    t = current_target(g->state.draw_framebuffer);
    if (t.colour == nullptr && t.depth == nullptr) {
        return false;
    }
    if (g->target_bound && g->bound_target == t) {
        return true;
    }
    DkCmdBuf c = cmd();
    // What the last targets wrote is done before anything reads it.
    static const bool lazy_targets=[] {
        const auto* v=std::getenv("OPENRAC_DK_LAZY_TARGET_BARRIERS");
        return v && std::string_view(v)=="1";
    }();
    // Read after write: the old targets were drawn to since the last barrier that waited and
    // invalidated the texture cache (ordered_epoch, not barrier_epoch: see dkgl_internal.h).
    const auto unwaited_write=[](const Image* image) {
        return image && image->draw_write_epoch==g->ordered_epoch;
    };
    // Write after read: the new targets were sampled since the last barrier that waited. Draws
    // that sample also stamp the target they write, so unwaited_write covers this today; the
    // check keeps the skip correct if a draw ever samples without writing.
    const auto unwaited_read=[](const Image* image) {
        return image && image->sampled_epoch==g->barrier_epoch;
    };
    if (g->target_bound && (!lazy_targets || g->need_image_barrier
        || unwaited_write(g->bound_target.colour) || unwaited_write(g->bound_target.depth)
        || unwaited_read(t.colour) || unwaited_read(t.depth))) {
        barrier(DkBarrier_Fragments, DkInvalidateFlags_Image);
        g->need_image_barrier = false;
        c = cmd();  // a long list may have been sent in parts
    } else if(g->target_bound) {
        ++g->counters.target_barriers_skipped;
    }
    DkImageView colour, depth;
    const DkImageView* colours[1] = {&colour};
    if (t.colour != nullptr) {
        dkImageViewDefaults(&colour, &t.colour->image);
    }
    if (t.depth != nullptr) {
        dkImageViewDefaults(&depth, &t.depth->image);
    }
    dkCmdBufBindRenderTargets(c, colours, t.colour != nullptr ? 1 : 0, t.depth != nullptr ? &depth : nullptr);
    g->sent = {};
    g->bound_target = t;
    g->target_bound = true;
    ++g->counters.target_changes;
    if (t.colour != nullptr) {
        t.colour->written_frame = g->frame;
    }
    if (t.depth != nullptr) {
        t.depth->written_frame = g->frame;
    }
    return true;
}

// The viewport and scissor, clipped to the target (deko3d's scissor must lie inside it).
void record_viewport_scissor(DkCmdBuf c, const Target& t) {
    const State& s = g->state;
    int x = 0, y = 0, w = t.width, h = t.height;
    if (cap(GL_SCISSOR_TEST)) {
        x = std::clamp(s.scissor[0], 0, t.width);
        y = std::clamp(s.scissor[1], 0, t.height);
        w = std::clamp(s.scissor[0] + s.scissor[2], 0, t.width) - x;
        h = std::clamp(s.scissor[1] + s.scissor[3], 0, t.height) - y;
    }
    auto& sent = g->sent;
    const int scissor[4] = {x, y, w, h};
    if (state_cache() && sent.view_valid && std::memcmp(sent.viewport, s.viewport, sizeof sent.viewport) == 0
        && std::memcmp(sent.scissor, scissor, sizeof scissor) == 0) {
        return;
    }
    const DkViewport vp{float(s.viewport[0]), float(s.viewport[1]), float(s.viewport[2]),
                        float(s.viewport[3]), 0.0f, 1.0f};
    dkCmdBufSetViewports(c, 0, &vp, 1);
    const DkScissor sc{std::uint32_t(x), std::uint32_t(y), std::uint32_t(std::max(w, 0)),
                       std::uint32_t(std::max(h, 0))};
    dkCmdBufSetScissors(c, 0, &sc, 1);
    std::memcpy(sent.viewport, s.viewport, sizeof sent.viewport);
    std::memcpy(sent.scissor, scissor, sizeof scissor);
    sent.view_valid = true;
}

DkCompareOp compare_op(GLenum f) {
    switch (f) {
        case GL_NEVER: return DkCompareOp_Never;
        case GL_LESS: return DkCompareOp_Less;
        case GL_EQUAL: return DkCompareOp_Equal;
        case GL_LEQUAL: return DkCompareOp_Lequal;
        case GL_GREATER: return DkCompareOp_Greater;
        case GL_NOTEQUAL: return DkCompareOp_NotEqual;
        case GL_GEQUAL: return DkCompareOp_Gequal;
        default: return DkCompareOp_Always;
    }
}

DkStencilOp stencil_op(GLenum op) {
    switch (op) {
        case GL_ZERO: return DkStencilOp_Zero;
        case kGlReplace: return DkStencilOp_Replace;
        case kGlIncr: return DkStencilOp_Incr;
        case kGlDecr: return DkStencilOp_Decr;
        case kGlInvert: return DkStencilOp_Invert;
        case GL_INCR_WRAP: return DkStencilOp_IncrWrap;
        case GL_DECR_WRAP: return DkStencilOp_DecrWrap;
        default: return DkStencilOp_Keep;
    }
}

DkBlendFactor blend_factor(GLenum f) {
    switch (f) {
        case GL_ZERO: return DkBlendFactor_Zero;
        case GL_ONE: return DkBlendFactor_One;
        case GL_SRC_COLOR: return DkBlendFactor_SrcColor;
        case GL_ONE_MINUS_SRC_COLOR: return DkBlendFactor_InvSrcColor;
        case GL_SRC_ALPHA: return DkBlendFactor_SrcAlpha;
        case GL_ONE_MINUS_SRC_ALPHA: return DkBlendFactor_InvSrcAlpha;
        case GL_DST_ALPHA: return DkBlendFactor_DstAlpha;
        case GL_ONE_MINUS_DST_ALPHA: return DkBlendFactor_InvDstAlpha;
        case GL_DST_COLOR: return DkBlendFactor_DstColor;
        case GL_ONE_MINUS_DST_COLOR: return DkBlendFactor_InvDstColor;
        case GL_CONSTANT_COLOR: return DkBlendFactor_ConstColor;
        case GL_ONE_MINUS_CONSTANT_COLOR: return DkBlendFactor_InvConstColor;
        case GL_CONSTANT_ALPHA: return DkBlendFactor_ConstAlpha;
        case GL_ONE_MINUS_CONSTANT_ALPHA: return DkBlendFactor_InvConstAlpha;
        case kGlSrcAlphaSaturate: return DkBlendFactor_SrcAlphaSaturate;
        default: return DkBlendFactor_One;
    }
}

DkBlendOp blend_op(GLenum e) {
    switch (e) {
        case GL_FUNC_SUBTRACT: return DkBlendOp_Sub;
        case GL_FUNC_REVERSE_SUBTRACT: return DkBlendOp_RevSub;
        case kGlMin: return DkBlendOp_Min;
        case kGlMax: return DkBlendOp_Max;
        default: return DkBlendOp_Add;
    }
}

// The fixed-function state of a draw, recorded each draw (a few words each; deko3d has no pipelines).
bool record_fixed_state(DkCmdBuf c, const Target& t) {
    const State& s = g->state;
    // Everything below depends only on these: the same as the last draw's, nothing is sent.
    std::array<std::uint8_t, 160> key{};
    {
        std::size_t at = 0;
        const auto put = [&](const void* p, std::size_t n) { std::memcpy(key.data() + at, p, n); at += n; };
        const std::uint32_t caps = (cap(GL_CULL_FACE) ? 1u : 0u) | (cap(kGlDepthClamp) ? 2u : 0u) | (cap(GL_BLEND) ? 4u : 0u)
                                   | (cap(GL_DEPTH_TEST) ? 8u : 0u) | (cap(GL_STENCIL_TEST) ? 16u : 0u)
                                   | (cap(GL_CLIP_DISTANCE0) ? 32u : 0u) | (cap(GL_CLIP_DISTANCE1) ? 64u : 0u)
                                   | (t.depth != nullptr ? 128u : 0u) | (t.depth != nullptr && t.depth->stencil ? 256u : 0u)
                                   | (t.colour != nullptr ? 512u : 0u);
        put(&caps, 4);
        put(&s.cull_face, 4); put(&s.front_face, 4); put(&s.polygon_mode, 4);
        put(s.colour_mask, sizeof s.colour_mask);
        put(&s.blend_equation, 4); put(&s.blend_src_rgb, 4); put(&s.blend_dst_rgb, 4); put(&s.blend_src_a, 4); put(&s.blend_dst_a, 4);
        put(s.blend_colour, sizeof s.blend_colour);
        put(&s.depth_func, 4); put(&s.depth_mask, sizeof s.depth_mask);
        put(&s.stencil_func, 4); put(&s.stencil_ref, 4); put(&s.stencil_value_mask, 4); put(&s.stencil_write_mask, 4);
        put(s.stencil_ops, sizeof s.stencil_ops);
    }
    if (state_cache() && g->sent.fixed_valid && key == g->sent.fixed) {
        return true;
    }
    DkRasterizerState r;
    dkRasterizerStateDefaults(&r);
    r.cullMode = !cap(GL_CULL_FACE)        ? DkFace_None
                 : s.cull_face == GL_FRONT ? DkFace_Front
                 : s.cull_face == GL_FRONT_AND_BACK ? DkFace_FrontAndBack
                                            : DkFace_Back;
    r.frontFace = packed_front_face(s.front_face);
    r.polygonModeFront = r.polygonModeBack = s.polygon_mode == GL_LINE ? DkPolygonMode_Line : DkPolygonMode_Fill;
    r.depthClampEnable = cap(kGlDepthClamp) ? 1 : 0;
    // GL's provoking vertex is the last.
    r.provokingVertex = DkProvokingVertex_Last;
    dkCmdBufBindRasterizerState(c, &r);

    DkColorState cs;
    dkColorStateDefaults(&cs);
    dkColorStateSetBlendEnable(&cs, 0, cap(GL_BLEND));
    dkCmdBufBindColorState(c, &cs);

    DkColorWriteState cw;
    dkColorWriteStateDefaults(&cw);
    const std::uint32_t mask = (s.colour_mask[0] ? DkColorMask_R : 0) | (s.colour_mask[1] ? DkColorMask_G : 0)
                               | (s.colour_mask[2] ? DkColorMask_B : 0) | (s.colour_mask[3] ? DkColorMask_A : 0);
    dkColorWriteStateSetMask(&cw, 0, t.colour != nullptr ? mask : 0);
    dkCmdBufBindColorWriteState(c, &cw);

    if (cap(GL_BLEND)) {
        DkBlendState b;
        dkBlendStateDefaults(&b);
        dkBlendStateSetOps(&b, blend_op(s.blend_equation), blend_op(s.blend_equation));
        dkBlendStateSetFactors(&b, blend_factor(s.blend_src_rgb), blend_factor(s.blend_dst_rgb),
                               blend_factor(s.blend_src_a), blend_factor(s.blend_dst_a));
        const auto key=blend_state_key(b);
        auto at=g->blend_commands.find(key);
        if(at==g->blend_commands.end()) {
            std::array<std::uint32_t,7> encoded{};
            if(!encode_blend_state(g->vertex_capture,b,encoded)) {
                g->error="deko3d: unsupported blend command encoding";
                log::error("{}",g->error);
                return false;
            }
            at=g->blend_commands.emplace(key,encoded).first;
        }
        dkCmdBufReplayCmds(c,at->second.data(),unsigned(at->second.size()));
        dkCmdBufSetBlendConst(c, s.blend_colour[0], s.blend_colour[1], s.blend_colour[2], s.blend_colour[3]);
    }

    // Clip distances as GL applies them: only the enabled ones (GL_CLIP_DISTANCE0/1, which the
    // level renderer enables just for the 2D layer's mobys on a wide target). deko3d 0.5 enables
    // all eight once at start-up (gpu_3d_base.cpp, ClipDistanceEnable 0xFF) and has no call for
    // it, so the register (method 0x544, a bit per distance) is written inline here.
    // OPENRAC_DK_CLIP_ALL=1 leaves all eight on, as before.
    static const bool clip_all = [] {
        const char* v = std::getenv("OPENRAC_DK_CLIP_ALL");
        return v != nullptr && std::string_view(v) == "1";
    }();
    if (!clip_all) {
        const std::uint32_t mask = (cap(GL_CLIP_DISTANCE0) ? 1u : 0u) | (cap(GL_CLIP_DISTANCE1) ? 2u : 0u);
        // IMMD (inline) header: mode 4 << 29, value << 16, subchannel 3D (0) << 13, method 0x544.
        const std::uint32_t word = (4u << 29) | (mask << 16) | 0x544u;
        dkCmdBufReplayCmds(c, &word, 1);
    }

    DkDepthStencilState ds;
    dkDepthStencilStateDefaults(&ds);
    const bool depth = t.depth != nullptr;
    ds.depthTestEnable = depth && cap(GL_DEPTH_TEST);
    // GL writes depth only while the test is on.
    ds.depthWriteEnable = depth && cap(GL_DEPTH_TEST) && s.depth_mask;
    ds.depthCompareOp = compare_op(s.depth_func);
    const bool stencil = t.depth != nullptr && t.depth->stencil && cap(GL_STENCIL_TEST);
    ds.stencilTestEnable = stencil;
    ds.stencilFrontFailOp = stencil_op(s.stencil_ops[0][0]);
    ds.stencilFrontDepthFailOp = stencil_op(s.stencil_ops[0][1]);
    ds.stencilFrontPassOp = stencil_op(s.stencil_ops[0][2]);
    ds.stencilFrontCompareOp = compare_op(s.stencil_func);
    ds.stencilBackFailOp = stencil_op(s.stencil_ops[1][0]);
    ds.stencilBackDepthFailOp = stencil_op(s.stencil_ops[1][1]);
    ds.stencilBackPassOp = stencil_op(s.stencil_ops[1][2]);
    ds.stencilBackCompareOp = compare_op(s.stencil_func);
    dkCmdBufBindDepthStencilState(c, &ds);
    if (stencil) {
        dkCmdBufSetStencil(c, DkFace_FrontAndBack, std::uint8_t(s.stencil_write_mask),
                           std::uint8_t(s.stencil_ref), std::uint8_t(s.stencil_value_mask));
    }
    g->sent.fixed = key;
    g->sent.fixed_valid = true;
    return true;
}

// ---- Buffers on the GPU ----

// Where a draw reads `b`: its own memory once it stopped changing, else this frame's ring.
DkGpuAddr resolve(Buffer& b) {
    if (b.data.empty()) {
        return 0;
    }
    if (b.block && b.block_version == b.version) {
        return b.block.gpu();
    }
    if (b.ring_epoch == g->ring_epoch && b.ring_version == b.version) {
        return b.ring_addr;
    }
    const auto size = std::uint32_t(b.data.size());
    // Unchanged since the frame before, or written once: memory of its own (the level's
    // geometry). Rewritten every frame: the ring.
    const bool settled = b.changes <= 1 || g->frame > b.changed_frame + 1;
    if (settled) {
        retire(b.block);
        b.block = allocate(g->buffer_heap, size, 256);
        if (b.block) {
            std::memcpy(b.block.cpu(), b.data.data(), size);
            b.block_version = b.version;
            return b.block.gpu();
        }
    }
    const RingSlice s = ring(size, 256);
    if (s.cpu == nullptr) {
        return 0;
    }
    std::memcpy(s.cpu, b.data.data(), size);
    b.ring_epoch = g->ring_epoch;
    b.ring_version = b.version;
    b.ring_addr = s.gpu;
    return s.gpu;
}

void changed(Buffer& b) {
    ++b.version;
    if (b.changed_frame != g->frame) {
        ++b.changes;
    }
    b.changed_frame = g->frame;
}

// ---- Vertex formats ----

bool attribute_format(const Attribute& a, DkVtxAttribSize& size, DkVtxAttribType& type) {
    const int n = std::clamp(a.size, 1, 4);
    switch (a.type) {
        case GL_FLOAT: {
            static constexpr DkVtxAttribSize s[] = {DkVtxAttribSize_1x32, DkVtxAttribSize_2x32, DkVtxAttribSize_3x32, DkVtxAttribSize_4x32};
            size = s[n - 1];
            type = DkVtxAttribType_Float;
            return true;
        }
        case GL_HALF_FLOAT: {
            static constexpr DkVtxAttribSize s[] = {DkVtxAttribSize_1x16, DkVtxAttribSize_2x16, DkVtxAttribSize_3x16, DkVtxAttribSize_4x16};
            size = s[n - 1];
            type = DkVtxAttribType_Float;
            return true;
        }
        case GL_INT:
        case GL_UNSIGNED_INT: {
            static constexpr DkVtxAttribSize s[] = {DkVtxAttribSize_1x32, DkVtxAttribSize_2x32, DkVtxAttribSize_3x32, DkVtxAttribSize_4x32};
            size = s[n - 1];
            const bool sign = a.type == GL_INT;
            type = a.integer ? (sign ? DkVtxAttribType_Sint : DkVtxAttribType_Uint)
                   : a.normalized ? (sign ? DkVtxAttribType_Snorm : DkVtxAttribType_Unorm)
                                  : (sign ? DkVtxAttribType_Sscaled : DkVtxAttribType_Uscaled);
            return true;
        }
        case GL_SHORT:
        case GL_UNSIGNED_SHORT: {
            static constexpr DkVtxAttribSize s[] = {DkVtxAttribSize_1x16, DkVtxAttribSize_2x16, DkVtxAttribSize_3x16, DkVtxAttribSize_4x16};
            size = s[n - 1];
            const bool sign = a.type == GL_SHORT;
            type = a.integer ? (sign ? DkVtxAttribType_Sint : DkVtxAttribType_Uint)
                   : a.normalized ? (sign ? DkVtxAttribType_Snorm : DkVtxAttribType_Unorm)
                                  : (sign ? DkVtxAttribType_Sscaled : DkVtxAttribType_Uscaled);
            return true;
        }
        case GL_UNSIGNED_BYTE:
        case kGlByte: {
            static constexpr DkVtxAttribSize s[] = {DkVtxAttribSize_1x8, DkVtxAttribSize_2x8, DkVtxAttribSize_3x8, DkVtxAttribSize_4x8};
            size = s[n - 1];
            const bool sign = a.type == kGlByte;
            type = a.integer ? (sign ? DkVtxAttribType_Sint : DkVtxAttribType_Uint)
                   : a.normalized ? (sign ? DkVtxAttribType_Snorm : DkVtxAttribType_Unorm)
                                  : (sign ? DkVtxAttribType_Sscaled : DkVtxAttribType_Uscaled);
            return true;
        }
        default:
            return false;
    }
}

int type_bytes(GLenum type) {
    switch (type) {
        case GL_UNSIGNED_BYTE: case kGlByte: return 1;
        case GL_SHORT: case GL_UNSIGNED_SHORT: case GL_HALF_FLOAT: return 2;
        default: return 4;
    }
}

// GL's integer and float input kinds, from a vertex input's GL type.
bool integer_input(std::uint32_t gl_type) {
    switch (gl_type) {
        case 0x1404: case 0x8B53: case 0x8B54: case 0x8B55:  // int, ivec2..4
        case 0x1405: case 0x8DC6: case 0x8DC7: case 0x8DC8:  // uint, uvec2..4
            return true;
        default:
            return false;
    }
}

DkPrimitive primitive(GLenum mode) {
    switch (mode) {
        case kGlPoints: return DkPrimitive_Points;
        case kGlLines: return DkPrimitive_Lines;
        case kGlLineLoop: return DkPrimitive_LineLoop;
        case kGlLineStrip: return DkPrimitive_LineStrip;
        case kGlTriangleStrip: return DkPrimitive_TriangleStrip;
        case kGlTriangleFan: return DkPrimitive_TriangleFan;
        default: return DkPrimitive_Triangles;
    }
}

// ---- The draw ----

Program* current_program() {
    Program* p = g->programs.find(g->state.program);
    return p != nullptr && p->linked ? p : nullptr;
}

// The texture handle a sampler uniform's unit gives.
DkResHandle unit_handle(unsigned unit, const Target& t) {
    Texture* tex = unit < kUnits ? g->textures.find(g->state.textures[unit]) : nullptr;
    Image* image = tex != nullptr ? tex->image.get() : nullptr;
    // A texture that is also the draw's target. GL leaves the read undefined, but Mesa (the
    // Switch's GL, which draws these right) reads what the target holds, and the GS emulation's
    // panels rely on it (renderer/direct.cpp: an offscreen square read while drawing into it).
    // The target is sampled after a fragment barrier, so earlier draws' pixels are there.
    // OPENRAC_DK_FEEDBACK=dummy: an opaque black texel instead, as before.
    static const bool dummy_feedback = [] {
        const char* v = std::getenv("OPENRAC_DK_FEEDBACK");
        return v != nullptr && std::string_view(v) == "dummy";
    }();
    if (image != nullptr && (image == t.colour || image == t.depth)) {
        static unsigned told = 0;
        if (told++ < 8) {
            log::info("dkgl: a draw samples its own {} target ({}x{}){}", image == t.colour ? "colour" : "depth",
                      image->width, image->height, dummy_feedback ? ": dummy texel (OPENRAC_DK_FEEDBACK=dummy)" : ": read after a fragment barrier");
        }
        if (dummy_feedback || image == t.depth) {
            image = nullptr;
        } else {
            g->feedback = true;
        }
    }
    if (image == nullptr) {
        image = g->dummy.get();
    }
    SamplerParams params = tex != nullptr ? tex->params : SamplerParams{};
    if (unit < kUnits && g->state.samplers[unit] != 0) {
        if (SamplerParams* s = g->samplers.find(g->state.samplers[unit])) {
            params = *s;
        }
    }
    static const bool cache_samplers=[] {
        const auto* v=std::getenv("OPENRAC_DK_SAMPLER_CACHE");
        return !v || std::string_view(v)!="0";
    }();
    std::uint32_t sampler;
    // Cache only descriptor lookup: feedback, resource touches and read epochs
    // above/below still run on every draw. Descriptor entries live for the context.
    auto* cached=unit<kUnits && cache_samplers && state_cache() ? &g->unit_sampler_cache[unit] : nullptr;
    if(cached && cached->valid && cached->params==params && cached->levels==image->levels) {
        sampler=cached->descriptor;
        ++g->counters.sampler_lookup_hits;
    } else {
        sampler=sampler_descriptor(params,image->levels);
        if(cached) *cached={true,params,image->levels,sampler};
        ++g->counters.sampler_lookup_misses;
    }
    image->sampled_epoch = g->barrier_epoch;
    touch(image);
    return dkMakeTextureHandle(image->descriptor, sampler);
}

bool prepare_draw(Program*& p, Target& t, DkCmdBuf& c) {
    p = current_program();
    if (p == nullptr) {
        return false;
    }
    if(omit_program(g->diagnostic,p->role)) {
        ++g->counters.diagnostic_omissions;
        return false;
    }
    // Before the target is bound: the barrier may send a long list to the GPU in parts.
    // A copy-engine write (texture upload, mip level 0, framebuffer copy) is ordered by a Full
    // barrier only; a 3D or 2D write by the image barrier.
    transfer_fence();
    if (g->need_image_barrier) {
        image_barrier();
    }
    if (!bind_target(t)) {
        return false;
    }
    touch(t.colour);
    touch(t.depth);
    c = cmd();
    // Shaders and the default block, for both stages.
    auto& sent = g->sent;
    const bool cache = state_cache();
    if (!cache || sent.program != p) {
        const DkShader* shaders[2] = {&p->vertex, &p->fragment};
        dkCmdBufBindShaders(c, DkStageFlag_GraphicsMask, shaders, 2);
        // Another program: its uniforms and textures are bound again too.
        const bool view = sent.view_valid, fixed = sent.fixed_valid;
        const auto fixed_key = sent.fixed;
        int vp[4], sc[4];
        std::memcpy(vp, sent.viewport, sizeof vp);
        std::memcpy(sc, sent.scissor, sizeof sc);
        const auto vertex_count = sent.vertex_count;
        const auto attribs = sent.attribs; const auto states = sent.states; const auto vbufs = sent.buffers;
        const auto index = sent.index; const auto index_bytes = sent.index_bytes; const auto restart = sent.restart;
        sent = {};
        // Viewport, scissor, fixed-function state, vertex inputs and index buffer are not the
        // program's: they stay as sent.
        sent.view_valid = view; std::memcpy(sent.viewport, vp, sizeof vp); std::memcpy(sent.scissor, sc, sizeof sc);
        sent.fixed_valid = fixed; sent.fixed = fixed_key;
        sent.vertex_count = vertex_count; sent.attribs = attribs; sent.states = states; sent.buffers = vbufs;
        sent.index = index; sent.index_bytes = index_bytes; sent.restart = restart;
        sent.program = p;
    }
    if (p->reflection.default_binding >= 0 && !p->uniforms.empty()) {
        const bool cpu_skinned = g->draw_override.active;
        if (p->uniforms_dirty || p->uploaded_epoch != g->ring_epoch || cpu_skinned) {
            const RingSlice s = ring(std::uint32_t(p->uniforms.size()), DK_UNIFORM_BUF_ALIGNMENT);
            if (s.cpu == nullptr) {
                return false;
            }
            std::memcpy(s.cpu, p->uniforms.data(), p->uniforms.size());
            if(g->diagnostic==Diagnostic::BindPose && p->role==ProgramRole::SkinnedMesh) {
                if(!override_skinning({static_cast<std::uint8_t*>(s.cpu),p->uniforms.size()},p->skinning_offset)) {
                    g->error="deko3d: bind-pose diagnostic could not locate skinning uniform";
                    log::error("{}",g->error);
                    return false;
                }
            }
            // Skinned on the CPU already: the shader takes its vertices as they are.
            if (cpu_skinned && !override_skinning({static_cast<std::uint8_t*>(s.cpu), p->uniforms.size()}, p->skinning_offset)) {
                return false;
            }
            p->uploaded_addr = s.gpu;
            p->uploaded_epoch = g->ring_epoch;
            p->uniforms_dirty = cpu_skinned;  // the next draw sends the real snapshot again
            ++g->counters.uniform_uploads;
        }
        const DkBufExtents ubo{p->uploaded_addr, std::uint32_t(align_up(p->uniforms.size(), 16))};
        const auto binding = std::uint32_t(p->reflection.default_binding);
        if (!cache || sent.ubo != ubo.addr || sent.ubo_size != ubo.size) {
            dkCmdBufBindUniformBuffers(c, DkStage_Vertex, binding, &ubo, 1);
            dkCmdBufBindUniformBuffers(c, DkStage_Fragment, binding, &ubo, 1);
            sent.ubo = ubo.addr;
            sent.ubo_size = ubo.size;
        }
    }
    // Textures: each sampler uniform at its binding, in both stages (the mesh's vertex stage samples).
    const auto& list = p->reflection.uniforms;
    // Reflection is immutable between links. Retain only sampler indices rather
    // than scan the entire loose-uniform list for every mesh/material draw.
    if(p->sampler_layout_revision!=p->link_revision || p->sampler_layout_size!=list.size()) {
        p->sampler_uniforms.clear();
        for(unsigned i=0;i<list.size();++i) {
            if(list[i].sampler && list[i].binding>=0 && list[i].binding<int(DK_NUM_TEXTURE_BINDINGS))
                p->sampler_uniforms.push_back(i);
        }
        p->sampler_layout_revision=p->link_revision;
        p->sampler_layout_size=list.size();
        ++g->counters.sampler_layout_rebuilds;
    }
    static const SamplerRebind sampler_rebind = [] {
        const SamplerRebind mode = parse_sampler_rebind(std::getenv("OPENRAC_DK_SAMPLER_REBIND"));
        log::info("dkgl: sampler set bound again before a sampler not read since its last bind: {}",
                  mode == SamplerRebind::On ? "on, batched per draw (OPENRAC_DK_SAMPLER_REBIND=0 turns it off)"
                  : mode == SamplerRebind::First ? "EXPERIMENT first: only a sampler's first read in each command list (OPENRAC_DK_SAMPLER_REBIND=first)"
                  : mode == SamplerRebind::New ? "EXPERIMENT new: only a sampler written since the set was last bound (OPENRAC_DK_SAMPLER_REBIND=new)"
                  : "off (OPENRAC_DK_SAMPLER_REBIND=0)");
        return mode;
    }();
    std::array<DkResHandle, DK_NUM_TEXTURE_BINDINGS> draw_handles{};
    std::array<unsigned, DK_NUM_TEXTURE_BINDINGS> draw_bindings{};
    std::array<bool, DK_NUM_TEXTURE_BINDINGS> draw_bound{};
    unsigned draw_samplers=0;
    for (const unsigned i : p->sampler_uniforms) {
        const Uniform& u = list[i];
        const int unit = i < p->units.size() ? p->units[i] : 0;
        const DkResHandle handle = unit_handle(unsigned(std::max(unit, 0)), t);
        const auto binding=unsigned(u.binding);
        if(!draw_bound[binding]) {
            draw_bound[binding]=true;
            draw_bindings[draw_samplers++]=binding;
        }
        draw_handles[binding]=handle;
    }
    // Resolve and invalidate newly created descriptors before rebinding once.
    c=cmd();
    std::array<std::uint32_t,DK_NUM_TEXTURE_BINDINGS> handles{};
    for(unsigned i=0;i<draw_samplers;++i) handles[i]=draw_handles[draw_bindings[i]];
    const std::span<const std::uint32_t> read_now{handles.data(),draw_samplers};
    const bool refresh = sampler_rebind == SamplerRebind::On ? refresh_draw_samplers(g->sampler_read,read_now)
                         : sampler_rebind == SamplerRebind::First ? refresh_first_samplers(g->sampler_read,read_now)
                         : sampler_rebind == SamplerRebind::New ? refresh_new_samplers(g->samplers_at_bind,read_now)
                         : false;
    if(refresh) {
        g->samplers_at_bind = std::uint32_t(g->sampler_cache.size());
        dkCmdBufBindSamplerDescriptorSet(c,
            dkMemBlockGetGpuAddr(g->descriptor_memory)+kImageDescriptors*sizeof(DkImageDescriptor),
            kSamplerDescriptors);
        ++g->counters.sampler_rebinds;
    }
    static const bool bind_runs=[] {
        const auto* v=std::getenv("OPENRAC_DK_TEXTURE_BIND_RUNS");
        return !v || std::string_view(v)!="0";
    }();
    std::array<bool,DK_NUM_TEXTURE_BINDINGS> dirty_bindings{};
    for(unsigned i=0;i<draw_samplers;++i) {
        const auto b=std::size_t(draw_bindings[i]);
        const auto handle=draw_handles[b];
        if (!cache || b >= 32 || !sent.textures_valid[0][b] || sent.textures[0][b] != handle) {
            dirty_bindings[b]=true;
            if (b < 32) {
                sent.textures[0][b] = sent.textures[1][b] = handle;
                sent.textures_valid[0][b] = sent.textures_valid[1][b] = true;
            }
        }
    }
    // Adjacent changed slots share one binding command per stage. Gaps retain
    // their previous binding; no descriptor refresh or feedback check is removed.
    for(unsigned first=0;first<DK_NUM_TEXTURE_BINDINGS;) {
        if(!dirty_bindings[first]) {++first;continue;}
        unsigned count=1;
        while(bind_runs && first+count<DK_NUM_TEXTURE_BINDINGS && dirty_bindings[first+count]) ++count;
        dkCmdBufBindTextures(c,DkStage_Vertex,first,draw_handles.data()+first,count);
        dkCmdBufBindTextures(c,DkStage_Fragment,first,draw_handles.data()+first,count);
        g->counters.texture_bind_calls+=2;
        g->counters.texture_bind_slots+=2*count;
        first+=count;
    }
    // A first-use sampler may have written descriptors after cmd() above.
    // Invalidate those writes before this draw, rather than the following draw.
    c = cmd();
    // Reading its own target: what earlier draws wrote is finished and out of the texture cache.
    if (g->feedback) {
        g->feedback = false;
        cmd_barrier(c, DkBarrier_Fragments, DkInvalidateFlags_Image);
        ++g->counters.barriers;
    }
    // Vertex inputs: each from a binding of its own (its offset into the buffer goes into the
    // address, which deko3d's 14-bit attribute offset could not hold); unfed inputs read GL's (0,0,0,1).
    const auto setup_vertices=[&]() -> bool {
    const VertexArray& va = vertex_array();
    auto& fast=g->vertex_setup;
    static const bool enabled=[] {
        const auto* v=std::getenv("OPENRAC_DK_VERTEX_CACHE");
        const bool on=!v || std::string_view(v)!="0";
        log::info("dkgl: repeated vertex setup {} (OPENRAC_DK_VERTEX_CACHE=0 disables)",on ? "cached" : "rebuilt");
        return on;
    }();
    const bool eligible=enabled && cache && !g->draw_override.active;
    bool unchanged=eligible && fast.valid && sent.vertex_count==fast.count
        && fast.vao==g->state.vertex_array && fast.program==g->state.program
        && fast.vao_revision==va.revision && fast.link_revision==p->link_revision
        && fast.epoch==g->ring_epoch && fast.reflected_inputs==p->reflection.attributes.size();
    if(unchanged) {
        for(unsigned loc=0;loc<fast.count;++loc) {
            const auto& attr=va.attributes[loc];
            const auto* b=attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
            if(fast.buffer_versions[loc]!=(b ? b->version : 0)) {unchanged=false;break;}
        }
    }
    if(unchanged) {++g->counters.vertex_setup_hits;return true;}
    ++g->counters.vertex_setup_misses;
    fast.valid=false;
    const auto vertex_epoch=g->ring_epoch;
    DkVtxAttribState attribs[kAttributes];
    DkVtxBufferState states[kAttributes];
    DkBufExtents buffers[kAttributes];
    std::uint32_t n = 0;
    std::memset(attribs, 0, sizeof attribs);
    std::memset(states, 0, sizeof states);
    std::memset(buffers, 0, sizeof buffers);  // compared whole, padding too
    for (const Reflection::Attribute& a : p->reflection.attributes) {
        if (a.location < 0 || a.location >= int(kAttributes)) {
            continue;
        }
        n = std::max(n, std::uint32_t(a.location) + 1);
    }
    for (std::uint32_t loc = 0; loc < n; ++loc) {
        const Attribute& attr = va.attributes[loc];
        Buffer* b = attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
        DkVtxAttribSize size{};
        DkVtxAttribType type{};
        DkGpuAddr base = b != nullptr ? resolve(*b) : 0;
        // A pointer past its buffer's end (a GPU "method error" on VertexArray Start at
        // [b197:739] was seen on Novalis, 2026-10-29): fed GL's constant instead, and said once.
        if (base != 0 && attr.offset >= b->data.size()) {
            static unsigned said = 0;
            if (said++ < 8) {
                log::error("dkgl: program {:016x} input {} offset {} past its buffer {} ({} bytes)",
                           p->key, loc, std::uint64_t(attr.offset), attr.buffer, b->data.size());
            }
            base = 0;
        }
        const auto& o = g->draw_override;
        if (base != 0 && o.active && attr.divisor == 0 && (loc == 0 || (loc == 1 && o.normals))) {
            // The CPU-skinned position (location 0) and normal (location 1).
            attribs[loc] = DkVtxAttribState{loc, 0, 0, DkVtxAttribSize_3x32, DkVtxAttribType_Float, 0};
            states[loc] = DkVtxBufferState{24, 0};
            buffers[loc] = DkBufExtents{o.skinned + (loc == 1 ? 12u : 0u), o.vertices * 24 - (loc == 1 ? 12u : 0u)};
        } else if (base != 0 && attribute_format(attr, size, type)) {
            const GLsizei stride = attr.stride != 0 ? attr.stride : attr.size * type_bytes(attr.type);
            attribs[loc] = DkVtxAttribState{loc, 0, 0, size, type, 0};
            states[loc] = DkVtxBufferState{std::uint32_t(stride), attr.divisor};
            // A CPU-skinned draw counts vertices from `first` and draws one instance at a time.
            const std::uint64_t shift = !o.active ? 0
                                        : attr.divisor == 0 ? std::uint64_t(o.first) * std::uint64_t(stride)
                                                            : std::uint64_t(o.instance / attr.divisor) * std::uint64_t(stride);
            const std::uint64_t start = attr.offset + shift;
            const std::uint64_t end = b->data.size();
            buffers[loc] = DkBufExtents{base + start, std::uint32_t(end > start ? end - start : 0)};
        } else {
            // The input's kind decides which constant: (0,0,0,1) as floats or as integers.
            bool integer = false;
            for (const Reflection::Attribute& a : p->reflection.attributes) {
                if (a.location == int(loc)) {
                    integer = integer_input(a.gl_type);
                }
            }
            attribs[loc] = DkVtxAttribState{loc, 0, 0, DkVtxAttribSize_4x32,
                                            integer ? DkVtxAttribType_Sint : DkVtxAttribType_Float, 0};
            states[loc] = DkVtxBufferState{0, 0};
            buffers[loc] = DkBufExtents{dkMemBlockGetGpuAddr(g->constant_attributes) + (integer ? 16 : 0), 16};
        }
    }
    const bool same_inputs = cache && sent.vertex_count == n
                             && std::memcmp(sent.attribs.data(), attribs, sizeof(DkVtxAttribState) * n) == 0
                             && std::memcmp(sent.states.data(), states, sizeof(DkVtxBufferState) * n) == 0
                             && std::memcmp(sent.buffers.data(), buffers, sizeof(DkBufExtents) * n) == 0;
    if (same_inputs) {
        // The same inputs as the last draw: nothing to send.
    } else if (n > 0) {
        sent.vertex_count = n;
        std::memcpy(sent.attribs.data(), attribs, sizeof(DkVtxAttribState) * n);
        std::memcpy(sent.states.data(), states, sizeof(DkVtxBufferState) * n);
        std::memcpy(sent.buffers.data(), buffers, sizeof(DkBufExtents) * n);
        dkCmdBufBindVtxAttribState(c, attribs, n);
        dkCmdBufBindVtxBufferState(c, states, n);
        if (!bind_vertex_buffers_safe(c,g->vertex_capture,0,{buffers,n})) {
            g->error="deko3d: vertex-binding command capture failed";
            log::error("{}",g->error);
            return false;
        }
    } else {
        sent.vertex_count = 0;
        dkCmdBufBindVtxAttribState(c, nullptr, 0);
        dkCmdBufBindVtxBufferState(c, nullptr, 0);
    }
    if(eligible && vertex_epoch==g->ring_epoch) {
        fast.vao=g->state.vertex_array;fast.program=g->state.program;
        fast.vao_revision=va.revision;fast.link_revision=p->link_revision;
        fast.epoch=g->ring_epoch;fast.reflected_inputs=p->reflection.attributes.size();fast.count=n;
        for(unsigned loc=0;loc<n;++loc) {
            const auto& attr=va.attributes[loc];
            const auto* b=attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
            fast.buffer_versions[loc]=b ? b->version : 0;
        }
        fast.valid=true;
    }
    return true;
    };
    if(!setup_vertices()) return false;
    if (!record_fixed_state(c, t)) return false;
    record_viewport_scissor(c, t);
    // Mark after setup: a barrier recorded during it may have advanced the epoch. Even masked
    // writes count conservatively.
    if(t.colour) t.colour->draw_write_epoch=g->ordered_epoch;
    if(t.depth) t.depth->draw_write_epoch=g->ordered_epoch;
    ++g->counters.draws;
    return true;
}

// The index buffer and primitive restart, when they differ from what the last draw sent.
void bind_index_buffer(DkCmdBuf c, DkIdxFormat format, DkGpuAddr addr, int bytes) {
    auto& sent = g->sent;
    const int restart = cap(GL_PRIMITIVE_RESTART) ? 1 : 0;
    if (state_cache() && sent.index == addr && sent.index_bytes == bytes && sent.restart == restart) {
        return;
    }
    dkCmdBufBindIdxBuffer(c, format, addr);
    dkCmdBufSetPrimitiveRestart(c, restart != 0, bytes == 1 ? 0xFFu : bytes == 2 ? 0xFFFFu : 0xFFFFFFFFu);
    sent.index = addr;
    sent.index_bytes = bytes;
    sent.restart = restart;
}

// ---- GL: state ----

const GLubyte* OPENRAC_GLAPI get_string(GLenum name) {
    switch (name) {
        case GL_VENDOR: return reinterpret_cast<const GLubyte*>("OpenRAC");
        case GL_RENDERER: return reinterpret_cast<const GLubyte*>("NVIDIA Tegra X1 (deko3d)");
        case GL_VERSION: return reinterpret_cast<const GLubyte*>("4.1 OpenRAC on deko3d");
        case GL_SHADING_LANGUAGE_VERSION: return reinterpret_cast<const GLubyte*>("4.10");
        default: return nullptr;
    }
}

// The extensions the renderers ask about that deko3d has.
constexpr const char* kExtensions[] = {"GL_ARB_texture_filter_anisotropic", "GL_EXT_texture_filter_anisotropic"};

const GLubyte* OPENRAC_GLAPI get_string_i(GLenum name, GLuint index) {
    if (name == GL_EXTENSIONS && index < std::size(kExtensions)) {
        return reinterpret_cast<const GLubyte*>(kExtensions[index]);
    }
    return nullptr;
}

void OPENRAC_GLAPI get_integer_v(GLenum pname, GLint* data) {
    if (data == nullptr) {
        return;
    }
    switch (pname) {
        case GL_NUM_EXTENSIONS: *data = GLint(std::size(kExtensions)); break;
        case GL_MAJOR_VERSION: *data = 4; break;
        case GL_MINOR_VERSION: *data = 1; break;
        case GL_MAX_TEXTURE_SIZE: *data = 8192; break;
        case GL_MAX_SAMPLES: *data = 1; break;  // MSAA not yet carried out
        case GL_SAMPLES: *data = 0; break;
        case GL_MAX_TEXTURE_BUFFER_SIZE: *data = 0; break;
        case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS: *data = 0; break;
        case kGlMaxTextureImageUnits: *data = GLint(kUnits); break;
        case kGlMaxVertexAttribs: *data = GLint(kAttributes); break;
        case GL_VIEWPORT:
            std::copy(std::begin(g->state.viewport), std::end(g->state.viewport), data);
            break;
        default: *data = 0; break;
    }
}

void OPENRAC_GLAPI get_float_v(GLenum pname, GLfloat* data) {
    if (data == nullptr) {
        return;
    }
    *data = pname == GL_MAX_TEXTURE_MAX_ANISOTROPY ? 16.0f : 0.0f;
}

GLenum OPENRAC_GLAPI get_error() {
    const GLenum e = g->state.error;
    g->state.error = GL_NO_ERROR;
    return e;
}

void OPENRAC_GLAPI enable(GLenum c) {
    if (const int i = cap_index(c); i >= 0) {
        g->state.caps[std::size_t(i)] = true;
    }
}

void OPENRAC_GLAPI disable(GLenum c) {
    if (const int i = cap_index(c); i >= 0) {
        g->state.caps[std::size_t(i)] = false;
    }
}

void OPENRAC_GLAPI clear_colour(GLfloat r, GLfloat gr, GLfloat b, GLfloat a) {
    g->state.clear_colour[0] = r;
    g->state.clear_colour[1] = gr;
    g->state.clear_colour[2] = b;
    g->state.clear_colour[3] = a;
}

void OPENRAC_GLAPI clear_depth(GLdouble d) { g->state.clear_depth = d; }
void OPENRAC_GLAPI clear_stencil(GLint s) { g->state.clear_stencil = s; }

void OPENRAC_GLAPI viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    g->state.viewport[0] = x;
    g->state.viewport[1] = y;
    g->state.viewport[2] = w;
    g->state.viewport[3] = h;
}

void OPENRAC_GLAPI scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
    g->state.scissor[0] = x;
    g->state.scissor[1] = y;
    g->state.scissor[2] = w;
    g->state.scissor[3] = h;
}

void OPENRAC_GLAPI depth_func(GLenum f) { g->state.depth_func = f; }
void OPENRAC_GLAPI depth_mask(GLboolean m) { g->state.depth_mask = m != GL_FALSE; }

void OPENRAC_GLAPI stencil_func(GLenum f, GLint ref, GLuint mask) {
    g->state.stencil_func = f;
    g->state.stencil_ref = ref;
    g->state.stencil_value_mask = mask;
}

void OPENRAC_GLAPI stencil_op_separate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    for (int side = 0; side < 2; ++side) {
        const bool applies = face == GL_FRONT_AND_BACK || (side == 0 && face == GL_FRONT) || (side == 1 && face == GL_BACK);
        if (applies) {
            g->state.stencil_ops[side][0] = sfail;
            g->state.stencil_ops[side][1] = dpfail;
            g->state.stencil_ops[side][2] = dppass;
        }
    }
}

void OPENRAC_GLAPI stencil_mask(GLuint m) { g->state.stencil_write_mask = m; }

void OPENRAC_GLAPI colour_mask(GLboolean r, GLboolean gr, GLboolean b, GLboolean a) {
    g->state.colour_mask[0] = r != GL_FALSE;
    g->state.colour_mask[1] = gr != GL_FALSE;
    g->state.colour_mask[2] = b != GL_FALSE;
    g->state.colour_mask[3] = a != GL_FALSE;
}

void OPENRAC_GLAPI blend_func_separate(GLenum sr, GLenum dr, GLenum sa, GLenum da) {
    g->state.blend_src_rgb = sr;
    g->state.blend_dst_rgb = dr;
    g->state.blend_src_a = sa;
    g->state.blend_dst_a = da;
}

void OPENRAC_GLAPI blend_equation(GLenum e) { g->state.blend_equation = e; }

void OPENRAC_GLAPI blend_colour(GLfloat r, GLfloat gr, GLfloat b, GLfloat a) {
    g->state.blend_colour[0] = r;
    g->state.blend_colour[1] = gr;
    g->state.blend_colour[2] = b;
    g->state.blend_colour[3] = a;
}

void OPENRAC_GLAPI cull_face(GLenum m) { g->state.cull_face = m; }
void OPENRAC_GLAPI front_face(GLenum m) { g->state.front_face = m; }
void OPENRAC_GLAPI polygon_mode(GLenum, GLenum m) { g->state.polygon_mode = m; }

void OPENRAC_GLAPI pixel_store_i(GLenum pname, GLint v) {
    if (pname == GL_UNPACK_ALIGNMENT) {
        g->state.unpack_alignment = std::max(1, v);
    } else if (pname == GL_PACK_ALIGNMENT) {
        g->state.pack_alignment = std::max(1, v);
    }
}

void OPENRAC_GLAPI finish() {
    flush_and_wait();
}

// ---- GL: clearing ----

void OPENRAC_GLAPI clear(GLbitfield mask) {
    Target t;
    transfer_fence();
    if (!bind_target(t)) {
        return;
    }
    if(t.colour && (mask & kGlColorBufferBit)) t.colour->draw_write_epoch=g->ordered_epoch;
    if(t.depth && (mask & (kGlDepthBufferBit | kGlStencilBufferBit))) t.depth->draw_write_epoch=g->ordered_epoch;
    touch(t.colour);
    touch(t.depth);
    DkCmdBuf c = cmd();
    record_viewport_scissor(c, t);
    const State& s = g->state;
    if ((mask & kGlColorBufferBit) != 0 && t.colour != nullptr) {
        const std::uint32_t m = (s.colour_mask[0] ? DkColorMask_R : 0) | (s.colour_mask[1] ? DkColorMask_G : 0)
                                | (s.colour_mask[2] ? DkColorMask_B : 0) | (s.colour_mask[3] ? DkColorMask_A : 0);
        if (m != 0) {
            dkCmdBufClearColorFloat(c, 0, m, s.clear_colour[0], s.clear_colour[1], s.clear_colour[2], s.clear_colour[3]);
        }
    }
    const bool depth = (mask & kGlDepthBufferBit) != 0 && t.depth != nullptr && s.depth_mask;
    const bool stencil = (mask & kGlStencilBufferBit) != 0 && t.depth != nullptr && t.depth->stencil;
    if (depth || stencil) {
        dkCmdBufClearDepthStencil(c, depth, float(std::clamp(s.clear_depth, 0.0, 1.0)),
                                  stencil ? std::uint8_t(s.stencil_write_mask) : 0,
                                  std::uint8_t(s.clear_stencil));
    }
}

// ---- GL: textures ----

void OPENRAC_GLAPI gen_textures(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->textures.make();
    }
}

void OPENRAC_GLAPI delete_textures(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        for (GLuint& unit : g->state.textures) {
            if (unit == names[i]) {
                unit = 0;
            }
        }
        g->textures.objects.erase(names[i]);  // its image retires with the last holder
    }
}

void OPENRAC_GLAPI bind_texture(GLenum, GLuint name) {
    if (name != 0 && g->textures.find(name) == nullptr) {
        g->textures.objects.emplace(name, Texture{});
        g->textures.next = std::max(g->textures.next, name + 1);
    }
    g->state.textures[g->state.active_unit] = name;
}

void OPENRAC_GLAPI active_texture(GLenum unit) {
    const GLuint u = unit - GL_TEXTURE0;
    if (u < kUnits) {
        g->state.active_unit = u;
    } else {
        fail(kGlInvalidEnum);
    }
}

// Copies `w` x `h` client pixels into the ring, rows padded for the copy engine; RGB becomes RGBA.
bool stage_pixels(const void* pixels, int w, int h, GLenum format, GLenum type, const Format& f,
                  RingSlice& out, std::uint32_t& pitch) {
    std::uint32_t staged_bytes=0;
    if (!staging_layout(w,h,unsigned(f.bytes),kRingBytes,pitch,staged_bytes)) {
        fail(kGlInvalidValue);
        log::error("dkgl: invalid texture staging extent {}x{}, {} bytes/texel",w,h,f.bytes);
        return false;
    }
    const int in_bytes = client_bytes(format, type);
    const bool expand_rgb=format==GL_RGB && type==GL_UNSIGNED_BYTE && f.bytes==4;
    if(in_bytes!=f.bytes && !expand_rgb) {
        fail(0x0502); // GL_INVALID_OPERATION; no silent zero-filled texture
        log::error("dkgl: unsupported texture conversion format {:#x}, type {:#x}, {} to {} bytes",format,type,in_bytes,f.bytes);
        return false;
    }
    const std::size_t in_pitch = align_up(std::size_t(w) * in_bytes, std::size_t(g->state.unpack_alignment));
    out = ring(staged_bytes, DK_IMAGE_LINEAR_STRIDE_ALIGNMENT);
    if (out.cpu == nullptr) {
        return false;
    }
    const auto* src = static_cast<const std::uint8_t*>(pixels);
    auto* dst = static_cast<std::uint8_t*>(out.cpu);
    for (int y = 0; y < h; ++y) {
        const std::uint8_t* row = src + in_pitch * std::size_t(y);
        std::uint8_t* to = dst + std::size_t(pitch) * std::size_t(y);
        if (in_bytes == f.bytes) {
            std::memcpy(to, row, std::size_t(w) * std::size_t(f.bytes));
        } else if (format == GL_RGB && type == GL_UNSIGNED_BYTE && f.bytes == 4) {
            for (int x = 0; x < w; ++x) {
                to[x * 4 + 0] = row[x * 3 + 0];
                to[x * 4 + 1] = row[x * 3 + 1];
                to[x * 4 + 2] = row[x * 3 + 2];
                to[x * 4 + 3] = 0xFF;
            }
        }
    }
    return true;
}

// Records the copy of staged pixels into a level's rectangle of `image`.
void upload(Image& image, int level, int x, int y, int w, int h, const RingSlice& staged, std::uint32_t pitch) {
    if (!texture_rectangle(image.width,image.height,image.levels,level,x,y,w,h)) {
        fail(kGlInvalidValue);
        log::error("dkgl: texture upload rectangle {}x{} at {},{} exceeds mip {} of {}x{}",w,h,x,y,level,image.width,image.height);
        return;
    }
    // The copy engine writes it: draws that may still read it (this frame's, or the last frame's
    // still on the GPU) finish first, by a Full barrier. A texture no 3D or 2D work has touched
    // since the last Full barrier (a new one, most uploads) needs no wait.
    // OPENRAC_DK_COUNTERS: the first few uploads into a texture the GPU may still read (each a
    // Full barrier) are named, to find the recurring ones.
    // On only for a positive interval, as the counter lines (common/env_interval.h): "=0" no
    // longer fills the upload-shape map or names busy uploads.
    static const bool counting = env_interval::on(std::getenv("OPENRAC_DK_COUNTERS"));
    const bool busy = before_transfer(&image, nullptr, CopySource::Upload);
    if (counting && g->upload_shapes.size() < 256) {
        auto& shape = g->upload_shapes[{std::uint32_t(image.width), std::uint32_t(image.height),
                                        std::uint32_t(image.format), std::uint32_t(level), std::uint32_t(w),
                                        std::uint32_t(h)}];
        ++shape[0];
        shape[1] += busy ? 1 : 0;
    }
    if (busy) {
        static unsigned named = 0;
        if (counting && named < 8) {
            ++named;
            log::info("dk copy: upload into a busy texture {}x{} format {} levels {} (level {} rect {}x{} at {},{}): Full barrier first",
                      image.width, image.height, int(image.format), image.levels, level, w, h, x, y);
        }
    }
    DkCmdBuf c = cmd();
    DkImageView view;
    dkImageViewDefaults(&view, &image.image);
    view.mipLevelOffset = std::uint8_t(level);
    view.mipLevelCount = 1;
    const DkCopyBuf src{staged.gpu, pitch, 0};
    const DkImageRect rect{std::uint32_t(x), std::uint32_t(y), 0, std::uint32_t(w), std::uint32_t(h), 1};
    dkCmdBufCopyBufferToImage(c, &src, &view, &rect, 0);
    after_transfer(CopySource::Upload);
    ++g->counters.texture_uploads;
}

// OPENRAC_DK_MIP_AUDIT=N (default 0): the first N mip chains the game makes are read back
// from the GPU and level 1 compared with level 0's 2x2 averages, on the game's own textures. A chain
// made from level 0 before the copy engine wrote it (the 2026-10-29 transfer-ordering fault)
// mismatches. Each audit waits for the GPU once.
int& mip_audits_left() {
    static int left = [] {
        const char* v = std::getenv("OPENRAC_DK_MIP_AUDIT");
        return v != nullptr ? std::max(0, std::atoi(v)) : 0;
    }();
    return left;
}

// The last glTexImage2D's level 0 texels (one texture's worth): TexturePool calls glGenerateMipmap
// right after it.
struct MipAuditSource {
    const Image* image = nullptr;
    std::vector<std::uint8_t> pixels;
};
MipAuditSource& mip_audit_source() {
    static MipAuditSource source;
    return source;
}

void audit_mip_level_1(Image& image, const std::vector<std::uint8_t>& pixels) {
    const int w = image.width, h = image.height, w1 = w / 2, h1 = h / 2;
    if (--mip_audits_left() < 0 || image.levels < 2 || pixels.size() != std::size_t(w) * std::size_t(h) * 4) {
        return;
    }
    const auto pitch = std::uint32_t(align_up(std::uint64_t(w1) * 4, DK_IMAGE_LINEAR_STRIDE_ALIGNMENT));
    const RingSlice s = ring(pitch * std::uint32_t(h1), DK_IMAGE_LINEAR_STRIDE_ALIGNMENT);
    if (s.cpu == nullptr) {
        return;
    }
    // The audit's own ordering is the strict one in either mode: it reports what the chain holds.
    barrier(DkBarrier_Full, DkInvalidateFlags_Image);
    ++g->counters.full_readback;
    DkImageView view;
    dkImageViewDefaults(&view, &image.image);
    view.mipLevelOffset = 1;
    view.mipLevelCount = 1;
    const DkImageRect rect{0, 0, 0, std::uint32_t(w1), std::uint32_t(h1), 1};
    const DkCopyBuf dst{s.gpu, pitch, 0};
    dkCmdBufCopyImageToBuffer(cmd(), &view, &rect, &dst, 0);
    dkCmdBufBarrier(cmd(), DkBarrier_Full, DkInvalidateFlags_L2Cache);
    flush_and_wait();
    const auto* got = static_cast<const std::uint8_t*>(s.cpu);
    const auto* src = pixels.data();
    unsigned match = 0, checked = 0, worst = 0;
    for (int sy = 0; sy < 4; ++sy) {
        for (int sx = 0; sx < 4; ++sx) {
            const int x = (w1 - 1) * sx / 3, y = (h1 - 1) * sy / 3;
            unsigned diff = 0;
            for (int ch = 0; ch < 4; ++ch) {
                const auto at = [&](int px, int py) { return unsigned(src[(std::size_t(py) * std::size_t(w) + std::size_t(px)) * 4 + std::size_t(ch)]); };
                const unsigned want = (at(2 * x, 2 * y) + at(2 * x + 1, 2 * y) + at(2 * x, 2 * y + 1) + at(2 * x + 1, 2 * y + 1) + 2) / 4;
                const unsigned have = got[std::size_t(y) * pitch + std::size_t(x) * 4 + std::size_t(ch)];
                diff = std::max(diff, want > have ? want - have : have - want);
            }
            ++checked;
            match += diff <= 3;
            worst = std::max(worst, diff);
        }
    }
    const bool ok = match == checked;
    if (ok) {
        log::info("dkgl mip audit PASS: {}x{} texture, level 1 matches level 0's averages at {}/{} texels (max diff {})",
                  w, h, match, checked, worst);
    } else {
        log::error("dkgl mip audit FAIL: {}x{} texture, level 1 matches level 0's averages at {}/{} texels (max diff {}){}",
                   w, h, match, checked, worst, legacy_transfer_sync() ? " [legacy transfer sync]" : "");
    }
}

void OPENRAC_GLAPI tex_image_2d(GLenum, GLint level, GLint internal, GLsizei w, GLsizei h, GLint,
                                GLenum format, GLenum type, const void* pixels) {
    Texture* t = bound_texture();
    const Format f = format_of(GLenum(internal));
    if (t == nullptr || f.dk == DkImageFormat_None || w <= 0 || h <= 0 || level < 0) {
        fail(f.dk == DkImageFormat_None ? kGlInvalidEnum : kGlInvalidValue);
        return;
    }
    if (level == 0) {
        const bool same = t->image && t->image->width == w && t->image->height == h && t->image->format == f.dk;
        if (!same) {
            // A new image: GL's levels beyond 0 of the old one go with it.
            const bool depth = f.dk == DkImageFormat_Z24S8 || f.dk == DkImageFormat_Z24X8 || f.dk == DkImageFormat_ZF32;
            t->image = make_image(f.dk, w, h, 1, true);
            (void)depth;
            t->internal = GLenum(internal);
        }
    } else if (!t->image || level >= int(t->image->levels)) {
        return;  // levels come from glGenerateMipmap
    }
    if (pixels != nullptr && t->image) {
        RingSlice staged;
        std::uint32_t pitch = 0;
        if (stage_pixels(pixels, w, h, format, type, f, staged, pitch)) {
            upload(*t->image, level, 0, 0, w, h, staged, pitch);
        }
        if (level == 0 && internal == 0x8814 && format == GL_RGBA && type == GL_FLOAT) {
            t->shadow.assign(static_cast<const float*>(pixels), static_cast<const float*>(pixels) + std::size_t(w) * std::size_t(h) * 4);
            ++t->shadow_version;
        } else if (level == 0) {
            t->shadow.clear();
        }
        mip_audit_source().image = nullptr;
        if (level == 0 && mip_audits_left() > 0 && f.dk == DkImageFormat_RGBA8_Unorm && format == GL_RGBA
            && type == GL_UNSIGNED_BYTE && w >= 4 && h >= 4 && w % 2 == 0 && h % 2 == 0) {
            const std::size_t in_pitch = align_up(std::size_t(w) * 4, std::size_t(g->state.unpack_alignment));
            auto& source = mip_audit_source();
            source.image = t->image.get();
            source.pixels.resize(std::size_t(w) * std::size_t(h) * 4);
            for (int row = 0; row < h; ++row) {
                std::memcpy(source.pixels.data() + std::size_t(row) * std::size_t(w) * 4,
                            static_cast<const std::uint8_t*>(pixels) + in_pitch * std::size_t(row), std::size_t(w) * 4);
            }
        }
    }
}

void OPENRAC_GLAPI tex_sub_image_2d(GLenum, GLint level, GLint x, GLint y, GLsizei w, GLsizei h,
                                    GLenum format, GLenum type, const void* pixels) {
    Texture* t = bound_texture();
    if (t == nullptr || !t->image || pixels == nullptr || w <= 0 || h <= 0 || level < 0
        || level >= int(t->image->levels)) {
        return;
    }
    const Format f = format_of(t->internal);
    RingSlice staged;
    std::uint32_t pitch = 0;
    if (f.dk != DkImageFormat_None && stage_pixels(pixels, w, h, format, type, f, staged, pitch)) {
        upload(*t->image, level, x, y, w, h, staged, pitch);
    }
    if (level == 0 && !t->shadow.empty() && format == GL_RGBA && type == GL_FLOAT
        && t->shadow.size() == std::size_t(t->image->width) * std::size_t(t->image->height) * 4) {
        const auto* src = static_cast<const float*>(pixels);
        for (int row = 0; row < h; ++row) {
            std::memcpy(t->shadow.data() + (std::size_t(y + row) * std::size_t(t->image->width) + std::size_t(x)) * 4,
                        src + std::size_t(row) * std::size_t(w) * 4, std::size_t(w) * 16);
        }
        ++t->shadow_version;
    }
}

void set_param(SamplerParams& p, int& max_level, GLenum pname, float v) {
    switch (pname) {
        case GL_TEXTURE_MIN_FILTER: p.min = GLenum(v); break;
        case GL_TEXTURE_MAG_FILTER: p.mag = GLenum(v); break;
        case GL_TEXTURE_WRAP_S: p.wrap_s = GLenum(v); break;
        case GL_TEXTURE_WRAP_T: p.wrap_t = GLenum(v); break;
        case GL_TEXTURE_MAX_ANISOTROPY: p.anisotropy = v; break;
        case GL_TEXTURE_MAX_LEVEL: max_level = int(v); break;
        default: break;
    }
}

void OPENRAC_GLAPI tex_parameter_i(GLenum, GLenum pname, GLint v) {
    if (Texture* t = bound_texture()) {
        set_param(t->params, t->max_level, pname, float(v));
    }
}

void OPENRAC_GLAPI tex_parameter_f(GLenum, GLenum pname, GLfloat v) {
    if (Texture* t = bound_texture()) {
        set_param(t->params, t->max_level, pname, v);
    }
}

void OPENRAC_GLAPI generate_mipmap(GLenum) {
    Texture* t = bound_texture();
    if (t == nullptr || !t->image || t->image->depth) {
        return;
    }
    const int w = t->image->width, h = t->image->height;
    unsigned levels = 1;
    while ((w >> levels) > 0 || (h >> levels) > 0) {
        ++levels;
    }
    levels = std::min<unsigned>(levels, unsigned(std::max(1, t->max_level + 1)));
    if (levels <= 1) {
        return;
    }
    barrier(DkBarrier_Full, DkInvalidateFlags_Image);
    ++g->counters.full_mipmap;
    DkCmdBuf c = cmd();
    // A new image with the whole chain: level 0 copied in, each level after blitted from the one before.
    ImagePtr chain = make_image(t->image->format, w, h, levels, true);
    if (!chain) {
        return;
    }
    DkImageView from, to;
    dkImageViewDefaults(&from, &t->image->image);
    dkImageViewDefaults(&to, &chain->image);
    to.mipLevelCount = 1;
    const DkImageRect whole{0, 0, 0, std::uint32_t(w), std::uint32_t(h), 1};
    dkCmdBufCopyImage(c, &from, &whole, &to, &whole, 0);
    after_transfer(CopySource::Mipmap);
    for (unsigned l = 1; l < levels; ++l) {
        // Level l-1 is complete before the 2D engine reads it: level 0 came from the copy
        // engine, which a 3D WaitForIdle (DkBarrier_Primitives) does not wait for. Without this
        // the smaller levels were made from whatever the image memory held before (another
        // texture, or nothing): wrong surfaces wherever a 3D texture is minified.
        if (l == 1 && !legacy_transfer_sync()) {
            barrier(DkBarrier_Full, DkInvalidateFlags_Image);
            ++g->counters.full_mipmap;
        } else {
            barrier(DkBarrier_Primitives, DkInvalidateFlags_Image);
        }
        c = cmd();  // a long list may have been sent in parts
        DkImageView src, dst;
        dkImageViewDefaults(&src, &chain->image);
        src.mipLevelOffset = std::uint8_t(l - 1);
        src.mipLevelCount = 1;
        dkImageViewDefaults(&dst, &chain->image);
        dst.mipLevelOffset = std::uint8_t(l);
        dst.mipLevelCount = 1;
        const DkImageRect a{0, 0, 0, std::uint32_t(std::max(1, w >> (l - 1))), std::uint32_t(std::max(1, h >> (l - 1))), 1};
        const DkImageRect b{0, 0, 0, std::uint32_t(std::max(1, w >> l)), std::uint32_t(std::max(1, h >> l)), 1};
        dkCmdBufBlitImage(c, &src, &a, &dst, &b, DkBlitFlag_FilterLinear, 0);
    }
    touch(chain.get());
    const Image* from_image = t->image.get();  // compared only: it may be freed below
    t->image = chain;
    g->need_image_barrier = true;
    auto& source = mip_audit_source();
    if (source.image != nullptr && source.image == from_image) {
        audit_mip_level_1(*chain, source.pixels);
    }
    source.image = nullptr;
}

void OPENRAC_GLAPI copy_tex_sub_image_2d(GLenum, GLint level, GLint xoff, GLint yoff, GLint x, GLint y,
                                         GLsizei w, GLsizei h) {
    Texture* t = bound_texture();
    const Target from = current_target(g->state.read_framebuffer);
    if (t == nullptr || !t->image || from.colour == nullptr || w <= 0 || h <= 0) {
        return;
    }
    const int sx = std::max(x, 0), sy = std::max(y, 0);
    const int dx = xoff + (sx - x), dy = yoff + (sy - y);
    const int cw = std::min({w - (sx - x), from.width - sx, (t->image->width >> level) - dx});
    const int ch = std::min({h - (sy - y), from.height - sy, (t->image->height >> level) - dy});
    if (cw <= 0 || ch <= 0 || dx < 0 || dy < 0) {
        return;
    }
    // Same format: the copy engine (DkBarrier_Fragments does not make it wait for the draws
    // that rendered the source); else the 2D engine.
    // The 2D engine unless OPENRAC_DK_CE_COPY=1 (see blit_framebuffer): ordered by the fragment
    // barrier below, no Full barriers.
    const char* ce = std::getenv("OPENRAC_DK_CE_COPY");
    const bool engine_copy = ce != nullptr && std::string_view(ce) == "1" && from.colour->format == t->image->format;
    transfer_fence();
    barrier(DkBarrier_Fragments, DkInvalidateFlags_Image);
    if (engine_copy) {
        before_transfer(from.colour, t->image.get(), CopySource::TextureCopy);
    }
    DkCmdBuf c = cmd();
    DkImageView src, dst;
    dkImageViewDefaults(&src, &from.colour->image);
    dkImageViewDefaults(&dst, &t->image->image);
    dst.mipLevelOffset = std::uint8_t(level);
    dst.mipLevelCount = 1;
    const DkImageRect a{std::uint32_t(sx), std::uint32_t(sy), 0, std::uint32_t(cw), std::uint32_t(ch), 1};
    const DkImageRect b{std::uint32_t(dx), std::uint32_t(dy), 0, std::uint32_t(cw), std::uint32_t(ch), 1};
    if (engine_copy) {
        dkCmdBufCopyImage(c, &src, &a, &dst, &b, 0);
        after_transfer(CopySource::TextureCopy);
    } else {
        dkCmdBufBlitImage(c, &src, &a, &dst, &b, 0, 0);
        touch(from.colour);
        touch(t->image.get());
    }
    g->need_image_barrier = true;
}

// ---- GL: sampler objects ----

void OPENRAC_GLAPI gen_samplers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->samplers.make();
    }
}

void OPENRAC_GLAPI delete_samplers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        for (GLuint& unit : g->state.samplers) {
            if (unit == names[i]) {
                unit = 0;
            }
        }
        g->samplers.objects.erase(names[i]);
    }
}

void OPENRAC_GLAPI bind_sampler(GLuint unit, GLuint sampler) {
    if (unit < kUnits) {
        g->state.samplers[unit] = sampler;
    }
}

void OPENRAC_GLAPI sampler_parameter_i(GLuint sampler, GLenum pname, GLint v) {
    if (SamplerParams* s = g->samplers.find(sampler)) {
        int unused = 0;
        set_param(*s, unused, pname, float(v));
    }
}

// ---- GL: buffers ----

void OPENRAC_GLAPI gen_buffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->buffers.make();
    }
}

void OPENRAC_GLAPI delete_buffers(GLsizei n, const GLuint* names) {
    g->vertex_setup.valid=false;
    for (GLsizei i = 0; i < n; ++i) {
        if (Buffer* b = g->buffers.find(names[i])) {
            retire(b->block);
        }
        g->buffers.objects.erase(names[i]);
        if (g->state.array_buffer == names[i]) {
            g->state.array_buffer = 0;
        }
    }
}

void OPENRAC_GLAPI bind_buffer(GLenum target, GLuint name) {
    if (name != 0 && g->buffers.find(name) == nullptr) {
        g->buffers.objects.emplace(name, Buffer{});
        g->buffers.next = std::max(g->buffers.next, name + 1);
    }
    *buffer_binding(target) = name;
}

void OPENRAC_GLAPI buffer_data(GLenum target, GLsizeiptr size, const void* data, GLenum) {
    Buffer* b = g->buffers.find(*buffer_binding(target));
    if (b == nullptr || size < 0) {
        return;
    }
    b->data.resize(std::size_t(size));
    if (data != nullptr && size > 0) {
        std::memcpy(b->data.data(), data, std::size_t(size));
    } else if (size > 0) {
        std::memset(b->data.data(), 0, std::size_t(size));
    }
    changed(*b);
}

void OPENRAC_GLAPI buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
    Buffer* b = g->buffers.find(*buffer_binding(target));
    if (b == nullptr || data == nullptr || offset < 0 || size <= 0
        || !geometry_span(b->data.size(),std::uint64_t(offset),1,0,std::uint64_t(size))) {
        if(b && size>0) {
            fail(kGlInvalidValue);
            log::error("dkgl geometry: rejected buffer update offset {} bytes {} storage {}",offset,size,b->data.size());
        }
        return;
    }
    std::memcpy(b->data.data() + offset, data, std::size_t(size));
    changed(*b);
}

void OPENRAC_GLAPI bind_buffer_base(GLenum, GLuint, GLuint) {}
void OPENRAC_GLAPI tex_buffer(GLenum, GLenum, GLuint) {}

// ---- GL: vertex arrays ----

void OPENRAC_GLAPI gen_vertex_arrays(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->vertex_arrays.make();
    }
}

void OPENRAC_GLAPI delete_vertex_arrays(GLsizei n, const GLuint* names) {
    g->vertex_setup.valid=false;
    for (GLsizei i = 0; i < n; ++i) {
        g->vertex_arrays.objects.erase(names[i]);
        if (g->state.vertex_array == names[i]) {
            g->state.vertex_array = 0;
        }
    }
}

void OPENRAC_GLAPI bind_vertex_array(GLuint name) {
    if (name != 0 && g->vertex_arrays.find(name) == nullptr) {
        g->vertex_arrays.objects.emplace(name, VertexArray{});
    }
    g->state.vertex_array = name;
}

Attribute* attribute(GLuint index) {
    VertexArray* va = g->vertex_arrays.find(g->state.vertex_array);
    return va != nullptr && index < kAttributes ? &va->attributes[index] : nullptr;
}

void changed_attribute() {
    if(auto* va=g->vertex_arrays.find(g->state.vertex_array)) ++va->revision;
}

void OPENRAC_GLAPI enable_vertex_attrib_array(GLuint index) {
    if (Attribute* a = attribute(index)) {
        if(!a->enabled) {a->enabled=true;changed_attribute();}
    }
}

void OPENRAC_GLAPI vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                                         GLsizei stride, const void* pointer) {
    if (Attribute* a = attribute(index)) {
        const auto offset=reinterpret_cast<std::uintptr_t>(pointer);
        if(a->offset==offset && a->buffer==g->state.array_buffer && a->size==size && a->type==type
            && a->normalized==(normalized!=GL_FALSE) && !a->integer && a->stride==stride) return;
        changed_attribute();
        a->buffer = g->state.array_buffer;
        a->size = size;
        a->type = type;
        a->normalized = normalized != GL_FALSE;
        a->integer = false;
        a->stride = stride;
        a->offset = reinterpret_cast<std::uintptr_t>(pointer);
    }
}

void OPENRAC_GLAPI vertex_attrib_i_pointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) {
    if (Attribute* a = attribute(index)) {
        const auto offset=reinterpret_cast<std::uintptr_t>(pointer);
        if(a->offset==offset && a->buffer==g->state.array_buffer && a->size==size && a->type==type
            && !a->normalized && a->integer && a->stride==stride) return;
        changed_attribute();
        a->buffer = g->state.array_buffer;
        a->size = size;
        a->type = type;
        a->normalized = false;
        a->integer = true;
        a->stride = stride;
        a->offset = reinterpret_cast<std::uintptr_t>(pointer);
    }
}

void OPENRAC_GLAPI vertex_attrib_divisor(GLuint index, GLuint divisor) {
    if (Attribute* a = attribute(index)) {
        if(a->divisor!=divisor) {a->divisor=divisor;changed_attribute();}
    }
}

// ---- GL: draws ----

void OPENRAC_GLAPI draw_arrays(GLenum mode, GLint first, GLsizei count) {
    Program* p = nullptr;
    Target t;
    DkCmdBuf c = nullptr;
    if (count <= 0 || !prepare_draw(p, t, c)) {
        return;
    }
    dkCmdBufDraw(c, primitive(mode), std::uint32_t(count), 1, std::uint32_t(std::max(first, 0)), 0);
}

// Bounded snapshots of real draw inputs, rather than more synthetic triangles.
// Sample throughout gameplay; never write a line for every draw or dump assets.
bool audit_geometry(Program& p,const Buffer& elements,std::uintptr_t offset,
                    GLsizei count,unsigned bytes,GLsizei instances) {
    static const bool enabled=[] {
        const auto* v=std::getenv("OPENRAC_DK_GEOMETRY_AUDIT");
        const bool on=v && std::string_view(v)=="1";
        log::info("dkgl: verbose geometry audit {} (OPENRAC_DK_GEOMETRY_AUDIT=1 enables)",on ? "on" : "off");
        return on;
    }();
    // OPENRAC_DK_PANEL_TRACE=1 traces the first 32 indexed draws into 1024x1024 offscreen panels;
    // a larger number N traces the first N (a vendor page can follow other menus' panels).
    static const unsigned panel_budget=[] {
        const auto* v=std::getenv("OPENRAC_DK_PANEL_TRACE");
        const int n=v ? std::atoi(v) : 0;
        return n<=0 ? 0u : n==1 ? 32u : unsigned(std::min(n,512));
    }();
    static const bool panels=panel_budget>0;
    static unsigned panel_draws=0;
    const bool panel=panels && panel_draws<panel_budget && g->state.draw_framebuffer!=0
        && g->bound_target.width==1024 && g->bound_target.height==1024;
    if(!enabled && !panel) return true;
    static std::uint64_t frame=~std::uint64_t(0);
    static unsigned in_frame=0,total=0,errors=0,ordinal=0;
    static std::map<std::pair<std::uint64_t,int>,std::uint64_t> sampled;
    if(frame!=g->frame) { frame=g->frame; in_frame=0; ordinal=0; }
    if(!panel && (total>=4096 || (g->frame%60)!=0)) return true;
    ++ordinal;
    int skinning=-1;
    if(p.skinning_offset>=0 && geometry_span(p.uniforms.size(),unsigned(p.skinning_offset),1,0,4))
        std::memcpy(&skinning,p.uniforms.data()+p.skinning_offset,4);
    // Reserve samples for each shader AND skinned/rigid path. Otherwise the
    // first terrain draws consume the budget before Ratchet/weapons are seen.
    auto& last=sampled[{p.key,skinning}];
    const bool first=last!=g->frame+1;
    if(!panel && !first && (ordinal%97)!=((g->frame/60)%97)) return true;
    if(!panel && in_frame>=16) return true;
    if(panel) {
        ++panel_draws;
        const auto& st=g->state;
        // What LevelScene::draw_layer's pass and material set: blend and depth writes tell the
        // moby passes apart (opaque, glow, translucent, effects, metal/shine).
        log::info("dkgl panel trace {}/{}: frame {} framebuffer {} blend {} eq {:#x} rgb {:#x}/{:#x} alpha {:#x}/{:#x} "
                  "depth test {} func {:#x} write {} colour mask {}{}{}{}",
            panel_draws,panel_budget,g->frame,st.draw_framebuffer,cap(GL_BLEND),st.blend_equation,
            st.blend_src_rgb,st.blend_dst_rgb,st.blend_src_a,st.blend_dst_a,cap(GL_DEPTH_TEST),st.depth_func,
            st.depth_mask,int(st.colour_mask[0]),int(st.colour_mask[1]),int(st.colour_mask[2]),int(st.colour_mask[3]));
    }
    ++in_frame; last=g->frame+1;
    ++total;
    IndexRange range;
    if(!geometry_indices(elements.data,offset,count,bytes,cap(GL_PRIMITIVE_RESTART),range)) return false;
    bool valid=true;
    const auto& va=vertex_array();
    for(const auto& input:p.reflection.attributes) {
        if(input.location<0 || input.location>=int(kAttributes)) continue;
        const auto& attr=va.attributes[unsigned(input.location)];
        if(!attr.enabled) continue;
        const auto* b=g->buffers.find(attr.buffer);
        const auto element=std::uint64_t(attr.size)*type_bytes(attr.type);
        const auto stride=attr.stride ? std::uint64_t(attr.stride) : element;
        const auto fetched=attr.divisor ? (std::uint64_t(instances)-1)/attr.divisor+1
                                       : range.any ? std::uint64_t(range.last)+1 : 0;
        if(!b || !geometry_span(b->data.size(),attr.offset,fetched,stride,element)) {
            valid=false;
            if(errors++<64) log::error("dkgl geometry: program {:016x} attribute {} buffer {} size {} offset {} stride {} element {} fetched {} exceeds storage",
                p.key,input.location,attr.buffer,b ? b->data.size() : 0,attr.offset,stride,element,fetched);
        }
    }
    const auto& s=g->state;
    log::info("dkgl geometry sample {}: frame {} program {:016x} vao {} indices {}..{} count {} offset {} instances {} valid {} cull {} front {:#x} face {:#x} depth {} func {:#x} blend {} target {}x{}",
        total,g->frame,p.key,s.vertex_array,range.first,range.last,count,offset,instances,valid,
        cap(GL_CULL_FACE),s.front_face,s.cull_face,cap(GL_DEPTH_TEST),s.depth_func,cap(GL_BLEND),g->bound_target.width,g->bound_target.height);
    for(std::size_t i=0;i<p.reflection.uniforms.size();++i) {
        const auto& u=p.reflection.uniforms[i];
        if(u.sampler) {
            const int unit=i<p.units.size() ? p.units[i] : 0;
            const auto* tex=unit>=0 && unit<int(kUnits) ? g->textures.find(s.textures[unsigned(unit)]) : nullptr;
            const auto* image=tex ? tex->image.get() : nullptr;
            log::info("dkgl geometry sampler {}: binding {} unit {} texture {} dimensions {}x{} format {}",
                u.name,u.binding,unit,unit>=0 && unit<int(kUnits) ? s.textures[unsigned(unit)] : 0,
                image ? image->width : 0,image ? image->height : 0,image ? int(image->format) : 0);
        } else if(u.name=="view_projection" && u.offset>=0 && geometry_span(p.uniforms.size(),unsigned(u.offset),1,0,64)) {
            std::array<float,16> matrix{};
            std::memcpy(matrix.data(),p.uniforms.data()+u.offset,64);
            for(unsigned column=0;column<4;++column)
                log::info("dkgl geometry projection column {}: [{}, {}, {}, {}]",column,
                    matrix[column*4],matrix[column*4+1],matrix[column*4+2],matrix[column*4+3]);
        } else if((u.name=="skinning" || u.name=="metal_pass"
                   || (panel && (u.name=="glow_pass" || u.name=="translucent_pass" || u.name=="effect_pass"
                                 || u.name=="glow_split" || u.name=="cutout" || u.name=="opaque_solid")))
                  && u.offset>=0 && geometry_span(p.uniforms.size(),unsigned(u.offset),1,0,4)) {
            std::int32_t value=0; std::memcpy(&value,p.uniforms.data()+u.offset,4);
            log::info("dkgl geometry uniform {}: {}",u.name,value);
        } else if(panel && (u.name=="material_colour" || u.name=="fog_colour")
                  && u.offset>=0 && geometry_span(p.uniforms.size(),unsigned(u.offset),1,0,16)) {
            std::array<float,4> v{}; std::memcpy(v.data(),p.uniforms.data()+u.offset,16);
            log::info("dkgl geometry uniform {}: [{}, {}, {}, {}]",u.name,v[0],v[1],v[2],v[3]);
        }
    }
    // Panel draws: every per-instance input by name (tint, lights = -1 - shine distance, ...), the
    // first instance's value, so a panel draw can be matched with the actor the frontend logged.
    if(panel) {
        for(const auto& input:p.reflection.attributes) {
            if(input.location<0 || input.location>=int(kAttributes)) continue;
            const auto& attr=va.attributes[unsigned(input.location)];
            const auto* b=attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
            if(!b || attr.divisor==0 || attr.size<1 || attr.size>4) continue;
            const bool is_float=attr.type==GL_FLOAT;
            const bool is_int=attr.type==GL_INT || attr.type==GL_UNSIGNED_INT;
            if(!is_float && !is_int) continue;
            const auto stride=attr.stride ? std::uint64_t(attr.stride) : std::uint64_t(attr.size)*4;
            if(!geometry_span(b->data.size(),attr.offset,1,stride,std::uint64_t(attr.size)*4)) continue;
            std::array<std::uint32_t,4> raw{};
            std::memcpy(raw.data(),b->data.data()+attr.offset,std::size_t(attr.size)*4);
            std::array<double,4> v{};
            for(int i=0;i<attr.size;++i) {
                if(is_float) { float f; std::memcpy(&f,&raw[unsigned(i)],4); v[unsigned(i)]=f; }
                else if(attr.type==GL_INT) v[unsigned(i)]=double(std::int32_t(raw[unsigned(i)]));
                else v[unsigned(i)]=double(raw[unsigned(i)]);
            }
            log::info("dkgl panel instance input {} (location {}): [{}, {}, {}, {}] of {} values",
                input.name,input.location,v[0],v[1],v[2],v[3],attr.size);
        }
    }
    // Decode float inputs safely. Report the first fetched vertex and instance
    // values so NaNs or wrong CPU data can be distinguished from shader faults.
    for(unsigned loc:{0u,4u,5u,6u,7u,9u,11u}) {
        const auto& attr=va.attributes[loc];
        const auto* b=attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
        if(!b || attr.type!=GL_FLOAT || attr.size<1 || attr.size>4 || !range.any) continue;
        const auto stride=attr.stride ? std::uint64_t(attr.stride) : std::uint64_t(attr.size)*4;
        const auto index=attr.divisor ? 0 : std::uint64_t(range.first);
        if(!geometry_span(b->data.size(),attr.offset,index+1,stride,std::uint64_t(attr.size)*4)) continue;
        std::array<float,4> value{};
        std::memcpy(value.data(),b->data.data()+attr.offset+index*stride,std::size_t(attr.size)*4);
        bool finite=true;
        for(int i=0;i<attr.size;++i) finite &= std::isfinite(value[unsigned(i)]);
        log::info("dkgl geometry input {}: buffer {} version {} stride {} offset {} value [{}, {}, {}, {}] finite {}",
            loc,attr.buffer,b->version,stride,attr.offset,value[0],value[1],value[2],value[3],finite);
    }
    return valid;
}

// ---- A real skinned draw, recomputed on the CPU ----
//
// OPENRAC_DK_DRAW_TRACE=N (default 0): the first N large skinned draws (one per vertex
// array, at most one each 90 frames) are traced before they are recorded:
//   - the joint palette the GPU holds (read back) against what the CPU sent (Texture::shadow);
//   - every vertex of the draw skinned on the CPU as mesh.vert skins it, with the draw's own
//     indices, positions, joints, weights, instance matrix, palette base and view_projection;
//   - where the skinned model lands (world box, clip-space coverage) against the same vertices
//     unskinned (the bind-pose diagnostic drew the landing ship whole).
// A palette the GPU holds differently, or a CPU result that is a whole model on screen while the
// GPU draws pieces, says where the fault is. Each trace waits for the GPU once.
int& draw_traces_left() {
    static int left = [] {
        const char* v = std::getenv("OPENRAC_DK_DRAW_TRACE");
        return v != nullptr ? std::max(0, std::atoi(v)) : 0;
    }();
    return left;
}

// Component values of `attr` for vertex (or instance) `index`, as the vertex fetch converts them.
bool fetch(const Attribute& attr, std::uint64_t index, std::array<float, 4>& out) {
    out = {0, 0, 0, 1};
    const Buffer* b = attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
    if (b == nullptr) {
        return false;
    }
    const int n = std::clamp(attr.size, 1, 4);
    const int size = type_bytes(attr.type);
    const std::uint64_t stride = attr.stride != 0 ? std::uint64_t(attr.stride) : std::uint64_t(n * size);
    const std::uint64_t at = attr.offset + index * stride;
    if (at + std::uint64_t(n * size) > b->data.size()) {
        return false;
    }
    const std::uint8_t* p = b->data.data() + at;
    for (int i = 0; i < n; ++i) {
        float v = 0;
        switch (attr.type) {
            case GL_FLOAT: std::memcpy(&v, p + i * 4, 4); break;
            case GL_UNSIGNED_BYTE: v = float(p[i]) / (attr.normalized && !attr.integer ? 255.0f : 1.0f); break;
            case kGlByte: v = float(std::int8_t(p[i])) / (attr.normalized && !attr.integer ? 127.0f : 1.0f); break;
            case GL_UNSIGNED_SHORT: { std::uint16_t u; std::memcpy(&u, p + i * 2, 2); v = float(u) / (attr.normalized && !attr.integer ? 65535.0f : 1.0f); break; }
            case GL_SHORT: { std::int16_t u; std::memcpy(&u, p + i * 2, 2); v = float(u) / (attr.normalized && !attr.integer ? 32767.0f : 1.0f); break; }
            case GL_UNSIGNED_INT: { std::uint32_t u; std::memcpy(&u, p + i * 4, 4); v = float(u); break; }
            case GL_INT: { std::int32_t u; std::memcpy(&u, p + i * 4, 4); v = float(u); break; }
            default: return false;
        }
        out[std::size_t(i)] = v;
    }
    return true;
}

void trace_skinned_draw(const Buffer& elements, std::uintptr_t offset, GLsizei count, unsigned bytes, GLsizei instances) {
    static std::uint64_t last_frame = 0;
    static std::vector<GLuint> traced;
    if (draw_traces_left() <= 0 || count < 600 || (last_frame != 0 && g->frame < last_frame + 90)) {
        return;
    }
    Program* p = current_program();
    if (p == nullptr || p->skinning_offset < 0 || p->skinning_offset + 4 > int(p->uniforms.size())) {
        return;
    }
    std::int32_t skinning = 0;
    std::memcpy(&skinning, p->uniforms.data() + p->skinning_offset, 4);
    const VertexArray& va = vertex_array();
    std::array<float, 4> base{};
    if (skinning == 0 || !fetch(va.attributes[9], 0, base) || base[0] < 0
        || std::find(traced.begin(), traced.end(), g->state.vertex_array) != traced.end()) {
        return;
    }
    // The palette: the sampler uniform's unit, as prepare_draw binds it.
    Texture* palette = nullptr;
    std::array<float, 16> vp{};
    bool have_vp = false;
    for (std::size_t i = 0; i < p->reflection.uniforms.size(); ++i) {
        const Uniform& u = p->reflection.uniforms[i];
        if (u.sampler && u.name == "palette_texture") {
            const int unit = i < p->units.size() ? p->units[i] : 0;
            palette = unit >= 0 && unit < int(kUnits) ? g->textures.find(g->state.textures[unsigned(unit)]) : nullptr;
        } else if (u.name == "view_projection" && u.offset >= 0 && u.offset + 64 <= int(p->uniforms.size())) {
            std::memcpy(vp.data(), p->uniforms.data() + u.offset, 64);
            have_vp = true;
        }
    }
    if (palette == nullptr || !palette->image || !have_vp) {
        return;
    }
    --draw_traces_left();
    last_frame = g->frame;
    traced.push_back(g->state.vertex_array);
    Image& image = *palette->image;
    const auto& shadow = palette->shadow;
    log::info("dkgl draw trace: frame {} program {:016x} vao {} count {} offset {} instances {} palette texture {} {}x{} format {} base {} shadow {}",
              g->frame, p->key, g->state.vertex_array, count, std::uint64_t(offset), instances, g->state.textures[1],
              image.width, image.height, int(image.format), base[0], shadow.size() / 4);
    // 1. The palette as the GPU holds it, once this frame's uploads so far have run.
    if (image.format == DkImageFormat_RGBA32_Float && !shadow.empty()) {
        const auto pitch = std::uint32_t(align_up(std::uint64_t(image.width) * 16, DK_IMAGE_LINEAR_STRIDE_ALIGNMENT));
        const RingSlice s = ring(pitch * std::uint32_t(image.height), DK_IMAGE_LINEAR_STRIDE_ALIGNMENT);
        if (s.cpu != nullptr) {
            barrier(DkBarrier_Full, DkInvalidateFlags_Image);
            ++g->counters.full_readback;
            DkImageView view;
            dkImageViewDefaults(&view, &image.image);
            const DkImageRect rect{0, 0, 0, std::uint32_t(image.width), std::uint32_t(image.height), 1};
            const DkCopyBuf dst{s.gpu, pitch, 0};
            dkCmdBufCopyImageToBuffer(cmd(), &view, &rect, &dst, 0);
            dkCmdBufBarrier(cmd(), DkBarrier_Full, DkInvalidateFlags_L2Cache);
            flush_and_wait();
            std::size_t differ = 0, first = ~std::size_t(0);
            for (int y = 0; y < image.height; ++y) {
                for (int x = 0; x < image.width; ++x) {
                    const float* gpu = reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(s.cpu) + std::size_t(y) * pitch) + std::size_t(x) * 4;
                    const float* cpu = shadow.data() + (std::size_t(y) * std::size_t(image.width) + std::size_t(x)) * 4;
                    if (std::memcmp(gpu, cpu, 16) != 0) {
                        if (differ++ == 0) first = std::size_t(y) * std::size_t(image.width) + std::size_t(x);
                    }
                }
            }
            if (differ == 0) {
                log::info("dkgl draw trace: GPU palette equals the CPU's, all {} texels", shadow.size() / 4);
            } else {
                const float* gpu = reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(s.cpu) + (first / std::size_t(image.width)) * pitch) + (first % std::size_t(image.width)) * 4;
                const float* cpu = shadow.data() + first * 4;
                log::error("dkgl draw trace: GPU palette DIFFERS at {} of {} texels; first texel {}: GPU [{}, {}, {}, {}] CPU [{}, {}, {}, {}]",
                           differ, shadow.size() / 4, first, gpu[0], gpu[1], gpu[2], gpu[3], cpu[0], cpu[1], cpu[2], cpu[3]);
            }
        }
    }
    // 2. Every vertex of the draw, instance 0, skinned as mesh.vert does.
    std::array<float, 16> model{};
    for (unsigned c = 0; c < 4; ++c) {
        std::array<float, 4> column{};
        fetch(va.attributes[4 + c], 0, column);
        std::copy(column.begin(), column.end(), model.begin() + c * 4);
    }
    const auto mul = [](const std::array<float, 16>& m, const std::array<float, 4>& v) {
        std::array<float, 4> r{};
        for (int row = 0; row < 4; ++row)
            r[std::size_t(row)] = m[std::size_t(row)] * v[0] + m[std::size_t(4 + row)] * v[1] + m[std::size_t(8 + row)] * v[2] + m[std::size_t(12 + row)] * v[3];
        return r;
    };
    const int palette_matrices = int(shadow.size() / 16);
    const auto joint = [&](int index, std::array<float, 16>& m) {
        if (index < 0 || index >= palette_matrices) { m.fill(0); return false; }
        std::copy_n(shadow.data() + std::size_t(index) * 16, 16, m.begin());
        return true;
    };
    struct Box { float lo[3] = {3e38f, 3e38f, 3e38f}, hi[3] = {-3e38f, -3e38f, -3e38f}; void add(const std::array<float, 4>& v) { for (int k = 0; k < 3; ++k) { lo[k] = std::min(lo[k], v[std::size_t(k)]); hi[k] = std::max(hi[k], v[std::size_t(k)]); } } };
    Box bind_world, skin_world, skin_ndc;
    std::vector<std::uint32_t> unique;
    unique.reserve(std::size_t(count));
    const bool restart = cap(GL_PRIMITIVE_RESTART);
    const auto sentinel = bytes == 1 ? 0xffu : bytes == 2 ? 0xffffu : 0xffffffffu;
    for (GLsizei i = 0; i < count; ++i) {
        std::uint32_t index = 0;
        std::memcpy(&index, elements.data.data() + offset + std::size_t(i) * bytes, bytes);
        if (!(restart && index == sentinel)) unique.push_back(index);
    }
    std::sort(unique.begin(), unique.end());
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    unsigned nonfinite = 0, behind = 0, inside = 0, out_of_palette = 0, unweighted = 0, max_joint = 0, shown = 0;
    float weight_min = 1e30f, weight_max = -1e30f;
    for (std::uint32_t index : unique) {
        std::array<float, 4> pos{}, joints{}, weights{};
        fetch(va.attributes[0], index, pos);
        pos[3] = 1;
        fetch(va.attributes[10], index, joints);
        fetch(va.attributes[11], index, weights);
        bind_world.add(mul(model, pos));
        const float total = weights[0] + weights[1] + weights[2] + weights[3];
        weight_min = std::min(weight_min, total);
        weight_max = std::max(weight_max, total);
        std::array<float, 4> skinned = pos;
        if (total > 0) {
            std::array<float, 16> skin{}, m{};
            for (int k = 0; k < 4; ++k) {
                const int j = int(base[0] + 0.5f) + int(joints[std::size_t(k)]);
                max_joint = std::max(max_joint, unsigned(std::max(j, 0)));
                if (!joint(j, m)) ++out_of_palette;
                for (int e = 0; e < 16; ++e) skin[std::size_t(e)] += weights[std::size_t(k)] * m[std::size_t(e)];
            }
            skinned = mul(skin, pos);
            skinned[3] = 1;
        } else {
            ++unweighted;
        }
        const auto world = mul(model, skinned);
        const auto clip = mul(vp, world);
        bool finite = true;
        for (float v : clip) finite &= std::isfinite(v);
        if (!finite) { ++nonfinite; continue; }
        skin_world.add(world);
        if (clip[3] <= 0) { ++behind; continue; }
        const std::array<float, 4> ndc{clip[0] / clip[3], clip[1] / clip[3], clip[2] / clip[3], 1};
        skin_ndc.add(ndc);
        inside += std::abs(ndc[0]) <= 1 && std::abs(ndc[1]) <= 1;
        if (shown++ < 4) {
            log::info("dkgl draw trace: vertex {} pos [{}, {}, {}] joints [{}, {}, {}, {}] weights [{}, {}, {}, {}] skinned [{}, {}, {}] world [{}, {}, {}] ndc [{}, {}]",
                      index, pos[0], pos[1], pos[2], joints[0], joints[1], joints[2], joints[3], weights[0], weights[1], weights[2], weights[3],
                      skinned[0], skinned[1], skinned[2], world[0], world[1], world[2], ndc[0], ndc[1]);
        }
    }
    log::info("dkgl draw trace: {} vertices ({}..{}); weight sums {}..{}, {} unweighted; joints up to {} of {} palette matrices, {} lookups outside it",
              unique.size(), unique.empty() ? 0 : unique.front(), unique.empty() ? 0 : unique.back(), weight_min, weight_max, unweighted,
              max_joint, palette_matrices, out_of_palette);
    log::info("dkgl draw trace: bind-pose world box [{}, {}, {}]..[{}, {}, {}]; skinned world box [{}, {}, {}]..[{}, {}, {}]",
              bind_world.lo[0], bind_world.lo[1], bind_world.lo[2], bind_world.hi[0], bind_world.hi[1], bind_world.hi[2],
              skin_world.lo[0], skin_world.lo[1], skin_world.lo[2], skin_world.hi[0], skin_world.hi[1], skin_world.hi[2]);
    log::info("dkgl draw trace: CPU-skinned clip result: {} on screen, {} behind the camera, {} non-finite; on-screen NDC box [{}, {}]..[{}, {}]",
              inside, behind, nonfinite, skin_ndc.lo[0], skin_ndc.lo[1], skin_ndc.hi[0], skin_ndc.hi[1]);
}

// ---- Skinning on the CPU ----
//
// OPENRAC_DK_CPU_SKINNING=1 (default off: the shader skins). Hardware run 2026-10-29 04:21: CPU
// skinning changed nothing visible and the GPU palette matched the CPU's, so GPU skinning is not
// the fault; kept as an A/B switch. A skinned mesh draw is
// skinned here, with mesh.vert's arithmetic, from the palette the CPU sent (Texture::shadow): the
// GPU gets finished positions and normals and the draw's skinning uniform is off. The hardware
// runs (2026-10-29) drew the landing ship whole only with skinning bypassed (bind-pose
// diagnostic); this keeps the animation and takes the GPU's joint fetch out of the picture.
bool cpu_skinning() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_DK_CPU_SKINNING");
        const bool enabled = v != nullptr && std::string_view(v) == "1";
        log::info("dkgl: skinned meshes skinned on the {} (OPENRAC_DK_CPU_SKINNING={})", enabled ? "CPU" : "GPU", enabled ? 1 : 0);
        return enabled;
    }();
    return on;
}

bool cpu_skinned_draw(GLenum mode, const Buffer& elements, std::uintptr_t offset, GLsizei count, unsigned bytes, GLsizei instances) {
    if (!cpu_skinning()) {
        return false;
    }
    Program* p = current_program();
    if (p == nullptr || p->skinning_offset < 0 || p->skinning_offset + 4 > int(p->uniforms.size())) {
        return false;
    }
    std::int32_t skinning = 0;
    std::memcpy(&skinning, p->uniforms.data() + p->skinning_offset, 4);
    if (skinning == 0) {
        return false;
    }
    // Inputs as mesh.vert reads them: position 0, normal 1, palette 9, joints 10, weights 11.
    bool has_normal = false, has_position = false;
    for (const auto& a : p->reflection.attributes) {
        has_position |= a.location == 0;
        has_normal |= a.location == 1;
    }
    const VertexArray& va = vertex_array();
    if (!has_position || !va.attributes[0].enabled || va.attributes[0].divisor != 0 || !va.attributes[9].enabled
        || !va.attributes[10].enabled || !va.attributes[11].enabled) {
        return false;
    }
    const Texture* palette = nullptr;
    for (std::size_t i = 0; i < p->reflection.uniforms.size(); ++i) {
        const Uniform& u = p->reflection.uniforms[i];
        if (u.sampler && u.name == "palette_texture") {
            const int unit = i < p->units.size() ? p->units[i] : 0;
            palette = unit >= 0 && unit < int(kUnits) ? g->textures.find(g->state.textures[unsigned(unit)]) : nullptr;
        }
    }
    if (palette == nullptr || palette->shadow.empty()) {
        return false;
    }
    IndexRange range;
    if (!geometry_indices(elements.data, offset, std::uint64_t(count), bytes, cap(GL_PRIMITIVE_RESTART), range) || !range.any
        || range.last - range.first >= (1u << 20)) {
        return false;
    }
    const std::uint32_t vertices = range.last - range.first + 1;
    const int matrices = int(palette->shadow.size() / 16);
    const float* shadow = palette->shadow.data();
    // The game's own layout reads directly: float positions, normals and weights, byte joints.
    const Attribute& a_pos = va.attributes[0];
    const Attribute& a_nrm = va.attributes[1];
    const Attribute& a_joint = va.attributes[10];
    const Attribute& a_weight = va.attributes[11];
    const Buffer* vb = g->buffers.find(a_pos.buffer);
    const auto direct = [&](const Attribute& a, GLenum type, int size) {
        const Buffer* b = a.enabled ? g->buffers.find(a.buffer) : nullptr;
        const int bytes_each = type == GL_FLOAT ? 4 : 1;
        const std::uint64_t stride = a.stride != 0 ? std::uint64_t(a.stride) : std::uint64_t(size * bytes_each);
        return b != nullptr && a.type == type && a.size >= size && a.divisor == 0
               && a.offset + std::uint64_t(range.last) * stride + std::uint64_t(size * bytes_each) <= b->data.size();
    };
    const bool fast = vb != nullptr && direct(a_pos, GL_FLOAT, 3) && (!has_normal || direct(a_nrm, GL_FLOAT, 3))
                      && direct(a_joint, GL_UNSIGNED_BYTE, 4) && direct(a_weight, GL_FLOAT, 4);
    const auto at = [&](const Attribute& a, int size_bytes, std::uint64_t index) {
        const std::uint64_t stride = a.stride != 0 ? std::uint64_t(a.stride) : std::uint64_t(size_bytes);
        return g->buffers.find(a.buffer)->data.data() + a.offset + index * stride;
    };
    // Skinned this frame already (another material of the same model): the same vertices.
    struct Skinned {
        std::uint64_t ring_epoch, palette_version, vertex_version;
        GLuint vertex_buffer;
        const Texture* palette;
        int base;
        std::uint32_t first, last;
        DkGpuAddr addr;
    };
    static std::vector<Skinned> cache;
    static std::uint64_t cache_epoch = 0;
    if (cache_epoch != g->ring_epoch) {
        cache.clear();
        cache_epoch = g->ring_epoch;
    }
    static unsigned told = 0;
    for (GLsizei instance = 0; instance < instances; ++instance) {
        std::array<float, 4> base{};
        fetch(va.attributes[9], std::uint64_t(instance) / std::max(1u, va.attributes[9].divisor), base);
        const int first_matrix = int(base[0] + 0.5f);
        DkGpuAddr skinned = 0;
        for (const Skinned& s : cache) {
            if (s.ring_epoch == g->ring_epoch && s.palette == palette && s.palette_version == palette->shadow_version
                && s.vertex_buffer == a_pos.buffer && vb != nullptr && s.vertex_version == vb->version
                && s.base == (base[0] >= 0 ? first_matrix : -1) && s.first <= range.first && s.last >= range.last) {
                skinned = s.addr + DkGpuAddr(range.first - s.first) * 24;
                break;
            }
        }
        if (skinned == 0) {
        const RingSlice out = ring(vertices * 24, 256);
        if (out.cpu == nullptr) {
            return instance > 0;  // the rest are lost either way; the first falls back to the GPU
        }
        auto* to = static_cast<float*>(out.cpu);
        for (std::uint32_t v = 0; v < vertices; ++v) {
            const std::uint64_t index = std::uint64_t(range.first) + v;
            std::array<float, 4> pos{}, nrm{}, joints{}, weights{};
            if (fast) {
                std::memcpy(pos.data(), at(a_pos, 12, index), 12);
                if (has_normal) std::memcpy(nrm.data(), at(a_nrm, 12, index), 12);
                const std::uint8_t* jb = at(a_joint, 4, index);
                joints = {float(jb[0]), float(jb[1]), float(jb[2]), float(jb[3])};
                std::memcpy(weights.data(), at(a_weight, 16, index), 16);
            } else {
                fetch(a_pos, index, pos);
                if (has_normal) fetch(a_nrm, index, nrm);
                fetch(a_joint, index, joints);
                fetch(a_weight, index, weights);
            }
            float px = pos[0], py = pos[1], pz = pos[2], nx = nrm[0], ny = nrm[1], nz = nrm[2];
            const float total = weights[0] + weights[1] + weights[2] + weights[3];
            if (base[0] >= 0 && total > 0) {
                // mat4 skin = sum of weight x joint matrix (column-major, four texels a matrix);
                // texelFetch past the palette reads zeros on this GPU.
                float m[16] = {};
                for (int k = 0; k < 4; ++k) {
                    const int j = first_matrix + int(joints[std::size_t(k)]);
                    if (j < 0 || j >= matrices) continue;
                    const float* jm = shadow + std::size_t(j) * 16;
                    for (int e = 0; e < 16; ++e) m[e] += weights[std::size_t(k)] * jm[e];
                }
                px = m[0] * pos[0] + m[4] * pos[1] + m[8] * pos[2] + m[12];
                py = m[1] * pos[0] + m[5] * pos[1] + m[9] * pos[2] + m[13];
                pz = m[2] * pos[0] + m[6] * pos[1] + m[10] * pos[2] + m[14];
                nx = m[0] * nrm[0] + m[4] * nrm[1] + m[8] * nrm[2];
                ny = m[1] * nrm[0] + m[5] * nrm[1] + m[9] * nrm[2];
                nz = m[2] * nrm[0] + m[6] * nrm[1] + m[10] * nrm[2];
            }
            float* row = to + std::size_t(v) * 6;
            row[0] = px; row[1] = py; row[2] = pz; row[3] = nx; row[4] = ny; row[5] = nz;
        }
        skinned = out.gpu;
        if (vb != nullptr) {
            cache.push_back({g->ring_epoch, palette->shadow_version, vb->version, a_pos.buffer, palette,
                             base[0] >= 0 ? first_matrix : -1, range.first, range.last, out.gpu});
        }
        }
        auto& o = g->draw_override;
        o.active = true;
        o.normals = has_normal;
        o.first = range.first;
        o.instance = std::uint32_t(instance);
        o.vertices = vertices;
        o.skinned = skinned;
        Program* drawn = nullptr;
        Target t;
        DkCmdBuf c = nullptr;
        const bool ready = prepare_draw(drawn, t, c);
        o.active = false;
        if (!ready) {
            return true;
        }
        const DkGpuAddr index_base = resolve(const_cast<Buffer&>(elements));
        if (index_base == 0) {
            return true;
        }
        const DkIdxFormat format = bytes == 1 ? DkIdxFormat_Uint8 : bytes == 2 ? DkIdxFormat_Uint16 : DkIdxFormat_Uint32;
        bind_index_buffer(c, format, index_base + offset, int(bytes));
        // Indices stay as stored; the vertex offset counts them from `first`.
        dkCmdBufDrawIndexed(c, primitive(mode), std::uint32_t(count), 1, 0, -std::int32_t(range.first), 0);
        if (told < 4) {
            ++told;
            log::info("dkgl: CPU-skinned draw: program {:016x} vao {} count {} vertices {}..{} instance {}/{} palette base {} of {} matrices",
                      p->key, g->state.vertex_array, count, range.first, range.last, instance + 1, instances, first_matrix, matrices);
        }
    }
    return true;
}

void draw_indexed(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instances) {
    Program* p = nullptr;
    Target t;
    DkCmdBuf c = nullptr;
    if (count <= 0 || instances <= 0) {
        return;
    }
    Buffer* elements = g->buffers.find(vertex_array().elements);
    const int bytes = type == GL_UNSIGNED_BYTE ? 1 : type == GL_UNSIGNED_SHORT ? 2 : type == GL_UNSIGNED_INT ? 4 : 0;
    const auto offset = reinterpret_cast<std::uintptr_t>(indices);
    if(elements==nullptr || !bytes || !geometry_span(elements->data.size(),offset,std::uint64_t(count),bytes,bytes)) {
        fail(kGlInvalidValue);
        static unsigned reports=0;
        if(reports++<32) log::error("dkgl geometry: rejected index range offset {} count {} bytes {} buffer bytes {}",offset,count,bytes,elements ? elements->data.size() : 0);
        return;
    }
    trace_skinned_draw(*elements, offset, count, unsigned(bytes), instances);
    if (cpu_skinned_draw(mode, *elements, offset, count, unsigned(bytes), instances)) {
        return;
    }
    if (!prepare_draw(p, t, c)) {
        return;
    }
    if(!audit_geometry(*p,*elements,offset,count,unsigned(bytes),instances)) {
        fail(kGlInvalidValue);
        return;
    }
    const DkGpuAddr base = resolve(*elements);
    if (base == 0) {
        return;
    }
    const DkIdxFormat format = bytes == 1 ? DkIdxFormat_Uint8 : bytes == 2 ? DkIdxFormat_Uint16 : DkIdxFormat_Uint32;
    bind_index_buffer(c, format, base + offset, bytes);
    dkCmdBufDrawIndexed(c, primitive(mode), std::uint32_t(count), std::uint32_t(instances), 0, 0, 0);
}

void OPENRAC_GLAPI draw_elements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    draw_indexed(mode, count, type, indices, 1);
}

void OPENRAC_GLAPI draw_elements_instanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instances) {
    draw_indexed(mode, count, type, indices, instances);
}

// ---- GL: shaders and programs ----

GLuint OPENRAC_GLAPI create_shader(GLenum type) {
    const GLuint name = g->next_shader_name++;
    g->shaders.objects.emplace(name, Shader{type, {}});
    return name;
}

void OPENRAC_GLAPI shader_source(GLuint shader, GLsizei count, const GLchar* const* strings, const GLint* lengths) {
    Shader* s = g->shaders.find(shader);
    if (s == nullptr) {
        return;
    }
    s->source.clear();
    for (GLsizei i = 0; i < count; ++i) {
        if (strings[i] == nullptr) {
            continue;
        }
        if (lengths != nullptr && lengths[i] >= 0) {
            s->source.append(strings[i], std::size_t(lengths[i]));
        } else {
            s->source.append(strings[i]);
        }
    }
}

void OPENRAC_GLAPI compile_shader(GLuint) {}  // checked at link, against the pack

void OPENRAC_GLAPI get_shader_iv(GLuint, GLenum pname, GLint* params) {
    if (params != nullptr) {
        *params = pname == GL_COMPILE_STATUS ? 1 : 0;
    }
}

void OPENRAC_GLAPI get_shader_info_log(GLuint, GLsizei size, GLsizei* length, GLchar* log) {
    if (length != nullptr) {
        *length = 0;
    }
    if (log != nullptr && size > 0) {
        log[0] = '\0';
    }
}

void OPENRAC_GLAPI delete_shader(GLuint shader) {
    g->shaders.objects.erase(shader);
}

GLuint OPENRAC_GLAPI create_program() {
    const GLuint name = g->next_shader_name++;
    g->programs.objects.emplace(name, Program{});
    return name;
}

void OPENRAC_GLAPI attach_shader(GLuint program, GLuint shader) {
    if (Program* p = g->programs.find(program)) {
        p->shaders.push_back(shader);
    }
}

void OPENRAC_GLAPI bind_attrib_location(GLuint, GLuint, const GLchar*) {}

void OPENRAC_GLAPI link_program(GLuint program) {
    Program* p = g->programs.find(program);
    if (p == nullptr) {
        return;
    }
    std::string vertex, fragment;
    for (GLuint name : p->shaders) {
        if (Shader* s = g->shaders.find(name)) {
            (s->type == GL_VERTEX_SHADER ? vertex : fragment) = s->source;
        }
    }
    if (p->code != nullptr) {
        retire(p->code);
        p->code = nullptr;
    }
    p->linked = load_program(*p, vertex, fragment);
}

void OPENRAC_GLAPI get_program_iv(GLuint program, GLenum pname, GLint* params) {
    Program* p = g->programs.find(program);
    if (params == nullptr) {
        return;
    }
    if (pname == GL_LINK_STATUS) {
        *params = p != nullptr && p->linked ? 1 : 0;
    } else if (pname == GL_INFO_LOG_LENGTH) {
        *params = p != nullptr ? GLint(p->log.size() + 1) : 0;
    } else {
        *params = 0;
    }
}

void OPENRAC_GLAPI get_program_info_log(GLuint program, GLsizei size, GLsizei* length, GLchar* log) {
    Program* p = g->programs.find(program);
    const std::string text = p != nullptr ? p->log : std::string();
    const GLsizei n = size > 0 ? std::min<GLsizei>(size - 1, GLsizei(text.size())) : 0;
    if (log != nullptr && size > 0) {
        std::memcpy(log, text.data(), std::size_t(n));
        log[n] = '\0';
    }
    if (length != nullptr) {
        *length = n;
    }
}

void OPENRAC_GLAPI delete_program(GLuint program) {
    if (Program* p = g->programs.find(program)) {
        retire(p->code);
    }
    g->programs.objects.erase(program);
    if (g->state.program == program) {
        g->state.program = 0;
    }
}

void OPENRAC_GLAPI use_program(GLuint program) {
    g->state.program = program;
}

// A uniform's location: its index in the reflection, shifted, and the array element below.
constexpr int kElementBits = 10;

GLint OPENRAC_GLAPI get_uniform_location(GLuint program, const GLchar* name) {
    Program* p = g->programs.find(program);
    if (p == nullptr || !p->linked || name == nullptr) {
        return -1;
    }
    std::string_view n(name);
    int element = 0;
    if (const std::size_t bracket = n.find('['); bracket != std::string_view::npos) {
        element = std::atoi(std::string(n.substr(bracket + 1)).c_str());
        n = n.substr(0, bracket);
    }
    const auto& list = p->reflection.uniforms;
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (list[i].name == n && element >= 0 && element < list[i].array_size) {
            return GLint(int(i) << kElementBits | element);
        }
    }
    return -1;
}

// Where a location's value goes in the current program's default block; null for none (or a sampler).
std::uint8_t* uniform_at(GLint location, int bytes, const Uniform** out = nullptr, int* index = nullptr) {
    Program* p = current_program();
    if (p == nullptr || location < 0) {
        return nullptr;
    }
    const auto i = std::size_t(location >> kElementBits);
    const int element = location & ((1 << kElementBits) - 1);
    if (i >= p->reflection.uniforms.size()) {
        return nullptr;
    }
    const Uniform& u = p->reflection.uniforms[i];
    if (out != nullptr) {
        *out = &u;
    }
    if (index != nullptr) {
        *index = int(i);
    }
    if (u.sampler || u.offset < 0) {
        return nullptr;
    }
    const std::size_t at = std::size_t(u.offset) + std::size_t(element) * std::size_t(std::max(u.array_stride, 0));
    if (at + std::size_t(bytes) > p->uniforms.size()) {
        return nullptr;
    }
    return p->uniforms.data() + at;
}

void write_uniform(std::uint8_t* to, const void* from, std::size_t bytes) {
    if (std::memcmp(to, from, bytes) != 0) {
        std::memcpy(to, from, bytes);
        if (Program* p = current_program()) {
            p->uniforms_dirty = true;
        }
    }
}

void OPENRAC_GLAPI uniform_1i(GLint location, GLint v) {
    const Uniform* u = nullptr;
    int index = -1;
    if (std::uint8_t* at = uniform_at(location, 4, &u, &index)) {
        write_uniform(at, &v, 4);
    } else if (u != nullptr && u->sampler && index >= 0 && std::size_t(index) < 64) {
        if (Program* p = current_program()) {
            p->units[std::size_t(index)] = v;
        }
    }
}

void OPENRAC_GLAPI uniform_1f(GLint location, GLfloat v) {
    if (std::uint8_t* at = uniform_at(location, 4)) {
        write_uniform(at, &v, 4);
    }
}

void OPENRAC_GLAPI uniform_2f(GLint location, GLfloat a, GLfloat b) {
    const GLfloat v[2] = {a, b};
    if (std::uint8_t* at = uniform_at(location, 8)) {
        write_uniform(at, v, 8);
    }
}

void OPENRAC_GLAPI uniform_3f(GLint location, GLfloat a, GLfloat b, GLfloat c) {
    const GLfloat v[3] = {a, b, c};
    if (std::uint8_t* at = uniform_at(location, 12)) {
        write_uniform(at, v, 12);
    }
}

void OPENRAC_GLAPI uniform_4f(GLint location, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
    const GLfloat v[4] = {a, b, c, d};
    if (std::uint8_t* at = uniform_at(location, 16)) {
        write_uniform(at, v, 16);
    }
}

void OPENRAC_GLAPI uniform_4fv(GLint location, GLsizei count, const GLfloat* v) {
    for (GLsizei i = 0; v != nullptr && location >= 0 && i < count; ++i) {
        if (std::uint8_t* at = uniform_at(location + i, 16)) {
            write_uniform(at, v + i * 4, 16);
        }
    }
}

void OPENRAC_GLAPI uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* v) {
    for (GLsizei i = 0; v != nullptr && location >= 0 && i < count; ++i) {
        std::uint8_t* at = uniform_at(location + i, 64);
        if (at == nullptr) {
            continue;
        }
        const GLfloat* m = v + i * 16;
        if (transpose == GL_FALSE) {
            write_uniform(at, m, 64);
        } else {
            float t[16];
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    t[c * 4 + r] = m[r * 4 + c];
                }
            }
            write_uniform(at, t, 64);
        }
    }
}

GLuint OPENRAC_GLAPI get_uniform_block_index(GLuint, const GLchar*) {
    return 0xFFFFFFFFu;  // GL_INVALID_INDEX: the game's programs have no named blocks
}

void OPENRAC_GLAPI uniform_block_binding(GLuint, GLuint, GLuint) {}

// ---- GL: framebuffers and renderbuffers ----

void OPENRAC_GLAPI gen_framebuffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->framebuffers.make();
    }
}

void OPENRAC_GLAPI delete_framebuffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->framebuffers.objects.erase(names[i]);
        if (g->state.draw_framebuffer == names[i]) {
            g->state.draw_framebuffer = 0;
        }
        if (g->state.read_framebuffer == names[i]) {
            g->state.read_framebuffer = 0;
        }
    }
    g->target_bound = false;  // a deleted target's images may be released
}

void OPENRAC_GLAPI bind_framebuffer(GLenum target, GLuint name) {
    if (name != 0 && g->framebuffers.find(name) == nullptr) {
        g->framebuffers.objects.emplace(name, Framebuffer{});
    }
    if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER) {
        g->state.draw_framebuffer = name;
    }
    if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER) {
        g->state.read_framebuffer = name;
    }
}

Framebuffer* framebuffer_for(GLenum target) {
    const GLuint name = target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer;
    return g->framebuffers.find(name);
}

void OPENRAC_GLAPI framebuffer_texture_2d(GLenum target, GLenum attachment, GLenum, GLuint texture, GLint) {
    Framebuffer* f = framebuffer_for(target);
    if (f == nullptr) {
        return;
    }
    Attachment& a = attachment == GL_COLOR_ATTACHMENT0 ? f->colour : f->depth;
    a = Attachment{texture, 0};
    g->target_bound = false;
}

void OPENRAC_GLAPI framebuffer_renderbuffer(GLenum target, GLenum attachment, GLenum, GLuint renderbuffer) {
    Framebuffer* f = framebuffer_for(target);
    if (f == nullptr) {
        return;
    }
    Attachment& a = attachment == GL_COLOR_ATTACHMENT0 ? f->colour : f->depth;
    a = Attachment{0, renderbuffer};
    g->target_bound = false;
}

GLenum OPENRAC_GLAPI check_framebuffer_status(GLenum target) {
    const Target t = current_target(target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer);
    return t.colour != nullptr || t.depth != nullptr ? GL_FRAMEBUFFER_COMPLETE : 0x8CD6;
}

void OPENRAC_GLAPI gen_renderbuffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->renderbuffers.make();
    }
}

void OPENRAC_GLAPI delete_renderbuffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->renderbuffers.objects.erase(names[i]);
    }
    g->target_bound = false;
}

void OPENRAC_GLAPI bind_renderbuffer(GLenum, GLuint name) {
    if (name != 0 && g->renderbuffers.find(name) == nullptr) {
        g->renderbuffers.objects.emplace(name, Renderbuffer{});
    }
    g->state.renderbuffer = name;
}

void OPENRAC_GLAPI renderbuffer_storage(GLenum, GLenum format, GLsizei w, GLsizei h) {
    Renderbuffer* r = g->renderbuffers.find(g->state.renderbuffer);
    const Format f = format_of(format);
    if (r == nullptr || f.dk == DkImageFormat_None) {
        return;
    }
    r->image = make_image(f.dk, w, h, 1, true);
    g->target_bound = false;
}

void OPENRAC_GLAPI renderbuffer_storage_multisample(GLenum target, GLsizei, GLenum format, GLsizei w, GLsizei h) {
    renderbuffer_storage(target, format, w, h);  // single-sampled: GL_MAX_SAMPLES says 1
}

void OPENRAC_GLAPI blit_framebuffer(GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0,
                                    GLint dx1, GLint dy1, GLbitfield mask, GLenum filter) {
    const Target from = current_target(g->state.read_framebuffer);
    const Target to = current_target(g->state.draw_framebuffer);
    transfer_fence();
    barrier(DkBarrier_Fragments, DkInvalidateFlags_Image);
    const auto rect = [](GLint a0, GLint b0, GLint a1, GLint b1, std::uint32_t& flags, std::uint32_t flip) {
        if (a1 < a0) {
            std::swap(a0, a1);
            flags ^= flip & DkBlitFlag_FlipX;
        }
        if (b1 < b0) {
            std::swap(b0, b1);
            flags ^= flip & DkBlitFlag_FlipY;
        }
        return DkImageRect{std::uint32_t(std::max(a0, 0)), std::uint32_t(std::max(b0, 0)), 0,
                           std::uint32_t(std::max(a1 - a0, 0)), std::uint32_t(std::max(b1 - b0, 0)), 1};
    };
    std::uint32_t flags = filter == GL_LINEAR ? DkBlitFlag_FilterLinear : DkBlitFlag_FilterNearest;
    const DkImageRect a = rect(sx0, sy0, sx1, sy1, flags, DkBlitFlag_FlipX | DkBlitFlag_FlipY);
    const DkImageRect b = rect(dx0, dy0, dx1, dy1, flags, DkBlitFlag_FlipX | DkBlitFlag_FlipY);
    const bool colour = (mask & kGlColorBufferBit) != 0 && from.colour != nullptr && to.colour != nullptr
                        && from.colour != to.colour;
    // Colour goes through the 2D engine, a 1:1 nearest blit for a same-sized copy: it is ordered
    // with the draws by the fragment barrier above. The copy engine would need a Full barrier on each
    // side (the frontend copies the world like this every frame, for the menus' background).
    // OPENRAC_DK_CE_COPY=1: same-sized colour copies on the copy engine, as before. Depth copies
    // stay on the copy engine either way.
    static const bool ce_copy = [] {
        const char* v = std::getenv("OPENRAC_DK_CE_COPY");
        return v != nullptr && std::string_view(v) == "1";
    }();
    const bool colour_copy = ce_copy && colour && a.width == b.width && a.height == b.height
                             && from.colour->format == to.colour->format && (flags & DkBlitFlag_Flip_Mask) == 0;
    const bool depth_copy = (mask & (kGlDepthBufferBit | kGlStencilBufferBit)) != 0 && from.depth != nullptr
                            && to.depth != nullptr && from.depth != to.depth && a.width == b.width
                            && a.height == b.height && from.depth->format == to.depth->format;
    if (colour_copy) {
        before_transfer(from.colour, to.colour, CopySource::BlitColour);
    }
    if (depth_copy) {
        before_transfer(from.depth, to.depth, CopySource::BlitDepth);
    }
    DkCmdBuf c = cmd();
    if (colour) {
        DkImageView src, dst;
        dkImageViewDefaults(&src, &from.colour->image);
        dkImageViewDefaults(&dst, &to.colour->image);
        if (colour_copy) {
            dkCmdBufCopyImage(c, &src, &a, &dst, &b, 0);
        } else {
            dkCmdBufBlitImage(c, &src, &a, &dst, &b, flags, 0);
            touch(from.colour);
            touch(to.colour);
        }
    }
    if (depth_copy) {
        DkImageView src, dst;
        dkImageViewDefaults(&src, &from.depth->image);
        dkImageViewDefaults(&dst, &to.depth->image);
        dkCmdBufCopyImage(c, &src, &a, &dst, &b, 0);
    }
    // One ordering for both copies; each is counted by its source.
    if (colour_copy) {
        after_transfer(CopySource::BlitColour);
    }
    if (depth_copy) {
        after_transfer(CopySource::BlitDepth);
    }
    g->need_image_barrier = true;
    g->target_bound = false;  // re-bind (and order) before the next draw into either
}

void OPENRAC_GLAPI invalidate_framebuffer(GLenum, GLsizei count, const GLenum* attachments) {
    Target t;
    if (!bind_target(t) || attachments == nullptr) {
        return;
    }
    DkCmdBuf c = cmd();
    for (GLsizei i = 0; i < count; ++i) {
        const GLenum a = attachments[i];
        if ((a == GL_COLOR_ATTACHMENT0 || a == kGlColor) && t.colour != nullptr) {
            dkCmdBufDiscardColor(c, 0);
        } else if ((a == GL_DEPTH_ATTACHMENT || a == GL_DEPTH_STENCIL_ATTACHMENT || a == GL_STENCIL_ATTACHMENT
                    || a == kGlDepth || a == kGlStencil) && t.depth != nullptr) {
            dkCmdBufDiscardDepthStencil(c);
        }
    }
}

void OPENRAC_GLAPI read_pixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum, GLenum, void* pixels) {
    const Target from = current_target(g->state.read_framebuffer);
    if (from.colour == nullptr || pixels == nullptr || w <= 0 || h <= 0 || from.colour->format != DkImageFormat_RGBA8_Unorm) {
        return;
    }
    const std::uint32_t pitch = std::uint32_t(align_up(std::size_t(w) * 4, DK_IMAGE_LINEAR_STRIDE_ALIGNMENT));
    const RingSlice s = ring(pitch * std::uint32_t(h), DK_IMAGE_LINEAR_STRIDE_ALIGNMENT);
    if (s.cpu == nullptr) {
        return;
    }
    barrier(DkBarrier_Full, DkInvalidateFlags_Image);
    ++g->counters.full_readback;
    DkCmdBuf c = cmd();
    DkImageView view;
    dkImageViewDefaults(&view, &from.colour->image);
    const DkImageRect rect{std::uint32_t(std::max(x, 0)), std::uint32_t(std::max(y, 0)), 0, std::uint32_t(w), std::uint32_t(h), 1};
    const DkCopyBuf dst{s.gpu, pitch, 0};
    dkCmdBufCopyImageToBuffer(c, &view, &rect, &dst, 0);
    dkCmdBufBarrier(c, DkBarrier_Full, DkInvalidateFlags_L2Cache);
    // Waited for here: the ring's bytes stay as the GPU wrote them until the ring's next use.
    flush_and_wait();
    const auto* keep = static_cast<const std::uint8_t*>(s.cpu);
    const std::size_t out_pitch = align_up(std::size_t(w) * 4, std::size_t(g->state.pack_alignment));
    for (int row = 0; row < h; ++row) {
        std::memcpy(static_cast<std::uint8_t*>(pixels) + out_pitch * std::size_t(row),
                    keep + std::size_t(pitch) * std::size_t(row), std::size_t(w) * 4);
    }
}

// The table renderer/gl.h's loader reads.
struct Entry {
    const char* name;
    Proc proc;
};

#define DKGL_ENTRY(gl_name, fn) {#gl_name, reinterpret_cast<Proc>(&fn)}
const Entry kEntries[] = {
    DKGL_ENTRY(glTexBuffer, tex_buffer),
    DKGL_ENTRY(glGetUniformBlockIndex, get_uniform_block_index),
    DKGL_ENTRY(glUniformBlockBinding, uniform_block_binding),
    DKGL_ENTRY(glBindBufferBase, bind_buffer_base),
    DKGL_ENTRY(glGetString, get_string),
    DKGL_ENTRY(glGetStringi, get_string_i),
    DKGL_ENTRY(glGetIntegerv, get_integer_v),
    DKGL_ENTRY(glGetFloatv, get_float_v),
    DKGL_ENTRY(glGetError, get_error),
    DKGL_ENTRY(glEnable, enable),
    DKGL_ENTRY(glDisable, disable),
    DKGL_ENTRY(glClear, clear),
    DKGL_ENTRY(glClearColor, clear_colour),
    DKGL_ENTRY(glClearDepth, clear_depth),
    DKGL_ENTRY(glClearStencil, clear_stencil),
    DKGL_ENTRY(glViewport, viewport),
    DKGL_ENTRY(glScissor, scissor),
    DKGL_ENTRY(glDepthFunc, depth_func),
    DKGL_ENTRY(glDepthMask, depth_mask),
    DKGL_ENTRY(glStencilFunc, stencil_func),
    DKGL_ENTRY(glStencilOpSeparate, stencil_op_separate),
    DKGL_ENTRY(glStencilMask, stencil_mask),
    DKGL_ENTRY(glColorMask, colour_mask),
    DKGL_ENTRY(glBlendFuncSeparate, blend_func_separate),
    DKGL_ENTRY(glBlendEquation, blend_equation),
    DKGL_ENTRY(glBlendColor, blend_colour),
    DKGL_ENTRY(glCullFace, cull_face),
    DKGL_ENTRY(glFrontFace, front_face),
    DKGL_ENTRY(glPolygonMode, polygon_mode),
    DKGL_ENTRY(glPixelStorei, pixel_store_i),
    DKGL_ENTRY(glReadPixels, read_pixels),
    DKGL_ENTRY(glCopyTexSubImage2D, copy_tex_sub_image_2d),
    DKGL_ENTRY(glFinish, finish),
    DKGL_ENTRY(glGenTextures, gen_textures),
    DKGL_ENTRY(glDeleteTextures, delete_textures),
    DKGL_ENTRY(glBindTexture, bind_texture),
    DKGL_ENTRY(glActiveTexture, active_texture),
    DKGL_ENTRY(glTexImage2D, tex_image_2d),
    DKGL_ENTRY(glTexSubImage2D, tex_sub_image_2d),
    DKGL_ENTRY(glTexParameteri, tex_parameter_i),
    DKGL_ENTRY(glTexParameterf, tex_parameter_f),
    DKGL_ENTRY(glGenerateMipmap, generate_mipmap),
    DKGL_ENTRY(glGenSamplers, gen_samplers),
    DKGL_ENTRY(glDeleteSamplers, delete_samplers),
    DKGL_ENTRY(glBindSampler, bind_sampler),
    DKGL_ENTRY(glSamplerParameteri, sampler_parameter_i),
    DKGL_ENTRY(glGenBuffers, gen_buffers),
    DKGL_ENTRY(glDeleteBuffers, delete_buffers),
    DKGL_ENTRY(glBindBuffer, bind_buffer),
    DKGL_ENTRY(glBufferData, buffer_data),
    DKGL_ENTRY(glBufferSubData, buffer_sub_data),
    DKGL_ENTRY(glGenVertexArrays, gen_vertex_arrays),
    DKGL_ENTRY(glDeleteVertexArrays, delete_vertex_arrays),
    DKGL_ENTRY(glBindVertexArray, bind_vertex_array),
    DKGL_ENTRY(glEnableVertexAttribArray, enable_vertex_attrib_array),
    DKGL_ENTRY(glVertexAttribPointer, vertex_attrib_pointer),
    DKGL_ENTRY(glVertexAttribIPointer, vertex_attrib_i_pointer),
    DKGL_ENTRY(glVertexAttribDivisor, vertex_attrib_divisor),
    DKGL_ENTRY(glDrawArrays, draw_arrays),
    DKGL_ENTRY(glDrawElements, draw_elements),
    DKGL_ENTRY(glDrawElementsInstanced, draw_elements_instanced),
    DKGL_ENTRY(glCreateShader, create_shader),
    DKGL_ENTRY(glShaderSource, shader_source),
    DKGL_ENTRY(glCompileShader, compile_shader),
    DKGL_ENTRY(glGetShaderiv, get_shader_iv),
    DKGL_ENTRY(glGetShaderInfoLog, get_shader_info_log),
    DKGL_ENTRY(glDeleteShader, delete_shader),
    DKGL_ENTRY(glCreateProgram, create_program),
    DKGL_ENTRY(glAttachShader, attach_shader),
    DKGL_ENTRY(glBindAttribLocation, bind_attrib_location),
    DKGL_ENTRY(glLinkProgram, link_program),
    DKGL_ENTRY(glGetProgramiv, get_program_iv),
    DKGL_ENTRY(glGetProgramInfoLog, get_program_info_log),
    DKGL_ENTRY(glDeleteProgram, delete_program),
    DKGL_ENTRY(glUseProgram, use_program),
    DKGL_ENTRY(glGetUniformLocation, get_uniform_location),
    DKGL_ENTRY(glUniform1i, uniform_1i),
    DKGL_ENTRY(glUniform1f, uniform_1f),
    DKGL_ENTRY(glUniform2f, uniform_2f),
    DKGL_ENTRY(glUniform3f, uniform_3f),
    DKGL_ENTRY(glUniform4f, uniform_4f),
    DKGL_ENTRY(glUniform4fv, uniform_4fv),
    DKGL_ENTRY(glUniformMatrix4fv, uniform_matrix_4fv),
    DKGL_ENTRY(glGenFramebuffers, gen_framebuffers),
    DKGL_ENTRY(glDeleteFramebuffers, delete_framebuffers),
    DKGL_ENTRY(glBindFramebuffer, bind_framebuffer),
    DKGL_ENTRY(glFramebufferTexture2D, framebuffer_texture_2d),
    DKGL_ENTRY(glFramebufferRenderbuffer, framebuffer_renderbuffer),
    DKGL_ENTRY(glCheckFramebufferStatus, check_framebuffer_status),
    DKGL_ENTRY(glGenRenderbuffers, gen_renderbuffers),
    DKGL_ENTRY(glDeleteRenderbuffers, delete_renderbuffers),
    DKGL_ENTRY(glBindRenderbuffer, bind_renderbuffer),
    DKGL_ENTRY(glRenderbufferStorage, renderbuffer_storage),
    DKGL_ENTRY(glRenderbufferStorageMultisample, renderbuffer_storage_multisample),
    DKGL_ENTRY(glBlitFramebuffer, blit_framebuffer),
    DKGL_ENTRY(glInvalidateFramebuffer, invalidate_framebuffer),
};
#undef DKGL_ENTRY

}  // namespace

Proc gl_proc(const char* name) {
    for (const Entry& e : kEntries) {
        if (std::strcmp(e.name, name) == 0) {
            return e.proc;
        }
    }
    return nullptr;
}

}  // namespace openrac::dkgl::detail
