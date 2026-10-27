// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The GL functions renderer/gl.h's table holds (vkgl_internal.h): state, textures, buffers, vertex
// arrays, draws, shaders and programs, uniforms, samplers and framebuffers, each carried out with
// the rest of the backend; and get_proc (vkgl.h), which hands the table its entries by name.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <volk.h>

#include "renderer/gl.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/shader_compiler.h"
#include "renderer/vulkan/vkgl.h"
#include "renderer/vulkan/vkgl_internal.h"
#include "renderer/world/gl_world.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

// ---- The GL functions ----

#define VKGL OPENRAC_GLAPI

const GLubyte* VKGL get_string(GLenum name) {
    static const char vendor[] = "OpenRAC";
    static const char glsl[] = "4.10";
    switch (name) {
        case GL_VENDOR:
            return reinterpret_cast<const GLubyte*>(vendor);
        case GL_RENDERER:
            return reinterpret_cast<const GLubyte*>(g->gl_renderer.c_str());
        case GL_VERSION:
            return reinterpret_cast<const GLubyte*>(g->gl_version.c_str());
        case GL_SHADING_LANGUAGE_VERSION:
            return reinterpret_cast<const GLubyte*>(glsl);
        default:
            fail(kGlInvalidEnum);
            return nullptr;
    }
}

const GLubyte* VKGL get_stringi(GLenum, GLuint) {
    return nullptr;  // no extensions: none of the GL ones the renderer looks for applies
}

void VKGL get_integerv(GLenum name, GLint* data) {
    const VkPhysicalDeviceLimits& l = g->properties.limits;
    switch (name) {
        case GL_MAX_TEXTURE_SIZE:
            *data = static_cast<GLint>(l.maxImageDimension2D);
            return;
        case GL_SAMPLES:
            *data = static_cast<GLint>(target_of(g->state.draw_framebuffer).samples);
            return;
        case GL_MAX_SAMPLES: {
            const VkSampleCountFlags counts =
                l.framebufferColorSampleCounts & l.framebufferDepthSampleCounts;
            GLint most = 1;
            for (GLint c = 64; c > 1; c >>= 1) {
                if ((counts & static_cast<VkSampleCountFlags>(c)) != 0) {
                    most = c;
                    break;
                }
            }
            *data = most;
            return;
        }
        case GL_NUM_EXTENSIONS:
            *data = 0;
            return;
        case GL_MAJOR_VERSION:
            *data = 4;
            return;
        case GL_MINOR_VERSION:
            *data = 1;
            return;
        case 0x90DD:
            *data = kUniformBindings;
            return;  // GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS
        case kGlUniformBufferOffsetAlignment:
            *data = static_cast<GLint>(l.minUniformBufferOffsetAlignment);
            return;
        case kGlMaxCombinedTextureUnits:
            *data = kUnits;
            return;
        case kGlMaxTextureBufferSize:
            *data =
                static_cast<GLint>(std::min<std::uint32_t>(l.maxTexelBufferElements, 0x7FFFFFFF));
            return;
        case kGlMaxVertexAttribs:
            *data = kAttributes;
            return;
        case kGlMaxUniformBlockSize:
            *data =
                static_cast<GLint>(std::min<std::uint32_t>(l.maxUniformBufferRange, 0x7FFFFFFF));
            return;
        case kGlFramebufferBinding:
            *data = static_cast<GLint>(g->state.draw_framebuffer);
            return;
        case kGlTextureBindingSlot:
            *data = static_cast<GLint>(g->state.unit_2d[g->state.active_unit]);
            return;
        case kGlViewportQuery:
            std::memcpy(data, g->state.viewport, sizeof g->state.viewport);
            return;
        default:
            *data = 0;
            return;
    }
}

void VKGL get_floatv(GLenum name, GLfloat* data) {
    if (name == GL_MAX_TEXTURE_MAX_ANISOTROPY) {
        *data = g->features.samplerAnisotropy ? g->properties.limits.maxSamplerAnisotropy : 1.0f;
        return;
    }
    GLint v = 0;
    get_integerv(name, &v);
    *data = static_cast<GLfloat>(v);
}

GLenum VKGL get_error() {
    return std::exchange(g->error, GL_NO_ERROR);
}

void enable_cap(GLenum cap, bool on) {
    State& s = g->state;
    switch (cap) {
        case GL_BLEND:
            s.blend = on;
            break;
        case GL_DEPTH_TEST:
            s.depth_test = on;
            break;
        case GL_CULL_FACE:
            s.cull = on;
            break;
        case GL_SCISSOR_TEST:
            s.scissor_test = on;
            break;
        case wgl::GL_STENCIL_TEST:
            s.stencil_test = on;
            break;
        case GL_PRIMITIVE_RESTART:
            s.primitive_restart = on;
            break;
        // Clip distances are always on in Vulkan for a shader that writes them; multisampling is
        // the target's.
        default:
            break;
    }
}

void VKGL enable(GLenum cap) {
    enable_cap(cap, true);
}

void VKGL disable(GLenum cap) {
    enable_cap(cap, false);
}

void VKGL clear_gl(GLbitfield mask) {
    clear(mask);
}

void VKGL clear_color(GLfloat r, GLfloat gr, GLfloat b, GLfloat a) {
    g->state.clear_colour[0] = r;
    g->state.clear_colour[1] = gr;
    g->state.clear_colour[2] = b;
    g->state.clear_colour[3] = a;
}

void VKGL clear_depth(GLdouble d) {
    g->state.clear_depth = static_cast<float>(std::clamp(d, 0.0, 1.0));
}

void VKGL viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    g->state.viewport[0] = x;
    g->state.viewport[1] = y;
    g->state.viewport[2] = w;
    g->state.viewport[3] = h;
}

void VKGL scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
    g->state.scissor[0] = x;
    g->state.scissor[1] = y;
    g->state.scissor[2] = w;
    g->state.scissor[3] = h;
}

void VKGL depth_func(GLenum f) {
    g->state.depth_func = f;
}

void VKGL depth_mask(GLboolean flag) {
    g->state.depth_mask = flag != GL_FALSE;
}

void VKGL colour_mask(GLboolean r, GLboolean gr, GLboolean b, GLboolean a) {
    g->state.colour_mask[0] = r != GL_FALSE;
    g->state.colour_mask[1] = gr != GL_FALSE;
    g->state.colour_mask[2] = b != GL_FALSE;
    g->state.colour_mask[3] = a != GL_FALSE;
}

void VKGL blend_func_separate(GLenum sr, GLenum dr, GLenum sa, GLenum da) {
    State& s = g->state;
    s.blend_src_rgb = sr;
    s.blend_dst_rgb = dr;
    s.blend_src_a = sa;
    s.blend_dst_a = da;
}

void VKGL blend_equation(GLenum mode) {
    g->state.blend_equation = mode;
}

void VKGL blend_color(GLfloat r, GLfloat gr, GLfloat b, GLfloat a) {
    g->state.blend_colour[0] = r;
    g->state.blend_colour[1] = gr;
    g->state.blend_colour[2] = b;
    g->state.blend_colour[3] = a;
}

void VKGL cull_face(GLenum mode) {
    g->state.cull_face = mode;
}

void VKGL front_face(GLenum mode) {
    g->state.front_face = mode;
}

void VKGL polygon_mode(GLenum, GLenum mode) {
    g->state.polygon_mode = mode;
}

void VKGL pixel_store(GLenum name, GLint value) {
    switch (name) {
        case GL_UNPACK_ALIGNMENT:
            g->state.unpack_alignment = value;
            break;
        case GL_PACK_ALIGNMENT:
            g->state.pack_alignment = value;
            break;
        case kGlUnpackRowLength:
            g->state.unpack_row_length = value;
            break;
        default:
            break;
    }
}

Image* read_image() {
    return target_of(g->state.read_framebuffer).colour;
}

void VKGL
read_pixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* pixels) {
    Image* image = read_image();
    if (image == nullptr || w <= 0 || h <= 0 || format != GL_RGBA || type != GL_UNSIGNED_BYTE
        || image->samples != VK_SAMPLE_COUNT_1_BIT) {
        fail(kGlInvalidEnum);
        return;
    }
    const int x0 = std::clamp(x, 0, image->width), y0 = std::clamp(y, 0, image->height);
    const int x1 = std::clamp(x + w, x0, image->width), y1 = std::clamp(y + h, y0, image->height);
    if (x1 <= x0 || y1 <= y0) {
        return;
    }
    const int cw = x1 - x0, ch = y1 - y0;
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = static_cast<VkDeviceSize>(cw) * static_cast<VkDeviceSize>(ch) * 4;
    info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo alloc{};
    alloc.usage = VMA_MEMORY_USAGE_AUTO;
    alloc.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    VmaAllocationInfo got{};
    if (!check(
            vmaCreateBuffer(g->allocator, &info, &alloc, &buffer, &allocation, &got),
            "a read-back buffer"
        )) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(image, true);
    barrier(cmd, *image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.imageOffset = {x0, y0, 0};
    copy.imageExtent = {static_cast<std::uint32_t>(cw), static_cast<std::uint32_t>(ch), 1};
    vkCmdCopyImageToBuffer(
        cmd, image->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copy
    );
    count_copy(copy.imageExtent.width, copy.imageExtent.height);
    flush_and_wait();
    vmaInvalidateAllocation(g->allocator, allocation, 0, VK_WHOLE_SIZE);
    // Rows come as GL gives them, from the bottom: the same rows (vkgl.h, "Rows").
    const int align = std::max(g->state.pack_alignment, 1);
    const std::size_t pitch = (static_cast<std::size_t>(w) * 4 + align - 1) / align * align;
    const auto* src = static_cast<const std::uint8_t*>(got.pMappedData);
    auto* dst = static_cast<std::uint8_t*>(pixels);
    for (int row = 0; row < ch; ++row) {
        std::memcpy(
            dst + pitch * static_cast<std::size_t>(row + (y0 - y))
                + static_cast<std::size_t>(x0 - x) * 4,
            src + static_cast<std::size_t>(row) * static_cast<std::size_t>(cw) * 4,
            static_cast<std::size_t>(cw) * 4
        );
    }
    vmaDestroyBuffer(g->allocator, buffer, allocation);
}

Texture* bound_texture(GLenum target) {
    const State& s = g->state;
    const std::size_t unit = s.active_unit;
    switch (target) {
        case GL_TEXTURE_2D:
            return g->textures.find(s.unit_2d[unit]);
        case wgl::GL_TEXTURE_BUFFER:
            return g->textures.find(s.unit_buffer[unit]);
        case wgl::GL_TEXTURE_2D_ARRAY:
            return g->textures.find(s.unit_array[unit]);
        default:
            return nullptr;
    }
}

void VKGL copy_tex_sub_image(
    GLenum target, GLint level, GLint xoff, GLint yoff, GLint x, GLint y, GLsizei w, GLsizei h
) {
    Texture* t = bound_texture(target);
    Image* from = read_image();
    if (t == nullptr || !t->image || from == nullptr || w <= 0 || h <= 0
        || from->samples != VK_SAMPLE_COUNT_1_BIT) {
        return;
    }
    Image& to = *t->image;
    // Clipped to both images, as GL leaves what lies outside the source undefined.
    const int sx = std::max(x, 0), sy = std::max(y, 0);
    const int dx = xoff + (sx - x), dy = yoff + (sy - y);
    const int cw = std::min({w - (sx - x), from->width - sx, (to.width >> level) - dx});
    const int ch = std::min({h - (sy - y), from->height - sy, (to.height >> level) - dy});
    if (cw <= 0 || ch <= 0 || dx < 0 || dy < 0 || level >= to.levels) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(from, true);
    to.touched = g->recording;
    barrier(cmd, *from, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    barrier(cmd, to, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageCopy copy{};
    copy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy.srcOffset = {sx, sy, 0};
    copy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, static_cast<std::uint32_t>(level), 0, 1};
    copy.dstOffset = {dx, dy, 0};
    copy.extent = {static_cast<std::uint32_t>(cw), static_cast<std::uint32_t>(ch), 1};
    if (from->format == to.format) {
        vkCmdCopyImage(
            cmd,
            from->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            to.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copy
        );
        count_copy(copy.extent.width, copy.extent.height);
    } else {
        VkImageBlit blit{};
        blit.srcSubresource = copy.srcSubresource;
        blit.srcOffsets[0] = {sx, sy, 0};
        blit.srcOffsets[1] = {sx + cw, sy + ch, 1};
        blit.dstSubresource = copy.dstSubresource;
        blit.dstOffsets[0] = {dx, dy, 0};
        blit.dstOffsets[1] = {dx + cw, dy + ch, 1};
        vkCmdBlitImage(
            cmd,
            from->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            to.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_NEAREST
        );
        count_copy(copy.extent.width, copy.extent.height);
    }
}

void VKGL finish() {
    flush_and_wait();
}

void VKGL gen_textures(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->textures.add();
    }
}

void VKGL delete_textures(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g->textures.objects.find(names[i]);
        if (it == g->textures.objects.end()) {
            continue;
        }
        retire(std::move(it->second.image));
        g->textures.objects.erase(it);
        for (GLuint& b : g->state.unit_2d) {
            b = b == names[i] ? 0 : b;
        }
        for (GLuint& b : g->state.unit_buffer) {
            b = b == names[i] ? 0 : b;
        }
    }
}

void VKGL bind_texture(GLenum target, GLuint name) {
    State& s = g->state;
    const std::size_t unit = s.active_unit;
    if (Texture* t = g->textures.find(name); t != nullptr && t->target == 0) {
        t->target = target;
    }
    switch (target) {
        case GL_TEXTURE_2D:
            s.unit_2d[unit] = name;
            break;
        case wgl::GL_TEXTURE_BUFFER:
            s.unit_buffer[unit] = name;
            break;
        case wgl::GL_TEXTURE_2D_ARRAY:
            s.unit_array[unit] = name;
            break;
        default:
            break;
    }
}

void VKGL active_texture(GLenum unit) {
    g->state.active_unit = std::min<GLuint>(unit - GL_TEXTURE0, kUnits - 1);
}

void VKGL tex_image_2d(
    GLenum target,
    GLint level,
    GLint internal,
    GLsizei w,
    GLsizei h,
    GLint,
    GLenum format,
    GLenum type,
    const void* pixels
) {
    Texture* t = bound_texture(target);
    if (t == nullptr || w <= 0 || h <= 0 || level < 0) {
        return;
    }
    const Format f = format_of(static_cast<GLenum>(internal));
    if (f.vk == VK_FORMAT_UNDEFINED) {
        fail(kGlInvalidEnum);
        return;
    }
    if (level == 0) {
        Image* old = t->image.get();
        const bool same =
            old != nullptr && old->width == w && old->height == h && old->format == f.vk;
        // Its contents replaced where nothing has drawn with it yet; a new image where something
        // has (what was recorded keeps the old one), unless levels past the first must be kept.
        if (!same || (old->touched == g->recording && old->levels == 1)) {
            retire(std::move(t->image));
            t->image = make_image(f, w, h, same ? old->levels : 1, VK_SAMPLE_COUNT_1_BIT);
        }
        t->internal = static_cast<GLenum>(internal);
    } else {
        if (!t->image || std::max(t->image->width >> level, 1) != w
            || std::max(t->image->height >> level, 1) != h) {
            return;  // a level that does not belong to the first one's chain
        }
        if (t->image->levels <= level) {
            t->image = with_chain(t->image);
        }
        if (t->image->levels <= level) {
            return;
        }
    }
    if (!t->image || pixels == nullptr) {
        return;
    }
    Image& image = *t->image;
    const Slice staged = stage_pixels(f, w, h, format, type, pixels);
    if (staged.data == nullptr) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(&image);
    copy_to_image(cmd, image, staged, level, 0, 0, w, h);
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VKGL tex_sub_image_2d(
    GLenum target,
    GLint level,
    GLint x,
    GLint y,
    GLsizei w,
    GLsizei h,
    GLenum format,
    GLenum type,
    const void* pixels
) {
    Texture* t = bound_texture(target);
    if (t == nullptr || !t->image || pixels == nullptr || w <= 0 || h <= 0
        || level >= t->image->levels) {
        return;
    }
    Image& image = *t->image;
    const Format f{image.format, image.aspect, format_of(t->internal).bytes};
    if (x < 0 || y < 0 || x + w > std::max(image.width >> level, 1)
        || y + h > std::max(image.height >> level, 1)) {
        return;
    }
    const Slice staged = stage_pixels(f, w, h, format, type, pixels);
    if (staged.data == nullptr) {
        return;
    }
    VkCommandBuffer cmd = cmd_for(&image);
    copy_to_image(cmd, image, staged, level, x, y, w, h);
    barrier(cmd, image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void set_param(SamplerParams& p, int* base, int* max, GLenum name, float value) {
    const GLenum e = static_cast<GLenum>(value);
    switch (name) {
        case GL_TEXTURE_MIN_FILTER:
            p.min = e;
            break;
        case GL_TEXTURE_MAG_FILTER:
            p.mag = e;
            break;
        case GL_TEXTURE_WRAP_S:
            p.wrap_s = e;
            break;
        case GL_TEXTURE_WRAP_T:
            p.wrap_t = e;
            break;
        case GL_TEXTURE_MAX_ANISOTROPY:
            p.anisotropy = value;
            break;
        case wgl::GL_TEXTURE_BASE_LEVEL:
            if (base != nullptr) {
                *base = static_cast<int>(value);
            }
            break;
        case GL_TEXTURE_MAX_LEVEL:
            if (max != nullptr) {
                *max = static_cast<int>(value);
            }
            break;
        default:
            break;
    }
}

void VKGL tex_parameter_i(GLenum target, GLenum name, GLint value) {
    if (Texture* t = bound_texture(target)) {
        set_param(t->params, &t->base_level, &t->max_level, name, static_cast<float>(value));
    }
}

void VKGL tex_parameter_f(GLenum target, GLenum name, GLfloat value) {
    if (Texture* t = bound_texture(target)) {
        set_param(t->params, &t->base_level, &t->max_level, name, value);
    }
}

void VKGL generate_mipmap(GLenum target) {
    Texture* t = bound_texture(target);
    if (t == nullptr || !t->image) {
        return;
    }
    t->image = with_chain(t->image);
    if (t->image->levels > 1) {
        generate_mipmaps(*t->image);
    }
}

void VKGL gen_buffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->buffers.add();
    }
}

void VKGL delete_buffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g->buffers.objects.find(names[i]);
        if (it != g->buffers.objects.end()) {
            g->submission.cpu_mirror_bytes -= it->second.data.size();
            destroy_buffer(it->second);
            g->buffers.objects.erase(it);
        }
    }
}

GLuint* buffer_binding(GLenum target) {
    State& s = g->state;
    switch (target) {
        case GL_ARRAY_BUFFER:
            return &s.array_buffer;
        case GL_ELEMENT_ARRAY_BUFFER:
            return &vertex_array().elements;
        case wgl::GL_UNIFORM_BUFFER:
            return &s.uniform_buffer;
        case wgl::GL_TEXTURE_BUFFER:
            return &s.texture_buffer;
        case GL_SHADER_STORAGE_BUFFER:
            return &s.storage_buffer;
        default:
            return nullptr;
    }
}

void VKGL bind_buffer(GLenum target, GLuint name) {
    if (target == GL_SHADER_STORAGE_BUFFER) {
        if (auto* b = g->buffers.find(name)) {
            b->storage = true;
        }
    }
    if (GLuint* b = buffer_binding(target)) {
        *b = name;
    }
}

void changed(Buffer& b) {
    ++b.version;
    b.changed_frame = g->frame_count;
}

// True for GL's static usages (GL_STATIC_DRAW, _READ, _COPY): filled once, drawn many times.
bool is_static_usage(GLenum usage) {
    return usage == GL_STATIC_DRAW || usage == kGlStaticRead || usage == kGlStaticCopy;
}

void VKGL buffer_data(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
    GLuint* binding = buffer_binding(target);
    Buffer* b = binding != nullptr ? g->buffers.find(*binding) : nullptr;
    if (b == nullptr || size < 0) {
        return;
    }
    g->submission.cpu_mirror_bytes -= b->data.size();
    b->data.assign(static_cast<std::size_t>(size), 0);
    g->submission.cpu_mirror_bytes += b->data.size();
    b->size = b->data.size();
    b->static_hint = is_static_usage(usage);
    b->rewritten = false;
    b->mirror_freed = false;
    b->staged = nullptr;
    if (data != nullptr && size > 0) {
        std::memcpy(b->data.data(), data, static_cast<std::size_t>(size));
    }
    changed(*b);
}

void VKGL buffer_sub_data(GLenum target, GLintptr offset, GLsizeiptr size, const void* data) {
    GLuint* binding = buffer_binding(target);
    Buffer* b = binding != nullptr ? g->buffers.find(*binding) : nullptr;
    if (b == nullptr || offset < 0 || size <= 0
        || static_cast<std::size_t>(offset + size) > b->size) {
        return;
    }

    // The rest of the buffer stays as it was: its mirror is read back first if it was let go.
    buffer_bytes(*b);
    std::memcpy(b->data.data() + offset, data, static_cast<std::size_t>(size));
    b->rewritten = true;
    changed(*b);
}

void VKGL gen_vertex_arrays(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->vertex_arrays.add();
    }
}

void VKGL delete_vertex_arrays(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->vertex_arrays.objects.erase(names[i]);
        if (g->state.vertex_array == names[i]) {
            g->state.vertex_array = 0;
        }
    }
}

void VKGL bind_vertex_array(GLuint name) {
    g->state.vertex_array = name;
}

void VKGL enable_vertex_attrib_array(GLuint index) {
    if (index < kAttributes) {
        vertex_array().attributes[index].enabled = true;
    }
}

void VKGL vertex_attrib_pointer(
    GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer
) {
    if (index >= kAttributes) {
        return;
    }
    Attribute& a = vertex_array().attributes[index];
    a.size = size;
    a.type = type;
    a.normalized = normalized != GL_FALSE;
    a.integer = false;
    a.format = vertex_format(a.type, a.size, a.normalized, a.integer);
    a.stride = stride;
    a.offset = reinterpret_cast<std::uintptr_t>(pointer);
    a.buffer = g->state.array_buffer;
}

void VKGL vertex_attrib_i_pointer(
    GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer
) {
    vertex_attrib_pointer(index, size, type, GL_FALSE, stride, pointer);
    if (index < kAttributes) {
        Attribute& a = vertex_array().attributes[index];
        a.integer = true;
        a.format = vertex_format(a.type, a.size, a.normalized, a.integer);
    }
}

void VKGL vertex_attrib_divisor(GLuint index, GLuint divisor) {
    if (index < kAttributes) {
        vertex_array().attributes[index].divisor = divisor;
    }
}

void VKGL draw_arrays(GLenum mode, GLint first, GLsizei count) {
    draw(mode, count, 1, false, 0, 0, first);
}

void VKGL draw_arrays_instanced(GLenum mode, GLint first, GLsizei count, GLsizei instances) {
    draw(mode, count, instances, false, 0, 0, first);
}

void VKGL draw_elements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    draw(mode, count, 1, true, type, reinterpret_cast<std::uintptr_t>(indices), 0);
}

void VKGL draw_elements_instanced(
    GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instances
) {
    draw(mode, count, instances, true, type, reinterpret_cast<std::uintptr_t>(indices), 0);
}

// ---- Shaders and programs ----

GLuint VKGL create_shader(GLenum type) {
    const GLuint name = g->shaders.add();
    g->shaders.find(name)->type = type;
    return name;
}

void VKGL
shader_source(GLuint name, GLsizei count, const GLchar* const* strings, const GLint* lengths) {
    Shader* sh = g->shaders.find(name);
    if (sh == nullptr) {
        return;
    }
    sh->source.clear();
    for (GLsizei i = 0; i < count; ++i) {
        if (lengths != nullptr && lengths[i] >= 0) {
            sh->source.append(strings[i], static_cast<std::size_t>(lengths[i]));
        } else {
            sh->source.append(strings[i]);
        }
    }
}

void VKGL compile_shader(GLuint name) {
    if (Shader* sh = g->shaders.find(name)) {
        sh->log.clear();
        sh->compiled = sh->type == GL_COMPUTE_SHADER
                           ? check_compute_stage(sh->source, sh->log)
                           : check_stage(sh->type == GL_VERTEX_SHADER, sh->source, sh->log);
    }
}

void write_log(const std::string& log, GLsizei size, GLsizei* length, GLchar* out) {
    const GLsizei n = size > 0 ? std::min<GLsizei>(size - 1, static_cast<GLsizei>(log.size())) : 0;
    if (out != nullptr && size > 0) {
        std::memcpy(out, log.data(), static_cast<std::size_t>(n));
        out[n] = '\0';
    }
    if (length != nullptr) {
        *length = n;
    }
}

void VKGL get_shader_iv(GLuint name, GLenum what, GLint* out) {
    Shader* sh = g->shaders.find(name);
    *out = 0;
    if (sh == nullptr) {
        return;
    }
    if (what == GL_COMPILE_STATUS) {
        *out = sh->compiled ? GL_TRUE : GL_FALSE;
    } else if (what == GL_INFO_LOG_LENGTH) {
        *out = sh->log.empty() ? 0 : static_cast<GLint>(sh->log.size() + 1);
    }
}

void VKGL get_shader_info_log(GLuint name, GLsizei size, GLsizei* length, GLchar* log) {
    Shader* sh = g->shaders.find(name);
    write_log(sh != nullptr ? sh->log : std::string(), size, length, log);
}

void VKGL delete_shader(GLuint name) {
    g->shaders.objects.erase(name);
}

GLuint VKGL create_program() {
    return g->programs.add();
}

void VKGL attach_shader(GLuint program, GLuint shader) {
    if (Program* p = g->programs.find(program)) {
        p->shaders.push_back(shader);
    }
}

void VKGL bind_attrib_location(GLuint, GLuint, const GLchar*) {
    // The renderer's shaders give their inputs locations of their own.
}

VkShaderModule module_of(const std::vector<std::uint32_t>& code) {
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = code.size() * sizeof(std::uint32_t);
    info.pCode = code.data();
    VkShaderModule module = VK_NULL_HANDLE;
    check(vkCreateShaderModule(g->device, &info, nullptr, &module), "a shader module");
    return module;
}

void VKGL link_program(GLuint name) {
    Program* p = g->programs.find(name);
    if (p == nullptr) {
        return;
    }
    std::string vs, fs, cs;
    for (GLuint s : p->shaders) {
        if (Shader* sh = g->shaders.find(s)) {
            (sh->type == GL_VERTEX_SHADER    ? vs
             : sh->type == GL_COMPUTE_SHADER ? cs
                                             : fs) = sh->source;
        }
    }
    if (p->serial != 0) {
        release_program(*p);
    }
    p->log.clear();
    p->linked = false;
    p->serial = ++g->program_serial;
    if (!compile_program(vs, fs, p->compiled, p->log, cs)) {
        return;
    }
    if (!material_program(*p, vs, fs)) {
        return;
    }
    const CompiledProgram& c = p->compiled;
    if (!c.compute.empty()) {
        p->compute = module_of(c.compute);
    } else {
        p->vertex = module_of(c.vertex);
        p->fragment = module_of(c.fragment);
    }
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    auto add = [&](int binding, VkDescriptorType type) {
        if (binding < 0) {
            return;
        }
        bindings.push_back(
            {static_cast<std::uint32_t>(binding),
             type,
             1,
             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
                 | VK_SHADER_STAGE_COMPUTE_BIT,
             nullptr}
        );
    };
    p->dynamic_uniforms = c.default_binding >= 0 && reuse_uniforms();
    add(c.default_binding,
        p->dynamic_uniforms ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                            : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    for (const ProgramBlock& b : c.blocks) {
        add(b.binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    }
    for (const auto& b : c.storage) {
        add(b.binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }
    for (const ProgramUniform& u : c.uniforms) {
        if (u.sampler && u.binding >= 0) {
            add(u.binding,
                u.kind == SamplerKind::Buffer ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                                              : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }
    }
    VkDescriptorSetLayoutCreateInfo set_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    set_info.bindingCount = static_cast<std::uint32_t>(bindings.size());
    set_info.pBindings = bindings.data();
    check(
        vkCreateDescriptorSetLayout(g->device, &set_info, nullptr, &p->set_layout),
        "a descriptor set layout"
    );
    VkPipelineLayoutCreateInfo layout_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    const VkDescriptorSetLayout layouts[] = {p->set_layout, g->material_layout};
    const VkPushConstantRange material_push{VK_SHADER_STAGE_VERTEX_BIT, 0, 4};
    layout_info.setLayoutCount = p->gpu_materials ? 2u : 1u;
    layout_info.pSetLayouts = layouts;
    if (p->gpu_materials) {
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = &material_push;
    }
    check(
        vkCreatePipelineLayout(g->device, &layout_info, nullptr, &p->layout), "a pipeline layout"
    );
    // Uniforms start at zero, as GL's do; a relink keeps nothing, as GL's does.
    p->uniforms.assign(
        p->gpu_materials ? p->material_start + kMaterialBytes
                         : static_cast<unsigned>(std::max(c.default_size, 0)),
        0
    );
    p->units.assign(c.uniforms.size(), 0);
    p->block_bindings.assign(c.blocks.size(), 0);
    p->uniforms_dirty = true;
    p->last_set = VK_NULL_HANDLE;
    p->last_key.clear();
    p->descriptor_sets.clear();
    if (p->compute) {
        VkComputePipelineCreateInfo info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        info.stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        info.stage.module = p->compute;
        info.stage.pName = "main";
        info.layout = p->layout;
        check(
            vkCreateComputePipelines(
                g->device, VK_NULL_HANDLE, 1, &info, nullptr, &p->compute_pipeline
            ),
            "a compute pipeline"
        );
        count_pipeline();
    }
    p->linked = p->layout && ((p->vertex && p->fragment) || p->compute_pipeline);

    // Its pipelines of earlier runs are made on the pre-warm worker before its first draw.
    prewarm_program(*p);
}

void VKGL get_program_iv(GLuint name, GLenum what, GLint* out) {
    Program* p = g->programs.find(name);
    *out = 0;
    if (p == nullptr) {
        return;
    }
    if (what == GL_LINK_STATUS) {
        *out = p->linked ? GL_TRUE : GL_FALSE;
    } else if (what == GL_INFO_LOG_LENGTH) {
        *out = p->log.empty() ? 0 : static_cast<GLint>(p->log.size() + 1);
    } else if (what == kGlActiveUniforms) {
        *out = 0;  // not listed (gl_stats' uniform listing and shader reloading are GL's alone)
    }
}

void VKGL get_program_info_log(GLuint name, GLsizei size, GLsizei* length, GLchar* log) {
    Program* p = g->programs.find(name);
    write_log(p != nullptr ? p->log : std::string(), size, length, log);
}

void VKGL delete_program(GLuint name) {
    auto it = g->programs.objects.find(name);
    if (it == g->programs.objects.end()) {
        return;
    }
    if (it->second.serial != 0) {
        release_program(it->second);
    }
    g->programs.objects.erase(it);
    if (g->state.program == name) {
        g->state.program = 0;
    }
}

void VKGL use_program(GLuint name) {
    g->state.program = name;
}

// A uniform's location: its entry in the program's list, and the element of an array.
constexpr int kElementBits = 12;

GLint VKGL get_uniform_location(GLuint program, const GLchar* name) {
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
    const auto& list = p->compiled.uniforms;
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (list[i].name == n && element >= 0 && element < list[i].array_size) {
            return static_cast<GLint>(i << kElementBits | static_cast<std::size_t>(element));
        }
    }
    return -1;
}

// Where a location's value goes in the current program's default block; null for none.
std::uint8_t* uniform_at(
    GLint location,
    int count_bytes,
    const ProgramUniform** out_uniform = nullptr,
    int* out_index = nullptr
) {
    Program* p = g->programs.find(g->state.program);
    if (p == nullptr || !p->linked || location < 0) {
        return nullptr;
    }
    const std::size_t index = static_cast<std::size_t>(location) >> kElementBits;
    const int element = location & ((1 << kElementBits) - 1);
    if (index >= p->compiled.uniforms.size()) {
        return nullptr;
    }
    const ProgramUniform& u = p->compiled.uniforms[index];
    if (out_uniform != nullptr) {
        *out_uniform = &u;
    }
    if (out_index != nullptr) {
        *out_index = static_cast<int>(index);
    }
    if (u.sampler || u.offset < 0) {
        return nullptr;
    }
    const std::size_t at =
        static_cast<std::size_t>(u.offset)
        + static_cast<std::size_t>(element) * static_cast<std::size_t>(u.array_stride);
    if (at + static_cast<std::size_t>(count_bytes) > p->uniforms.size()) {
        return nullptr;
    }
    if (element >= u.array_size) {
        return nullptr;
    }
    return p->uniforms.data() + at;
}

void write_uniform(std::uint8_t* destination, const void* source, std::size_t bytes) {
    ++g->submission.uniform_writes;
    // Compare bytes, preserving signed zero and NaN payloads exactly.
    if (reuse_uniforms() && std::memcmp(destination, source, bytes) == 0) {
        ++g->submission.uniform_writes_skipped;
        return;
    }
    std::memcpy(destination, source, bytes);
    auto* p = g->programs.find(g->state.program);
    if (!p->gpu_materials || destination < p->uniforms.data() + p->material_start) {
        p->uniforms_dirty = true;
    }
}

void VKGL uniform_1i(GLint location, GLint v) {
    const ProgramUniform* u = nullptr;
    int index = -1;
    if (std::uint8_t* at = uniform_at(location, 4, &u, &index)) {
        write_uniform(at, &v, 4);
    } else if (u != nullptr && u->sampler) {
        g->programs.find(g->state.program)->units[static_cast<std::size_t>(index)] = v;
    }
}

void VKGL uniform_1ui(GLint location, GLuint v) {
    if (std::uint8_t* at = uniform_at(location, 4)) {
        write_uniform(at, &v, 4);
    }
}

void VKGL uniform_1f(GLint location, GLfloat v) {
    if (std::uint8_t* at = uniform_at(location, 4)) {
        write_uniform(at, &v, 4);
    }
}

void VKGL uniform_2f(GLint location, GLfloat a, GLfloat b) {
    const float v[] = {a, b};
    if (std::uint8_t* at = uniform_at(location, 8)) {
        write_uniform(at, v, 8);
    }
}

void VKGL uniform_3f(GLint location, GLfloat a, GLfloat b, GLfloat c) {
    const float v[] = {a, b, c};
    if (std::uint8_t* at = uniform_at(location, 12)) {
        write_uniform(at, v, 12);
    }
}

void VKGL uniform_4f(GLint location, GLfloat a, GLfloat b, GLfloat c, GLfloat d) {
    const float v[] = {a, b, c, d};
    if (std::uint8_t* at = uniform_at(location, 16)) {
        write_uniform(at, v, 16);
    }
}

void VKGL uniform_4fv(GLint location, GLsizei count, const GLfloat* v) {
    if (location < 0 || count <= 0 || !v) {
        return;
    }
    for (GLsizei i = 0; i < count; ++i) {
        if (std::uint8_t* at = uniform_at(location + i, 16)) {
            write_uniform(at, v + i * 4, 16);
        }
    }
}

void VKGL uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* v) {
    if (location < 0 || count <= 0 || !v) {
        return;
    }
    for (GLsizei i = 0; i < count; ++i) {
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

GLuint VKGL get_uniform_block_index(GLuint program, const GLchar* name) {
    Program* p = g->programs.find(program);
    if (p == nullptr || !p->linked || name == nullptr) {
        return wgl::GL_INVALID_INDEX;
    }
    for (std::size_t i = 0; i < p->compiled.blocks.size(); ++i) {
        if (p->compiled.blocks[i].name == name) {
            return static_cast<GLuint>(i);
        }
    }
    return wgl::GL_INVALID_INDEX;
}

void VKGL uniform_block_binding(GLuint program, GLuint block, GLuint binding) {
    Program* p = g->programs.find(program);
    if (p != nullptr && block < p->block_bindings.size()) {
        p->block_bindings[block] = binding;
    }
}

void VKGL bind_buffer_base(GLenum target, GLuint index, GLuint buffer) {
    if (target == GL_SHADER_STORAGE_BUFFER && index < kUniformBindings) {
        g->state.storage_bindings[index] = buffer;
        if (auto* b = g->buffers.find(buffer)) {
            b->storage = true;
        }
    }
    if (target == wgl::GL_UNIFORM_BUFFER && index < kUniformBindings) {
        g->state.uniform_bindings[index] = buffer;

        // A uniform block's bytes are read on the CPU when it is shorter than the block.
        if (Buffer* b = g->buffers.find(buffer)) {
            buffer_bytes(*b);
            b->keep_mirror = true;
        }
    }
    if (GLuint* b = buffer_binding(target)) {
        *b = buffer;
    }
}

void VKGL tex_buffer(GLenum target, GLenum format, GLuint buffer) {
    if (Texture* t = bound_texture(target)) {
        t->buffer = buffer;
        t->buffer_format = format;
    }
}

void VKGL
tex_image_3d(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) {
    fail(kGlInvalidEnum);  // no renderer draws with a texture array yet
}

void VKGL gen_samplers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->samplers.add();
    }
}

void VKGL delete_samplers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->samplers.objects.erase(names[i]);
    }
}

void VKGL bind_sampler(GLuint unit, GLuint sampler) {
    if (unit < kUnits) {
        g->state.unit_sampler[unit] = sampler;
    }
}

void VKGL sampler_parameter_i(GLuint sampler, GLenum name, GLint value) {
    if (SamplerParams* p = g->samplers.find(sampler)) {
        set_param(*p, nullptr, nullptr, name, static_cast<float>(value));
    }
}

void VKGL stencil_func(GLenum func, GLint ref, GLuint mask) {
    g->state.stencil_func = func;
    g->state.stencil_ref = ref;
    g->state.stencil_value_mask = mask;
}

void VKGL stencil_op_separate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    const StencilFace f{sfail, dpfail, dppass};
    if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
        g->state.stencil_front = f;
    }
    if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
        g->state.stencil_back = f;
    }
}

void VKGL stencil_mask(GLuint mask) {
    g->state.stencil_write_mask = mask;
}

void VKGL clear_stencil(GLint s) {
    g->state.clear_stencil = s;
}

// ---- Framebuffers ----

void VKGL gen_framebuffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->framebuffers.add();
    }
}

void VKGL delete_framebuffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        g->framebuffers.objects.erase(names[i]);
        if (g->state.draw_framebuffer == names[i]) {
            g->state.draw_framebuffer = 0;
        }
        if (g->state.read_framebuffer == names[i]) {
            g->state.read_framebuffer = 0;
        }
    }
}

void VKGL bind_framebuffer(GLenum target, GLuint name) {
    if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER) {
        g->state.draw_framebuffer = name;
    }
    if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER) {
        g->state.read_framebuffer = name;
    }
}

Framebuffer* bound_framebuffer(GLenum target) {
    return g->framebuffers.find(
        target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer
    );
}

void VKGL framebuffer_texture_2d(GLenum target, GLenum attachment, GLenum, GLuint texture, GLint) {
    Framebuffer* fb = bound_framebuffer(target);
    if (fb == nullptr) {
        return;
    }
    if (attachment == GL_COLOR_ATTACHMENT0) {
        fb->colour_texture = texture;
        fb->colour_renderbuffer = 0;
    } else if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        fb->depth_texture = texture;
        fb->depth_renderbuffer = 0;
    }
}

void VKGL framebuffer_renderbuffer(GLenum target, GLenum attachment, GLenum, GLuint renderbuffer) {
    Framebuffer* fb = bound_framebuffer(target);
    if (fb == nullptr) {
        return;
    }
    if (attachment == GL_COLOR_ATTACHMENT0) {
        fb->colour_renderbuffer = renderbuffer;
        fb->colour_texture = 0;
    } else if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        fb->depth_renderbuffer = renderbuffer;
        fb->depth_texture = 0;
    }
}

GLenum VKGL check_framebuffer_status(GLenum target) {
    const Target t = target_of(
        target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer
    );
    return t.colour != nullptr || t.depth != nullptr ? GL_FRAMEBUFFER_COMPLETE
                                                     : 0x8CD7;  // INCOMPLETE_MISSING_ATTACHMENT
}

void VKGL gen_renderbuffers(GLsizei n, GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        names[i] = g->renderbuffers.add();
    }
}

void VKGL delete_renderbuffers(GLsizei n, const GLuint* names) {
    for (GLsizei i = 0; i < n; ++i) {
        auto it = g->renderbuffers.objects.find(names[i]);
        if (it != g->renderbuffers.objects.end()) {
            retire(std::move(it->second.image));
            g->renderbuffers.objects.erase(it);
        }
    }
}

void VKGL bind_renderbuffer(GLenum, GLuint name) {
    g->state.renderbuffer = name;
}

void VKGL
renderbuffer_storage_multisample(GLenum, GLsizei samples, GLenum format, GLsizei w, GLsizei h) {
    Renderbuffer* r = g->renderbuffers.find(g->state.renderbuffer);
    const Format f = format_of(format);
    if (r == nullptr || f.vk == VK_FORMAT_UNDEFINED) {
        return;
    }
    VkSampleCountFlagBits count = VK_SAMPLE_COUNT_1_BIT;
    for (int c = 64; c > 1; c >>= 1) {
        if (samples >= c) {
            count = static_cast<VkSampleCountFlagBits>(c);
            break;
        }
    }
    retire(std::move(r->image));
    r->image = make_image(f, w, h, 1, count);
}

void VKGL renderbuffer_storage(GLenum target, GLenum format, GLsizei w, GLsizei h) {
    renderbuffer_storage_multisample(target, 0, format, w, h);
}

// GL's names of the default framebuffer's buffers in glInvalidateFramebuffer (OpenGL ES 3.0).
constexpr GLenum kGlColour = 0x1800;   // GL_COLOR
constexpr GLenum kGlDepth = 0x1801;    // GL_DEPTH
constexpr GLenum kGlStencil = 0x1802;  // GL_STENCIL

void VKGL invalidate_framebuffer(GLenum target, GLsizei count, const GLenum* attachments) {
    // The reference path: nothing is let go, as before the call existed here.
    if (!renderer::render_opt::enabled(renderer::render_opt::Opt::AttachmentDiscard)
        || attachments == nullptr) {
        return;
    }

    const GLuint name =
        target == GL_READ_FRAMEBUFFER ? g->state.read_framebuffer : g->state.draw_framebuffer;
    bool colour = false;
    VkImageAspectFlags depth = 0;

    // Each buffer named, in the default framebuffer's names or a framebuffer object's.
    for (GLsizei n = 0; n < count; ++n) {
        const GLenum attachment = attachments[n];

        // Colour 0 is the only colour attachment the backend draws into.
        if (attachment == (name == 0 ? kGlColour : GL_COLOR_ATTACHMENT0)) {
            colour = true;
        }

        // Depth, stencil, or both at once (a framebuffer object's name for the pair).
        if (attachment == (name == 0 ? kGlDepth : GL_DEPTH_ATTACHMENT)
            || (name != 0 && attachment == GL_DEPTH_STENCIL_ATTACHMENT)) {
            depth |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        // The stencil aspect, alone or with depth.
        if (attachment == (name == 0 ? kGlStencil : GL_STENCIL_ATTACHMENT)
            || (name != 0 && attachment == GL_DEPTH_STENCIL_ATTACHMENT)) {
            depth |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    discard_attachments(target_of(name), colour, depth);
}

// The rectangles of glBlitFramebuffer's `r` (source x0, y0, x1, y1, then the destination's) as a
// Vulkan blit of `aspect` from `src` into `dst`, each clamped to its image. False when either is
// empty, and nothing is to be copied.
bool blit_region(
    const Image& src,
    const Image& dst,
    VkImageAspectFlags aspect,
    const std::array<GLint, 8>& r,
    VkImageBlit& b
) {
    b.srcSubresource = {aspect, 0, 0, 1};
    b.srcOffsets[0] = {std::clamp(r[0], 0, src.width), std::clamp(r[1], 0, src.height), 0};
    b.srcOffsets[1] = {std::clamp(r[2], 0, src.width), std::clamp(r[3], 0, src.height), 1};
    b.dstSubresource = {aspect, 0, 0, 1};
    b.dstOffsets[0] = {std::clamp(r[4], 0, dst.width), std::clamp(r[5], 0, dst.height), 0};
    b.dstOffsets[1] = {std::clamp(r[6], 0, dst.width), std::clamp(r[7], 0, dst.height), 1};
    return b.srcOffsets[0].x != b.srcOffsets[1].x && b.srcOffsets[0].y != b.srcOffsets[1].y
           && b.dstOffsets[0].x != b.dstOffsets[1].x && b.dstOffsets[0].y != b.dstOffsets[1].y;
}

// The owner of the colour image framebuffer `name` reads from: its texture's or renderbuffer's.
// Null for the default framebuffer and for one with no colour.
ImagePtr colour_owner(GLuint name) {
    const Framebuffer* fb = g->framebuffers.find(name);

    // The default framebuffer's image is the backend's own, never a picture that is presented.
    if (fb == nullptr) {
        return nullptr;
    }

    // A texture first, as target_of looks.
    if (Texture* t = g->textures.find(fb->colour_texture); t != nullptr && t->image) {
        return t->image;
    }

    Renderbuffer* r = g->renderbuffers.find(fb->colour_renderbuffer);
    return r != nullptr ? r->image : nullptr;
}

// Holds back glBlitFramebuffer's colour copy of a picture into the default framebuffer when the
// pass before it only cleared that framebuffer (present-single-copy): the copy then waits for
// present_to_screen (PendingPresent). False, with nothing changed, when it cannot wait.
bool hold_present_copy(const std::array<GLint, 8>& r, GLenum filter) {
    const ImagePtr source = colour_owner(g->state.read_framebuffer);
    Image* dst = g->default_colour.get();
    VkImageBlit region{};

    // A single-sampled picture into the default framebuffer, as the frontend's present() copies.
    if (!renderer::render_opt::enabled(renderer::render_opt::Opt::PresentSingleCopy)
        || g->present_copy.pending || !source || source->samples != VK_SAMPLE_COUNT_1_BIT
        || dst == nullptr || g->state.draw_framebuffer != 0
        || !blit_region(*source, *dst, VK_IMAGE_ASPECT_COLOR_BIT, r, region)) {
        return false;
    }

    VkClearColorValue clear{};

    // The rest of the default framebuffer must be one known colour: the pass before only cleared it.
    if (!drop_clear_pass(clear)) {
        return false;
    }

    PendingPresent& waiting = g->present_copy;
    waiting.pending = true;
    waiting.source = source;
    waiting.region = region;
    waiting.filter = filter == GL_LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    waiting.clear = clear;
    return true;
}

void VKGL blit_framebuffer(
    GLint sx0,
    GLint sy0,
    GLint sx1,
    GLint sy1,
    GLint dx0,
    GLint dy0,
    GLint dx1,
    GLint dy1,
    GLbitfield mask,
    GLenum filter
) {
    const Target from = target_of(g->state.read_framebuffer);
    const Target to = target_of(g->state.draw_framebuffer);
    auto blit_one = [&](Image* src, Image* dst, VkImageAspectFlags aspect, VkFilter f) {
        if (src == nullptr || dst == nullptr || src == dst) {
            return;
        }
        VkCommandBuffer cmd = cmd_for(src, true);
        dst->touched = g->recording;
        // Matching multisample depth targets use the unscaled image-copy path below.
        if (src->samples != dst->samples && src->samples != VK_SAMPLE_COUNT_1_BIT
            && (aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
            // vkCmdResolveImage is colour-only. Resolve depth/stencil through
            // dynamic rendering; SAMPLE_ZERO is required by Vulkan 1.2+.
            if (dst->samples != VK_SAMPLE_COUNT_1_BIT || sx0 != dx0 || sy0 != dy0 || sx1 != dx1
                || sy1 != dy1 || sx0 < 0 || sy0 < 0 || sx1 <= sx0 || sy1 <= sy0) {
                fail(0x0502);  // GL_INVALID_OPERATION: unsupported depth resolve region
                return;
            }
            barrier(cmd, *src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            barrier(cmd, *dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            // The resolve writes all of the destination: what it held before is not needed.
            dst->discarded = 0;
            VkRenderingAttachmentInfo depth{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
            depth.imageView = attachment_view(*src);
            depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth.resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            depth.resolveImageView = attachment_view(*dst);
            depth.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
            info.renderArea =
                {{sx0, sy0},
                 {static_cast<std::uint32_t>(std::min({sx1, src->width, dst->width}) - sx0),
                  static_cast<std::uint32_t>(std::min({sy1, src->height, dst->height}) - sy0)}};
            info.layerCount = 1;
            info.pDepthAttachment = &depth;
            if ((aspect & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
                info.pStencilAttachment = &depth;
            }
            vkCmdBeginRendering(cmd, &info);
            vkCmdEndRendering(cmd);
            count_render_pass();
            count_command(Command::RenderPass);
            count_attachment(*src, info.renderArea.extent, true, true);
            count_attachment(*dst, info.renderArea.extent, false, true);
            return;
        }
        barrier(cmd, *src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        barrier(cmd, *dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        if ((aspect & VK_IMAGE_ASPECT_DEPTH_BIT) != 0 && sx1 - sx0 == dx1 - dx0
            && sy1 - sy0 == dy1 - dy0 && sx1 > sx0 && sy1 > sy0) {
            // Depth/stencil formats need not support BLIT_DST. An unscaled
            // copy has no such requirement; each aspect is copied separately.
            VkImageCopy copies[2]{};
            std::uint32_t count = 0;
            for (const auto bit : {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT}) {
                if ((aspect & bit) == 0) {
                    continue;
                }
                auto& copy = copies[count++];
                copy.srcSubresource = {static_cast<VkImageAspectFlags>(bit), 0, 0, 1};
                copy.dstSubresource = copy.srcSubresource;
                copy.srcOffset = {sx0, sy0, 0};
                copy.dstOffset = {dx0, dy0, 0};
                copy.extent = {
                    static_cast<std::uint32_t>(sx1 - sx0), static_cast<std::uint32_t>(sy1 - sy0), 1
                };
            }
            vkCmdCopyImage(
                cmd,
                src->image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dst->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                count,
                copies
            );
            count_copy(copies[0].extent.width, copies[0].extent.height);
            return;
        }
        if (src->samples != VK_SAMPLE_COUNT_1_BIT) {
            // A multisampled source is resolved: one size, no flip (as GL requires).
            VkImageResolve r{};
            r.srcSubresource = {aspect, 0, 0, 1};
            r.srcOffset = {std::min(sx0, sx1), std::min(sy0, sy1), 0};
            r.dstSubresource = {aspect, 0, 0, 1};
            r.dstOffset = {std::min(dx0, dx1), std::min(dy0, dy1), 0};
            r.extent =
                {static_cast<std::uint32_t>(std::abs(sx1 - sx0)),
                 static_cast<std::uint32_t>(std::abs(sy1 - sy0)),
                 1};
            r.extent.width = std::min<std::uint32_t>(
                r.extent.width, static_cast<std::uint32_t>(dst->width - r.dstOffset.x)
            );
            r.extent.height = std::min<std::uint32_t>(
                r.extent.height, static_cast<std::uint32_t>(dst->height - r.dstOffset.y)
            );
            vkCmdResolveImage(
                cmd,
                src->image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dst->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &r
            );
            count_copy(r.extent.width, r.extent.height);
            return;
        }
        VkImageBlit b{};
        if (!blit_region(*src, *dst, aspect, {sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1}, b)) {
            return;
        }
        vkCmdBlitImage(
            cmd,
            src->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dst->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &b,
            f
        );
        count_copy(
            static_cast<std::uint32_t>(std::abs(b.dstOffsets[1].x - b.dstOffsets[0].x)),
            static_cast<std::uint32_t>(std::abs(b.dstOffsets[1].y - b.dstOffsets[0].y))
        );
    };
    const std::array<GLint, 8> rects = {sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1};

    // The picture copied to the window with nothing else asked: it can wait for the present.
    if (mask == GL_COLOR_BUFFER_BIT && hold_present_copy(rects, filter)) {
        return;
    }

    if ((mask & GL_COLOR_BUFFER_BIT) != 0) {
        blit_one(
            from.colour,
            to.colour,
            VK_IMAGE_ASPECT_COLOR_BIT,
            filter == GL_LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST
        );
    }
    if ((mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0 && from.depth != nullptr
        && to.depth != nullptr && from.depth->format == to.depth->format) {
        blit_one(from.depth, to.depth, from.depth->aspect, VK_FILTER_NEAREST);
    }
}

#undef VKGL

struct Entry {
    const char* name;
    Proc proc;
};

#define VKGL_ENTRY(name, fn) {#name, reinterpret_cast<Proc>(&fn)}
const Entry kEntries[] = {
    VKGL_ENTRY(glGetString, get_string),
    VKGL_ENTRY(glGetStringi, get_stringi),
    VKGL_ENTRY(glGetIntegerv, get_integerv),
    VKGL_ENTRY(glGetFloatv, get_floatv),
    VKGL_ENTRY(glGetError, get_error),
    VKGL_ENTRY(glEnable, enable),
    VKGL_ENTRY(glDisable, disable),
    VKGL_ENTRY(glClear, clear_gl),
    VKGL_ENTRY(glClearColor, clear_color),
    VKGL_ENTRY(glClearDepth, clear_depth),
    VKGL_ENTRY(glViewport, viewport),
    VKGL_ENTRY(glScissor, scissor),
    VKGL_ENTRY(glDepthFunc, depth_func),
    VKGL_ENTRY(glDepthMask, depth_mask),
    VKGL_ENTRY(glColorMask, colour_mask),
    VKGL_ENTRY(glBlendFuncSeparate, blend_func_separate),
    VKGL_ENTRY(glBlendEquation, blend_equation),
    VKGL_ENTRY(glBlendColor, blend_color),
    VKGL_ENTRY(glCullFace, cull_face),
    VKGL_ENTRY(glFrontFace, front_face),
    VKGL_ENTRY(glPolygonMode, polygon_mode),
    VKGL_ENTRY(glPixelStorei, pixel_store),
    VKGL_ENTRY(glReadPixels, read_pixels),
    VKGL_ENTRY(glCopyTexSubImage2D, copy_tex_sub_image),
    VKGL_ENTRY(glFinish, finish),
    VKGL_ENTRY(glGenTextures, gen_textures),
    VKGL_ENTRY(glDeleteTextures, delete_textures),
    VKGL_ENTRY(glBindTexture, bind_texture),
    VKGL_ENTRY(glActiveTexture, active_texture),
    VKGL_ENTRY(glTexImage2D, tex_image_2d),
    VKGL_ENTRY(glTexSubImage2D, tex_sub_image_2d),
    VKGL_ENTRY(glTexParameteri, tex_parameter_i),
    VKGL_ENTRY(glTexParameterf, tex_parameter_f),
    VKGL_ENTRY(glGenerateMipmap, generate_mipmap),
    VKGL_ENTRY(glGenBuffers, gen_buffers),
    VKGL_ENTRY(glDeleteBuffers, delete_buffers),
    VKGL_ENTRY(glBindBuffer, bind_buffer),
    VKGL_ENTRY(glBufferData, buffer_data),
    VKGL_ENTRY(glBufferSubData, buffer_sub_data),
    VKGL_ENTRY(glGenVertexArrays, gen_vertex_arrays),
    VKGL_ENTRY(glDeleteVertexArrays, delete_vertex_arrays),
    VKGL_ENTRY(glBindVertexArray, bind_vertex_array),
    VKGL_ENTRY(glEnableVertexAttribArray, enable_vertex_attrib_array),
    VKGL_ENTRY(glVertexAttribPointer, vertex_attrib_pointer),
    VKGL_ENTRY(glVertexAttribIPointer, vertex_attrib_i_pointer),
    VKGL_ENTRY(glVertexAttribDivisor, vertex_attrib_divisor),
    VKGL_ENTRY(glDrawArrays, draw_arrays),
    VKGL_ENTRY(glDispatchCompute, dispatch_compute),
    VKGL_ENTRY(glMemoryBarrier, memory_barrier),
    VKGL_ENTRY(glDrawArraysInstanced, draw_arrays_instanced),
    VKGL_ENTRY(glDrawElements, draw_elements),
    VKGL_ENTRY(glDrawElementsInstanced, draw_elements_instanced),
    VKGL_ENTRY(glCreateShader, create_shader),
    VKGL_ENTRY(glShaderSource, shader_source),
    VKGL_ENTRY(glCompileShader, compile_shader),
    VKGL_ENTRY(glGetShaderiv, get_shader_iv),
    VKGL_ENTRY(glGetShaderInfoLog, get_shader_info_log),
    VKGL_ENTRY(glDeleteShader, delete_shader),
    VKGL_ENTRY(glCreateProgram, create_program),
    VKGL_ENTRY(glAttachShader, attach_shader),
    VKGL_ENTRY(glBindAttribLocation, bind_attrib_location),
    VKGL_ENTRY(glLinkProgram, link_program),
    VKGL_ENTRY(glGetProgramiv, get_program_iv),
    VKGL_ENTRY(glGetProgramInfoLog, get_program_info_log),
    VKGL_ENTRY(glDeleteProgram, delete_program),
    VKGL_ENTRY(glUseProgram, use_program),
    VKGL_ENTRY(glGetUniformLocation, get_uniform_location),
    VKGL_ENTRY(glUniform1i, uniform_1i),
    VKGL_ENTRY(glUniform1ui, uniform_1ui),
    VKGL_ENTRY(glUniform1f, uniform_1f),
    VKGL_ENTRY(glUniform2f, uniform_2f),
    VKGL_ENTRY(glUniform3f, uniform_3f),
    VKGL_ENTRY(glUniform4f, uniform_4f),
    VKGL_ENTRY(glUniform4fv, uniform_4fv),
    VKGL_ENTRY(glUniformMatrix4fv, uniform_matrix_4fv),
    VKGL_ENTRY(glGetUniformBlockIndex, get_uniform_block_index),
    VKGL_ENTRY(glUniformBlockBinding, uniform_block_binding),
    VKGL_ENTRY(glBindBufferBase, bind_buffer_base),
    VKGL_ENTRY(glTexBuffer, tex_buffer),
    VKGL_ENTRY(glTexImage3D, tex_image_3d),
    VKGL_ENTRY(glGenSamplers, gen_samplers),
    VKGL_ENTRY(glDeleteSamplers, delete_samplers),
    VKGL_ENTRY(glBindSampler, bind_sampler),
    VKGL_ENTRY(glSamplerParameteri, sampler_parameter_i),
    VKGL_ENTRY(glStencilFunc, stencil_func),
    VKGL_ENTRY(glStencilOpSeparate, stencil_op_separate),
    VKGL_ENTRY(glStencilMask, stencil_mask),
    VKGL_ENTRY(glClearStencil, clear_stencil),
    VKGL_ENTRY(glGenFramebuffers, gen_framebuffers),
    VKGL_ENTRY(glDeleteFramebuffers, delete_framebuffers),
    VKGL_ENTRY(glBindFramebuffer, bind_framebuffer),
    VKGL_ENTRY(glFramebufferTexture2D, framebuffer_texture_2d),
    VKGL_ENTRY(glFramebufferRenderbuffer, framebuffer_renderbuffer),
    VKGL_ENTRY(glCheckFramebufferStatus, check_framebuffer_status),
    VKGL_ENTRY(glGenRenderbuffers, gen_renderbuffers),
    VKGL_ENTRY(glDeleteRenderbuffers, delete_renderbuffers),
    VKGL_ENTRY(glBindRenderbuffer, bind_renderbuffer),
    VKGL_ENTRY(glRenderbufferStorage, renderbuffer_storage),
    VKGL_ENTRY(glRenderbufferStorageMultisample, renderbuffer_storage_multisample),
    VKGL_ENTRY(glBlitFramebuffer, blit_framebuffer),
    VKGL_ENTRY(glInvalidateFramebuffer, invalidate_framebuffer),
};
#undef VKGL_ENTRY

}  // namespace

// ---- The GL functions ----

void make_present_copy() {
    PendingPresent waiting = std::move(g->present_copy);
    g->present_copy = PendingPresent{};

    // Nothing waits.
    if (!waiting.pending) {
        return;
    }

    Image& dst = *g->default_colour;
    Image& src = *waiting.source;
    VkCommandBuffer cmd = cmd_for(&src, true);
    dst.touched = g->recording;
    const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier(cmd, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkCmdClearColorImage(
        cmd, dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &waiting.clear, 1, &range
    );
    count_command(Command::Copy);
    barrier(cmd, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    // The clear's writes done before the blit's, which overlap them.
    barrier(cmd, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vkCmdBlitImage(
        cmd,
        src.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &waiting.region,
        waiting.filter
    );
    count_copy(
        static_cast<std::uint32_t>(
            std::abs(waiting.region.dstOffsets[1].x - waiting.region.dstOffsets[0].x)
        ),
        static_cast<std::uint32_t>(
            std::abs(waiting.region.dstOffsets[1].y - waiting.region.dstOffsets[0].y)
        )
    );
}

void destroy_buffer(Buffer& b) {
    forget_views(b);
    if (b.own != VK_NULL_HANDLE) {
        defer([buffer = b.own, allocation = b.own_allocation] {
            vmaDestroyBuffer(g->allocator, buffer, allocation);
        });
    }
    b.own = VK_NULL_HANDLE;
    b.own_mapped = nullptr;
}

// ---- Shaders and programs ----

void release_program(Program& p) {
    // No pre-warm job may use its modules or layout once they are handed to defer.
    prewarm_forget(p.serial);
    // Relinking and deletion retire every pipeline referenced by the program's fast lookup.
    p.last_pipeline = VK_NULL_HANDLE;
    p.last_pipeline_key.clear();
    defer([vertex = p.vertex,
           fragment = p.fragment,
           compute = p.compute,
           pipeline = p.compute_pipeline,
           set_layout = p.set_layout,
           layout = p.layout] {
        vkDestroyShaderModule(g->device, vertex, nullptr);
        vkDestroyShaderModule(g->device, fragment, nullptr);
        vkDestroyShaderModule(g->device, compute, nullptr);
        vkDestroyPipeline(g->device, pipeline, nullptr);
        vkDestroyPipelineLayout(g->device, layout, nullptr);
        vkDestroyDescriptorSetLayout(g->device, set_layout, nullptr);
    });
    // Its pipelines go with it.
    // Called by erase_program, on this thread, for each pipeline of the program: destroyed once
    // the frames that may still draw with it are done.
    g->pipelines.erase_program(p.serial, [](VkPipeline pipeline) {
        defer([pipeline] { vkDestroyPipeline(g->device, pipeline, nullptr); });
    });
    p.vertex = p.fragment = VK_NULL_HANDLE;
    p.compute = VK_NULL_HANDLE;
    p.compute_pipeline = VK_NULL_HANDLE;
    p.set_layout = VK_NULL_HANDLE;
    p.layout = VK_NULL_HANDLE;
}

}  // namespace openrac::vkgl::detail

namespace openrac::vkgl {

using namespace detail;

// ---- The device, the screen and the frames ----

Proc get_proc(const char* name) {
    if (name == nullptr) {
        return nullptr;
    }
    for (const Entry& e : kEntries) {
        if (std::strcmp(e.name, name) == 0) {
            return e.proc;
        }
    }
    return nullptr;
}

}  // namespace openrac::vkgl
