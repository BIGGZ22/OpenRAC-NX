// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene/disc_cache.h"
#include "tests/check.h"
#include <chrono>
#include <fstream>
using namespace openrac;
int main() {
    auto path = std::filesystem::temp_directory_path() / ("openrac-cache-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    auto file = path / "scene.orcache";
    scene::LevelData l;
    l.level = 3; l.vertices.resize(3); l.indices = {0,1,2}; l.materials.resize(1);
    scene::Model model; model.name = "synthetic"; model.primitives.push_back({0,3,0}); l.models.push_back(model);
    renderer::Rgba8Image image(1,1); image.pixels = {12,34,56,255};
    l.images.push_back(image); l.image_paths.push_back("test"); l.translucent_images.push_back(false);
    l.materials[0].image = 0; l.moby_classes[71] = {0,false};
    l.instances[2].push_back({0}); l.light_colours = {0x80776655}; l.pvs.grid = {1,2,3,4};
    scene::write_disc_cache(file,l);
    scene::LevelData loaded; std::string error;
    CHECK(scene::read_disc_cache(file,loaded,error));
    CHECK(loaded.level == 3 && loaded.vertices.size() == 3 && loaded.indices == l.indices);
    CHECK(loaded.images.size() == 1 && loaded.images[0].pixels == image.pixels);
    CHECK(loaded.moby_classes == l.moby_classes && loaded.light_colours == l.light_colours);
    CHECK(loaded.pvs.grid == l.pvs.grid && loaded.instances[2].size() == 1);
    {
        std::fstream corrupt(file,std::ios::in|std::ios::out|std::ios::binary);
        corrupt.seekp(100); corrupt.put('!');
    }
    CHECK(!scene::read_disc_cache(file,loaded,error));
    CHECK(error.find("checksum") != std::string::npos);
    std::filesystem::resize_file(file,8);
    CHECK(!scene::read_disc_cache(file,loaded,error));
    std::filesystem::remove(file); std::filesystem::remove(path);
    return test::result();
}
