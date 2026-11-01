// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
namespace openrac::dkgl::detail {
// Refresh once for the whole draw, then remember EVERY sampler that draw reads.
// Refreshing inside the uniform loop forgets earlier samplers in the same draw
// and can force multiple rebinds again on each following identical draw.
inline bool refresh_draw_samplers(std::vector<bool>& read,std::span<const std::uint32_t> handles) {
    bool refresh=false;
    for(const auto handle:handles) {
        const auto sampler=handle>>20;
        if(sampler<read.size() && !read[sampler]) refresh=true;
    }
    if(refresh) std::fill(read.begin(),read.end(),false);
    for(const auto handle:handles) {
        const auto sampler=handle>>20;
        if(sampler<read.size()) read[sampler]=true;
    }
    return refresh;
}

// The rules OPENRAC_DK_SAMPLER_REBIND selects. On (the default) is refresh_draw_samplers: the
// sms-switch rule, which also forgets every other sampler at each refresh, so draws that alternate
// between sampler sets refresh on every switch. First and New are narrower, opt-in experiments for
// a hardware A/B; neither is the default until a console run shows no stale-sampler surfaces.
enum class SamplerRebind { Off, On, First, New };

inline SamplerRebind parse_sampler_rebind(const char* v) {
    if(v==nullptr) return SamplerRebind::On;
    const std::string_view s(v);
    if(s=="0") return SamplerRebind::Off;
    if(s=="first") return SamplerRebind::First;
    if(s=="new") return SamplerRebind::New;
    return SamplerRebind::On;  // "1" and anything unrecognised keep the proven default
}

// First: refresh before a sampler's first read in the current command list (the symptom as the
// notes record it: a first read partway through a list), and keep what was read before.
inline bool refresh_first_samplers(std::vector<bool>& read,std::span<const std::uint32_t> handles) {
    bool refresh=false;
    for(const auto handle:handles) {
        const auto sampler=handle>>20;
        if(sampler<read.size() && !read[sampler]) {
            read[sampler]=true;
            refresh=true;
        }
    }
    return refresh;
}

// New: refresh only before a sampler whose descriptor was written after the set was last bound
// (`bound` descriptors existed then; sampler descriptors are append-only and never rewritten).
inline bool refresh_new_samplers(std::uint32_t bound,std::span<const std::uint32_t> handles) {
    return std::any_of(handles.begin(),handles.end(),[bound](std::uint32_t handle) {
        return (handle>>20)>=bound;
    });
}
}
