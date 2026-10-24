// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Included inside vkgl_draw.cpp: native instance work shares its frame-slot owners.
constexpr unsigned kGpuSummaryAt = kGpuPageGroups + 5 * kGpuPageDraws;
constexpr unsigned kGpuSummaryWords = 4;
// A group record whose first word has this bit reads its candidates from the resident words
// (binding 3, GpuResident) at the rest of that word, not from the page's payload.
constexpr std::uint32_t kGpuResidentBit = 0x80000000u;
// A candidate's words: eight of bounds and visibility, then thirty of instance data.
constexpr unsigned kGpuCandidateWords = 38;
// A view's words in the payload: six planes of four, 32 of PVS, the plane count and the flags.
constexpr unsigned kGpuViewWords = 58;
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
layout(std430, binding = 3) readonly buffer ResidentWords {
    uint res[];
};
// Word `at` of a candidate: from the resident words for a resident group, else the page's.
uint candidate(bool resident, uint at) {
    return resident ? res[at] : src[at];
}
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
    bool resident = (begin & 0x80000000u) != 0u;
    begin &= 0x7fffffffu;
    if (lane == 0) {
        total = 0;
    }
    barrier();
    for (uint chunk = 0; chunk < count; chunk += 64) {
        uint index = chunk + lane, base = begin + index * 38;
        bool visible = index < count;
        if (visible) {
            uint flags = candidate(resident, base + 7), word = candidate(resident, base + 6);
            if ((flags & 2) == 0 || (src[view + 57] & 2) == 0) {
                uint byte = word >> 8, bit = word & 255;
                if ((src[view + 57] & 1) != 0 && byte < 128 && bit != 0 && (bit & (bit - 1)) == 0) {
                    uint mask = (src[view + 24 + byte / 4] >> ((byte % 4) * 8)) & 255;
                    visible = (mask & bit) != 0;
                }
                if (visible && (flags & 1) != 0) {
                    vec3 low = vec3(
                        uintBitsToFloat(candidate(resident, base + 0)),
                        uintBitsToFloat(candidate(resident, base + 1)),
                        uintBitsToFloat(candidate(resident, base + 2))
                    );
                    vec3 high = vec3(
                        uintBitsToFloat(candidate(resident, base + 3)),
                        uintBitsToFloat(candidate(resident, base + 4)),
                        uintBitsToFloat(candidate(resident, base + 5))
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
                dst[target + w] = candidate(resident, base + 8 + w);
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
    // The page's input, output and arguments, and the resident words.
    VkDescriptorSetLayoutBinding bindings[4]{};
    for (unsigned i = 0; i < 4; ++i) {
        bindings[i] =
            {i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    }
    VkDescriptorSetLayoutCreateInfo sets{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    sets.bindingCount = 4;
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
        count_pipeline();
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
    f.gpu.resident = VK_NULL_HANDLE;
    for (auto& page : f.gpu.pages) {
        page.groups.clear();
        page.commands.clear();
        page.payload.clear();
        page.instances = 0;
        page.descriptors = VK_NULL_HANDLE;
    }
}

// Returns the page a batch of `count` instances and `words` payload words goes into: the
// recording's last page, or a new one when that is full (a page referenced by recorded draws is
// never grown or moved).
GpuPage& gpu_page_for(unsigned count, unsigned words) {
    auto& work = frame().gpu;
    // The page's header, group records, commands and summary are 4 words each beside the payload.
    auto fits = [&](const GpuPage& page) {
        return page.instances + count <= kGpuPageInstances && page.groups.size() < kGpuPageGroups
               && (page.payload.size() + words + 4 * (page.groups.size() + 1) + 4 * kGpuPageDraws
                   + 4) * 4
                      <= kGpuInputBytes;
    };

    // The last page is full, or there is none yet in this recording.
    if (!work.used || !fits(work.pages[work.used - 1])) {
        // Every page of the slot is in use: one more, kept for the slot's later recordings.
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

    return work.pages[work.used - 1];
}

// Appends `view` to `page`'s payload, as the culling reads it (kGpuViewWords words); returns
// the word where it starts.
unsigned gpu_write_view(GpuPage& page, const renderer::GpuCullView& view) {
    constexpr unsigned kPvsAt = 24;  // after the six planes of four words
    const auto view_at = static_cast<unsigned>(page.payload.size());
    page.payload.resize(page.payload.size() + kGpuViewWords);
    std::memcpy(page.payload.data() + view_at, view.planes.data(), sizeof(view.planes));
    std::memcpy(page.payload.data() + view_at + kPvsAt, view.pvs.data(), sizeof(view.pvs));
    page.payload[view_at + kGpuViewWords - 2] = view.plane_count;
    page.payload[view_at + kGpuViewWords - 1] =
        (view.use_pvs ? 1u : 0u) | (view.bypass_ui ? 2u : 0u);
    return view_at;
}

// Makes sure this recording's culling can read the resident words from device memory: when they
// changed since their last copy, a new copy is staged through the ring into the upload command
// buffer, and the old one goes when the frames that read it have retired. False when this
// recording already reads an older copy (its pages bind one resident buffer).
bool gpu_resident_ready() {
    auto& resident = g->gpu.resident;
    auto& work = frame().gpu;

    // Changed since the copy: a new one, unless this recording's pages read the old one.
    if (resident.dirty) {
        // The batches already made this recording read the old copy: the rest take the payload.
        if (work.resident != VK_NULL_HANDLE) {
            return false;
        }

        const VkDeviceSize bytes = VkDeviceSize(resident.words.size()) * 4;
        const Slice staged = ring_copy(resident.words.data(), bytes);

        // No staging: the per-frame copy draws this frame, and the upload is tried again.
        if (staged.data == nullptr) {
            return false;
        }

        Chunk copy = gpu_buffer(
            bytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false
        );
        const VkCommandBuffer cmd = upload_cmd();
        const VkBufferCopy region{staged.offset, 0, bytes};
        vkCmdCopyBuffer(cmd, staged.buffer, copy.buffer, 1, &region);
        count_command(Command::Copy);
        // The culling dispatches, recorded later into the same command buffer, read the copy.
        VkMemoryBarrier written{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        written.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        written.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            1,
            &written,
            0,
            nullptr,
            0,
            nullptr
        );
        count_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        // An older copy is read by frames still in flight: it goes when this slot retires.
        if (resident.buffer.buffer != VK_NULL_HANDLE) {
            defer([old = resident.buffer] {
                vmaDestroyBuffer(g->allocator, old.buffer, old.allocation);
            });
        }

        resident.buffer = copy;
        resident.dirty = false;
    }

    work.resident = resident.buffer.buffer;
    return true;
}

// A batch of the registered instances of buffer `name` against `view`: only the view and a group
// record go into the page. Empty when the buffer is not registered, was written since, holds
// another count, or the resident words cannot be read this recording: the caller copies then.
renderer::GpuInstanceBatch gpu_resident_batch(
    unsigned name, const Buffer& source, unsigned count, const renderer::GpuCullView& view
) {
    const auto& groups = g->gpu.resident.groups;
    const auto found = groups.find(name);

    // Not registered, or not as it is now.
    if (found == groups.end() || found->second.version != source.version
        || found->second.count != count) {
        return {};
    }

    // Its words are not readable this recording.
    if (!gpu_resident_ready()) {
        return {};
    }

    auto& page = gpu_page_for(count, kGpuViewWords);
    const unsigned view_at = gpu_write_view(page, view);
    const auto group = static_cast<unsigned>(page.groups.size());
    page.groups.push_back({found->second.first | kGpuResidentBit, page.instances, count, view_at});
    page.instances += count;
    // Only the view is written into the page's payload, in its 32-bit words.
    g->submission.gpu_payload_bytes += std::uint64_t{kGpuViewWords} * 4;
    ++g->gpu.batches;
    g->gpu.candidates += count;
    return {g->recording, frame().gpu.used - 1, group};
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

    // Registered static instances: the view and a group record only (gpu-resident-instances).
    if (renderer::render_opt::enabled(renderer::render_opt::Opt::GpuResidentInstances)) {
        // The resident path took it; otherwise the instances are copied below, as before.
        if (const auto batch = gpu_resident_batch(buffer, *source, count, view)) {
            return batch;
        }
    }

    auto& work = frame().gpu;
    auto& page = gpu_page_for(count, kGpuViewWords + count * kGpuCandidateWords);
    const unsigned view_at = gpu_write_view(page, view);
    const auto first = static_cast<unsigned>(page.payload.size());
    page.payload.resize(page.payload.size() + count * 38);
    for (unsigned i = 0; i < count; ++i) {
        std::memcpy(page.payload.data() + first + i * 38, &bounds[i], sizeof(bounds[i]));
        std::memcpy(page.payload.data() + first + i * 38 + 8, source->data.data() + i * 120, 120);
    }
    const auto group = static_cast<unsigned>(page.groups.size());
    page.groups.push_back({first, page.instances, count, view_at});
    page.instances += count;
    // The view and the instances written into the page's payload, in its 32-bit words.
    g->submission.gpu_payload_bytes += std::uint64_t{page.payload.size() - view_at} * 4;
    ++g->gpu.batches;
    g->gpu.candidates += count;
    return {g->recording, work.used - 1, group};
}

bool gpu_register(unsigned buffer, std::span<const renderer::GpuCullBounds> bounds) {
    // Nothing culls on the GPU, or nothing to register.
    if (!gpu_driven_active() || bounds.empty()) {
        return false;
    }

    const Buffer* source = g->buffers.find(buffer);
    const auto count = static_cast<unsigned>(bounds.size());

    // A batch of it must fit a page, and the buffer must hold one instance per bound.
    if (source == nullptr || count > kGpuPageInstances
        || source->data.size() != bounds.size() * sizeof(std::uint32_t) * 30) {
        return false;
    }

    auto& resident = g->gpu.resident;
    const std::size_t first = resident.words.size();

    // The culling names a resident word with 31 bits.
    if (first + std::size_t{count} * kGpuCandidateWords >= kGpuResidentBit) {
        return false;
    }

    resident.words.resize(first + std::size_t{count} * kGpuCandidateWords);

    // Each instance as a candidate of the payload is laid out: its bounds, then its data.
    for (unsigned i = 0; i < count; ++i) {
        std::uint32_t* at = resident.words.data() + first + std::size_t{i} * kGpuCandidateWords;
        std::memcpy(at, &bounds[i], sizeof(bounds[i]));
        std::memcpy(
            at + sizeof(bounds[i]) / 4,
            source->data.data() + std::size_t{i} * 120,
            (kGpuCandidateWords - sizeof(bounds[i]) / 4) * 4
        );
    }

    resident.groups[buffer] = {static_cast<std::uint32_t>(first), count, source->version};
    resident.dirty = true;
    return true;
}

void gpu_release_resident() {
    auto& resident = g->gpu.resident;

    // The copy may be read by frames in flight: it goes when the recording slot retires.
    if (resident.buffer.buffer != VK_NULL_HANDLE) {
        defer([old = resident.buffer] {
            vmaDestroyBuffer(g->allocator, old.buffer, old.allocation);
        });
    }

    resident = GpuResident{};
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

                // A resident group's candidates are in the resident words, not after the commands.
                if ((group[0] & kGpuResidentBit) == 0) {
                    group[0] += payload_at;
                }

                group[3] += payload_at;
                std::memcpy(input + 4 + n * 4, group.data(), 16);
            }
            std::memcpy(input + command_at, page.commands.data(), commands * 16);
            std::memcpy(input + payload_at, page.payload.data(), page.payload.size() * 4);
            // The header, groups, commands and payload just written into the mapped page.
            g->submission.gpu_payload_bytes +=
                std::uint64_t{payload_at} * 4 + page.payload.size() * 4;
            page.descriptors = allocate_set(g->gpu.set_layout);
            if (!page.descriptors) {
                throw std::runtime_error("cannot allocate GPU draw descriptors");
            }
            // Binding 3, the resident words, is the input page again where none are read.
            const VkDescriptorBufferInfo resident =
                work.resident != VK_NULL_HANDLE
                    ? VkDescriptorBufferInfo{work.resident, 0, VK_WHOLE_SIZE}
                    : VkDescriptorBufferInfo{page.input.buffer, 0, page.input.size};
            VkDescriptorBufferInfo buffers[4] =
                {{page.input.buffer, 0, page.input.size},
                 {page.output.buffer, 0, page.output.size},
                 {page.arguments.buffer, 0, page.arguments.size},
                 resident};
            VkWriteDescriptorSet writes[4]{};
            for (unsigned n = 0; n < 4; ++n) {
                writes[n] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writes[n].dstSet = page.descriptors;
                writes[n].dstBinding = n;
                writes[n].descriptorCount = 1;
                writes[n].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writes[n].pBufferInfo = &buffers[n];
            }
            vkUpdateDescriptorSets(g->device, 4, writes, 0, nullptr);
            g->submission.descriptor_writes += std::size(writes);
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
            count_command(Command::BindDescriptors);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, g->gpu.compact);
            count_command(Command::BindPipeline);
            vkCmdDispatch(cmd, groups, 1, 1);
            count_command(Command::Dispatch);
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
            count_barrier(
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
            );
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, g->gpu.arguments);
            count_command(Command::BindPipeline);
            vkCmdDispatch(cmd, (commands + 63) / 64, 1, 1);
            count_command(Command::Dispatch);
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
                count_barrier(
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                );
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, g->gpu.summary);
                count_command(Command::BindPipeline);
                vkCmdDispatch(cmd, 1, 1, 1);
                count_command(Command::Dispatch);
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
                count_barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
                const VkBufferCopy region{kGpuSummaryAt * 4, 0, kGpuSummaryWords * 4};
                vkCmdCopyBuffer(cmd, page.arguments.buffer, page.readback.buffer, 1, &region);
                count_command(Command::Copy);
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
                count_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
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
        count_barrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
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
    if (g->gpu.resident.buffer.buffer) {
        vmaDestroyBuffer(
            g->allocator, g->gpu.resident.buffer.buffer, g->gpu.resident.buffer.allocation
        );
    }
    g->gpu.resident = GpuResident{};
    vkDestroyPipeline(g->device, g->gpu.compact, nullptr);
    vkDestroyPipeline(g->device, g->gpu.arguments, nullptr);
    vkDestroyPipeline(g->device, g->gpu.summary, nullptr);
    vkDestroyPipelineLayout(g->device, g->gpu.layout, nullptr);
    vkDestroyDescriptorSetLayout(g->device, g->gpu.set_layout, nullptr);
}
