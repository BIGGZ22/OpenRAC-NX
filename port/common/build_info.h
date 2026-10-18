// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What this build of the port is: OpenRAC's version (the launcher's, launcher/src-tauri/
// tauri.conf.json) and the checkout's commit. Written at each build by cmake/BuildInfo.cmake.

#pragma once

namespace openrac::build_info {

// "0.7.0".
const char* version();
// "f3df7852" with "-dirty" when the checkout had changes; "unknown" outside a git checkout.
const char* commit();

}  // namespace openrac::build_info
