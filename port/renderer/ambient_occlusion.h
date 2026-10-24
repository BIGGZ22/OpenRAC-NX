// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The renderer's post-process hook for ambient occlusion: the interface Renderer::render calls
// after the world's depth prepass (occlusion for the world's lighting to read) or after the world's
// last pass (occlusion composited over the colour), when a frame asks for it (FrameInput::gtao).
// The renderer owns the interface; the enhanced renderer's GTAO (enhanced/renderer/gtao.h)
// implements it, and whoever makes a Renderer registers one (Renderer::set_ambient_occlusion), so
// the renderer never includes enhanced/ (CODING_CONVENTIONS.md, section 18).
//
// It has one implementation. The scene's lighting and shadow hooks (restructure package P2-39) are
// passes of the level renderer rather than of the frame, so they do not take this shape.

#pragma once

namespace openrac::renderer {

struct FrameInput;
struct RenderTarget;

// Ambient occlusion over a frame's depth. Created, run and released on the thread that holds the
// graphics context.
class AmbientOcclusion {
public:
    AmbientOcclusion() = default;
    virtual ~AmbientOcclusion() = default;
    AmbientOcclusion(const AmbientOcclusion&) = delete;
    AmbientOcclusion& operator=(const AmbientOcclusion&) = delete;

    // Evaluates the occlusion of `target`'s depth. With `apply_to_colour` it is composited over the
    // target's colour; without, it is left for the world's lighting to read (visibility_texture(),
    // or raw_texture() when `evaluate_only`). Returns false when it drew nothing (no compute
    // support, a shader that did not build, a target too big for its buffer).
    virtual bool render(
        const FrameInput& input,
        const RenderTarget& target,
        bool apply_to_colour,
        bool evaluate_only
    ) = 0;

    // The last evaluation's packed records, a texture-buffer view (0: none yet).
    virtual unsigned raw_texture() const = 0;

    // The last evaluation's filtered visibility, one texel a pixel of the target (0: none yet).
    virtual unsigned visibility_texture() const = 0;

    // The depth the last evaluation sampled: the target's own, or a copy (0: none yet).
    virtual unsigned depth_texture() const = 0;

    // Frees its GL objects; the context is still current.
    virtual void release() = 0;
};

}  // namespace openrac::renderer
