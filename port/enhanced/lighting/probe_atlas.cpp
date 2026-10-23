// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/lighting/probe_atlas.h"

#include <bit>
#include <cmath>
#include <stdexcept>

#include "common/float_encoding.h"

namespace openrac::lighting {
namespace {
float unpack(unsigned bits, unsigned mantissa_bits, bool signed_value) {
    const auto mantissa = bits & ((1u << mantissa_bits) - 1);
    const auto exponent = (bits >> mantissa_bits) & 31;
    if (exponent == 31) {
        throw std::runtime_error("nonfinite packed probe texel");
    }
    const float value = std::ldexp(
        static_cast<float>(mantissa + (exponent ? 1u << mantissa_bits : 0u)),
        (exponent ? static_cast<int>(exponent) - 15 : -14) - static_cast<int>(mantissa_bits)
    );
    return signed_value && (bits & (1u << (mantissa_bits + 5))) ? -value : value;
}

void put(std::vector<std::uint8_t>& bytes, std::size_t at, unsigned value, unsigned size) {
    for (unsigned i = 0; i < size; ++i) {
        bytes[at + i] = static_cast<std::uint8_t>(value >> (8 * i));
    }
}

unsigned get(std::span<const std::uint8_t> bytes, std::size_t at, unsigned size) {
    unsigned value = 0;
    for (unsigned i = 0; i < size; ++i) {
        value |= unsigned(bytes[at + i]) << (8 * i);
    }
    return value;
}

// Map from a probe's local octahedral texel to the shared row-major atlas.
std::size_t atlas_texel(std::size_t probe, unsigned x, unsigned y, unsigned side) {
    return (probe / kProbeAtlasTilesX * side + y) * kProbeAtlasTilesX * side
           + probe % kProbeAtlasTilesX * side + x;
}
}  // namespace

std::size_t ProbeVolume::atlas_rows() const {
    return (grid.probe_count() + kProbeAtlasTilesX - 1) / kProbeAtlasTilesX;
}

std::size_t ProbeVolume::atlas_capacity() const {
    return atlas_rows() * kProbeAtlasTilesX;
}

std::span<const std::uint8_t> ProbeVolume::gpu_plane(unsigned plane) const {
    const auto texels = atlas_capacity() * grid.texels_per_probe();
    const std::size_t sizes[] = {texels * 4, texels * 8, atlas_capacity() * 8};
    if (plane > 2) {
        throw std::runtime_error("invalid probe texture plane");
    }
    const auto offset = gpu_offset + (plane > 0 ? sizes[0] : 0) + (plane > 1 ? sizes[1] : 0);
    if (offset > gpu_bytes.size() || sizes[plane] > gpu_bytes.size() - offset) {
        throw std::runtime_error("truncated GPU probe planes");
    }
    return std::span(gpu_bytes).subspan(offset, sizes[plane]);
}

std::vector<std::uint8_t> pack_probe_planes(const ProbeVolume& v) {
    v.validate();
    const auto side = v.grid.map_resolution + 2;
    const auto texels = v.atlas_capacity() * v.grid.texels_per_probe();
    std::vector<std::uint8_t> out(texels * 12 + v.atlas_capacity() * 8);
    for (std::size_t p = 0; p < v.probes.size(); ++p) {
        const auto& probe = v.probes[p];
        for (unsigned c = 0; c < 4; ++c) {
            auto value = float_encoding::pack_small_float(
                c < 3 ? probe.offset[c] : (probe.valid ? 1.0f : 0.0f), 10, true
            );
            // Half rounding must not move a relocated probe past its cell limit.
            if (c < 3 && std::abs(unpack(value, 10, true)) > v.grid.spacing[c] * 0.45f) {
                --value;
            }
            put(out, texels * 12 + p * 8 + c * 2, value, 2);
        }
        for (unsigned y = 0; y < side; ++y) {
            for (unsigned x = 0; x < side; ++x) {
                const auto source = p * side * side + y * side + x;
                const auto target = atlas_texel(p, x, y, side);
                const auto& e = v.irradiance[source];
                put(out,
                    target * 4,
                    float_encoding::pack_small_float(encode_probe_irradiance(e[0]), 6, false)
                        | (float_encoding::pack_small_float(encode_probe_irradiance(e[1]), 6, false)
                           << 11)
                        | (float_encoding::pack_small_float(encode_probe_irradiance(e[2]), 5, false)
                           << 22),
                    4);
                for (unsigned c = 0; c < 2; ++c) {
                    put(out,
                        texels * 4 + target * 8 + c * 4,
                        std::bit_cast<unsigned>(v.distance_moments[source][c]),
                        4);
                }
            }
        }
    }
    return out;
}

void read_probe_planes(ProbeVolume& v, bool expand) {
    const auto e = v.gpu_plane(0), m = v.gpu_plane(1), metadata = v.gpu_plane(2);
    v.probes.resize(v.grid.probe_count());
    if (expand) {
        v.irradiance.resize(v.grid.probe_count() * v.grid.texels_per_probe());
        v.distance_moments.resize(v.irradiance.size());
    }
    const auto side = v.grid.map_resolution + 2;
    for (std::size_t p = 0; p < v.probes.size(); ++p) {
        for (unsigned c = 0; c < 3; ++c) {
            v.probes[p].offset[c] = unpack(get(metadata, p * 8 + c * 2, 2), 10, true);
        }
        const float valid = unpack(get(metadata, p * 8 + 6, 2), 10, true);
        if (valid != 0 && valid != 1) {
            throw std::runtime_error("invalid packed probe validity");
        }
        v.probes[p].valid = valid == 1;
        for (unsigned y = 0; y < side; ++y) {
            for (unsigned x = 0; x < side; ++x) {
                const auto source = atlas_texel(p, x, y, side);
                const auto target = p * side * side + y * side + x;
                const auto packed = get(e, source * 4, 4);
                if (((packed >> 6) & 31) == 31 || ((packed >> 17) & 31) == 31
                    || ((packed >> 27) & 31) == 31) {
                    throw std::runtime_error("nonfinite packed probe irradiance");
                }
                const Float2 moments{
                    std::bit_cast<float>(get(m, source * 8, 4)),
                    std::bit_cast<float>(get(m, source * 8 + 4, 4))
                };
                if (!std::isfinite(moments[0]) || !std::isfinite(moments[1]) || moments[0] < 0
                    || moments[0] > v.grid.max_distance || moments[1] < 0
                    || moments[1] > v.grid.max_distance * v.grid.max_distance * 1.001f) {
                    throw std::runtime_error("invalid packed probe moments");
                }
                if (expand) {
                    for (unsigned c = 0; c < 3; ++c) {
                        const auto channel =
                            c == 0 ? packed : (c == 1 ? packed >> 11 : packed >> 22);
                        v.irradiance[target][c] =
                            std::pow(unpack(channel, c == 2 ? 5 : 6, false), kProbeIrradianceGamma);
                    }
                    v.distance_moments[target] = moments;
                }
            }
        }
    }
}
}  // namespace openrac::lighting
