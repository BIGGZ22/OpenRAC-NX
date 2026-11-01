// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <algorithm>
namespace openrac::dkgl::detail {
// Checked in wide arithmetic before narrowing values for the DMA engine.
constexpr bool texture_rectangle(int width,int height,unsigned levels,int level,int x,int y,int w,int h) {
    if(width<=0 || height<=0 || level<0 || level>=31 || unsigned(level)>=levels
        || x<0 || y<0 || w<=0 || h<=0) return false;
    const auto mw=std::max(1,width>>level), mh=std::max(1,height>>level);
    return x<=mw && y<=mh && w<=mw-x && h<=mh-y;
}
constexpr bool staging_layout(int width,int height,unsigned texel_bytes,std::uint32_t maximum,
                              std::uint32_t& pitch,std::uint32_t& size) {
    if(width<=0 || height<=0 || !texel_bytes || texel_bytes>16) return false;
    const std::uint64_t row=std::uint64_t(width)*texel_bytes;
    const std::uint64_t aligned=(row+31)/32*32;
    if(aligned>maximum || std::uint64_t(height)>maximum/aligned) return false;
    const std::uint64_t bytes=aligned*std::uint64_t(height);
    pitch=std::uint32_t(aligned); size=std::uint32_t(bytes);
    return true;
}
}
