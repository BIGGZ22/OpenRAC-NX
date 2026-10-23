// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/lighting/probe_atlas.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "tests/check.h"
using namespace openrac::lighting;

int main() {
    ProbeGrid grid;
    grid.dimensions = {5, 5, 3};  // Crosses a canonical atlas row; final row is partial.
    grid.map_resolution = 2;
    auto volume = ProbeVolume::allocate(grid);
    for (std::size_t p = 0; p < volume.probes.size(); ++p) {
        volume.probes[p] = {{1, -2, 0.5f}, p % 2 == 0};
        for (std::size_t i = 0; i < grid.texels_per_probe(); ++i) {
            volume.irradiance[p * grid.texels_per_probe() + i] = {32, 32, 32};
            volume.distance_moments[p * grid.texels_per_probe() + i] = {float(p), float(p * p)};
        }
    }
    auto bytes = encode_probes(volume);
    const auto* original = bytes.data();
    auto gpu = decode_gpu_probes(std::move(bytes));
    CHECK(gpu.gpu_bytes.data() == original);  // The runtime retains the file read buffer.
    CHECK(gpu.irradiance.empty() && gpu.distance_moments.empty());
    CHECK(gpu.atlas_rows() == 2 && gpu.atlas_capacity() == 128);
    const auto irradiance = gpu.gpu_plane(0), moments = gpu.gpu_plane(1),
               metadata = gpu.gpu_plane(2);
    CHECK(
        irradiance.size() == 128 * 16 * 4 && moments.size() == 128 * 16 * 8
        && metadata.size() == 128 * 8
    );
    for (std::size_t p = 0; p < volume.probes.size(); ++p) {
        const auto texel = (p / 64 * 4 + 1) * 64 * 4 + p % 64 * 4 + 2;
        CHECK(
            irradiance[texel * 4] == 0 && irradiance[texel * 4 + 1] == 4
            && irradiance[texel * 4 + 2] == 32 && irradiance[texel * 4 + 3] == 128
        );
        CHECK(metadata[p * 8] == 0 && metadata[p * 8 + 1] == 0x3c);
        CHECK(metadata[p * 8 + 2] == 0 && metadata[p * 8 + 3] == 0xc0);
        CHECK(metadata[p * 8 + 6] == 0 && metadata[p * 8 + 7] == (p % 2 == 0 ? 0x3c : 0));
    }
    CHECK(std::all_of(metadata.begin() + 75 * 8, metadata.end(), [](auto byte) {
        return byte == 0;
    }));
    CHECK(encode_probes(gpu) == gpu.gpu_bytes);
    const auto cpu = decode_probes(gpu.gpu_bytes);
    CHECK(cpu.probes[64].offset == volume.probes[64].offset);
    CHECK(cpu.distance_moments == volume.distance_moments);
    for (auto e : cpu.irradiance) {
        CHECK(std::abs(e[0] - 32) < .001f && std::abs(e[2] - 32) < .001f);
    }
    // Invalid packed metadata is rejected before any GPU upload.
    auto corrupt = gpu.gpu_bytes;
    corrupt[gpu.gpu_offset + irradiance.size() + moments.size() + 1] = 0x7c;  // half infinity
    bool rejected = false;
    try {
        decode_gpu_probes(std::move(corrupt));
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    CHECK(rejected);
    // Quantization preserves the relocation limit even when nearest-half rounds out.
    volume.probes[0].offset = {grid.spacing[0] * 0.45f, -grid.spacing[1] * 0.45f, 0};
    const auto edge = decode_probes(encode_probes(volume));
    CHECK(std::abs(edge.probes[0].offset[0]) <= grid.spacing[0] * 0.45f);
    CHECK(std::abs(edge.probes[0].offset[1]) <= grid.spacing[1] * 0.45f);
    return openrac::test::result();
}
