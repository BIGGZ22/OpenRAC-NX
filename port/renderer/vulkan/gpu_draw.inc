// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Included inside vkgl.cpp: native instance work shares its frame-slot owners.
constexpr unsigned kGpuSummaryAt = kGpuPageGroups + 5 * kGpuPageDraws;
constexpr unsigned kGpuSummaryWords = 4;
// Input words: a four-word header, four-word group records, then candidates
// containing eight bound/visibility words and thirty instance words. Output
// arguments reserve kGpuPageGroups count words before five-word indexed draws.
constexpr const char* kGpuCullSource = R"GLSL(#version 450 core
layout(local_size_x = 64) in;
layout(std430, binding = 0) readonly buffer InputWords {
    uint src[];
};
layout(std430, binding = 1) writeonly buffer OutputWords {
    uint dst[];
};
layout(std430, binding = 2) buffer DrawWords {
    uint args[];
};
shared uint prefix[64];
shared uint total;
#ifdef SUMMARIZE
shared uvec4 sums[64];
uvec4 addCounters(uvec4 a, uvec4 b) {
    uint low = a.z + b.z;
    return uvec4(a.xy + b.xy, low, a.w + b.w + uint(low < a.z));
}
#endif
void main() {
#ifdef SUMMARIZE
    uint lane = gl_LocalInvocationID.x;
    uvec4 sum = uvec4(0);
    for (uint group = lane; group < src[0]; group += 64) {
        sum.x += args[group];
    }
    for (uint drawID = lane; drawID < src[1]; drawID += 64) {
        uint at = 4096 + drawID * 5;
        uint count = args[at], instances = args[at + 1], high, low;
        umulExtended(count / 3, instances, high, low);
        sum = addCounters(sum, uvec4(0, uint(count != 0 && instances != 0), low, high));
    }
    sums[lane] = sum;
    barrier();
    for (uint step = 32; step > 0; step /= 2) {
        if (lane < step) {
            sums[lane] = addCounters(sums[lane], sums[lane + step]);
        }
        barrier();
    }
    if (lane == 0) {
        for (uint i = 0; i < 4; ++i) {
            args[SUMMARY_AT + i] = sums[0][i];
        }
    }
#elif defined(MAKE_ARGUMENTS)
    uint drawID = gl_GlobalInvocationID.x;
    if (drawID >= src[1]) {
        return;
    }
    uint record = src[2] + drawID * 4;
    uint group = src[record + 2];
    uint outBase = src[4 + group * 4 + 1];
    uint at = 4096 + drawID * 5;
    args[at] = src[record];
    args[at + 1] = args[group];
    args[at + 2] = src[record + 1];
    args[at + 3] = 0;
    args[at + 4] = outBase;
#else
    uint group = gl_WorkGroupID.x, lane = gl_LocalInvocationID.x;
    if (group >= src[0]) {
        return;
    }
    uint record = 4 + group * 4;
    uint begin = src[record], outputBase = src[record + 1], count = src[record + 2],
         view = src[record + 3];
    if (lane == 0) {
        total = 0;
    }
    barrier();
    for (uint chunk = 0; chunk < count; chunk += 64) {
        uint index = chunk + lane, base = begin + index * 38;
        bool visible = index < count;
        if (visible) {
            uint flags = src[base + 7], word = src[base + 6];
            if ((flags & 2) == 0 || (src[view + 57] & 2) == 0) {
                uint byte = word >> 8, bit = word & 255;
                if ((src[view + 57] & 1) != 0 && byte < 128 && bit != 0 && (bit & (bit - 1)) == 0) {
                    uint mask = (src[view + 24 + byte / 4] >> ((byte % 4) * 8)) & 255;
                    visible = (mask & bit) != 0;
                }
                if (visible && (flags & 1) != 0) {
                    vec3 low = vec3(
                        uintBitsToFloat(src[base]),
                        uintBitsToFloat(src[base + 1]),
                        uintBitsToFloat(src[base + 2])
                    );
                    vec3 high = vec3(
                        uintBitsToFloat(src[base + 3]),
                        uintBitsToFloat(src[base + 4]),
                        uintBitsToFloat(src[base + 5])
                    );
                    for (uint plane = 0; plane < src[view + 56]; ++plane) {
                        vec4 p = uintBitsToFloat(uvec4(
                            src[view + plane * 4],
                            src[view + plane * 4 + 1],
                            src[view + plane * 4 + 2],
                            src[view + plane * 4 + 3]
                        ));
                        vec3 v = mix(low, high, greaterThanEqual(p.xyz, vec3(0)));
                        precise float distance = ((p.x * v.x + p.y * v.y) + p.z * v.z) + p.w;
                        if (distance < 0) {
                            visible = false;
                        }
                    }
                }
            }
        }
        prefix[lane] = visible ? 1 : 0;
        barrier();
        for (uint step = 1; step < 64; step *= 2) {
            uint add = lane >= step ? prefix[lane - step] : 0;
            barrier();
            prefix[lane] += add;
            barrier();
        }
        if (visible) {
            uint target = (outputBase + total + prefix[lane] - 1) * 30;
            for (uint w = 0; w < 30; ++w) {
                dst[target + w] = src[base + 8 + w];
            }
        }
        barrier();
        if (lane == 0) {
            total += prefix[63];
        }
        barrier();
    }
    if (lane == 0) {
        args[group] = total;
    }
#endif
}
)GLSL";

bool gpu_make_engine() {
    auto& engine = g->gpu;
    if (engine.failed) {
        return false;
    }
    if (engine.compact && engine.arguments && engine.summary) {
        return true;
    }
    engine.failed = true;
    VkDescriptorSetLayoutBinding bindings[3]{};
    for (unsigned i = 0; i < 3; ++i) {
        bindings[i] =
            {i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    }
    VkDescriptorSetLayoutCreateInfo sets{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    sets.bindingCount = 3;
    sets.pBindings = bindings;
    if (!check(
            vkCreateDescriptorSetLayout(g->device, &sets, nullptr, &engine.set_layout),
            "GPU draw bindings"
        )) {
        return false;
    }
    VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layout.setLayoutCount = 1;
    layout.pSetLayouts = &engine.set_layout;
    if (!check(
            vkCreatePipelineLayout(g->device, &layout, nullptr, &engine.layout), "GPU draw layout"
        )) {
        return false;
    }
    for (unsigned pass = 0; pass < 3; ++pass) {
        std::string source(kGpuCullSource), error;
        if (pass) {
            source.insert(
                source.find('\n') + 1,
                pass == 1 ? "#define MAKE_ARGUMENTS 1\n"
                          : "#define SUMMARIZE 1\n#define SUMMARY_AT "
                                + std::to_string(kGpuSummaryAt) + "\n"
            );
        }
        CompiledProgram compiled;
        if (!compile_program({}, {}, compiled, error, source)) {
            log::error("GPU instance shader: {}", error);
            return false;
        }
        VkShaderModuleCreateInfo module_info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        module_info.codeSize = compiled.compute.size() * sizeof(std::uint32_t);
        module_info.pCode = compiled.compute.data();
        VkShaderModule module = VK_NULL_HANDLE;
        if (!check(
                vkCreateShaderModule(g->device, &module_info, nullptr, &module),
                "GPU instance shader"
            )) {
            return false;
        }
        VkComputePipelineCreateInfo pipeline{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipeline.stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        pipeline.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipeline.stage.module = module;
        pipeline.stage.pName = "main";
        pipeline.layout = engine.layout;
        auto* result = pass == 2   ? &engine.summary
                       : pass == 1 ? &engine.arguments
                                   : &engine.compact;
        const bool ok = check(
            vkCreateComputePipelines(g->device, g->pipeline_cache, 1, &pipeline, nullptr, result),
            "GPU instance pipeline"
        );
        vkDestroyShaderModule(g->device, module, nullptr);
        if (!ok) {
            return false;
        }
    }
    engine.failed = false;
    log::info("Vulkan GPU instance culling and indirect draws enabled");
    return true;
}

Chunk gpu_buffer(VkDeviceSize size, VkBufferUsageFlags usage, bool mapped) {
    Chunk result;
    result.size = size;
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = size;
    info.usage = usage;
    VmaAllocationCreateInfo allocation{};
    allocation.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if (mapped) {
        allocation.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                           | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocation.requiredFlags =
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    VmaAllocationInfo got{};
    if (!check(
            vmaCreateBuffer(
                g->allocator, &info, &allocation, &result.buffer, &result.allocation, &got
            ),
            "GPU instance page"
        )) {
        throw std::runtime_error("cannot allocate GPU instance page");
    }
    result.mapped = static_cast<std::uint8_t*>(got.pMappedData);
    return result;
}

// GPUOpen VMA's readback pattern: random host access prefers cached memory.
// https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html
Chunk gpu_counter_readback() {
    Chunk result;
    result.size = kGpuSummaryWords * 4;
    VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    info.size = result.size;
    info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo allocation{};
    allocation.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocation.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    allocation.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    VmaAllocationInfo got{};
    if (!check(
            vmaCreateBuffer(
                g->allocator, &info, &allocation, &result.buffer, &result.allocation, &got
            ),
            "GPU counter readback"
        )) {
        throw std::runtime_error("cannot allocate GPU counter readback");
    }
    result.mapped = static_cast<std::uint8_t*>(got.pMappedData);
    return result;
}

void gpu_reset(Frame& f) {
    f.gpu.used = 0;
    f.gpu.recording = 0;
    f.gpu.counters = false;
    for (auto& page : f.gpu.pages) {
        page.groups.clear();
        page.commands.clear();
        page.payload.clear();
        page.instances = 0;
        page.descriptors = VK_NULL_HANDLE;
    }
}

renderer::GpuInstanceBatch gpu_make_batch(
    unsigned buffer,
    std::span<const renderer::GpuCullBounds> bounds,
    const renderer::GpuCullView& view
) {
    if (!gpu_driven_active() || bounds.empty()) {
        return {};
    }
    auto* source = g->buffers.find(buffer);
    const auto count = static_cast<unsigned>(bounds.size());
    if (!source || count > kGpuPageInstances || source->data.size() != bounds.size() * 120) {
        throw std::runtime_error("invalid GPU instance source");
    }
    auto& work = frame().gpu;
    auto fits = [&](const GpuPage& page) {
        return page.instances + count <= kGpuPageInstances && page.groups.size() < kGpuPageGroups
               && (page.payload.size() + 58 + count * 38 + 4 * (page.groups.size() + 1)
                   + 4 * kGpuPageDraws + 4)
                          * 4
                      <= kGpuInputBytes;
    };
    if (!work.used || !fits(work.pages[work.used - 1])) {
        if (work.used == work.pages.size()) {
            work.pages.emplace_back();
            auto& page = work.pages.back();
            page.input = gpu_buffer(kGpuInputBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
            page.output = gpu_buffer(
                VkDeviceSize(kGpuPageInstances) * 120,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                false
            );
            page.arguments = gpu_buffer(
                VkDeviceSize(kGpuSummaryAt + kGpuSummaryWords) * 4,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
                    | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                false
            );
        }
        ++work.used;
    }
    auto& page = work.pages[work.used - 1];
    const auto view_at = static_cast<unsigned>(page.payload.size());
    page.payload.resize(page.payload.size() + 58);
    std::memcpy(page.payload.data() + view_at, view.planes.data(), 96);
    std::memcpy(page.payload.data() + view_at + 24, view.pvs.data(), 128);
    page.payload[view_at + 56] = view.plane_count;
    page.payload[view_at + 57] = (view.use_pvs ? 1u : 0u) | (view.bypass_ui ? 2u : 0u);
    const auto first = static_cast<unsigned>(page.payload.size());
    page.payload.resize(page.payload.size() + count * 38);
    for (unsigned i = 0; i < count; ++i) {
        std::memcpy(page.payload.data() + first + i * 38, &bounds[i], sizeof(bounds[i]));
        std::memcpy(page.payload.data() + first + i * 38 + 8, source->data.data() + i * 120, 120);
    }
    const auto group = static_cast<unsigned>(page.groups.size());
    page.groups.push_back({first, page.instances, count, view_at});
    page.instances += count;
    ++g->gpu.batches;
    g->gpu.candidates += count;
    return {g->recording, work.used - 1, group};
}

bool gpu_submit_draw(renderer::GpuInstanceBatch batch, unsigned first, unsigned count) {
    if (!batch || batch.recording != g->recording || batch.page >= frame().gpu.used) {
        return false;
    }
    auto& page = frame().gpu.pages[batch.page];
    if (batch.group >= page.groups.size() || !count) {
        return false;
    }
    if (page.commands.size() >= kGpuPageDraws) {
        throw std::runtime_error("GPU indirect page command capacity exceeded");
    }
    const auto command = static_cast<unsigned>(page.commands.size());
    page.commands.push_back({count, first, batch.group, 0});
    stats::count(stats::Counter::DrawCalls);
    stats::count(stats::Counter::Triangles, std::uint64_t(count / 3) * page.groups[batch.group][2]);
    draw(
        GL_TRIANGLES,
        static_cast<GLsizei>(count),
        1,
        true,
        GL_UNSIGNED_INT,
        std::uintptr_t(first) * 4,
        0,
        &page,
        command
    );
    ++g->gpu.indirect;
    return true;
}

void gpu_record() {
    auto& work = frame().gpu;
    // Empty recordings publish zero totals too, rather than retaining a stale scene.
    work.recording = g->recording;
    work.counters = stats::enabled();
    if (!work.counters) {
        g->gpu.completed = {};
        g->gpu.counter_floor = work.recording;
    }
    if (!work.used) {
        return;
    }
    g->gpu.timing_upload = true;
    {
        const stats::Scope timing("GPU instance visibility", stats::kGpu);
        const auto cmd = upload_cmd();
        for (unsigned i = 0; i < work.used; ++i) {
            auto& page = work.pages[i];
            if (page.commands.empty()) {
                continue;
            }
            const auto groups = static_cast<unsigned>(page.groups.size());
            const auto commands = static_cast<unsigned>(page.commands.size());
            const unsigned command_at = 4 + groups * 4, payload_at = command_at + commands * 4;
            auto* input = reinterpret_cast<std::uint32_t*>(page.input.mapped);
            input[0] = groups;
            input[1] = commands;
            input[2] = command_at;
            input[3] = 0;
            for (unsigned n = 0; n < groups; ++n) {
                auto group = page.groups[n];
                group[0] += payload_at;
                group[3] += payload_at;
                std::memcpy(input + 4 + n * 4, group.data(), 16);
            }
            std::memcpy(input + command_at, page.commands.data(), commands * 16);
            std::memcpy(input + payload_at, page.payload.data(), page.payload.size() * 4);
            page.descriptors = allocate_set(g->gpu.set_layout);
            if (!page.descriptors) {
                throw std::runtime_error("cannot allocate GPU draw descriptors");
            }
            VkDescriptorBufferInfo buffers[3] =
                {{page.input.buffer, 0, page.input.size},
                 {page.output.buffer, 0, page.output.size},
                 {page.arguments.buffer, 0, page.arguments.size}};
            VkWriteDescriptorSet writes[3]{};
            for (unsigned n = 0; n < 3; ++n) {
                writes[n] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writes[n].dstSet = page.descriptors;
                writes[n].dstBinding = n;
                writes[n].descriptorCount = 1;
                writes[n].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writes[n].pBufferInfo = &buffers[n];
            }
            vkUpdateDescriptorSets(g->device, 3, writes, 0, nullptr);
            vkCmdBindDescriptorSets(
                cmd,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                g->gpu.layout,
                0,
                1,
                &page.descriptors,
                0,
                nullptr
            );
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, g->gpu.compact);
            vkCmdDispatch(cmd, groups, 1, 1);
            VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                0,
                1,
                &barrier,
                0,
                nullptr,
                0,
                nullptr
            );
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, g->gpu.arguments);
            vkCmdDispatch(cmd, (commands + 63) / 64, 1, 1);
            if (work.counters) {
                if (!page.readback.buffer) {
                    page.readback = gpu_counter_readback();
                }
                vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    0,
                    1,
                    &barrier,
                    0,
                    nullptr,
                    0,
                    nullptr
                );
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, g->gpu.summary);
                vkCmdDispatch(cmd, 1, 1, 1);
                VkBufferMemoryBarrier copy{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
                copy.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                copy.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                copy.srcQueueFamilyIndex = copy.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                copy.buffer = page.arguments.buffer;
                copy.offset = kGpuSummaryAt * 4;
                copy.size = kGpuSummaryWords * 4;
                vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0,
                    nullptr,
                    1,
                    &copy,
                    0,
                    nullptr
                );
                const VkBufferCopy region{kGpuSummaryAt * 4, 0, kGpuSummaryWords * 4};
                vkCmdCopyBuffer(cmd, page.arguments.buffer, page.readback.buffer, 1, &region);
                copy.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                copy.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
                copy.buffer = page.readback.buffer;
                copy.offset = 0;
                vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_HOST_BIT,
                    0,
                    0,
                    nullptr,
                    1,
                    &copy,
                    0,
                    nullptr
                );
                ++g->submission.gpu_counter_dispatches;
                ++g->submission.gpu_counter_copies;
            }
        }
        VkMemoryBarrier visible{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        visible.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        visible.dstAccessMask =
            VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            1,
            &visible,
            0,
            nullptr,
            0,
            nullptr
        );
    }
    g->gpu.timing_upload = false;
}

void gpu_results(Frame& f) {
    if (!stats::enabled()) {
        g->gpu.completed = {};
        return;
    }
    if (!f.gpu.counters || !f.gpu.recording || f.gpu.reported == f.gpu.recording) {
        return;
    }
    // Called at existing retirement/readback boundaries only. Never introduce a
    // counter-specific wait, and never let an older slot replace a newer flush.
    if (!f.submitted || vkGetFenceStatus(g->device, f.fence) != VK_SUCCESS) {
        return;
    }
    f.gpu.reported = f.gpu.recording;
    if (f.gpu.recording <= g->gpu.completed.recording || f.gpu.recording <= g->gpu.counter_floor) {
        return;
    }
    GpuDrawCounters result;
    result.recording = f.gpu.recording;
    for (unsigned i = 0; i < f.gpu.used; ++i) {
        const auto& page = f.gpu.pages[i];
        if (page.commands.empty()) {
            continue;
        }
        std::array<std::uint32_t, kGpuSummaryWords> summary{};
        check(
            vmaInvalidateAllocation(g->allocator, page.readback.allocation, 0, VK_WHOLE_SIZE),
            "invalidate GPU counters"
        );
        std::memcpy(summary.data(), page.readback.mapped, sizeof(summary));
        result.readback_bytes += sizeof(summary);
        result.visible_instances += summary[0];
        result.draws += summary[1];
        result.triangles += std::uint64_t(summary[2]) | (std::uint64_t(summary[3]) << 32);
    }
    g->gpu.completed = result;
}

void gpu_release() {
    for (auto& slot : g->frames) {
        for (auto& page : slot.gpu.pages) {
            for (auto* buffer : {&page.input, &page.output, &page.arguments, &page.readback}) {
                if (buffer->buffer) {
                    vmaDestroyBuffer(g->allocator, buffer->buffer, buffer->allocation);
                }
            }
        }
        slot.gpu.pages.clear();
    }
    vkDestroyPipeline(g->device, g->gpu.compact, nullptr);
    vkDestroyPipeline(g->device, g->gpu.arguments, nullptr);
    vkDestroyPipeline(g->device, g->gpu.summary, nullptr);
    vkDestroyPipelineLayout(g->device, g->gpu.layout, nullptr);
    vkDestroyDescriptorSetLayout(g->device, g->gpu.set_layout, nullptr);
}
