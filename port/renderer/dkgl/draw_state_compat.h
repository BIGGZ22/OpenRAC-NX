// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <cstdint>
#include <deko3d.h>
#include "renderer/gl.h"

namespace openrac::dkgl::detail {
// dk_pack.py flips clip-space Y. Deko3D 0.5 already compensates its viewport
// origin in face classification; compensate the additional shader flip here.
constexpr DkFrontFace packed_front_face(gl::GLenum face) {
    return face==gl::GL_CW ? DkFrontFace_CCW : DkFrontFace_CW;
}
constexpr std::uint32_t blend_register(DkBlendFactor factor) {
    const auto value=std::uint32_t(factor);
    return (value&31u)|((value&32u) ? 0xc000u : 0u);
}
constexpr std::uint32_t blend_state_key(const DkBlendState& state) {
    return std::uint32_t(state.colorBlendOp)|(std::uint32_t(state.srcColorBlendFactor)<<3)
        |(std::uint32_t(state.dstColorBlendFactor)<<9)|(std::uint32_t(state.alphaBlendOp)<<15)
        |(std::uint32_t(state.srcAlphaBlendFactor)<<18)|(std::uint32_t(state.dstAlphaBlendFactor)<<24);
}
// Deko3D 0.5 encodes dstColorBlendFactor into FuncAlphaDst. Like night100's
// public capture/replay workaround, validate the packet before fixing that field.
// Reference: devkitPro/deko3d v0.5.0 source/maxwell/gpu_3d_state.cpp.
inline bool encode_blend_state(DkCmdBuf scratch,const DkBlendState& state,
                               std::array<std::uint32_t,7>& result) {
    std::array<std::uint32_t,16> captured{};
    constexpr std::uint32_t canary=0xb1e05afe;
    captured.back()=canary;
    dkCmdBufBeginCaptureCmds(scratch,captured.data(),unsigned(captured.size()));
    dkCmdBufBindBlendStates(scratch,0,&state,1);
    const auto count=dkCmdBufEndCaptureCmds(scratch);
    if(count!=7 || captured.back()!=canary || captured[0]!=0x20060781u
        || captured[1]!=std::uint32_t(state.colorBlendOp)
        || captured[2]!=blend_register(state.srcColorBlendFactor)
        || captured[3]!=blend_register(state.dstColorBlendFactor)
        || captured[4]!=std::uint32_t(state.alphaBlendOp)
        || captured[5]!=blend_register(state.srcAlphaBlendFactor)
        || (captured[6]!=blend_register(state.dstColorBlendFactor)
            && captured[6]!=blend_register(state.dstAlphaBlendFactor))) return false;
    std::copy_n(captured.begin(),result.size(),result.begin());
    result[6]=blend_register(state.dstAlphaBlendFactor);
    return true;
}
}
