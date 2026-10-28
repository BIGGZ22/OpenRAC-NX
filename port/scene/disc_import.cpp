// SPDX-License-Identifier: GPL-3.0-or-later
// Native counterpart of editor/level.py and editor/port.py, using the shared
// format readers. No interpreter, GL context or copyrighted data is embedded.
#include "scene/disc_import.h"
#include "assets/disc/frontend.h"
#include "assets/disc/transition.h"
#include "assets/disc/wad.h"
#include "assets/geometry/gadget.h"
#include "assets/geometry/sky.h"
#include "assets/geometry/tfrag.h"
#include "assets/geometry/tfrag_lighting.h"
#include "assets/geometry/tie_lighting.h"
#include "assets/geometry/shrub_lighting.h"
#include "assets/world/spaceships.h"
#include "assets/world/occlusion.h"
#include <map>
#include <cmath>
#include <limits>
#include <cstring>
namespace openrac::scene {
namespace {
using namespace assets;
namespace d = assets::disc;
namespace r = assets::rac1;
std::vector<u8> decoded(ByteView b) { return d::is_wad(b) ? d::wad_decompress(b) : b.to_vector(); }
void tick(const SetupProgress& cb, std::string text) { if (cb && !cb(text, 0, 0)) throw std::runtime_error("Setup cancelled; completed levels are kept"); }
struct Source {
    std::vector<u8> index, data, gs, gameplay;
    d::LevelCore core;
};
Source source(const d::Disc& disc, int id) {
    Source s;
    if (id == 99) {
        const auto range = d::global_sector_range(disc.toc(), 0x14e8);
        auto title = d::TitleWorld::parse(disc.iso().read_sectors(range.offset, range.size));
        s.index = std::move(title.index); s.data = std::move(title.data);
        s.gs = std::move(title.gs); s.gameplay = std::move(title.gameplay);
    } else if (id == 98) {
        const auto range = d::global_sector_range(disc.toc(), 0x13b8);
        auto bytes = decoded(disc.iso().read_sectors(range.offset, range.size));
        ByteView lump(bytes);
        const auto h = [&](std::size_t i) { return lump.read<s32>(i * 4, "flight header"); };
        const int base = h(1);
        lump.check(0, base, "flight data base");
        s.index.resize(0x100 + base);
        std::memcpy(s.index.data() + 0x100, bytes.data(), base);
        const auto put = [&](int at, s32 value) { std::memcpy(s.index.data() + at, &value, 4); };
        const auto table = [&](int at, int n, int off) { put(at, h(n)); put(at + 4, h(off) + 0x100); };
        table(0,2,3); put(0x10,h(0x12)); table(0x18,4,5); table(0x38,6,7);
        put(0x60,h(0xc)); put(0x90,h(0x10)); put(0x94,h(0x11));
        s.data = lump.tail(base, "flight core").to_vector(); put(0x8c, static_cast<s32>(s.data.size()));
        s.gs = lump.sub(h(0), base - h(0), "flight GS").to_vector();
        s.gameplay.resize(0x100);
    } else {
        auto files = disc.level(static_cast<u32>(id));
        if (!files.core_index || !files.core_data || !files.gs_ram || !files.gameplay_pal) fail("Level {} is missing required data", id);
        s.index = decoded(*files.core_index); s.data = decoded(*files.core_data);
        s.gs = decoded(*files.gs_ram); s.gameplay = decoded(*files.gameplay_pal);
    }
    s.core = d::parse_level_core(s.index, s.data.size());
    return s;
}
using TextureKey = std::pair<int,int>;
struct Builder {
    LevelData l;
    std::map<TextureKey,int> textures;
    int image(RgbaImage img, int group, int index) {
        renderer::Rgba8Image out;
        out.width = static_cast<int>(img.width); out.height = static_cast<int>(img.height); out.pixels = std::move(img.rgba);
        bool alpha = false;
        for (std::size_t n = 3; n < out.pixels.size(); n += 4) if (out.pixels[n] < 250) { alpha = true; break; }
        int id = static_cast<int>(l.images.size());
        l.images.push_back(std::move(out)); l.translucent_images.push_back(alpha);
        l.image_paths.push_back(std::format("native/{}/{}", group, index)); textures[{group,index}] = id;
        return id;
    }
    int material(int group, int texture, int clamp, bool metal = false, bool blend = false) {
        Material m;
        auto it = textures.find({group,texture});
        if (it != textures.end()) m.image = it->second;
        m.metal = metal; m.clamp_s = (clamp & 1) != 0; m.clamp_t = (clamp & 2) != 0;
        m.blend = blend; m.cutout = !blend && !metal && m.image >= 0 && l.translucent_images[m.image];
        for (std::size_t i = 0; i < l.materials.size(); ++i) {
            const auto& a = l.materials[i];
            if (a.image == m.image && a.metal == m.metal && a.cutout == m.cutout && a.blend == m.blend && a.clamp_s == m.clamp_s && a.clamp_t == m.clamp_t) return static_cast<int>(i);
        }
        l.materials.push_back(m); return static_cast<int>(l.materials.size() - 1);
    }
    unsigned mesh(const Mesh& mesh, const std::string& name, const std::function<int(int)>& mat,
        const std::vector<float>& slots = {}, const std::vector<r::MobySkin>& skins = {}, float scale = 1,
        int shell = -1, int fragment = -1) {
        Model model; model.name = name;
        const auto base = static_cast<unsigned>(l.vertices.size());
        bool first = true;
        for (std::size_t n = 0; n < mesh.vertices.size(); ++n) {
            const auto& a = mesh.vertices[n]; Vertex v{};
            for (int k = 0; k < 3; ++k) { v.position[k] = a.position[k] * scale; v.normal[k] = a.normal[k]; }
            std::copy(a.uv.begin(), a.uv.end(), v.uv);
            for (int k = 0; k < 4; ++k) v.colour[k] = a.rgba[k] / 128.0f;
            v.light_slot = n < slots.size() ? slots[n] : -1;
            if (n < skins.size()) for (unsigned k = 0; k < skins[n].count; ++k) {
                v.joints[k] = skins[n].joints[k]; v.weights[k] = skins[n].weights[k] / 256.0f;
                model.joints = std::max(model.joints, int(v.joints[k]) + 1);
            }
            for (int k = 0; k < 3; ++k) {
                model.min[k] = first ? v.position[k] : std::min(model.min[k], v.position[k]);
                model.max[k] = first ? v.position[k] : std::max(model.max[k], v.position[k]);
            }
            first = false; l.vertices.push_back(v);
        }
        // Preserve triangle order within a material, as the GLB export does.
        std::map<int,std::vector<u32>> grouped;
        for (const auto& t : mesh.triangles) for (auto v : t.vertices) {
            if (v >= mesh.vertices.size()) fail("{}: invalid mesh index", name);
            grouped[mat(t.texture)].push_back(base + v);
        }
        for (const auto& [material, indices] : grouped) {
            Primitive p; p.material = material; p.first_index = static_cast<u32>(l.indices.size());
            p.index_count = static_cast<u32>(indices.size()); p.shell = shell; p.terrain_fragment = fragment;
            l.indices.insert(l.indices.end(), indices.begin(), indices.end()); model.primitives.push_back(p);
        }
        l.models.push_back(std::move(model)); return static_cast<unsigned>(l.models.size() - 1);
    }
    void moby(const r::CoreClassEntry& entry, const r::MobyClass& c) {
        if (l.moby_classes.contains(entry.o_class) || c.high_lod.empty()) return;
        if (!(c.header.scale > 0) || !std::isfinite(c.header.scale)) fail("Invalid scale for moby {}", entry.o_class);
        auto high = r::moby_mesh(c, c.high_lod);
        std::vector<float> slots;
        for (std::size_t p = 0; p < c.high_lod.size(); ++p) {
            for (std::size_t n = 0; n < c.high_lod[p].vertices.size(); ++n) slots.push_back(p >= c.header.glow_high ? -2.0f : -1.0f);
        }
        auto material_for = [&](int slot) { return material(1, entry.texture_table_index(slot), false); };
        const auto skins = c.skeleton.matrices.empty() ? std::vector<r::MobySkin>{} : high.skins;
        auto model = mesh(high.mesh, std::format("moby_{}", entry.o_class), material_for, slots, skins, 1 / c.header.scale);
        if (!c.metal.empty()) {
            auto metal = r::moby_mesh(c, c.metal);
            auto shine = mesh(metal.mesh, "metal", [&](int t) { return material(5,t,false,true); }, {},
                c.skeleton.matrices.empty() ? std::vector<r::MobySkin>{} : metal.skins, 1 / c.header.scale);
            auto& dst = l.models[model]; auto& src = l.models[shine];
            dst.primitives.insert(dst.primitives.end(), src.primitives.begin(), src.primitives.end());
            dst.joints = std::max(dst.joints, src.joints);
            l.models.pop_back();
        }
        l.moby_classes[entry.o_class] = {model,false};
    }
};
std::vector<r::CoreClassEntry> classes(ByteView index, int at) {
    return r::read_core_table<r::CoreClassEntry>(index, index.read<s32>(at,"class count"), index.read<s32>(at+4,"class offset"), "classes");
}
std::vector<r::CoreTextureEntry> tex_table(ByteView index, int at) {
    return r::read_core_table<r::CoreTextureEntry>(index, index.read<s32>(at,"texture count"), index.read<s32>(at+4,"texture offset"), "textures");
}
}
LevelData import_disc_level(const d::Disc& disc, int id, const r::NormalTable& normals, const SetupProgress& progress) {
    tick(progress, "Reading level data");
    auto s = source(disc,id); Builder b; b.l.level = id;
    const ByteView index(s.index), data(s.data), gs(s.gs), gameplay(s.gameplay);
    const auto tfrag = tex_table(index,0x30), moby = tex_table(index,0x38), tie = tex_table(index,0x40), shrub = tex_table(index,0x48);
    const auto mobys = classes(index,0x18), ties = classes(index,0x20);
    const auto shrubs = r::read_core_table<r::CoreShrubClassEntry>(index,index.read<s32>(0x28,"shrub count"),index.read<s32>(0x2c,"shrub offset"),"shrubs");
    const auto textures_at = index.read<s32>(0x60,"texture data");
    for (auto& t : r::decode_level_textures({tfrag,moby,tie,shrub,shrubs},data.tail(textures_at,"texture data"),gs)) {
        b.image(std::move(t.image),static_cast<int>(t.table),static_cast<int>(t.index));
    }
    for (const auto [at,palette,key] : {std::array<int,3>{0x90,0x94,-2},std::array<int,3>{0x98,0x9c,-3}}) {
        const int pixels = index.read<s32>(at,"metal pixels"), clut = index.read<s32>(palette,"metal palette");
        const int side = key == -2 ? 128 : 64;
        if (pixels > 0 && std::size_t(pixels) + side*side <= gs.size() && clut >= 0 && std::size_t(clut)+1024 <= gs.size())
            b.image(decode_indexed8(gs.sub(pixels,side*side,"metal image"),side,side,gs.sub(clut,1024,"metal palette")),5,key);
    }
    RgbaImage grey; grey.width = grey.height = 8; grey.rgba.resize(8*8*4,128);
    for (unsigned i = 3; i < grey.rgba.size(); i += 4) grey.rgba[i] = 255;
    b.image(std::move(grey),1,-1);
    const auto lights = r::parse_light_bank(gameplay);
    const auto tie_instances = r::parse_tie_instances(gameplay);
    const auto shrub_instances = r::parse_shrub_instances(gameplay);
    std::optional<assets::OcclusionMappings> visibility;
    if (auto grid = s.core.block(data,"occlusion")) {
        const auto at = gameplay.read<u32>(0x8c,"visibility mappings");
        if (at) { visibility = assets::read_occlusion_mappings(Game::Rac1,gameplay.tail(at,"visibility mappings")); b.l.pvs.grid = grid->to_vector(); }
    }
    assets::LevelOcclusion occlusion;
    if (visibility) {
        std::vector<s32> keys;
        for (const auto& i : tie_instances) keys.push_back(i.occlusion_index);
        assets::resolve_tie_occlusion(*visibility,keys,occlusion);
    }
    const auto clamp = [](const assets::AdGif& reg) {
        return reg.address == 0x08 ? int((reg.data_lo & 3) == 1) | (int((reg.data_hi & 3) == 1) << 1) : 0;
    };
    for (const auto& set : lights.sets) for (const auto& v : {set.color_a,set.dir_a,set.color_b,set.dir_b}) b.l.light_sets.insert(b.l.light_sets.end(),v.begin(),v.end());
    if (auto sky_bytes = s.core.block(data,"sky")) {
        tick(progress,"Decoding sky");
        auto sky = r::parse_sky(*sky_bytes);
        for (auto& t : r::decode_sky_textures(*sky_bytes,sky)) b.image(std::move(t.image),6,static_cast<int>(t.index));
        for (std::size_t n = 0; n < sky.shells.size(); ++n) {
            auto model = b.mesh(r::sky_shell_mesh(sky.shells[n]),std::format("Sky_{}",n),[&](int t) { return b.material(6,t,3,false,true); },{},{},1,static_cast<int>(n));
            b.l.instances[0].push_back({model});
        }
        for (int k = 0; k < 3; ++k) b.l.background[k] = sky.header.colour[k] / 255.0f;
    }
    if (id != 98) if (auto terrain = s.core.block(data,"tfrags")) {
        auto frags = r::parse_tfrags(*terrain);
        if (visibility) {
            std::vector<u8> keys;
            for (const auto& f : frags) keys.push_back(f.header.tri_count);
            assets::resolve_tfrag_occlusion(*visibility,keys,occlusion);
        }
        for (std::size_t n = 0; n < frags.size(); ++n) {
            tick(progress,"Decoding and lighting terrain");
            frags[n].rgba = r::light_tfrag(frags[n],lights,normals);
            auto mesh = r::tfrag_mesh(frags[n],0);
            const auto triangles = r::tfrag_triangles(frags[n],0);
            for (std::size_t t = 0; t < triangles.size(); ++t) mesh.triangles[t].texture = triangles[t].ad_gif;
            auto model = b.mesh(mesh,std::format("Tfrag_{}",n),[&](int t) {
                const auto& gif = frags[n].ad_gifs.at(t);
                return b.material(0,static_cast<int>(gif.texture_index()),clamp(gif.clamp));
            },{},{},1,-1,static_cast<int>(n));
            Instance placed; placed.model = model;
            if (n < occlusion.tfrag.size()) {
                placed.occlusion = occlusion.tfrag[n].word;
                for (auto& p : b.l.models[model].primitives) p.occlusion = placed.occlusion;
            }
            b.l.instances[1].push_back(placed);
        }
    }
    for (auto& item : r::parse_level_tie_classes(ties,data)) {
        tick(progress,"Decoding and lighting props");
        std::vector<float> slots;
        for (const auto& p : item.tie.lods[0]) for (const auto& v : p.vertices) slots.push_back(v.color);
        auto model = b.mesh(r::tie_mesh(item.tie,0),std::format("tie_{}",item.entry.o_class),[&](int slot) {
            const int wrap = slot >= 0 && std::size_t(slot) < item.tie.ad_gifs.size() ? clamp(item.tie.ad_gifs[slot].clamp) : 0;
            return b.material(2,item.entry.texture_table_index(slot),wrap);
        },slots);
        for (std::size_t n = 0; n < tie_instances.size(); ++n) if (const auto& i = tie_instances[n]; i.o_class == item.entry.o_class) {
            Instance placed; placed.model = model; const auto m = i.world_matrix(); std::memcpy(placed.matrix.data(),m.data(),64);
            placed.lights = static_cast<int>(b.l.light_colours.size());
            if (n < occlusion.tie.size()) placed.occlusion = occlusion.tie[n].word;
            for (const auto& c : r::light_tie_instance(item.tie,i,lights)) { u32 packed; std::memcpy(&packed,c.data(),4); b.l.light_colours.push_back(packed); }
            b.l.instances[2].push_back(placed); b.l.draw_distances[2].push_back(std::min(float(i.draw_distance),720.0f));
        }
    }
    for (auto& item : r::parse_level_shrub_classes(shrubs,data)) {
        tick(progress,"Decoding and lighting shrubs");
        std::vector<float> slots;
        for (const auto& p : item.shrub.packets) for (const auto& v : p.vertices) slots.push_back(v.normal);
        auto mesh = r::shrub_mesh(item.shrub);
        std::size_t face = 0;
        for (const auto& packet : item.shrub.packets) for (const auto& triangle : r::shrub_triangles(packet)) {
            int wrap = 0;
            for (const auto& gif : packet.ad_gifs) if (gif.tex0.data_lo == triangle.texture) { wrap = clamp(gif.clamp); break; }
            mesh.triangles.at(face++).texture = triangle.texture | (wrap << 16);
        }
        // Match the PC exporter: strip-join faces have zero area, and the
        // remaining faces wind toward the summed vertex normals.
        std::vector<MeshTriangle> drawn;
        for (auto triangle : mesh.triangles) {
            const auto& a = mesh.vertices[triangle.vertices[0]];
            const auto& c = mesh.vertices[triangle.vertices[2]];
            const auto& v = mesh.vertices[triangle.vertices[1]];
            std::array<double,3> ab,ac,cross;
            for (int k = 0; k < 3; ++k) { ab[k] = double(v.position[k])-a.position[k]; ac[k] = double(c.position[k])-a.position[k]; }
            cross = {ab[1]*ac[2]-ab[2]*ac[1],ab[2]*ac[0]-ab[0]*ac[2],ab[0]*ac[1]-ab[1]*ac[0]};
            double area = 0, facing = 0;
            for (int k = 0; k < 3; ++k) { area += cross[k]*cross[k]; facing += cross[k]*(a.normal[k]+v.normal[k]+c.normal[k]); }
            if (area < 1e-24) continue;
            if (facing < 0) std::swap(triangle.vertices[0],triangle.vertices[2]);
            drawn.push_back(triangle);
        }
        mesh.triangles = std::move(drawn);
        auto model = b.mesh(mesh,std::format("shrub_{}",item.entry.base.o_class),[&](int t) { return b.material(3,item.entry.base.texture_table_index(t & 0xffff),t >> 16); },slots);
        for (const auto& i : shrub_instances) if (i.o_class == item.entry.base.o_class) {
            Instance placed; placed.model = model; const auto m = i.world_matrix(); std::memcpy(placed.matrix.data(),m.data(),64);
            placed.lights = static_cast<int>(b.l.light_colours.size());
            for (const auto& c : r::light_shrub_instance(item.shrub,i,lights)) { u32 packed; std::memcpy(&packed,c.data(),4); b.l.light_colours.push_back(packed); }
            b.l.instances[3].push_back(placed);
            float distance = std::max(16.0f,i.draw_distance);
            if (item.shrub.billboard) distance = std::max(distance,float(static_cast<int>(item.shrub.billboard->fade_distance) & 255) + 24);
            b.l.draw_distances[3].push_back(std::min(distance,500.0f));
        }
    }
    for (const auto& item : r::parse_level_moby_classes(mobys,data)) { tick(progress,"Decoding characters and objects"); b.moby(item.entry,item.moby); }
    const auto gadgets = r::read_core_table<r::CoreGadgetEntry>(index,index.read<s32>(0x80,"gadget count"),index.read<s32>(0x84,"gadget table"),"gadgets");
    for (const auto& item : r::parse_gadget_classes(gadgets,mobys,data)) { tick(progress,"Decoding weapons and gadgets"); b.moby(item.moby.entry,item.moby.moby); }
    for (unsigned k = 0; k < 3; ++k) {
        tick(progress,"Decoding ships");
        const auto range = d::global_sector_range(disc.toc(),0x12c8 + (k+1)*8);
        const auto bytes = decoded(disc.iso().read_sectors(range.offset,range.size));
        const auto ship = assets::read_spaceship_file(Game::Rac1,bytes);
        for (int extra = 0; extra < 2; ++extra) {
            const int texture = 1000 + int(k)*2 + extra;
            b.image((extra ? ship.extra_texture : ship.ship_texture).image().decode(),1,texture);
            auto c = r::parse_moby_class(extra ? ship.extra_class : ship.ship_class);
            r::CoreClassEntry entry; entry.o_class = extra ? assets::kRac1ShipExtraClasses[k] : assets::kRac1ShipClasses[k];
            // These keys exceed the disc's u8 slots; bind their one image directly.
            auto high = r::moby_mesh(c,c.high_lod);
            auto model = b.mesh(high.mesh,std::format("ship_{}",entry.o_class),[&](int) { return b.material(1,texture,false); },{},
                c.skeleton.matrices.empty() ? std::vector<r::MobySkin>{} : high.skins,1/c.header.scale);
            b.l.moby_classes[entry.o_class] = {model,false};
        }
    }
    bool first = true;
    for (unsigned layer = 1; layer < 4; ++layer) for (const auto& i : b.l.instances[layer]) {
        const auto& model = b.l.models[i.model];
        for (unsigned corner = 0; corner < 8; ++corner) {
            renderer::Vec3 p; for (int k = 0; k < 3; ++k) p[k] = corner & (1u << k) ? model.max[k] : model.min[k];
            p = renderer::transform_point(i.matrix,p);
            for (int k = 0; k < 3; ++k) { b.l.bounds_min[k] = first ? p[k] : std::min(b.l.bounds_min[k],p[k]); b.l.bounds_max[k] = first ? p[k] : std::max(b.l.bounds_max[k],p[k]); }
            first = false;
        }
    }
    return std::move(b.l);
}
}
