// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Alpha-only material queries used inside Embree traversal.
#pragma once
#include "enhanced/lighting/bake.h"

namespace openrac::lighting::bake_detail {
// Match diffuse bilinear coverage without evaluating colour or emission.
float textured_opacity(const BakeScene& scene, const BakeTriangle& triangle, float u, float v);
}  // namespace openrac::lighting::bake_detail
