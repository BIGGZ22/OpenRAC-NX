// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Draws (vkgl_internal.h): one GL draw call recorded into the frame's main command buffer. The
// native material path (gpu_materials.inc) and the GPU-driven instances (gpu_draw.inc) are included
// into this file and share its state.

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <regex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <volk.h>

#include "common/log.h"
#include "common/stats.h"
#include "renderer/gl.h"
#include "renderer/gpu_draw.h"
#include "renderer/render_opt.h"
#include "renderer/vulkan/shader_compiler.h"
#include "renderer/vulkan/vkgl_internal.h"

namespace openrac::vkgl::detail {

using namespace openrac::gl;

namespace {

// ---- Draws ----

VertexArray g_default_vertex_array;  // vertex array 0

// ---- Dynamic state, recorded when it changes ----

// Returns whether a draw leaves out the state its command buffer holds already (vk-state-skip).
// Off, every draw records all of it, as before; the tracker is kept up to date either way.
bool skip_repeats() {
    return renderer::render_opt::enabled(renderer::render_opt::Opt::VkStateSkip);
}

// Records the GL viewport and scissor, or a pass's indexed viewports with their scissors, one
// command each, into `cmd` without RecordedState: for more viewports than it can remember.
void record_viewports_untracked(VkCommandBuffer cmd) {
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;

    // Each indexed viewport is its own scissor too.
    for (const auto& r : g->indexed_viewports) {
        const auto x = static_cast<float>(r[0]);
        const auto y = static_cast<float>(r[1]);
        const auto width = static_cast<float>(r[2]);
        const auto height = static_cast<float>(r[3]);
        viewports.push_back({x, y, width, height, 0.0f, 1.0f});
        scissors.push_back(
            {{r[0], r[1]}, {static_cast<std::uint32_t>(r[2]), static_cast<std::uint32_t>(r[3])}}
        );
    }

    vkCmdSetViewport(cmd, 0, static_cast<std::uint32_t>(viewports.size()), viewports.data());
    vkCmdSetScissor(cmd, 0, static_cast<std::uint32_t>(scissors.size()), scissors.data());
    count_command(Command::DynamicState, 2);  // the viewports and the scissors
    g->recorded.viewport_count = 0;
    g->recorded.scissor_count = 0;
}

// Records into `cmd` the viewports and scissors a draw into `t` uses: the GL viewport and the
// scissor clipped to `t`, or the indexed viewports a pass set, each its own scissor. Each of the
// two commands is left out when it would set what `cmd` holds.
void record_viewports(VkCommandBuffer cmd, const Target& t) {
    const std::size_t count = std::max<std::size_t>(1, g->indexed_viewports.size());

    // A draw into more viewports than the tracker holds is recorded in full.
    if (count > kTrackedViewports) {
        record_viewports_untracked(cmd);
        return;
    }

    const State& s = g->state;
    std::array<VkViewport, kTrackedViewports> viewports{};
    std::array<VkRect2D, kTrackedViewports> scissors{};

    // The GL viewport and scissor, unless a pass drew into indexed viewports, each its own scissor.
    if (g->indexed_viewports.empty()) {
        viewports[0] =
            {static_cast<float>(s.viewport[0]),
             static_cast<float>(s.viewport[1]),
             static_cast<float>(s.viewport[2]),
             static_cast<float>(s.viewport[3]),
             0.0f,
             1.0f};
        scissors[0] = scissor_rect(t);
    } else {
        // Each of the pass's viewports.
        for (std::size_t n = 0; n < count; ++n) {
            const auto& r = g->indexed_viewports[n];
            viewports[n] =
                {static_cast<float>(r[0]),
                 static_cast<float>(r[1]),
                 static_cast<float>(r[2]),
                 static_cast<float>(r[3]),
                 0.0f,
                 1.0f};
            scissors[n] = {
                {r[0], r[1]}, {static_cast<std::uint32_t>(r[2]), static_cast<std::uint32_t>(r[3])}
            };
        }
    }

    RecordedState& recorded = g->recorded;
    const auto n = static_cast<std::uint32_t>(count);
    const bool skip = skip_repeats();

    // VkViewport and VkRect2D have no padding, so equal bytes are equal state.
    if (!skip || recorded.viewport_count != n
        || std::memcmp(viewports.data(), recorded.viewports.data(), count * sizeof(VkViewport))
               != 0) {
        vkCmdSetViewport(cmd, 0, n, viewports.data());
        count_command(Command::DynamicState);
        recorded.viewport_count = n;
        recorded.viewports = viewports;
    }

    // The scissors, apart: a 2D draw often moves the scissor and keeps the viewport.
    if (!skip || recorded.scissor_count != n
        || std::memcmp(scissors.data(), recorded.scissors.data(), count * sizeof(VkRect2D)) != 0) {
        vkCmdSetScissor(cmd, 0, n, scissors.data());
        count_command(Command::DynamicState);
        recorded.scissor_count = n;
        recorded.scissors = scissors;
    }
}

// Records `value` into `known` and returns true when it differs from what `known` holds, or
// when vk-state-skip is off; false when the command that sets it can be left out.
template <typename Value>
bool changes(std::optional<Value>& known, const Value& value, bool skip) {
    // The command buffer holds this value already.
    if (skip && known == value) {
        return false;
    }

    known = value;
    return true;
}

// Records into `cmd` the blend constants and the three stencil states of the GL state, each
// command left out when it would set what `cmd` holds.
void record_blend_and_stencil(VkCommandBuffer cmd) {
    const State& s = g->state;
    RecordedState& recorded = g->recorded;
    const bool skip = skip_repeats();
    std::array<std::uint32_t, 4> blend_bits{};
    std::memcpy(blend_bits.data(), s.blend_colour, sizeof blend_bits);

    // The blend constants, by their bits.
    if (changes(recorded.blend_bits, blend_bits, skip)) {
        vkCmdSetBlendConstants(cmd, s.blend_colour);
        count_command(Command::DynamicState);
    }

    const auto reference = static_cast<std::uint32_t>(s.stencil_ref);

    // The stencil reference, compare mask and write mask, one command each, for both faces.
    if (changes(recorded.stencil_reference, reference, skip)) {
        vkCmdSetStencilReference(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, reference);
        count_command(Command::DynamicState);
    }

    // The compare mask.
    if (changes(recorded.stencil_compare_mask, s.stencil_value_mask, skip)) {
        vkCmdSetStencilCompareMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, s.stencil_value_mask);
        count_command(Command::DynamicState);
    }

    // The write mask.
    if (changes(recorded.stencil_write_mask, s.stencil_write_mask, skip)) {
        vkCmdSetStencilWriteMask(cmd, VK_STENCIL_FACE_FRONT_AND_BACK, s.stencil_write_mask);
        count_command(Command::DynamicState);
    }
}

}  // namespace

// ---- Draws ----

VertexArray& vertex_array() {
    if (VertexArray* va = g->vertex_arrays.find(g->state.vertex_array)) {
        return *va;
    }
    return g_default_vertex_array;
}

#include "gpu_materials.inc"

void draw(
    GLenum mode,
    GLsizei count,
    GLsizei instances,
    bool indexed,
    GLenum index_type,
    std::uintptr_t index_offset,
    GLint first,
    GpuPage* gpu_page,
    unsigned gpu_command
) {
    // The draw's count, and its time for the vk_draw_ms gauge. The two clock reads are taken only
    // while the debug tools measure (vk-draw-clock): the gauge is published only then.
    struct Timed {
        // Whether this draw is timed: always on the reference path.
        bool on = !renderer::render_opt::enabled(renderer::render_opt::Opt::VkDrawClock)
                  || stats::enabled();

        // When the draw began; unread when it is not timed.
        Clock::time_point start = on ? Clock::now() : Clock::time_point{};

        ~Timed() {
            // An untimed draw adds nothing to draw_s, which stays 0 for an unmeasured frame.
            if (on) {
                g->measures.draw_s += seconds_since(start);
            }

            ++g->measures.draws;
        }
    } timed;

    Program* p = g->programs.find(g->state.program);
    if (p == nullptr || !p->linked || count <= 0 || instances <= 0) {
        return;
    }
    const Target t = target_of(g->state.draw_framebuffer);
    if ((t.colour == nullptr && t.depth == nullptr) || t.width <= 0 || t.height <= 0) {
        return;
    }
    const State& s = g->state;
    if (s.viewport[2] <= 0 || s.viewport[3] <= 0) {
        return;
    }
    // What it samples, in the layout for sampling: moved before the drawing begins.
    std::vector<Sampled> samplers;
    for (std::size_t i = 0; i < p->compiled.uniforms.size(); ++i) {
        if (p->compiled.uniforms[i].sampler && p->compiled.uniforms[i].binding >= 0) {
            samplers.push_back(sampled(*p, i));
        }
    }
    for (Sampled& sm : samplers) {
        Image* image = sm.image;
        if (image != nullptr && (image == t.colour || image == t.depth)) {
            // A texture that is also what the draw writes, left bound to a unit the draw does not
            // read (the 2D path's untextured quads keep the frame's texture on unit 0): GL leaves
            // a read of it undefined, and Vulkan cannot describe an attachment as sampled. GL's
            // incomplete texture stands in.
            sm.image = g->dummy_image.get();
            sm.view = sample_view(*sm.image, 0, 0);
            continue;
        }
        if (image == nullptr) {
            continue;
        }
        if (image->layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            end_rendering();
            barrier(frame().main, *image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        image->touched = g->recording;
    }

    // The vertex inputs: each from its own binding, or GL's (0, 0, 0, 1).
    const VertexArray& va = vertex_array();
    Input inputs[kAttributes];
    VkBuffer buffers[kAttributes];
    VkDeviceSize offsets[kAttributes];
    std::uint32_t n = 0;
    for (const CompiledProgram::Attribute& a : p->compiled.attributes) {
        if (a.location < 0 || a.location >= kAttributes || n >= kAttributes) {
            continue;
        }
        const Attribute& attr = va.attributes[static_cast<std::size_t>(a.location)];
        Buffer* b = attr.enabled ? g->buffers.find(attr.buffer) : nullptr;
        const VkFormat format =
            attr.enabled ? vertex_format(attr.type, attr.size, attr.normalized, attr.integer)
                         : VK_FORMAT_UNDEFINED;
        if (b != nullptr && format != VK_FORMAT_UNDEFINED) {
            // Small persistent instance buffers corrupt vertex fetches on the
            // AMD Windows driver. Keep instance data in the frame ring; large
            // per-vertex and index buffers can still use persistent storage.
            const BufferRef r = gpu_page && attr.divisor
                                    ? BufferRef{gpu_page->output.buffer, 0, gpu_page->output.size}
                                    : resolve(*b, attr.divisor == 0);
            const GLsizei stride =
                attr.stride != 0 ? attr.stride : attr.size * type_bytes(attr.type);
            inputs[n] =
                {static_cast<std::uint32_t>(a.location),
                 static_cast<std::uint32_t>(format),
                 static_cast<std::uint32_t>(stride),
                 attr.divisor != 0 ? 1u : 0u};
            buffers[n] = r.buffer;
            offsets[n] = r.offset + attr.offset;
        } else {
            const int kind = input_kind(a.gl_type);
            inputs[n] =
                {static_cast<std::uint32_t>(a.location),
                 static_cast<std::uint32_t>(
                     kind == 0   ? VK_FORMAT_R32G32B32A32_SFLOAT
                     : kind == 1 ? VK_FORMAT_R32G32B32A32_UINT
                                 : VK_FORMAT_R32G32B32A32_SINT
                 ),
                 0u,
                 0u};
            buffers[n] = g->dummy_buffer;
            offsets[n] = kind == 0 ? kDefaultFloat : kDefaultInteger;
        }
        ++n;
    }

    const VkPipeline pipeline = pipeline_for(*p, mode, t, inputs, n);
    if (pipeline == VK_NULL_HANDLE) {
        return;
    }
    const auto material = material_binding(*p, t);
    const VkDescriptorSet set = descriptors(*p, samplers);
    BufferRef index{};
    VkIndexType vk_index = VK_INDEX_TYPE_UINT32;
    std::uint32_t first_index = 0;
    if (indexed) {
        Buffer* eb = g->buffers.find(va.elements);
        if (eb == nullptr || eb->data.empty()) {
            return;
        }
        if (index_type == GL_UNSIGNED_BYTE) {
            // Vulkan's 8-bit indices are an extension: made 16-bit.
            const std::size_t from = std::min<std::size_t>(index_offset, eb->data.size());
            const std::size_t have =
                std::min<std::size_t>(static_cast<std::size_t>(count), eb->data.size() - from);
            Slice sl = ring(static_cast<VkDeviceSize>(have * 2));
            auto* out = reinterpret_cast<std::uint16_t*>(sl.data);
            for (std::size_t i = 0; i < have; ++i) {
                const std::uint8_t v = eb->data[from + i];
                out[i] = v == 0xFF ? 0xFFFF : v;
            }
            index = {sl.buffer, sl.offset, have * 2};
            vk_index = VK_INDEX_TYPE_UINT16;
            count = static_cast<GLsizei>(have);
        } else {
            index = resolve(*eb);
            const std::uint32_t bytes = index_type == GL_UNSIGNED_SHORT ? 2 : 4;
            vk_index = bytes == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
            first_index = static_cast<std::uint32_t>(index_offset / bytes);
        }
    }

    ensure_rendering(t);
    VkCommandBuffer cmd = frame().main;
    GpuEngine::Bindings gpu_bindings;
    VkDeviceSize argument_offset = VkDeviceSize(kGpuPageGroups + 5 * gpu_command) * 4;
    if (gpu_page) {
        gpu_bindings.pipeline = pipeline;
        gpu_bindings.descriptors = set;
        gpu_bindings.materials = material.set;
        gpu_bindings.dynamic_offset = static_cast<std::uint32_t>(p->uniforms_offset);
        gpu_bindings.inputs = n;
        std::copy_n(buffers, n, gpu_bindings.vertices.begin());
        std::copy_n(offsets, n, gpu_bindings.offsets.begin());
        gpu_bindings.index = index.buffer;
        gpu_bindings.index_offset = index.offset;
        gpu_bindings.index_type = vk_index;
        std::copy_n(s.viewport, 4, gpu_bindings.viewport.begin());
        const auto sc = scissor_rect(t);
        gpu_bindings
            .scissor = {sc.offset.x, sc.offset.y, int(sc.extent.width), int(sc.extent.height)};
        std::copy_n(s.blend_colour, 4, gpu_bindings.blend.begin());
        gpu_bindings.stencil =
            {static_cast<unsigned>(s.stencil_ref), s.stencil_value_mask, s.stencil_write_mask};
        auto& pending = g->gpu;
        if (stats::enabled() && pending.pending_count) {
            const auto& before = pending.pending_bindings;
            const auto& after = gpu_bindings;
            const bool reasons[] =
                {before.pipeline != after.pipeline,
                 before.descriptors != after.descriptors,
                 before.dynamic_offset != after.dynamic_offset,
                 before.vertices != after.vertices || before.offsets != after.offsets,
                 before.index != after.index || before.index_offset != after.index_offset,
                 before.materials != after.materials,
                 before.viewport != after.viewport || before.scissor != after.scissor,
                 before.blend != after.blend || before.stencil != after.stencil,
                 pending.pending_arguments != gpu_page->arguments.buffer
                     || argument_offset != pending.pending_offset + pending.pending_count * 20,
                 material.set
                     && material.draw != pending.pending_material_base + pending.pending_count};
            for (std::size_t i = 0; i < pending.batch_breaks.size(); ++i) {
                pending.batch_breaks[i] += reasons[i] ? 1u : 0u;
            }
        }
        if (g->features.multiDrawIndirect && pending.pending_count
            && pending.pending_count < g->properties.limits.maxDrawIndirectCount
            && pending.pending_arguments == gpu_page->arguments.buffer
            && argument_offset == pending.pending_offset + pending.pending_count * 20
            && pending.pending_bindings == gpu_bindings
            && (!material.set
                || material.draw == pending.pending_material_base + pending.pending_count)) {
            ++pending.pending_count;
            ++pending.merged;
            return;
        }
    }
    gpu_flush_draws();
    if (pipeline != g->bound_pipeline) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        count_command(Command::BindPipeline);
        g->bound_pipeline = pipeline;
    }
    if (set != VK_NULL_HANDLE) {
        const auto dynamic_offset = static_cast<std::uint32_t>(p->uniforms_offset);
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            p->layout,
            0,
            1,
            &set,
            p->dynamic_uniforms ? 1u : 0u,
            p->dynamic_uniforms ? &dynamic_offset : nullptr
        );
        count_command(Command::BindDescriptors);
    }
    if (material.set) {
        vkCmdBindDescriptorSets(
            cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p->layout, 1, 1, &material.set, 0, nullptr
        );
        vkCmdPushConstants(cmd, p->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 4, &material.draw);
        count_command(Command::BindDescriptors, 2);  // the material set and its push constant
    }
    if (n > 0) {
        vkCmdBindVertexBuffers(cmd, 0, n, buffers, offsets);
        count_command(Command::BindBuffers);
    }
    record_viewports(cmd, t);
    record_blend_and_stencil(cmd);
    if (indexed) {
        vkCmdBindIndexBuffer(cmd, index.buffer, index.offset, vk_index);
        count_command(Command::BindBuffers);
        if (gpu_page) {
            g->gpu.pending_bindings = gpu_bindings;
            g->gpu.pending_arguments = gpu_page->arguments.buffer;
            g->gpu.pending_offset = argument_offset;
            g->gpu.pending_count = 1;
            g->gpu.pending_material_base = material.draw;
        } else {
            vkCmdDrawIndexed(
                cmd,
                static_cast<std::uint32_t>(count),
                static_cast<std::uint32_t>(instances),
                first_index,
                0,
                0
            );
            count_command(Command::Draw);
            ++g->measures.native_draws;
        }
    } else {
        vkCmdDraw(
            cmd,
            static_cast<std::uint32_t>(count),
            static_cast<std::uint32_t>(instances),
            static_cast<std::uint32_t>(first),
            0
        );
        count_command(Command::Draw);
        ++g->measures.native_draws;
    }
}

#include "gpu_draw.inc"

}  // namespace openrac::vkgl::detail
