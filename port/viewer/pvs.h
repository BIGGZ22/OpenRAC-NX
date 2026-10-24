// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Forwards to scene/pvs.h (the level's baked camera visibility), where the scene module now keeps
// it. game/common still includes this path, because another lane owns its files while the
// restructure's lanes run; P2-37 points it at scene/ and removes this file.

#pragma once

#include "scene/pvs.h"
