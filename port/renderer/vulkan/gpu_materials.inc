// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// Included inside vkgl_draw.cpp: the native material path leaves GL shader sources unchanged.

bool material_enabled() {
    static const bool& off = stats::probe(
        "OPENRAC_GPU_MATERIALS_OFF",
        "Reference: individual mesh material uniforms and textures (requires shader reload)"
    );
    return g->bindless_supported && !off;
}

bool material_layout() {
    if (g->material_layout) {
        return true;
    }
    const VkDescriptorSetLayoutBinding bindings[] =
        {{0,
          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          g->material_texture_capacity,
          VK_SHADER_STAGE_FRAGMENT_BIT,
          nullptr},
         {1,
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          1,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
          nullptr}};
    const VkDescriptorBindingFlags flags[] = {
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, 0
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo options{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO
    };
    options.bindingCount = 2;
    options.pBindingFlags = flags;
    VkDescriptorSetLayoutCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.pNext = &options;
    info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    info.bindingCount = 2;
    info.pBindings = bindings;
    return check(
        vkCreateDescriptorSetLayout(g->device, &info, nullptr, &g->material_layout),
        "bindless material layout"
    );
}

bool material_program(Program& p, const std::string& vertex, const std::string& fragment) {
    p.gpu_materials = false;
    if (!material_enabled() || vertex.find("in mat4 model;") == std::string::npos) {
        return true;
    }
    const auto& original = p.compiled;
    if (std::none_of(original.uniforms.begin(), original.uniforms.end(), [](const auto& u) {
            return u.name == "material_colour" || u.name == "shadow_view";
        })) {
        return true;
    }
    if (!material_layout()) {
        return false;
    }

    struct Field {
        const char* name;
        const char* value;
        unsigned offset;
    };

    constexpr Field fields[] =
        {{"material_colour", "O_MATERIAL.colour", 0},
         {"material_emission", "O_MATERIAL.emission_diffuse.xyz", 16},
         {"material_diffuse_scale", "O_MATERIAL.emission_diffuse.w", 28},
         {"cutout", "int(O_MATERIAL.flags.x)", 32},
         {"emission_mask", "int(O_MATERIAL.flags.y)", 36},
         {"emission_vertex_colour", "int(O_MATERIAL.flags.z)", 40},
         {"probe_effect", "int(O_MATERIAL.flags.w)", 44},
         // Opaque-only programs fix probe_effect to zero and reuse its word.
         {"opaque_solid", "int(O_MATERIAL.flags.w)", 44},
         {"hero_shadow_receiver", "int(O_MATERIAL.resources.z)", 56},
         // Depth programs use the spare resource word for their atlas view;
         // colour programs use it for terrain albedo. They never coexist.
         {"shadow_view", "int(O_MATERIAL.resources.w)", 60},
         {"probe_terrain_albedo", "int(O_MATERIAL.resources.w)", 60},
         {"skinning", "int(O_MATERIAL.draw_flags.x)", 64},
         {"lighting", "int(O_MATERIAL.draw_flags.y)", 68},
         {"probe_receiver", "int(O_MATERIAL.draw_flags.z)", 72},
         {"probe_mode", "int(O_MATERIAL.draw_flags.w)", 76}};
    // Specialised colour programs already separate static and skinned work.
    // Receiver/mode are common to each run: retaining real uniforms allows
    // scalar branches instead of divergent per-draw buffer loads. Lighting
    // remains per draw so terrain and lit static layers can still batch.
    const bool specialised = vertex.find("#define OPENRAC_COLOUR_SPECIALISED") != std::string::npos;
    const auto per_draw = [&](std::string_view name) {
        return !specialised || (name != "probe_mode" && name != "probe_receiver" && name != "skinning");
    };
    std::string vs(vertex), fs(fragment), macros;
    for (const auto& field : fields) {
        if (!per_draw(field.name)) {
            continue;
        }
        if (std::none_of(original.uniforms.begin(), original.uniforms.end(), [&](const auto& u) {
                return u.name == field.name;
            })) {
            continue;
        }
        const std::regex declaration(
            std::string(R"(uniform\s+(vec4|vec3|float|int)\s+)") + field.name + R"(\s*;)"
        );
        vs = std::regex_replace(vs, declaration, "");
        fs = std::regex_replace(fs, declaration, "");
        macros += std::string("#define ") + field.name + " (" + field.value + ")\n";
    }
    for (const char* name : {"tex_T0", "emission_texture"}) {
        const std::regex declaration(std::string(R"(uniform\s+sampler2D\s+)") + name + R"(\s*;)");
        fs = std::regex_replace(fs, declaration, "");
    }
    const std::string vertex_header = R"(#extension GL_ARB_shader_draw_parameters : require

layout(push_constant) uniform OpenracDrawBase {
    uint base;
}

openrac_base;
flat out uint openrac_draw;
)";
    const std::string fragment_header =
        std::string(R"(#extension GL_EXT_nonuniform_qualifier : require
flat in uint openrac_draw;

struct OpenracMaterial {
    vec4 colour;
    vec4 emission_diffuse;
    uvec4 flags;
    uvec4 resources;
    uvec4 draw_flags;
};

layout(std430, set = 1, binding = 1) readonly buffer OpenracMaterials {
    uint openrac_draws[32768];
    OpenracMaterial openrac_materials[];
};
layout(set=1,binding=0) uniform sampler2D openrac_textures[
)") + std::to_string(g->material_texture_capacity)
        + R"(];
#define O_MATERIAL (openrac_materials[openrac_draws[openrac_draw]])
#define tex_T0 openrac_textures[nonuniformEXT(O_MATERIAL.resources.x)]
#define emission_texture openrac_textures[nonuniformEXT(O_MATERIAL.resources.y)]
)" + macros;
    vs.replace(0, vs.find('\n'), "#version 450 core");
    fs.replace(0, fs.find('\n'), "#version 450 core");
    vs.insert(vs.find('\n') + 1, vertex_header);
    // Shadow view selection and colour-layer flags are read per draw in the
    // vertex stage as well as by fragment material/lighting evaluation.
    if (std::any_of(original.uniforms.begin(), original.uniforms.end(), [](const auto& u) {
            return u.name == "shadow_view" || u.name == "skinning"
                   || u.name == "probe_receiver" || u.name == "probe_mode";
        })) {
        const auto begin = fragment_header.find("struct OpenracMaterial");
        const auto end = fragment_header.find("layout(set=1,binding=0)");
        const auto main_at = vs.find("void main() {");
        vs.insert(
            main_at,
            fragment_header.substr(begin, end - begin)
                + "\n#define O_MATERIAL (openrac_materials[openrac_draws[openrac_draw]])\n" + macros
        );
    }
    fs.insert(fs.find('\n') + 1, fragment_header);
    const auto main = vs.find("void main() {");
    if (main == std::string::npos) {
        p.log = "material vertex entry missing";
        return false;
    }
    vs.insert(
        main + std::strlen("void main() {"),
        "\nopenrac_draw = openrac_base.base + uint(gl_DrawIDARB);\n"
    );
    CompiledProgram compiled;
    if (!compile_program(vs, fs, compiled, p.log)) {
        return false;
    }
    std::erase_if(compiled.uniforms, [](const auto& u) { return u.name == "openrac_textures"; });
    std::erase_if(compiled.storage, [](const auto& b) { return b.name == "OpenracMaterials"; });
    std::erase_if(compiled.blocks, [](const auto& b) { return b.name == "OpenracDrawBase"; });
    p.material_start = (static_cast<unsigned>(std::max(compiled.default_size, 0)) + 15u) & ~15u;
    for (auto u : original.uniforms) {
        if (u.name == "tex_T0" || u.name == "emission_texture") {
            u.binding = -2;
            compiled.uniforms.push_back(std::move(u));
            continue;
        }
        for (const auto& field : fields) {
            if (per_draw(field.name) && u.name == field.name) {
                u.offset = static_cast<int>(p.material_start + field.offset);
                compiled.uniforms.push_back(std::move(u));
                break;
            }
        }
    }
    p.compiled = std::move(compiled);
    p.gpu_materials = true;
    return true;
}

void material_reset(Frame& f) {
    f.material_pages_used = 0;
    for (auto& page : f.material_pages) {
        page.draws = 0;
        page.materials.clear();
        page.textures.clear();
    }
}

std::array<std::uint64_t, 2> material_texture_key(const Sampled& sample) {
    return {
        reinterpret_cast<std::uint64_t>(sample.view),
        reinterpret_cast<std::uint64_t>(sample.sampler)
    };
}

MaterialPage& material_page(const std::array<Sampled, 2>& textures) {
    auto& f = frame();
    const auto fits = [&](const MaterialPage& page) {
        unsigned additional = 0;
        for (unsigned i = 0; i < 2; ++i) {
            if (textures[i].image && !page.textures.contains(material_texture_key(textures[i]))
                && (i == 0 || !textures[0].image
                    || material_texture_key(textures[0]) != material_texture_key(textures[i]))) {
                ++additional;
            }
        }
        return page.draws < kMaterialDraws
               && page.textures.size() + additional <= g->material_texture_capacity;
    };
    if (!f.material_pages_used || !fits(f.material_pages[f.material_pages_used - 1])) {
        if (f.material_pages_used == f.material_pages.size()) {
            auto& page = f.material_pages.emplace_back();
            page.data = gpu_buffer(
                VkDeviceSize(kMaterialDraws) * (4 + kMaterialBytes),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                true
            );
            const VkDescriptorPoolSize sizes[] =
                {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, g->material_texture_capacity},
                 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}};
            VkDescriptorPoolCreateInfo pool{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
            pool.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
            pool.maxSets = 1;
            pool.poolSizeCount = 2;
            pool.pPoolSizes = sizes;
            if (!check(
                    vkCreateDescriptorPool(g->device, &pool, nullptr, &page.pool), "material pool"
                )) {
                throw std::runtime_error("cannot allocate material pool");
            }
            VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            alloc.descriptorPool = page.pool;
            alloc.descriptorSetCount = 1;
            alloc.pSetLayouts = &g->material_layout;
            if (!check(
                    vkAllocateDescriptorSets(g->device, &alloc, &page.set), "material descriptors"
                )) {
                throw std::runtime_error("cannot allocate material descriptors");
            }
            ++g->submission.descriptor_sets_allocated;
            const VkDescriptorBufferInfo buffer{page.data.buffer, 0, page.data.size};
            VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write.dstSet = page.set;
            write.dstBinding = 1;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            write.pBufferInfo = &buffer;
            vkUpdateDescriptorSets(g->device, 1, &write, 0, nullptr);
            ++g->submission.descriptor_writes;
        }
        ++f.material_pages_used;
    }
    return f.material_pages[f.material_pages_used - 1];
}

unsigned material_texture(MaterialPage& page, const Sampled& sample) {
    const auto key = material_texture_key(sample);
    auto [it, added] = page.textures.try_emplace(key, static_cast<unsigned>(page.textures.size()));
    if (added) {
        const VkDescriptorImageInfo image{
            sample.sampler, sample.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write.dstSet = page.set;
        write.dstBinding = 0;
        write.dstArrayElement = it->second;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &image;
        vkUpdateDescriptorSets(g->device, 1, &write, 0, nullptr);
        ++g->submission.descriptor_writes;
        ++g->material_textures;
    }
    return it->second;
}

struct MaterialBinding {
    VkDescriptorSet set = VK_NULL_HANDLE;
    unsigned draw = 0;
};

MaterialBinding material_binding(Program& p, const Target& target) {
    if (!p.gpu_materials) {
        return {};
    }
    // Resolve and transition before binding the target. Resource identity includes sampler and mip view.
    std::array<Sampled, 2> textures{};
    for (std::size_t i = 0; i < p.compiled.uniforms.size(); ++i) {
        const auto& u = p.compiled.uniforms[i];
        if (!u.sampler || u.binding != -2) {
            continue;
        }
        auto sample = sampled(p, i);
        if (sample.image == target.colour || sample.image == target.depth) {
            sample.image = g->dummy_image.get();
            sample.view = sample_view(*sample.image, 0, 0);
        }
        if (sample.image->layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            end_rendering();
            barrier(frame().main, *sample.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        sample.image->touched = g->recording;
        textures[u.name == "tex_T0" ? 0 : 1] = sample;
    }
    auto& page = material_page(textures);
    std::array<std::uint32_t, kMaterialWords> record;
    std::memcpy(record.data(), p.uniforms.data() + p.material_start, kMaterialBytes);
    for (unsigned i = 0; i < 2; ++i) {
        // Depth/shadow programs may not use an emission sampler.
        record[12 + i] = textures[i].image ? material_texture(page, textures[i]) : 0;
    }
    auto [it, added] =
        page.materials.try_emplace(record, static_cast<unsigned>(page.materials.size()));
    if (added) {
        std::memcpy(
            page.data.mapped + kMaterialDraws * 4 + it->second * kMaterialBytes,
            record.data(),
            kMaterialBytes
        );
        ++g->material_records;
    }
    const unsigned draw = page.draws++;
    std::memcpy(page.data.mapped + draw * 4, &it->second, 4);
    ++g->material_draws;
    return {page.set, draw};
}

void material_release() {
    for (auto& f : g->frames) {
        for (auto& page : f.material_pages) {
            if (page.pool) {
                vkDestroyDescriptorPool(g->device, page.pool, nullptr);
            }
            if (page.data.buffer) {
                vmaDestroyBuffer(g->allocator, page.data.buffer, page.data.allocation);
            }
        }
    }
    if (g->material_layout) {
        vkDestroyDescriptorSetLayout(g->device, g->material_layout, nullptr);
    }
}
