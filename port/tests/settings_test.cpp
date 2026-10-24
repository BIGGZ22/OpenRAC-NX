// SPDX-License-Identifier: GPL-3.0-or-later
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>

#include "game/common/settings.h"
#include "tests/check.h"

int main() {
    using namespace openrac;
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto folder =
        std::filesystem::temp_directory_path() / ("openrac-settings-" + std::to_string(stamp));
    if (!std::filesystem::create_directory(folder)) {
        return 1;
    }
    const auto file = folder / "settings.ini";
    settings::load(file);
    CHECK(settings::get().gtao);
    CHECK(settings::get().texture_filter == renderer::TextureFilter::EdgeAware);
    CHECK(settings::get().anisotropy == 0);
    CHECK(settings::get().anisotropy_samples(true) == 4);
    CHECK(settings::get().anisotropy_samples(false) == 1);
    // Saving another setting keeps the backend-dependent default automatic.
    settings::save();
    settings::load(file);
    CHECK(settings::get().anisotropy == 0);
    for (int samples : settings::kAnisotropies) {
        std::ofstream(file) << "anisotropy = " << samples << "\n";
        settings::load(file);
        CHECK(settings::get().anisotropy_samples(true) == samples);
        CHECK(settings::get().anisotropy_samples(false) == samples);
        settings::save();
        settings::load(file);
        CHECK(settings::get().anisotropy == samples);
    }
    for (const char* value : {"auto", "invalid"}) {
        std::ofstream(file) << "anisotropy = " << value << "\n";
        settings::load(file);
        CHECK(settings::get().anisotropy_samples(true) == 4);
        CHECK(settings::get().anisotropy_samples(false) == 1);
    }

    for (auto filter : {renderer::TextureFilter::Normal, renderer::TextureFilter::EdgeAware}) {
        settings::get().texture_filter = filter;
        settings::save();
        settings::get().texture_filter = renderer::TextureFilter::Normal;
        settings::load(file);
        CHECK(settings::get().texture_filter == filter);
    }
    CHECK(std::string(settings::name(renderer::TextureFilter::EdgeAware)) == "Edge-aware (2x)");
    CHECK(std::string(settings::name(renderer::TextureFilter::Count)) == "Edge-aware (2x)");
    std::ofstream(file) << "texture_filter = bspline\n";
    settings::load(file);
    CHECK(settings::get().texture_filter == renderer::TextureFilter::EdgeAware);
    std::ofstream(file) << "texture_filter = catmull-rom\n";
    settings::load(file);
    CHECK(settings::get().texture_filter == renderer::TextureFilter::EdgeAware);
    std::ofstream(file) << "texture_filter = unknown\n";
    settings::load(file);
    CHECK(settings::get().texture_filter == renderer::TextureFilter::EdgeAware);
    // Enhanced rendering is off until the player turns it on (the launcher's Mods entry).
    CHECK(!settings::get().enhanced_rendering);
    CHECK(settings::get().enhanced_graphics);
    CHECK(settings::get().enhanced_shadows);
    CHECK(settings::get().probe_lighting);
    CHECK(!settings::get().probe_spheres);
    CHECK(settings::get().probe_gi_multiplier == 1.0f);
    CHECK(settings::get().probe_normal_bias == 0.5f);
    CHECK(settings::get().probe_view_bias == 0.5f);
    CHECK(settings::get().gtao_settings.strength == 1.0f);
    CHECK(settings::get().gtao_settings.quality == renderer::GtaoQuality::Performance);
    // An old file keeps the defaults for keys it does not contain.
    std::ofstream(file) << "gtao = 1\ngtao_strength = 0.7\n";
    settings::load(file);
    CHECK(settings::get().texture_filter == renderer::TextureFilter::EdgeAware);
    auto& options = settings::get();
    CHECK(options.enhanced_graphics && options.probe_lighting && options.gtao);
    CHECK(!options.enhanced_rendering);
    CHECK(options.gtao_settings.quality == renderer::GtaoQuality::Performance);
    CHECK(options.enhanced_shadows);
    options.enhanced_rendering = true;
    options.enhanced_graphics = false;
    options.enhanced_shadows = false;
    options.probe_lighting = false;
    options.probe_spheres = true;
    CHECK(options.probe_normal_bias == 0.5f && options.probe_view_bias == 0.5f);
    options.probe_gi_multiplier = 2.5f;
    options.probe_normal_bias = 0.25f;
    options.probe_view_bias = 1.25f;
    settings::save();
    options = settings::Settings{};
    settings::load(file);
    CHECK(options.enhanced_rendering);
    CHECK(!options.enhanced_graphics && !options.probe_lighting);
    CHECK(options.probe_spheres);
    CHECK(!options.enhanced_shadows);
    CHECK(options.probe_gi_multiplier == 2.5f);
    CHECK(options.probe_normal_bias == 0.25f);
    CHECK(options.probe_view_bias == 1.25f);
    CHECK(options.gtao && std::abs(options.gtao_settings.strength - 0.7f) < 0.0001f);
    options.enhanced_graphics = true;
    settings::save();
    settings::load(file);
    CHECK(options.enhanced_graphics && !options.probe_lighting && options.gtao);
    CHECK(!options.enhanced_shadows);
    CHECK(options.probe_gi_multiplier == 2.5f);
    options.enhanced_shadows = true;
    settings::save();
    settings::load(file);
    CHECK(options.enhanced_shadows);
    std::ofstream(file) << "probe_gi_multiplier = nan\nprobe_gi_multiplier = 6\n"
                           "probe_gi_multiplier = -1\n";
    settings::load(file);
    CHECK(options.probe_gi_multiplier == 1.0f);
    for (float multiplier : {0.0f, 5.0f}) {
        options.probe_gi_multiplier = multiplier;
        settings::save();
        settings::load(file);
        CHECK(options.probe_gi_multiplier == multiplier);
    }
    std::ofstream(file) << "probe_normal_bias = nan\nprobe_normal_bias = 3\n"
                           "probe_normal_bias = -1\nprobe_view_bias = inf\n"
                           "probe_view_bias = 3\nprobe_view_bias = -1\n";
    settings::load(file);
    CHECK(options.probe_normal_bias == 0.5f && options.probe_view_bias == 0.5f);
    for (float bias : {0.0f, 2.0f}) {
        options.probe_normal_bias = bias;
        options.probe_view_bias = 2.0f - bias;
        settings::save();
        settings::load(file);
        CHECK(options.probe_normal_bias == bias);
        CHECK(options.probe_view_bias == 2.0f - bias);
    }
    for (auto quality : {renderer::GtaoQuality::Quality, renderer::GtaoQuality::Performance}) {
        options.gtao_settings.quality = quality;
        settings::save();
        settings::load(file);
        CHECK(options.gtao_settings.quality == quality);
    }
    std::ofstream(file) << "gtao_quality = quality\n";
    settings::load(file);
    CHECK(options.gtao_settings.quality == renderer::GtaoQuality::Quality);
    std::ofstream(file) << "gtao_quality = unknown\n";
    settings::load(file);
    CHECK(options.gtao_settings.quality == renderer::GtaoQuality::Performance);
    // Only this test's file and newly created empty folder are removed.
    std::filesystem::remove(file);
    std::filesystem::remove(folder);
    return test::result();
}
