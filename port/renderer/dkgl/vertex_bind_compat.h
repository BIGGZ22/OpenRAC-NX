// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <deko3d.h>

namespace openrac::dkgl::detail {
// Deko3D 0.5 dkCmdBufBindVtxBuffers reserves 5*n command words but emits 6*n.
// Capture with ample scratch space, then replay: ReplayCmds reserves the actual
// word count. Keep the SDK's own encoder; never patch installed SDK memory/layout.
inline bool bind_vertex_buffers_safe(DkCmdBuf destination, DkCmdBuf scratch,
                                     unsigned first, std::span<const DkBufExtents> buffers) {
    constexpr unsigned limit=16, words_per_buffer=6;
    if (first>limit || buffers.size()>limit-first) return false;
    if (buffers.empty()) return true;
    std::array<std::uint32_t,limit*words_per_buffer+1> words{};
    constexpr std::uint32_t canary=0xd3c05afe;
    words.back()=canary;
    dkCmdBufBeginCaptureCmds(scratch,words.data(),unsigned(words.size()));
    dkCmdBufBindVtxBuffers(scratch,first,buffers.data(),unsigned(buffers.size()));
    const auto count=dkCmdBufEndCaptureCmds(scratch);
    if (count!=buffers.size()*words_per_buffer || words.back()!=canary) return false;
    dkCmdBufReplayCmds(destination,words.data(),count);
    return true;
}
}
