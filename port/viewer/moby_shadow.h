// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Forwards to scene/moby_shadow.h (a moby's shadow), where the scene module now keeps it.
// game/common still includes this path, because another lane owns its files while the restructure's
// lanes run; P2-37 points it at scene/ and removes this file.

#pragma once

#include "scene/moby_shadow.h"
