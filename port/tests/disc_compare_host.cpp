// SPDX-License-Identifier: GPL-3.0-or-later
// Compare on-device conversion with a locally prepared PC export. No assets are bundled.
#include "scene/disc_cache.h"
#include "assets/disc/checksum.h"
#include <iostream>
#include <set>
#include <cmath>
#include <format>
#include <map>
using namespace openrac;
int main(int argc,char** argv) {
    if (argc != 3) return 2;
    int errors = 0;
    for (int id : {99,0,98,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18}) {
        scene::LevelData a,b; std::string reason;
        auto folder = std::format("level_{:02}",id);
        if (!scene::read_disc_cache(std::filesystem::path(argv[1])/folder/"scene.orcache",a,reason) || !scene::load_level(std::filesystem::path(argv[2])/folder,b,reason)) { std::cerr << folder << ": " << reason << '\n'; ++errors; continue; }
        std::set<std::string> textures;
        std::vector<std::string> a_hash,b_hash;
        for (const auto& img : a.images) { auto hash = assets::disc::to_hex(assets::disc::sha256(img.pixels)); textures.insert(hash); a_hash.push_back(hash); }
        int missing = 0;
        for (const auto& img : b.images) { auto hash = assets::disc::to_hex(assets::disc::sha256(img.pixels)); if (!textures.contains(hash)) ++missing; b_hash.push_back(hash); }
        const auto materials = [](const scene::LevelData& scene,const std::vector<std::string>& hashes,unsigned model) {
            std::map<std::string,unsigned> counts;
            for (const auto& p : scene.models[model].primitives) {
                const auto& m = scene.materials[p.material];
                const auto key = std::format("{}/{}/{}/{}/{}/{}",m.image < 0 ? "none" : hashes[m.image],m.clamp_s,m.clamp_t,m.cutout,m.blend,m.metal);
                counts[key] += p.index_count;
            }
            return counts;
        };
        unsigned material_mismatches = 0;
        int classes = 0, triangles = 0, bounds = 0;
        for (const auto& [key,cls] : b.moby_classes) {
            if (cls.second) continue;
            auto found = a.moby_classes.find(key);
            if (found == a.moby_classes.end()) { ++classes; continue; }
            const auto& am = a.models[found->second.first]; const auto& bm = b.models[cls.first];
            if (materials(a,a_hash,found->second.first) != materials(b,b_hash,cls.first)) { ++material_mismatches; std::cerr << folder << " moby " << key << " material mismatch\n"; }
            unsigned ac = 0,bc = 0; for (const auto& p : am.primitives) ac += p.index_count; for (const auto& p : bm.primitives) bc += p.index_count;
            if (ac != bc) { ++triangles; std::cerr << folder << " moby " << key << " triangles " << ac/3 << " vs " << bc/3 << '\n'; }
            for (int k = 0; k < 3; ++k) if (std::abs(am.min[k]-bm.min[k]) > .001f || std::abs(am.max[k]-bm.max[k]) > .001f) { ++bounds; break; }
        }
        bool scenery = true;
        for (unsigned layer = 1; layer < 4; ++layer) {
            std::uint64_t ac = 0,bc = 0;
            std::map<std::string,std::uint64_t> at,bt;
            for (const auto& i : a.instances[layer]) for (const auto& p : a.models[i.model].primitives) ac += p.index_count;
            for (const auto& i : b.instances[layer]) for (const auto& p : b.models[i.model].primitives) bc += p.index_count;
            for (const auto& i : a.instances[layer]) for (const auto& [key,n] : materials(a,a_hash,i.model)) at[key] += n;
            for (const auto& i : b.instances[layer]) for (const auto& [key,n] : materials(b,b_hash,i.model)) bt[key] += n;
            if (at != bt) { ++material_mismatches; std::cerr << folder << " layer " << layer << " material mismatch\n"; }
            if (ac != bc) { scenery = false; std::cerr << folder << " layer " << layer << " triangles " << ac/3 << " vs " << bc/3 << '\n'; }
        }
        std::cout << folder << ": missing textures=" << missing << " missing classes=" << classes << " class triangle mismatches=" << triangles << " class bounds mismatches=" << bounds << " material mismatches=" << material_mismatches << " scenery=" << scenery << '\n';
        if (missing || classes || triangles || bounds || !scenery || material_mismatches) ++errors;
    }
    return errors ? 1 : 0;
}
