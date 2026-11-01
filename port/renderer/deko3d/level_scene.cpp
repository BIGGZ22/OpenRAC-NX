// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/deko3d/level_scene.h"
#include <algorithm>
#include <map>
#include <tuple>

namespace openrac::deko {
void LevelScene::release() {
    m_level=nullptr; m_mesh=0; m_material_textures.clear();
    m_renderer.release_resources();
}
bool LevelScene::upload(const scene::LevelData& level, std::string& error) {
    // Validate all asset references before allocating or retiring the old level.
    for (const auto& model : level.models) for (const auto& primitive : model.primitives) {
        if (primitive.material<0 || std::size_t(primitive.material)>=level.materials.size()
            || primitive.first_index>level.indices.size()
            || primitive.index_count>level.indices.size()-primitive.first_index || primitive.index_count%3) {
            error="Native level has an invalid material or primitive range."; return false;
        }
    }
    for (const auto& material : level.materials)
        if (material.image < -1 || (material.image>=0 && std::size_t(material.image)>=level.images.size())) {
            error="Native level material references an invalid image."; return false;
        }
    release();
    m_mesh=m_renderer.create_mesh(level.vertices,level.indices,error);
    if (!m_mesh) return false;
    m_material_textures.resize(level.materials.size());
    // Each distinct image/sampler pair is uploaded once, not once per object.
    std::map<std::tuple<int,bool,bool>,TextureId> textures;
    for (std::size_t i=0;i<level.materials.size();++i) {
        const auto& material=level.materials[i];
        if (material.image<0) continue;
        const auto key=std::tuple{material.image,material.clamp_s,material.clamp_t};
        auto [at,inserted]=textures.try_emplace(key);
        if (inserted) {
            at->second=m_renderer.create_texture(level.images[material.image],true,!material.clamp_s,!material.clamp_t,error);
            if (!at->second) { release(); return false; }
        }
        m_material_textures[i]=at->second;
    }
    m_level=&level;
    return true;
}
bool LevelScene::append_static(scene::Layer layer, std::vector<SceneDraw>& out, std::string& error) const {
    if (!m_level || std::size_t(layer)>=scene::kLayerCount) { error="Native level layer is unavailable."; return false; }
    return append(layer,m_level->instances[std::size_t(layer)],{},out,error);
}
bool LevelScene::append(scene::Layer layer, std::span<const scene::Instance> instances,
                        std::span<const renderer::Mat4> palette, std::vector<SceneDraw>& out,
                        std::string& error) const {
    if (!m_level || !m_mesh || std::size_t(layer)>=scene::kLayerCount) { error="Native level layer is unavailable."; return false; }
    const auto begin=out.size();
    // Native frame arrays own only host data; malformed capture fails atomically.
    const auto fail=[&](const char* message) { out.resize(begin); error=message; return false; };
    // Solid geometry first, then translucent surfaces, then chrome/glass packets.
    for (unsigned pass=0;pass<3;++pass) for (const auto& instance : instances) {
        if (instance.model>=m_level->models.size()) return fail("Native scene instance has an invalid model.");
        const auto& model=m_level->models[instance.model];
        std::span<const renderer::Mat4> joints;
        if (instance.palette>=0 && model.joints>0) {
            const auto base=std::size_t(instance.palette), count=std::size_t(model.joints);
            if (count>256 || base>palette.size() || count>palette.size()-base) return fail("Native scene pose is outside the captured joint palette.");
            joints=palette.subspan(base,count);
        }
        std::span<const std::uint32_t> colours;
        if (instance.lights>=0 && layer!=scene::Layer::Mobys) {
            const auto base=std::size_t(instance.lights);
            const unsigned count=layer==scene::Layer::Shrubs ? 24 : 64;
            if (base>m_level->light_colours.size() || count>m_level->light_colours.size()-base)
                return fail("Native scene instance lighting is outside the level colour table.");
            colours=std::span(m_level->light_colours).subspan(base,count);
        }
        for (const auto& primitive : model.primitives) {
            const auto& material=m_level->materials[primitive.material];
            const bool blended=material.blend || instance.blend!=0;
            if (pass==0 && (material.metal || blended)) continue;
            if (pass==1 && (material.metal || (!blended && !material.cutout))) continue;
            if (pass==2 && !material.metal) continue;
            SceneDraw draw;
            draw.mesh=m_mesh; draw.texture=m_material_textures[primitive.material];
            draw.first_index=primitive.first_index; draw.index_count=primitive.index_count;
            draw.model=instance.matrix; draw.tint=instance.tint; draw.material=material.colour;
            draw.moby_light=instance.moby_light; draw.ambient=instance.moby_ambient;
            draw.joints=joints; draw.light_colours=colours;
            draw.depth_test=layer!=scene::Layer::Sky;
            draw.depth_write=pass==0 && layer!=scene::Layer::Sky;
            draw.metal=material.metal;
            draw.blend=pass==0 ? Blend::Opaque : instance.blend==2 ? Blend::Additive : Blend::Alpha;
            draw.alpha_test=material.cutout ? (pass==0 ? 2u : 3u) : 0u;
            out.push_back(draw);
        }
    }
    return true;
}
}
