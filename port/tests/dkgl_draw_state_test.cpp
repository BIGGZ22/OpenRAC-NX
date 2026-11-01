// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/dkgl/draw_state_compat.h"
#include <cassert>
#include <cstdio>
#include <set>
using namespace openrac::dkgl::detail;
namespace {
struct Encoder { std::uint32_t* words=nullptr; unsigned count=0; bool fixed=false,bad=false,short_packet=false; };
Encoder& get(DkCmdBuf command) { return *reinterpret_cast<Encoder*>(command); }
}
extern "C" {
void dkCmdBufBeginCaptureCmds(DkCmdBuf command,std::uint32_t* words,std::uint32_t capacity) {
    assert(capacity>7); get(command).words=words;
}
void dkCmdBufBindBlendStates(DkCmdBuf command,std::uint32_t first,const DkBlendState* states,std::uint32_t count) {
    assert(first==0 && count==1);
    auto& e=get(command); const auto& s=*states;
    const std::uint32_t values[]{e.bad ? 0u : 0x20060781u,std::uint32_t(s.colorBlendOp),blend_register(s.srcColorBlendFactor),
        blend_register(s.dstColorBlendFactor),std::uint32_t(s.alphaBlendOp),blend_register(s.srcAlphaBlendFactor),
        blend_register(e.fixed ? s.dstAlphaBlendFactor : s.dstColorBlendFactor)};
    std::copy(std::begin(values),std::end(values),e.words); e.count=e.short_packet ? 6 : 7;
}
std::uint32_t dkCmdBufEndCaptureCmds(DkCmdBuf command) { return get(command).count; }
}
int main() {
    // Our extra vertex Y flip reverses a triangle's signed area. Preserve
    // front/back identity by changing the front-face convention, not cull mode.
    const float original_area=1, packed_area=-original_area;
    assert(packed_area<0 && packed_front_face(openrac::gl::GL_CCW)==DkFrontFace_CW);
    assert(packed_front_face(openrac::gl::GL_CW)==DkFrontFace_CCW);
    const DkBlendFactor factors[]{DkBlendFactor_Zero,DkBlendFactor_One,DkBlendFactor_SrcColor,DkBlendFactor_InvSrcColor,
        DkBlendFactor_SrcAlpha,DkBlendFactor_InvSrcAlpha,DkBlendFactor_DstAlpha,DkBlendFactor_InvDstAlpha,
        DkBlendFactor_DstColor,DkBlendFactor_InvDstColor,DkBlendFactor_SrcAlphaSaturate,DkBlendFactor_Src1Color,
        DkBlendFactor_InvSrc1Color,DkBlendFactor_Src1Alpha,DkBlendFactor_InvSrc1Alpha,DkBlendFactor_ConstColor,
        DkBlendFactor_InvConstColor,DkBlendFactor_ConstAlpha,DkBlendFactor_InvConstAlpha};
    std::set<std::uint32_t> keys;
    for(auto color:factors) for(auto alpha:factors) for(unsigned op=1;op<=5;++op) {
        DkBlendState s{}; dkBlendStateDefaults(&s);
        s.colorBlendOp=DkBlendOp(op); s.dstColorBlendFactor=color; s.dstAlphaBlendFactor=alpha;
        assert(keys.insert(blend_state_key(s)).second);
        for(bool already_fixed:{false,true}) {
            Encoder encoder; encoder.fixed=already_fixed;
            std::array<std::uint32_t,7> encoded{};
            assert(encode_blend_state(reinterpret_cast<DkCmdBuf>(&encoder),s,encoded));
            assert(encoded[3]==blend_register(color) && encoded[6]==blend_register(alpha));
            assert(encoded[1]==op && encoded[2]==blend_register(s.srcColorBlendFactor));
        }
    }
    DkBlendState s{}; dkBlendStateDefaults(&s);
    std::array<std::uint32_t,7> encoded{};
    Encoder bad; bad.bad=true;
    assert(!encode_blend_state(reinterpret_cast<DkCmdBuf>(&bad),s,encoded));
    Encoder short_packet; short_packet.short_packet=true;
    assert(!encode_blend_state(reinterpret_cast<DkCmdBuf>(&short_packet),s,encoded));
    std::puts("PASS: packed-Y winding, separate-alpha factors, cache keys and invalid SDK packets");
}
