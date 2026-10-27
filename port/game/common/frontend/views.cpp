// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A mod's views of the game (interface 1.4, openrac_mod.h's openrac_view_source, sdk/docs/views.md):
// the eyes of a headset, a panel for the 2D. In each frame that draws the scene, after the window's
// picture, the mod says which views it wants; each is drawn from the same scene into a picture of
// its own and copied where the mod said (its GL texture, or its VkImage). Once the frame is
// submitted, the mod is told. Without a mod's views nothing here runs.

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "common/log.h"
#include "frontend.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/scene_frame_internal.h"
#include "platform/window.h"
#include "renderer/gl.h"
#include "renderer/math.h"

namespace openrac::frontend {
namespace {

static_assert(OPENRAC_VIEW_MAX == std::tuple_size_v<decltype(State::views)>);

// The view frustum's planes, as the window's (scene_frame.cpp's set_camera).
constexpr float kNear = 0.05f;
constexpr float kFar = 2000.0f;
constexpr int kLargest = 8192;

std::optional<openrac_view_source> g_source;  // set_view_source's
std::array<openrac_view, OPENRAC_VIEW_MAX> g_views{};
int g_count = 0;
bool g_pending = false;  // begin was called and end not yet
bool g_warned = false;   // a view refused once, said once

// The GL proc for openrac_graphics.gl_proc, in its C shape.
void* gl_proc(const char* name) {
    return reinterpret_cast<void*>(platform::Window::gl_loader()(name));
}

// The world position of the eye of a view matrix (a rotation and a translation, column-major).
renderer::Vec3 eye_position(const renderer::Mat4& m) {
    return {
        -(m[0] * m[12] + m[1] * m[13] + m[2] * m[14]),
        -(m[4] * m[12] + m[5] * m[13] + m[6] * m[14]),
        -(m[8] * m[12] + m[9] * m[13] + m[10] * m[14]),
    };
}

// The camera of view `v`: the game's, moved and turned to the eye and given its frustum.
renderer::Camera view_camera(const renderer::Camera& game, const openrac_view& v) {
    std::array<float, 4> turn =
        {v.orientation[0], v.orientation[1], v.orientation[2], v.orientation[3]};

    // An orientation left all zero is the camera's own.
    if (turn == std::array<float, 4>{}) {
        turn[3] = 1.0f;
    }

    renderer::Camera camera = game;
    camera.view =
        renderer::eye_view(game.view, {v.position[0], v.position[1], v.position[2]}, turn);
    camera.position = eye_position(camera.view);
    const float aspect = static_cast<float>(v.width) / static_cast<float>(v.height);

    // No field of view given: the game's vertical one, opened to the view's shape.
    if (v.angle_left == 0.0f && v.angle_right == 0.0f && v.angle_up == 0.0f
        && v.angle_down == 0.0f) {
        const float focal = game.projection[5] > 0.0f ? game.projection[5] : 1.0f;
        camera.projection =
            renderer::perspective_reversed(2.0f * std::atan(1.0f / focal), aspect, kNear, kFar);
    } else {
        camera.projection = renderer::perspective_off_centre_reversed(
            v.angle_left, v.angle_right, v.angle_up, v.angle_down, kNear, kFar
        );
    }

    return camera;
}

// Draws view `v` of the frame's scene into `picture`.
void render_view(SceneFrame& frame, const openrac_view& v, renderer::FrameBuffer& picture) {
    State& owner = frame.owner;
    renderer::FrameInput input = frame.input;
    // The window's own hand-offs (its read-back copy, the temporal history, the probes' choice by
    // its depth) are the window's picture's, not a view's.
    input.before_hud = {};
    input.on_before_particles = {};
    input.on_resolve_world = {};
    input.after_world_depth = {};
    input.keep_colour = false;
    // Ambient occlusion keeps targets of the window's size: a view of another would make them anew.
    input.gtao = false;

    if (v.kind == OPENRAC_VIEW_2D) {
        input.direct_before_world = {};
        input.clear_colour = {0.0f, 0.0f, 0.0f, 0.0f};
    } else {
        input.camera = view_camera(frame.input.camera, v);
    }

    const float aspect = static_cast<float>(v.width) / static_cast<float>(v.height);
    renderer::RenderTarget target{
        picture.id(), v.width, v.height, std::min(1.0f, kTvAspect / aspect)
    };
    target.depth_texture = picture.depth_texture();

    // The buckets the kind leaves out, given back as the frame had them.
    std::vector<std::pair<renderer::BucketRenderer*, bool>> kept;
    for (const auto& bucket : owner.renderer->renderers()) {
        const bool hud = bucket->bucket() == renderer::Bucket::Hud;
        const bool drop =
            (v.kind == OPENRAC_VIEW_WORLD && hud) || (v.kind == OPENRAC_VIEW_2D && !hud);
        if (drop) {
            kept.emplace_back(bucket.get(), bucket->enabled);
            bucket->enabled = false;
        }
    }

    owner.renderer->render(input, target);

    for (const auto& [bucket, enabled] : kept) {
        bucket->enabled = enabled;
    }
}

// The framebuffer around the mod's GL texture `texture`, made the first time.
unsigned target_of(State& owner, unsigned texture) {
    for (const auto& [name, framebuffer] : owner.view_targets) {
        if (name == texture) {
            return framebuffer;
        }
    }

    unsigned framebuffer = 0;
    gl::glGenFramebuffers(1, &framebuffer);
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, framebuffer);
    gl::glFramebufferTexture2D(
        gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0, gl::GL_TEXTURE_2D, texture, 0
    );
    const bool complete =
        gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) == gl::GL_FRAMEBUFFER_COMPLETE;
    gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0);

    // A texture that cannot be drawn into is not one: nothing is copied there.
    if (!complete) {
        gl::glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }

    owner.view_targets.emplace_back(texture, framebuffer);
    return framebuffer;
}

// Copies the view's picture where the mod said. False when it could not be.
bool copy_out(State& owner, const openrac_view& v, const renderer::FrameBuffer& picture) {
    if (gl::is_vulkan()) {
        return v.image == nullptr
               || platform::copy_to_vulkan_image(
                   picture.colour_texture(), v.image, v.image_layout, v.width, v.height
               );
    }

    if (v.texture == 0) {
        return true;
    }

    const unsigned framebuffer = target_of(owner, v.texture);
    if (framebuffer == 0) {
        return false;
    }

    picture.blit_to(framebuffer, v.width, v.height);
    return true;
}

// Draws view `index` (anew when `world`) and gives it to the mod.
void draw_view(SceneFrame& frame, int index, bool world) {
    State& owner = frame.owner;
    openrac_view& v = g_views[index];
    v.drawn = 0;
    v.picture = 0;

    // Sizes and kinds the port does not draw: the view is left out, said once.
    if (v.width < 1 || v.height < 1 || v.width > kLargest || v.height > kLargest
        || v.kind > OPENRAC_VIEW_2D) {
        if (!g_warned) {
            log::warn(
                "mod views: view {} is {} by {} of kind {}, which is not drawn",
                index,
                v.width,
                v.height,
                v.kind
            );
            g_warned = true;
        }
        return;
    }

    renderer::FrameBuffer& picture = owner.views[index];
    const bool sampleable = enhanced_graphics_available() && frame.graphics.enhanced_graphics;

    if (world && owner.renderer) {
        if (picture.width() != v.width || picture.height() != v.height
            || (picture.depth_texture() != 0) != sampleable) {
            std::string error;
            if (!picture.create(v.width, v.height, error, 1, sampleable)) {
                log::warn("mod views: view {}: {}", index, error);
                return;
            }
        }
        render_view(frame, v, picture);
        v.drawn = 1;
    }

    // Nothing drawn at this size yet: nothing to give.
    if (picture.width() != v.width || picture.height() != v.height) {
        return;
    }

    v.picture = picture.colour_texture();
    if (!copy_out(owner, v, picture) && !g_warned) {
        log::warn(
            "mod views: view {} cannot be copied where the mod said (openrac_view's texture or "
            "image)",
            index
        );
        g_warned = true;
    }
}

}  // namespace

const openrac_graphics* mod_graphics() {
    static openrac_graphics graphics{};
    graphics = openrac_graphics{};
    graphics.size = sizeof graphics;

    // No window: nothing to draw with yet.
    if (!g || !g->window) {
        return &graphics;
    }

    if (gl::is_vulkan()) {
        const platform::VulkanNative native = platform::vulkan_native();
        graphics.backend = OPENRAC_GRAPHICS_VULKAN;
        graphics.vk_instance = native.instance;
        graphics.vk_physical_device = native.physical_device;
        graphics.vk_device = native.device;
        graphics.vk_queue = native.queue;
        graphics.vk_queue_family = native.queue_family;
        graphics.vk_queue_index = native.queue_index;
        graphics.vk_get_instance_proc_addr = native.get_instance_proc_addr;
    } else {
        graphics.backend = gl::is_es() ? OPENRAC_GRAPHICS_OPENGL_ES : OPENRAC_GRAPHICS_OPENGL;
        graphics.gl_proc = gl_proc;
    }

    return &graphics;
}

void set_view_source(const openrac_view_source* source) {
    if (source != nullptr) {
        g_source = *source;
    } else {
        g_source.reset();
    }
    g_pending = false;
    g_count = 0;
}

bool views_wanted() {
    return g_source.has_value() && g && g->window;
}

void draw_views(SceneFrame& frame, bool world) {
    // A frame drawn twice before its swap (none should be) ends the first one's views first.
    finish_views();
    g_views = {};
    g_count = std::clamp(
        g_source->begin(g_source->user, g_views.data(), OPENRAC_VIEW_MAX), 0, OPENRAC_VIEW_MAX
    );
    g_pending = true;

    for (int i = 0; i < g_count; ++i) {
        draw_view(frame, i, world);
    }
}

void finish_views() {
    if (g_pending && g_source) {
        g_pending = false;
        g_source->end(g_source->user, g_views.data(), g_count);
    }
}

void release_views(State& owner) {
    finish_views();

    // Told once: the views end with the context.
    if (g_source && g_source->closing != nullptr) {
        g_source->closing(g_source->user);
    }
    g_source.reset();

    for (const auto& [texture, framebuffer] : owner.view_targets) {
        if (framebuffer != 0) {
            gl::glDeleteFramebuffers(1, &framebuffer);
        }
    }

    owner.view_targets.clear();
    for (renderer::FrameBuffer& picture : owner.views) {
        picture.release();
    }
}

}  // namespace openrac::frontend
