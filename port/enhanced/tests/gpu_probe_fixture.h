// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "enhanced/lighting/probes.h"

namespace openrac::test {
// Exercise the exact baked representation, including GPU format quantization.
inline lighting::ProbeVolume gpu_probes(const lighting::ProbeVolume& source) {
    return lighting::decode_gpu_probes(lighting::encode_probes(source));
}

inline std::vector<lighting::ProbeVolume> gpu_regions(
    const std::vector<lighting::ProbeVolume>& source
) {
    std::vector<lighting::ProbeVolume> out;
    for (const auto& volume : source) {
        out.push_back(gpu_probes(volume));
    }
    return out;
}
}  // namespace openrac::test
