// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <span>
#include <string>
#include <vector>
#include "renderer/deko3d/scene_renderer.h"

namespace openrac::deko {
// Native adapter for the game's extracted level plus captured live instances.
// The source level must outlive the adapter and remain unchanged until release.
class LevelScene {
public:
    explicit LevelScene(SceneRenderer& renderer) : m_renderer(renderer) {}
    bool upload(const scene::LevelData& level, std::string& error);
    // Appends to an ordered frame, so effects and HUD can be inserted between layers.
    // Joint matrices are the game's evaluated F_j matrices, never live guest pointers.
    bool append(scene::Layer layer, std::span<const scene::Instance> instances,
                std::span<const renderer::Mat4> palette, std::vector<SceneDraw>& out,
                std::string& error) const;
    bool append_static(scene::Layer layer, std::vector<SceneDraw>& out, std::string& error) const;
    void release();
private:
    SceneRenderer& m_renderer;
    const scene::LevelData* m_level = nullptr;
    MeshId m_mesh = 0;
    std::vector<TextureId> m_material_textures;
};
}
